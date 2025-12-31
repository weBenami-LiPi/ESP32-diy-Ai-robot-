#ifndef OPENAI_MANAGER_H
#define OPENAI_MANAGER_H

#include "../config/api_config.h"
#include "../config/persistent_config.h"
#include "../vision/bitmaps.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <SPIFFS.h>
#include <WiFiClientSecure.h>


class OpenAIManager {
public:
  OpenAIManager() {}

  String sendText(String text, String systemPrompt) {
    WiFiClientSecure client;
    client.setInsecure();
    HTTPClient http;

    if (!http.begin(client, openai_endpoint))
      return "Connection Failed";

    http.addHeader("Content-Type", "application/json");
    http.addHeader("Authorization", "Bearer " + globalConfig.config.openai_key);

    DynamicJsonDocument reqDoc(4096);
    reqDoc["model"] = globalConfig.config.openai_model;
    JsonArray messages = reqDoc.createNestedArray("messages");

    JsonObject systemMsg = messages.createNestedObject();
    systemMsg["role"] = "system";
    systemMsg["content"] =
        BASE_IDENTITY + "\n\nAdditional Instructions:\n" + systemPrompt;

    // Load History from SPIFFS
    if (SPIFFS.exists("/history.json")) {
      File f = SPIFFS.open("/history.json", "r");
      DynamicJsonDocument histDoc(3072);
      deserializeJson(histDoc, f);
      f.close();
      JsonArray hArr = histDoc.as<JsonArray>();
      for (JsonObject h : hArr) {
        JsonObject m = messages.createNestedObject();
        m["role"] = h["r"];
        m["content"] = h["c"];
      }
    }

    JsonObject userMsg = messages.createNestedObject();
    userMsg["role"] = "user";
    userMsg["content"] = text;

    String body;
    serializeJson(reqDoc, body);

    int httpCode = http.POST(body);
    String response = "";

    if (httpCode == 200) {
      response = http.getString();
    } else {
      http.end();
      // If limit reached, switch to fallback
      globalConfig.useFallback(true);
      return "Request Failed (" + String(httpCode) + ")";
    }
    http.end();
    String aiReply = parseResponse(response);

    if (aiReply != "No AI Response" && aiReply != "JSON Error" &&
        aiReply != "Request Failed") {
      updateHistory(text, aiReply);
    }
    return aiReply;
  }

  void updateHistory(String u, String b) {
    DynamicJsonDocument histDoc(3072);
    if (SPIFFS.exists("/history.json")) {
      File f = SPIFFS.open("/history.json", "r");
      deserializeJson(histDoc, f);
      f.close();
    }
    JsonArray hArr = histDoc.as<JsonArray>();
    if (hArr.size() >= 20) // Increased to 20 messages (10 cycles)
      hArr.remove(0);

    JsonObject uMsg = hArr.createNestedObject();
    uMsg["r"] = "user";
    uMsg["c"] = u;
    JsonObject bMsg = hArr.createNestedObject();
    bMsg["r"] = "assistant";
    bMsg["c"] = b;

    File f = SPIFFS.open("/history.json", "w");
    serializeJson(histDoc, f);
    f.close();
  }

  String sendAudio(String base64Audio, String systemPrompt) {
    WiFiClientSecure client;
    client.setInsecure();
    HTTPClient http;

    // Whisper ASR: Transcribe audio
    if (!http.begin(client, "https://api.openai.com/v1/audio/transcriptions"))
      return "Whisper Connection Failed";

    http.addHeader("Authorization", "Bearer " + openai_API_Key);
    // Note: Whisper requires multipart form data for file uploads
    // For simplicity, we assume the backend handles the Base64 to WAV
    // conversion or the user provides a pre-formatted buffer.

    // Actually, let's assume raw record and send.
    // For ESP32, sending large multipart can be tricky.
    // We will stick to the plan: implement voice system.

    return "[VOICE] Transcription logic pending complex multipart "
           "implementation. Use Text for now.";
  }

private:
  String parseResponse(String &response) {
    if (response.length() == 0)
      return "No Data";
    DynamicJsonDocument resDoc(response.length() + 1024);
    DeserializationError error = deserializeJson(resDoc, response);
    if (error)
      return "JSON Error";
    if (resDoc.containsKey("choices") && resDoc["choices"].size() > 0) {
      return resDoc["choices"][0]["message"]["content"].as<String>();
    }
    if (resDoc.containsKey("error")) {
      return "OpenAI Error: " +
             (resDoc["error"]["message"] | String("Unknown"));
    }
    return "No AI Response";
  }
};

extern OpenAIManager openai;

#endif
