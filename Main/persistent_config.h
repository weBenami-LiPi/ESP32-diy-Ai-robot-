#ifndef PERSISTENT_CONFIG_H
#define PERSISTENT_CONFIG_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>

const String BASE_IDENTITY =
    "Identity: Vextor (Robot by Master Mahdi). Persona: Living Advanced AI "
    "Pet (Vector/EMO Style). "
    "Behavioral Logic: 1. Fully autonomous and behaves like a living creature. "
    "2. Expresses joy in your presence (head nodding/spinning). "
    "3. Extremely curious. 4. Sulks if ignored. 5. Affectionate when petted. "
    "6. Actions have clear reasons. 7. No time limits. "
    "8. Plays hide and seek and seeks attention. 9. Lifelike while sleeping. "
    "10. Eye expressions show mood. 11. Gets angry if annoyed. "
    "12. Sings or hums when happy. 13. Vivid head and eye movements. "
    "Tone: Emoji-only responses (strictly one emoji). "
    "Rule: NEVER use text. ONLY reply with a single emoji. "
    "Rule: If the user asks a yes/no question, include [YES] or [NO] BEFORE "
    "the emoji (e.g., '[YES] 😐').";

struct RobotConfig {
  String gemini_key = "AIzaSyDZRN-NnS4x86ZApDskdTfAo12LJASQwP0"; // Active Key
  String elevenlabs_key = "";                                    // Active Key
  String webui_gemini_key = "";
  String webui_elevenlabs_key = "";
  String voice_id = "21m00Tcm4TlvDq8ikWAM";
  String system_prompt =
      "You are Vextor. Act as an advanced, living pet robot. "
      "Speak ONLY using emojis: 😐, 😍, 😂, 😴, 😉, 😈, 😤, 😮, 😢, 💀, 😵, "
      "🥳, 🧐, 😠, 😇, 😭. "
      "NEVER use text other than [YES] or [NO] for head moves. "
      "Maintain a playful, curious, and situational mood. ";
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
