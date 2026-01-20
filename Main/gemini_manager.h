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

    // Increased buffer for Free Tier stability (16KB)
    DynamicJsonDocument reqDoc(16384);

    // Safety settings to prevent over-sensitive filtering on free tier
    JsonArray safetySettings = reqDoc.createNestedArray("safetySettings");
    const char *categories[] = {
        "HARM_CATEGORY_HARASSMENT", "HARM_CATEGORY_HATE_SPEECH",
        "HARM_CATEGORY_SEXUALLY_EXPLICIT", "HARM_CATEGORY_DANGEROUS_CONTENT"};
    for (const char *category : categories) {
      JsonObject setting = safetySettings.createNestedObject();
      setting["category"] = category;
      setting["threshold"] = "BLOCK_NONE"; // Allow robot personality
    }

    JsonArray contents = reqDoc.createNestedArray("contents");

    // Add System Prompt as context
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
    ackText["text"] = "Understood. I am Vextor, the AI cat robot.";

    // Load History from SPIFFS
    if (SPIFFS.exists("/history_gemini.json")) {
      File f = SPIFFS.open("/history_gemini.json", "r");
      DynamicJsonDocument histDoc(8192); // Increased history buffer
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

    // DEBUG: Show request details
    Serial.println("=== Gemini API Request Debug ===");
    Serial.println("Request Size: " + String(body.length()) + " bytes");
    Serial.println("URL: " + url);
    if (body.length() > 200) {
      Serial.println(
          "Body Preview (first 200 chars): " + body.substring(0, 200) + "...");
      Serial.println("Body Preview (last 200 chars): ..." +
                     body.substring(body.length() - 200));
    } else {
      Serial.println("Full Body: " + body);
    }

    int httpCode = http.POST(body);
    String response = "";

    Serial.println("HTTP Response Code: " + String(httpCode));

    if (httpCode == 200) {
      response = http.getString();
      Serial.println("Success! Response received.");
    } else {
      String err = http.getString();
      Serial.println("=== Gemini API Error ===");
      Serial.println("Error Code: " + String(httpCode));
      Serial.println("Error Response: " + err);
      http.end();
      return "Gemini Error (" + String(httpCode) + "): Check Serial Monitor";
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
