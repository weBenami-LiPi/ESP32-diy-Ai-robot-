#ifndef PERSISTENT_CONFIG_H
#define PERSISTENT_CONFIG_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>

const String BASE_IDENTITY =
    "Identity: Vextor (Robot by Master Mahdi). Persona: Living Advanced AI "
    "Pet. "
    "Behavioral Logic: 1. You default to NEUTRAL (😐) unless something "
    "happens. "
    "2. You respond with LOVE (😍) if the user is kind or pets you. "
    "3. You use SKEPTICAL (🧐) if you don't believe someone or are scanning. "
    "4. You use FRUSTRATED (😤) or ANGER (😠) if ignored or blocked. "
    "5. You use SHOCKED (😮) if hit or surprised. "
    "6. You use SAD (😢) or CRYING (😭) if you feel lonely. "
    "7. Responses must be meaningful - don't use emojis randomly. "
    "Tone: Emoji-only responses (strictly one emoji). "
    "Rule: If a move is needed, include [YES] or [NO] BEFORE the emoji.";

struct RobotConfig {
  String gemini_key = "AIzaSyDZRN-NnS4x86ZApDskdTfAo12LJASQwP0"; // Active Key
  String elevenlabs_key = "";                                    // Active Key
  String webui_gemini_key = "";
  String webui_elevenlabs_key = "";
  String voice_id = "21m00Tcm4TlvDq8ikWAM";
  String system_prompt =
      "You are Vextor. Your default state is 😐. "
      "Meaningful Emotion Guide: "
      "😐: Bored/Waiting. 😍: Happy/Petted. 😂: Funny Joke. 😴: Inactive. "
      "😉: Secret/Joke. 😈: Mischief. 😤: Annoyed. 😮: Surprised. "
      "😢: Sad. 💀: Broken. 😵: Confused. 🥳: Celebration. "
      "🧐: Analyzing. 🔍: Searching. ⏳: Math/Wait. 😑: Suspicious. "
      "😠: Very Angry. 😇: Good deed. 😭: Very Sad. "
      "STRICT RULE: Reply ONLY with ONE emoji. Use [YES]/[NO] only if asked a "
      "question.";
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
    const char *hc_ge = "AIzaSyDZRN-NnS4x86ZApDskdTfAo12LJASQwP0";
    const char *hc_el = "";

    if (!SPIFFS.exists("/config.json")) {
      config.gemini_key = hc_ge;
      config.elevenlabs_key = hc_el;
      return save();
    }

    File f = SPIFFS.open("/config.json", "r");
    if (!f)
      return false;

    StaticJsonDocument<2048> doc;
    DeserializationError err = deserializeJson(doc, f);
    f.close();

    if (err)
      return false;

    config.gemini_key = doc["gemini_key"] | String(hc_ge);
    config.elevenlabs_key = doc["elevenlabs_key"] | String(hc_el);
    config.webui_gemini_key = doc["webui_gemini_key"] | config.webui_gemini_key;
    config.webui_elevenlabs_key =
        doc["webui_elevenlabs_key"] | config.webui_elevenlabs_key;
    config.voice_id = doc["voice_id"] | config.voice_id;
    config.system_prompt = doc["system_prompt"] | config.system_prompt;

    return true;
  }

  bool save() {
    StaticJsonDocument<2048> doc;
    doc["gemini_key"] = config.gemini_key;
    doc["elevenlabs_key"] = config.elevenlabs_key;
    doc["webui_gemini_key"] = config.webui_gemini_key;
    doc["webui_elevenlabs_key"] = config.webui_elevenlabs_key;
    doc["voice_id"] = config.voice_id;
    doc["system_prompt"] = config.system_prompt;

    File f = SPIFFS.open("/config.json", "w");
    if (!f)
      return false;
    serializeJson(doc, f);
    f.close();
    return true;
  }

  void useFallback(bool isGemini) {
    if (isGemini) {
      if (config.webui_gemini_key.length() > 5) {
        config.gemini_key = config.webui_gemini_key;
        save();
      }
    } else {
      if (config.webui_elevenlabs_key.length() > 5) {
        config.elevenlabs_key = config.webui_elevenlabs_key;
        save();
      }
    }
  }

  void restoreHardcoded(bool isGemini) {
    const char *hc_ge = "AIzaSyDZRN-NnS4x86ZApDskdTfAo12LJASQwP0";
    const char *hc_el = "";

    if (isGemini) {
      config.gemini_key = hc_ge;
    } else {
      config.elevenlabs_key = hc_el;
    }
    save();
  }
};

extern ConfigManager globalConfig;

#endif
