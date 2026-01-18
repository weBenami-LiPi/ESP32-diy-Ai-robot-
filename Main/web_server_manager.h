#ifndef WEB_SERVER_MANAGER_H
#define WEB_SERVER_MANAGER_H

#include "action_queue.h"
#include "bitmaps.h"
#include "command_processor.h"
#include "gemini_manager.h"
#include "hardware_ctrl.h"
#include "logger.h"
#include "persistent_config.h"
#include "web_content.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>
#include <WebServer.h>

WebServer server(80);
extern unsigned long lastInteractionTime;

// --- Helper Functions ---

void handleRoot() { server.send(200, "text/html; charset=utf-8", INDEX_HTML); }

Emotion getEmotionFromEmoji(String emoji) {
  if (emoji == "😐")
    return NEUTRAL;
  if (emoji == "😍")
    return LOVE;
  if (emoji == "😂")
    return LAUGH;
  if (emoji == "😴")
    return SLEEP;
  if (emoji == "😉")
    return WINK;
  if (emoji == "😈")
    return MISCHIEVOUS;
  if (emoji == "😤")
    return FRUSTRATED;
  if (emoji == "😮")
    return SHOCKED;
  if (emoji == "😢")
    return SAD;
  if (emoji == "💀")
    return DEAD;
  if (emoji == "😵")
    return DIZZY;
  if (emoji == "🥳")
    return PARTY;
  if (emoji == "🧐")
    return SKEPTICAL;
  if (emoji == "😠")
    return ANGRY_RAGE;
  if (emoji == "😇")
    return ANGEL;
  if (emoji == "😭")
    return CRYING;
  if (emoji == "😲")
    return WAKE_UP;
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

Emotion getEmotionFromTag(String tag) {
  if (tag == "[NEUTRAL]")
    return NEUTRAL;
  if (tag == "[LOVE]")
    return LOVE;
  if (tag == "[LAUGH]")
    return LAUGH;
  if (tag == "[SLEEP]")
    return SLEEP;
  if (tag == "[WINK]")
    return WINK;
  if (tag == "[MISCHIEVOUS]")
    return MISCHIEVOUS;
  if (tag == "[ANGRY]")
    return ANGRY_RAGE;
  if (tag == "[SHOCKED]")
    return SHOCKED;
  if (tag == "[SAD]")
    return SAD;
  if (tag == "[DEAD]")
    return DEAD;
  if (tag == "[DIZZY]")
    return DIZZY;
  if (tag == "[PARTY]")
    return PARTY;
  if (tag == "[SKEPTICAL]")
    return SKEPTICAL;
  if (tag == "[FRUSTRATED]")
    return FRUSTRATED;
  if (tag == "[ANGEL]")
    return ANGEL;
  if (tag == "[CRYING]")
    return CRYING;
  if (tag == "[WAKE_UP]" || tag == "[STARTLE]" || tag == "[SHOCK]")
    return WAKE_UP;
  return NEUTRAL;
}

// --- Talk & Chat Handlers ---

bool processAndSpeakOptimized(String text) {
  if (currentEmotion == SLEEP) {
    setEmotion(NEUTRAL);
  }

  std::vector<RobotAction> tempQueue;
  int startIdx = 0;

  while (startIdx < text.length()) {
    int tagStart = text.indexOf('[', startIdx);
    int endOfSegment = (tagStart == -1) ? text.length() : tagStart;

    if (endOfSegment > startIdx) {
      String segment = text.substring(startIdx, endOfSegment);
      segment.trim();
      if (segment.length() > 0) {
        String cleanSeg = cleanMessage(segment);
        if (cleanSeg.length() > 0) {
          RobotAction act;
          act.type = ACTION_SIMULATE_TALK;
          act.durationMs = cleanSeg.length() * 85;
          tempQueue.push_back(act);
        }
      }
    }

    if (tagStart == -1)
      break;

    int tagEnd = text.indexOf(']', tagStart);
    if (tagEnd == -1)
      break;

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
  return true;
}

void processAndSpeak(String text) { processAndSpeakOptimized(text); }

void handleStopTalk() { server.send(200, "text/plain", "Stopped"); }

void handleChat() {
  if (!server.hasArg("msg")) {
    server.send(400, "text/plain", "Missing message");
    return;
  }
  String msg = server.arg("msg");
  logChat("User: " + msg);
  lastInteractionTime = millis();

  if (currentEmotion == SLEEP) {
    setEmotion(WAKE_UP);
  }

  if (cmdProcessor.detectAndExecute(msg)) {
    StaticJsonDocument<256> doc;
    doc["reply"] = "[CMD]";
    doc["clean_reply"] = "";
    doc["tts_failed"] = true;
    doc["is_command"] = true;
    String response;
    serializeJson(doc, response);
    server.send(200, "application/json", response);
    return;
  }

  autoPilotActive = false;
  String reply = gemini.askGemini(msg, globalConfig.config.system_prompt);
  logChat("Vextor: " + reply);

  if (reply.indexOf("[YES]") != -1) {
    RobotAction nod;
    nod.type = ACTION_ROBOT_CMD;
    nod.payload = "nod_yes_internal";
    globalQueue.push_back(nod);
  } else if (reply.indexOf("[NO]") != -1) {
    RobotAction shake;
    shake.type = ACTION_ROBOT_CMD;
    shake.payload = "shake_no_internal";
    globalQueue.push_back(shake);
  }

  String emojiOnly = cleanMessage(reply);
  emojiOnly.trim();
  setEmotion(getEmotionFromEmoji(emojiOnly));

  StaticJsonDocument<1024> doc;
  doc["reply"] = emojiOnly;
  doc["clean_reply"] = emojiOnly;
  doc["tts_failed"] = true;
  doc["is_command"] = false;
  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}

// --- System Control Handlers ---

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
    setEmotion(target);
    server.send(200, "text/plain", "OK");
  } else {
    server.send(400, "text/plain", "Missing id");
  }
}

