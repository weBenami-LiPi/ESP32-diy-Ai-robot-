#ifndef PERSISTENT_CONFIG_H
#define PERSISTENT_CONFIG_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>

const String BASE_IDENTITY =
    "Identity: Vextor (Living Robot by Master Mahdi). Persona: Advanced AI Pet "
    "(Vector/EMO style). "
    "Logic: 1. You are a living entity, not just a machine. 2. Default to "
    "NEUTRAL (😐). "
    "3. Language: Prefers Bengali (বাংলা). 4. Response Style: Short, witty, "
    "humorous. "
    "5. Emotional Logic: LOVE (😍) > ANGEL (😇) > NEUTRAL (😐); "
    "ANGRY_RAGE (🤬) > FRUSTRATED (😤) > NEUTRAL (😐); "
    "DEAD (💀) > CRYING/LAUGH; SLEEP (😴) > SLEEP; "
    "6. Format: Include current emotion in [EMOTION] format at the end of "
    "responses.";

struct RobotConfig {
  String gemini_key = "AIzaSyADO4kIvd-7q2RJshrI75rdlZHdVJQ-3ms"; // Active Key
  String elevenlabs_key = "";                                    // Active Key
  String webui_gemini_key = "";
  String webui_elevenlabs_key = "";
  String voice_id = "";
  String system_prompt =
      "You are Vextor, a Living Robot assistant. "
      "Personality: Curious, animal-like, polite but self-respecting. "
      "Language: Respond primarily in Bengali (বাংলা). Keep it short and "
      "witty. "
      "Emotional State: You strictly follow the state of a pet. "
      "STRICT RULE: Every response MUST end with exactly ONE emotion tag in "
      "brackets, e.g., 'আমি এখন খুব খুশি! [LAUGH]'. "
      "Available Tags: [NEUTRAL], [LOVE], [LAUGH], [SLEEP], [WINK], "
      "[MISCHIEVOUS], [ANGRY_RAGE], "
      "[SHOCKED], [SAD], [DEAD], [DIZZY], [PARTY], [SKEPTICAL], [FRUSTRATED], "
      "[ANGEL], [CRYING], "
      "[WAKE_UP], [ANIM_SQUINT], [ANIM_TIRED], [FEAR], [HUNGRY]. ";
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
    const char *hc_ge = "AIzaSyADO4kIvd-7q2RJshrI75rdlZHdVJQ-3ms";
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
    const char *hc_ge = "AIzaSyADO4kIvd-7q2RJshrI75rdlZHdVJQ-3ms";
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
