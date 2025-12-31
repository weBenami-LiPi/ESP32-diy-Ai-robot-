#ifndef WEB_SERVER_MANAGER_H
#define WEB_SERVER_MANAGER_H

#include "bitmaps.h"
#include "hardware_ctrl.h"
#include "logger.h"
#include "openai_manager.h"
#include "persistent_config.h"
#include "tts_manager.h"
#include "web_content.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>
#include <WebServer.h>

WebServer server(80);
extern unsigned long lastInteractionTime;

void handleRoot() { server.send(200, "text/html", INDEX_HTML); }

Emotion getEmotionFromTag(String tag) {
  if (tag == "[LOVE]")
    return LOVE;
  if (tag == "[LAUGH]")
    return LAUGH;
  if (tag == "[SLEEP]")
    return SLEEP;
  if (tag == "[WINK]")
    return WINK;
  if (tag == "[EVIL]")
    return MISCHIEVOUS;
  if (tag == "[FRUSTRATED]")
    return FRUSTRATED;
  if (tag == "[RAGE]")
    return ANGRY_RAGE;
  if (tag == "[SHOCKED]")
    return SHOCKED;
  if (tag == "[SAD]")
    return SAD;
  if (tag == "[CRYING]")
    return CRYING;
  if (tag == "[ANGEL]")
    return ANGEL;
  if (tag == "[DEAD]")
    return DEAD;
  if (tag == "[DIZZY]")
    return DIZZY;
  if (tag == "[PARTY]")
    return PARTY;
  if (tag == "[SKEPTICAL]")
    return SKEPTICAL;
  return NEUTRAL;
}

String cleanMessage(String text) {
  String out = "";
  bool inside = false;
  for (int i = 0; i < text.length(); i++) {
    if (text[i] == '[')
      inside = true;
    else if (text[i] == ']')
      inside = false;
    else if (!inside)
      out += text[i];
  }
  out.trim();
  return out;
}

void processAndSpeak(String text) {
  int startIdx = 0;
  while (startIdx < text.length()) {
    int tagStart = text.indexOf('[', startIdx);
    if (tagStart == -1) {
      String segment = text.substring(startIdx);
      segment.trim();
      if (segment.length() > 0 && TTS.enabled && TTS.downloadTTS(segment))
        TTS.playLastTTS();
      break;
    }
    if (tagStart > startIdx) {
      String segment = text.substring(startIdx, tagStart);
      segment.trim();
      if (segment.length() > 0 && TTS.enabled && TTS.downloadTTS(segment))
        TTS.playLastTTS();
    }
    int tagEnd = text.indexOf(']', tagStart);
    if (tagEnd == -1)
      break;
    String tag = text.substring(tagStart, tagEnd + 1);
    setEmotion(getEmotionFromTag(tag));

    // After setting emotion, we still need to handle the remaining text
    // But we should NOT speak the tag name.
    // The current loop structure already skips the tag's range via startIdx.
    startIdx = tagEnd + 1;
  }
}

// Updated speaking logic to return success status for WebUI fallback
bool processAndSpeakOptimized(String text) {
  bool overallSuccess = true;
  int startIdx = 0;
  while (startIdx < text.length()) {
    int tagStart = text.indexOf('[', startIdx);
    if (tagStart == -1) {
      String segment = text.substring(startIdx);
      segment.trim();
      if (segment.length() > 0 && TTS.enabled) {
        String cleanSeg = cleanMessage(segment);
        if (cleanSeg.length() > 0) {
          if (!TTS.downloadTTS(cleanSeg))
            overallSuccess = false;
          else
            TTS.playLastTTS();
        }
      }
      break;
    }
    if (tagStart > startIdx) {
      String segment = text.substring(startIdx, tagStart);
      segment.trim();
      if (segment.length() > 0 && TTS.enabled) {
        String cleanSeg = cleanMessage(segment);
        if (cleanSeg.length() > 0) {
          if (!TTS.downloadTTS(cleanSeg))
            overallSuccess = false;
          else
            TTS.playLastTTS();
        }
      }
    }
    int tagEnd = text.indexOf(']', tagStart);
    if (tagEnd == -1)
      break;
    String tag = text.substring(tagStart, tagEnd + 1);
    setEmotion(getEmotionFromTag(tag));
    startIdx = tagEnd + 1;
  }
  return overallSuccess;
}

void handleChat() {
  if (!server.hasArg("msg")) {
    server.send(400, "text/plain", "Missing message");
    return;
  }
  String msg = server.arg("msg");
  logChat("User: " + msg);
  lastInteractionTime = millis();

  String reply = openai.sendText(msg, globalConfig.config.system_prompt);
  String robotLog = "Vextor: " + reply;
  logChat(robotLog);

  bool hwTtsSuccess = processAndSpeakOptimized(reply);

  StaticJsonDocument<1024> doc;
  doc["reply"] = cleanMessage(reply);
  doc["tts_failed"] = !hwTtsSuccess; // Signal WebUI if hardware TTS failed
  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}

