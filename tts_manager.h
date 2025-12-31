#ifndef TTS_MANAGER_H
#define TTS_MANAGER_H

#include "api_config.h"
#include "config.h"
#include "logger.h"
#include "persistent_config.h"
#include <Arduino.h>
#include <HTTPClient.h>
#include <SPIFFS.h>
#include <WiFiClientSecure.h>

class TTSManager {
public:
  bool isPlaying = false;
  bool enabled = true;
  unsigned long talkStartTime = 0;

  TTSManager() {}

  void setup() {
    // Audio hardware setup removed to fix compilation issues
  }

  bool downloadTTS(String text) {
    if (WiFi.status() != WL_CONNECTED)
      return false;
    stop();
    WiFiClientSecure client;
    client.setInsecure();
    HTTPClient http;
    String url = "https://api.elevenlabs.io/v1/text-to-speech/" +
                 globalConfig.config.voice_id +
                 "/stream?optimize_streaming_latency=2";

    if (!http.begin(client, url))
      return false;
    http.addHeader("xi-api-key", globalConfig.config.elevenlabs_key);
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Accept", "audio/mpeg");

    StaticJsonDocument<768> doc;
    doc["text"] = (text.length() > 500) ? text.substring(0, 500) : text;
    doc["model_id"] = "eleven_monolingual_v1";
    JsonObject voiceSettings = doc.createNestedObject("voice_settings");
    voiceSettings["stability"] = 0.5;
    voiceSettings["similarity_boost"] = 0.75;

    String payload;
    serializeJson(doc, payload);
    int httpCode = http.POST(payload);

    if (httpCode == 200) {
      int len = http.getSize();
      WiFiClient *stream = http.getStreamPtr();
      File f = SPIFFS.open("/tts.mp3", "w");
      if (!f) {
        http.end();
        return false;
      }
      uint8_t buff[2048];
      while (http.connected() && (len > 0 || len == -1)) {
        size_t available = stream->available();
        if (available > 0) {
          size_t toRead = min(available, (size_t)sizeof(buff));
          int c = stream->readBytes(buff, toRead);
          f.write(buff, c);
          if (len > 0)
            len -= c;
        }
        yield();
      }
      f.close();
      http.end();
      return true;
    } else {
      http.end();
      // If limit reached, switch to fallback
      globalConfig.useFallback(false);
      return false;
    }
  }

  File ttsFile;

  void playLastTTS() {
    if (ttsFile)
      ttsFile.close();
    ttsFile = SPIFFS.open("/tts.mp3", "r");
    if (!ttsFile)
      return;

    isPlaying = true;
    talkStartTime = millis();
    logChat("Playing non-blocking TTS...");
  }

  void stop() {
    isPlaying = false;
    if (ttsFile)
      ttsFile.close();
    if (SPIFFS.exists("/tts.mp3")) {
      SPIFFS.remove("/tts.mp3");
    }
  }

  void loop() {
    if (!isPlaying)
      return;

    if (ttsFile && ttsFile.available()) {
      uint8_t buf[512];
      size_t read = ttsFile.read(buf, sizeof(buf));
      size_t written = 0;
      // Use shorter timeout for non-blocking feel
      i2s_write(I2S_PORT_SPK, buf, read, &written, 10);
    } else {
      stop();
      logChat("Finished TTS playback");
    }
  }
};

TTSManager TTS;

#endif