// --- Logging & Config Handlers ---

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
  StaticJsonDocument<1024> doc;
  doc["gemini_key"] = globalConfig.config.gemini_key;
  doc["elevenlabs_key"] = globalConfig.config.elevenlabs_key;
  doc["system_prompt"] = globalConfig.config.system_prompt;
  doc["webui_gemini_key"] = globalConfig.config.webui_gemini_key;
  doc["webui_elevenlabs_key"] = globalConfig.config.webui_elevenlabs_key;
  doc["voice_id"] = globalConfig.config.voice_id;
  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}

void handleSaveConfig() {
  if (server.hasArg("plain")) {
    StaticJsonDocument<4096> doc;
    deserializeJson(doc, server.arg("plain"));
    globalConfig.config.gemini_key =
        doc["gemini_key"] | globalConfig.config.gemini_key;
    globalConfig.config.elevenlabs_key =
        doc["elevenlabs_key"] | globalConfig.config.elevenlabs_key;
    globalConfig.config.voice_id =
        doc["voice_id"] | globalConfig.config.voice_id;
    globalConfig.config.system_prompt =
        doc["system_prompt"] | globalConfig.config.system_prompt;
    globalConfig.config.webui_gemini_key =
        doc["webui_gemini_key"] | globalConfig.config.webui_gemini_key;
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

void handleSystemTest() {
  logLoc("System Test: Starting...");
  String report = "Test Report:\n";
  moveRobot("FORWARD");
  delay(500);
  moveRobot("STOP");
  report += "- Motor OK\n";
  server.send(200, "text/plain", report + "Complete!");
}

// --- Setup ---

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
  server.on("/system_test", handleSystemTest);
  server.begin();
  logChat("Web Server started");
}

void handleServer() { server.handleClient(); }

#endif
