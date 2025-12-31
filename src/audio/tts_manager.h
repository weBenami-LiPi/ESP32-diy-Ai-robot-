#ifndef TTS_MANAGER_H
#define TTS_MANAGER_H

#include "../config/api_config.h"
#include "../config/config.h"
#include "../config/persistent_config.h"
#include "../core/logger.h"
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

  bool downloadTTS(String text, String filename = "/tts.mp3") {
    if (WiFi.status() != WL_CONNECTED)
      return false;
    // Don't stop current playback here if we are pre-buffering!
    // But since we are downloading upfront, it's fine.

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
      File f = SPIFFS.open(filename, "w");
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
  String currentFilename = "";

  void playFile(String filename) {
    if (ttsFile)
      ttsFile.close();

    if (!SPIFFS.exists(filename))
      return;

    ttsFile = SPIFFS.open(filename, "r");
    if (!ttsFile)
      return;

    currentFilename = filename;
    isPlaying = true;
    talkStartTime = millis();
    logChat("Playing: " + filename);
  }

  void playLastTTS() { playFile("/tts.mp3"); }

  bool isSimulatingTalk = false;
  unsigned long simulationDuration = 0;

  void simulateTalk(int durationMs) {
    if (isPlaying)
      return;
    isSimulatingTalk = true;
    simulationDuration = durationMs;
    talkStartTime = millis();
    logChat("Simulating talk for " + String(durationMs) + "ms");
  }

  void stop() {
    isPlaying = false;
    isSimulatingTalk = false;
    if (ttsFile)
      ttsFile.close();
    if (currentFilename.length() > 0 && SPIFFS.exists(currentFilename)) {
      SPIFFS.remove(currentFilename);
      currentFilename = "";
    }
  }

  void loop() {
    if (isSimulatingTalk) {
      if (millis() - talkStartTime > simulationDuration) {
        stop();
        logChat("Finished simulated talk");
      }
      return;
    }

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
