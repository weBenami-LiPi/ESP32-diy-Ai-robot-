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

#include "action_queue.h"

void handleStopTalk() {
  TTS.stop();
  server.send(200, "text/plain", "Stopped");
}

// ... (other includes are fine, this is just to ensure it's there or I assume
// it's added at top)

bool processAndSpeakOptimized(String text) {
  // Wake up immediately if receiving a message
  if (currentEmotion == SLEEP) {
    setEmotion(NEUTRAL);
  }

  std::vector<RobotAction> tempQueue;
  bool overallSuccess = true;
  int startIdx = 0;
  static int reqCounter = 0;
  reqCounter++;

  while (startIdx < text.length()) {
    int tagStart = text.indexOf('[', startIdx);

    // Handle Text Segment before Tag (or to end)
    String segment = "";
    int endOfSegment = (tagStart == -1) ? text.length() : tagStart;

    if (endOfSegment > startIdx) {
      segment = text.substring(startIdx, endOfSegment);
      segment.trim();

      if (segment.length() > 0) {
        String cleanSeg = cleanMessage(segment);
        if (cleanSeg.length() > 0) {
          bool downloadSuccess = false;
          String fname =
              "/tts_" + String(reqCounter) + "_" + String(millis()) + ".mp3";

          if (TTS.enabled && TTS.downloadTTS(cleanSeg, fname)) {
            downloadSuccess = true;
          }

          if (downloadSuccess) {
            RobotAction act;
            act.type = ACTION_PLAY_AUDIO;
            act.payload = fname;
            tempQueue.push_back(act);
          } else {
            overallSuccess = false;
            // Fallback: Simulate lip sync for browser audio
            RobotAction act;
            act.type = ACTION_SIMULATE_TALK;
            // Estimate 85ms per character for better sync (revised from 150ms)
            act.durationMs = cleanSeg.length() * 85;
            tempQueue.push_back(act);
          }
        }
      }
    }

    if (tagStart == -1)
      break;

    int tagEnd = text.indexOf(']', tagStart);
    if (tagEnd == -1)
      break; // Malformed tag

    String tag = text.substring(tagStart, tagEnd + 1);
    if (tag.startsWith("[CMD:")) {
      String cmd = tag.substring(5, tag.length() - 1);
      RobotAction cmdAct;
      cmdAct.type = ACTION_ROBOT_CMD;
      cmdAct.payload = cmd;
      tempQueue.push_back(cmdAct);
    } else {
      RobotAction emoAct;
      emoAct.type = ACTION_SET_EMOTION;
      emoAct.emotionVal = getEmotionFromTag(tag);
      tempQueue.push_back(emoAct);
    }

    startIdx = tagEnd + 1;
  }

  if (!tempQueue.empty()) {
    for (const auto &act : tempQueue) {
      globalQueue.push_back(act);
    }
  }
  return overallSuccess;
}

void processAndSpeak(String text) {
  // Use the optimized version logic but ignore return value
  processAndSpeakOptimized(text);
}

void handleChat() {
  if (!server.hasArg("msg")) {
    server.send(400, "text/plain", "Missing message");
    return;
  }
  String msg = server.arg("msg");
  logChat("User: " + msg);
  lastInteractionTime = millis();

  // Wake the robot if it was sleeping
  if (currentEmotion == SLEEP) {
    setEmotion(WAKE_UP);
  }

  String reply = openai.sendText(msg, globalConfig.config.system_prompt);
  String robotLog = "Vextor: " + reply;
  logChat(robotLog);

  // CRITICAL: pass the ORIGINAL reply with tags so the robot can set
  // emotions/lip sync
  bool hwTtsSuccess = processAndSpeakOptimized(reply);

  StaticJsonDocument<1024> doc;
  doc["reply"] =
      reply; // Include tags for browser to handle if needed, or robot logic
  doc["clean_reply"] = cleanMessage(reply); // For display in Chat UI
  doc["tts_failed"] = !hwTtsSuccess;
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
  server.on("/save_config", handleSaveConfig);
  server.on("/stop_talk", handleStopTalk);

  server.begin();
  logChat("Web Server started");
}

void handleServer() { server.handleClient(); }

#endif
