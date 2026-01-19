#ifndef GEMINI_MANAGER_H
#define GEMINI_MANAGER_H

#include "persistent_config.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <SPIFFS.h>
#include <WiFiClientSecure.h>

class GeminiManager {
public:
  GeminiManager() {}

  String askGemini(String text, String systemPrompt) {
    WiFiClientSecure client;
    client.setInsecure();
    HTTPClient http;

    String url = "https://generativelanguage.googleapis.com/v1/models/"
                 "gemini-1.5-flash:generateContent?key=" +
                 globalConfig.config.gemini_key;

    if (!http.begin(client, url))
      return "Connection Failed";

    http.addHeader("Content-Type", "application/json");

    DynamicJsonDocument reqDoc(8192);
    JsonArray contents = reqDoc.createNestedArray("contents");

    // Add System Prompt as context in the first user message for Gemini (or use
    // system_instruction if supported, but simple role-play works well)
    JsonObject systemMsg = contents.createNestedObject();
    systemMsg["role"] = "user";
    JsonArray systemParts = systemMsg.createNestedArray("parts");
    JsonObject systemText = systemParts.createNestedObject();
    systemText["text"] = BASE_IDENTITY + "\n\nInstructions:\n" + systemPrompt;

    // Acknowledge the role
    JsonObject ackMsg = contents.createNestedObject();
    ackMsg["role"] = "model";
    JsonArray ackParts = ackMsg.createNestedArray("parts");
    JsonObject ackText = ackParts.createNestedObject();
    ackText["text"] = "Understood. I am Vextor, the Cute Cat robot. I will "
                      "respond only with emojis and use [YES]/[NO] tags.";

    // Load History from SPIFFS
    if (SPIFFS.exists("/history_gemini.json")) {
      File f = SPIFFS.open("/history_gemini.json", "r");
      DynamicJsonDocument histDoc(4096);
      deserializeJson(histDoc, f);
      f.close();
      JsonArray hArr = histDoc.as<JsonArray>();
      for (JsonObject h : hArr) {
        JsonObject m = contents.createNestedObject();
        m["role"] = h["r"]; // "user" or "model"
        JsonArray p = m.createNestedArray("parts");
        JsonObject t = p.createNestedObject();
        t["text"] = h["c"];
      }
    }

    // Current User Input
    JsonObject userMsg = contents.createNestedObject();
    userMsg["role"] = "user";
    JsonArray userParts = userMsg.createNestedArray("parts");
    JsonObject userText = userParts.createNestedObject();
    userText["text"] = text;

    String body;
    serializeJson(reqDoc, body);

    int httpCode = http.POST(body);
    String response = "";

    if (httpCode == 200) {
      response = http.getString();
    } else {
      String err = http.getString();
      http.end();
      return "Gemini Error (" + String(httpCode) + ")";
    }
    http.end();

    String aiReply = parseResponse(response);
    if (aiReply != "No AI Response" && !aiReply.startsWith("Error")) {
      updateHistory(text, aiReply);
    }
    return aiReply;
  }

  void updateHistory(String u, String m) {
    DynamicJsonDocument histDoc(4096);
    if (SPIFFS.exists("/history_gemini.json")) {
      File f = SPIFFS.open("/history_gemini.json", "r");
      deserializeJson(histDoc, f);
      f.close();
    }
    JsonArray hArr = histDoc.as<JsonArray>();

    if (hArr.size() >= 10) { // Keep last 5 exchanges
      hArr.remove(0);
      hArr.remove(0);
    }

    JsonObject uMsg = hArr.createNestedObject();
    uMsg["r"] = "user";
    uMsg["c"] = u;

    JsonObject mMsg = hArr.createNestedObject();
    mMsg["r"] = "model";
    mMsg["c"] = m;

    File f = SPIFFS.open("/history_gemini.json", "w");
    serializeJson(histDoc, f);
    f.close();
  }

private:
  String parseResponse(String &response) {
    DynamicJsonDocument resDoc(8192);
    DeserializationError error = deserializeJson(resDoc, response);
    if (error)
      return "JSON Error";

    if (resDoc.containsKey("candidates") && resDoc["candidates"].size() > 0) {
      return resDoc["candidates"][0]["content"]["parts"][0]["text"]
          .as<String>();
    }
    return "No AI Response";
  }
};

extern GeminiManager gemini;

#endif