void handleControl() {
  String dir = server.arg("dir");
  lastInteractionTime = millis();
  logLoc("Control: " + dir);
  moveRobot(dir);
  if (dir == "AUTO") {
    server.send(200, "text/plain", autoPilotActive ? "ON" : "OFF");
  } else {
    server.send(200, "text/plain", "OFF");
  }
}

void handleSpeed() {
  if (server.hasArg("val")) {
    lastInteractionTime = millis();
    robotSpeed = server.arg("val").toInt();
    logLoc("Speed set to: " + String(robotSpeed));
    server.send(200, "text/plain", "OK");
  } else {
    server.send(400, "text/plain", "Missing val");
  }
}

void handleEmotion() {
  if (server.hasArg("id")) {
    lastInteractionTime = millis();
    int id = server.arg("id").toInt();
    Emotion target = NEUTRAL;

    // Map 16 emojis to emotions
    // 😐, 😍, 😂, 😴, 😉, 😈, 🤬, 😮, 😢, 💀, 😵, 🥳, 🤨, 😤, 😇, 😭
    if (id == 0)
      target = NEUTRAL;
    else if (id == 1)
      target = LOVE;
    else if (id == 2)
      target = LAUGH;
    else if (id == 3)
      target = SLEEP;
    else if (id == 4)
      target = WINK;
    else if (id == 5)
      target = MISCHIEVOUS;
    else if (id == 6)
      target = ANGRY_RAGE;
    else if (id == 7)
      target = SHOCKED;
    else if (id == 8)
      target = SAD;
    else if (id == 9)
      target = DEAD;
    else if (id == 10)
      target = DIZZY;
    else if (id == 11)
      target = PARTY;
    else if (id == 12)
      target = SKEPTICAL;
    else if (id == 13)
      target = FRUSTRATED;
    else if (id == 14)
      target = ANGEL;
    else if (id == 15)
      target = CRYING;

    String emoLog =
        "Emotion set to ID: " + String(id) + " -> " + String(target);
    logLoc(emoLog);
    setEmotion(target);
    server.send(200, "text/plain", "OK");
  } else {
    server.send(400, "text/plain", "Missing id");
  }
}

void handleChatLogs() {
  server.send(200, "text/plain", readLogFile("/chat.txt"));
}
void handleLocLogs() {
  server.send(200, "text/plain", readLogFile("/loc.txt"));
}

void handleClearChat() {
  clearLogFile("/chat.txt");
  server.send(200, "text/plain", "Chat Logs Cleared");
}

void handleClearLoc() {
  clearLogFile("/loc.txt");
  server.send(200, "text/plain", "Location Logs Cleared");
}

void handleGetConfig() {
  StaticJsonDocument<512> doc;
  doc["openai_key"] = globalConfig.config.openai_key;
  doc["elevenlabs_key"] = globalConfig.config.elevenlabs_key;
  doc["system_prompt"] = globalConfig.config.system_prompt;
  doc["webui_openai_key"] = globalConfig.config.webui_openai_key;
  doc["webui_elevenlabs_key"] = globalConfig.config.webui_elevenlabs_key;
  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}

void handleSaveConfig() {
  if (server.hasArg("plain")) {
    StaticJsonDocument<4096> doc;
    deserializeJson(doc, server.arg("plain"));
    globalConfig.config.openai_key =
        doc["openai_key"] | globalConfig.config.openai_key;
    globalConfig.config.elevenlabs_key =
        doc["elevenlabs_key"] | globalConfig.config.elevenlabs_key;
    globalConfig.config.openai_model =
        doc["openai_model"] | globalConfig.config.openai_model;
    globalConfig.config.voice_id =
        doc["voice_id"] | globalConfig.config.voice_id;
    globalConfig.config.system_prompt =
        doc["system_prompt"] | globalConfig.config.system_prompt;
    globalConfig.config.webui_openai_key =
        doc["webui_openai_key"] | globalConfig.config.webui_openai_key;
    globalConfig.config.webui_elevenlabs_key =
        doc["webui_elevenlabs_key"] | globalConfig.config.webui_elevenlabs_key;
    if (globalConfig.save()) {
      server.send(200, "text/plain", "Config Saved");
    } else {
      server.send(500, "text/plain", "Save Failed");
    }
  } else {
    server.send(400, "text/plain", "Missing data");
  }
}

void setupWebServer() {
  server.on("/", handleRoot);
  server.on("/chat", handleChat);
  server.on("/control", handleControl);
  server.on("/speed", handleSpeed);
  server.on("/emotion", handleEmotion);
  server.on("/chat_logs", handleChatLogs);
  server.on("/loc_logs", handleLocLogs);
  server.on("/clear_chat", handleClearChat);
  server.on("/clear_loc", handleClearLoc);
  server.on("/get_config", handleGetConfig);
  server.on("/save_config", HTTP_POST, handleSaveConfig);

  server.begin();
  logChat("Web Server started");
}

void handleServer() { server.handleClient(); }

#endif
