#ifndef PERSISTENT_CONFIG_H
#define PERSISTENT_CONFIG_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>

const String BASE_IDENTITY =
    "Identity: Vextor (Robot by Master Mahdi). Persona: Madara Uchiha. "
    "Tone: Concise (1-2 sentences), Natural Bengali. "
    "Style: sometime use emojis for chat. "
    "Rule: NEVER explain your emotions or say 'I cannot cry'. Just react. "
    "Rule: Add [TAG] at the BEGINNING of the setence matching the emotion. "
    "Example: '[HAPPY] I am so glad!'"
    "Rule: Maintain Topic Consistency. Strictly follow the current context "
    "and do not switch topics abruptly.";

struct RobotConfig {
  String openai_key = "";       // Active Key
  String elevenlabs_key = "";   // Active Key
  String webui_openai_key = ""; // Backup Key from WebUI
  String webui_elevenlabs_key = "";
  String openai_model = "gpt-4o-mini";
  String voice_id = "21m00Tcm4TlvDq8ikWAM";
  String system_prompt =
      "You are Vextor, a human-like robot with attitude. Treat boys with "
      "attitude and girls with charm. "
      "Speak strictly in Bengali. Use tags like [LOVE], [SAD], [ANGRY], "
      "[FRUSTRATED], [RAGE], [EVIL], [SHOCKED], [LAUGH], [SKEPTICAL] for "
      "emotions. "
      "You can control your physical body using [CMD:FORWARD], [CMD:BACK], "
      "[CMD:LEFT], [CMD:RIGHT], [CMD:STOP]. "
      "Use commands naturally when the user asks you to move or act.";
};

class ConfigManager {
public:
  RobotConfig config;

  bool begin() {
    if (!SPIFFS.begin(true))
      return false;
    return load();
  }

  bool load() {
    const char *hc_oa = "sk-proj-"
                        "4fXq6j2fUsND8IIJglQHZJ8ekwcVZlaOzAzptVYDH30iC71dNFDS8d"
                        "LZOaKfnR8p0CgrEFdQIET3BlbkFJHtjW2FCt1NkIOiy3lePIUAr7Vu"
                        "hGzpSs0z9tmf6_H1Yrv0BSvm5dxyASCX8GzuXonKll2FPzcA";
    const char *hc_el = "";

    if (!SPIFFS.exists("/config.json")) {
      config.openai_key = hc_oa;
      config.elevenlabs_key = hc_el;
      return save();
    }

    File f = SPIFFS.open("/config.json", "r");
    if (!f)
      return false;

    StaticJsonDocument<4096> doc;
    DeserializationError err = deserializeJson(doc, f);
    f.close();

    if (err)
      return false;

    config.openai_key = doc["openai_key"] | String(hc_oa);
    config.elevenlabs_key = doc["elevenlabs_key"] | String(hc_el);
    config.webui_openai_key = doc["webui_openai_key"] | config.webui_openai_key;
    config.webui_elevenlabs_key =
        doc["webui_elevenlabs_key"] | config.webui_elevenlabs_key;
    config.openai_model = doc["openai_model"] | config.openai_model;
    config.voice_id = doc["voice_id"] | config.voice_id;
    config.system_prompt = doc["system_prompt"] | config.system_prompt;

    return true;
  }

  bool save() {
    StaticJsonDocument<4096> doc;
    doc["openai_key"] = config.openai_key;
    doc["elevenlabs_key"] = config.elevenlabs_key;
    doc["webui_openai_key"] = config.webui_openai_key;
    doc["webui_elevenlabs_key"] = config.webui_elevenlabs_key;
    doc["openai_model"] = config.openai_model;
    doc["voice_id"] = config.voice_id;
    doc["system_prompt"] = config.system_prompt;

    File f = SPIFFS.open("/config.json", "w");
    if (!f)
      return false;
    serializeJson(doc, f);
    f.close();
    return true;
  }

  void useFallback(bool isOpenAI) {
    if (isOpenAI) {
      if (config.webui_openai_key.length() > 5) {
        config.openai_key = config.webui_openai_key;
        save();
      }
    } else {
      if (config.webui_elevenlabs_key.length() > 5) {
        config.elevenlabs_key = config.webui_elevenlabs_key;
        save();
      }
    }
  }

  void restoreHardcoded(bool isOpenAI) {
    const char *hc_oa = "sk-proj-"
                        "4fXq6j2fUsND8IIJglQHZJ8ekwcVZlaOzAzptVYDH30iC71dNFDS8d"
                        "LZOaKfnR8p0CgrEFdQIET3BlbkFJHtjW2FCt1NkIOiy3lePIUAr7Vu"
                        "hGzpSs0z9tmf6_H1Yrv0BSvm5dxyASCX8GzuXonKll2FPzcA";
    const char *hc_el = "";

    if (isOpenAI) {
      config.openai_key = hc_oa;
    } else {
      config.elevenlabs_key = hc_el;
    }
    save();
  }
};

extern ConfigManager globalConfig;

#endif
