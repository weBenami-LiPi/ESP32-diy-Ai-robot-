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

void handleRoot() { server.send(200, "text/html; charset=utf-8", INDEX_HTML); }

Emotion getEmotionFromEmoji(String emoji) {
  if (emoji == "😐" || emoji == "😶")
    return NEUTRAL;
  if (emoji == "😍" || emoji == "❤️" || emoji == "😘")
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
  if (emoji == "🔍")
    return ANIM_SCAN;
  if (emoji == "⏳")
    return ANIM_LOADING;
  if (emoji == "😑")
    return ANIM_SQUINT;
  if (emoji == "🥱")
    return ANIM_TIRED;
  if (emoji == "😱")
    return FEAR;
  if (emoji == "😫")
    return HUNGRY;
  return EMO_NONE;
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
        // Efficient multi-byte emoji detector (2-4 bytes)
        Emotion emoFromMsg = EMO_NONE;
        for (int i = 0; i < segment.length(); i++) {
          unsigned char c = (unsigned char)segment[i];
          if (c >= 0x80) { // Potential multi-byte emoji character
            for (int len = 4; len >= 2; len--) {
              if (i + len <= segment.length()) {
                Emotion found =
                    getEmotionFromEmoji(segment.substring(i, i + len));
                if (found != EMO_NONE) {
                  emoFromMsg = found;
                  i += (len - 1);
                  break;
                }
              }
            }
            if (emoFromMsg != EMO_NONE)
              break;
          }
        }

        if (emoFromMsg != EMO_NONE) {
          RobotAction emoAct;
          emoAct.type = ACTION_SET_EMOTION;
          emoAct.emotionVal = emoFromMsg;
          tempQueue.push_back(emoAct);
        }

        // --- SILENCED FOR LIFE ENGINE (PET MODE) ---
        // String cleanSeg = cleanMessage(segment);
        // if (cleanSeg.length() > 0) {
        //   RobotAction act;
        //   act.type = ACTION_SIMULATE_TALK;
        //   act.durationMs = cleanSeg.length() * 85;
        //   tempQueue.push_back(act);
        // }
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
      // Handle special command triggers for head gestures
      if (act.type == ACTION_ROBOT_CMD) {
        if (act.payload == "nod_yes_internal" ||
            act.payload.indexOf("nod_yes") != -1) {
          RobotAction gesture;
          gesture.type = ACTION_ROBOT_CMD;
          gesture.payload = "nod_yes";
          globalQueue.push_back(gesture);
        } else if (act.payload == "shake_no_internal" ||
                   act.payload.indexOf("shake_no") != -1) {
          RobotAction gesture;
          gesture.type = ACTION_ROBOT_CMD;
          gesture.payload = "shake_no";
          globalQueue.push_back(gesture);
        } else {
          globalQueue.push_back(act);
        }
      } else {
        globalQueue.push_back(act);
      }
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

  // Unified Processing: This handles emojis, [YES], [NO], [GLITCH], etc.
  processAndSpeakOptimized(reply);

  StaticJsonDocument<1024> doc;
  doc["reply"] = reply;
  doc["clean_reply"] = cleanMessage(reply);
  doc["tts_failed"] = true;
  doc["is_command"] = false;
  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}

void handleControl() {
  String dir = server.arg("dir");
  lastInteractionTime = millis();
  logLoc("Control: " + dir);
  moveRobot(dir);

  // Blue LED control: ON for movement, OFF for stop
  if (dir == "STOP") {
    digitalWrite(ONBOARD_LED, LOW);
  } else if (dir != "AUTO") {
    digitalWrite(ONBOARD_LED, HIGH);
  }

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
    String emotionName = "UNKNOWN";

    if (id == 0) {
      target = NEUTRAL;
      emotionName = "NEUTRAL";
    } else if (id == 1) {
      target = LOVE;
      emotionName = "LOVE";
    } else if (id == 2) {
      target = LAUGH;
      emotionName = "LAUGH";
    } else if (id == 3) {
      target = SLEEP;
      emotionName = "SLEEP";
    } else if (id == 4) {
      target = WINK;
      emotionName = "WINK";
    } else if (id == 5) {
      target = MISCHIEVOUS;
      emotionName = "MISCHIEVOUS";
    } else if (id == 6) {
      target = FRUSTRATED;
      emotionName = "FRUSTRATED";
    } else if (id == 7) {
      target = SHOCKED;
      emotionName = "SHOCKED";
    } else if (id == 8) {
      target = SAD;
      emotionName = "SAD";
    } else if (id == 9) {
      target = DEAD;
      emotionName = "DEAD";
    } else if (id == 10) {
      target = DIZZY;
      emotionName = "DIZZY";
    } else if (id == 11) {
      target = PARTY;
      emotionName = "PARTY";
    } else if (id == 12) {
      target = SKEPTICAL;
      emotionName = "SKEPTICAL";
    } else if (id == 13) {
      target = ANGRY_RAGE;
      emotionName = "ANGRY_RAGE";
    } else if (id == 14) {
      target = ANGEL;
      emotionName = "ANGEL";
    } else if (id == 15) {
      target = CRYING;
      emotionName = "CRYING";
    } else if (id == 16) {
      target = FEAR;
      emotionName = "FEAR";
    } else if (id == 17) {
      target = HUNGRY;
      emotionName = "HUNGRY";
    }

    Serial.println("Web Emotion Triggered: ID=" + String(id) + " -> " +
                   emotionName);
    setEmotion(target, true); // Explicitly set high priority
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
