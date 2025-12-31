#include "soc/rtc_cntl_reg.h"
#include "soc/soc.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <SPIFFS.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>

#include "api_config.h"
#include "audio_manager.h"
#include "bitmaps.h"
#include "body_sync.h"
#include "config.h"
#include "hardware_ctrl.h"
#include "logger.h"
#include "openai_manager.h"
#include "persistent_config.h"
#include "tts_manager.h"
#include "web_server_manager.h"

Adafruit_SSD1306 display(128, 64, OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET,
                         OLED_CS);
ConfigManager globalConfig;
OpenAIManager openai;
Servo headServo;
int currentHeadAngle = 90;
bool isTalkingNow = false;
Emotion currentEmotion = NEUTRAL;
int currentFrame = 0;
int pupilX = 0, pupilY = 0, targetPupilX = 0, targetPupilY = 0;
int currentEyeHeight = EYE_HEIGHT, targetEyeHeight = EYE_HEIGHT;
unsigned long blinkTimer = 0;
int nextBlink = 4000;
unsigned long lastEmotionUpdate = 0;
unsigned long lastInteractionTime = 0;
unsigned long lastApiCheck = 0;
const unsigned long apiCheckInterval = 5 * 60 * 60 * 1000; // 5 hours

void setEmotion(Emotion e) {
  currentEmotion = e;
  currentFrame = 0;
  lastEmotionUpdate = millis();
}

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  Serial.begin(115200);
  globalConfig.begin();
  SPIFFS.begin(true);
  display.begin(SSD1306_SWITCHCAPVCC);
  setupHardware();
  setupAudio();
  TTS.setup();
  WiFi.begin(ssid, password);

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Connecting to:");
  display.println(ssid);
  display.display();

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    display.print(".");
    display.display();
  }
  Serial.println("\nConnected!");
  logLoc("Connected to WiFi: " + String(ssid));

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Connected!");
  display.println("IP:");
  display.println(WiFi.localIP());
  display.display();
  delay(2000);

  setupWebServer();

  // Boot-up API Status Check & Greeting
  globalConfig.restoreHardcoded(true);
  globalConfig.restoreHardcoded(false);

  if (TTS.downloadTTS("Hey whatsapp")) {
    TTS.playLastTTS();
  }
}

void loop() {
  if (Serial.available() > 0) {
    String msg = Serial.readStringUntil('\n');
    msg.trim();
    if (msg.length() > 0) {
      logChat("Serial User: " + msg);
      setEmotion(NEUTRAL);
      String reply = openai.sendText(msg, globalConfig.config.system_prompt);
      logChat("Vextor: " + reply);
      Serial.println("Vextor: " + reply);
      processAndSpeak(reply);
    }
  }

  TTS.loop();
  bool talk = TTS.isPlaying;
  isTalkingNow = talk;
  BodySync.setSpeaking(talk);
  BodySync.update();

  unsigned long now = millis();

  // Dynamic Blinking Logic
  int minBlink = isTalkingNow ? 10000 : 7000;
  int maxBlink = isTalkingNow ? 15000 : 11000;

  if (now - blinkTimer > nextBlink) {
    blinkTimer = now;
    nextBlink = random(minBlink, maxBlink);
    targetEyeHeight = 2; // Close eyes
  } else if (currentEyeHeight <= 4) {
    targetEyeHeight = EYE_HEIGHT; // Open eyes
  }

  if (isTalkingNow || (currentEmotion != NEUTRAL && currentEmotion != SLEEP) ||
      autoPilotActive) {
    lastInteractionTime = now;
  }

  if (now - lastInteractionTime > 120000) { // 2 minutes
    setEmotion(SLEEP);
  }

  // Natural Curiosity & Saccade System
  static unsigned long lastEyeMove = 0;
  static unsigned long nextEyeMove = 2000;
  static int eyeMoveSpeed = 2;

  if (isTalkingNow || currentEmotion != NEUTRAL) {
    targetPupilX = 0;
    targetPupilY = 0;
    eyeMoveSpeed = 2;
  } else if (!autoPilotActive && (now - lastInteractionTime > 3000)) {
    // Only look around if idle for > 3 seconds
    if (now - lastEyeMove > nextEyeMove) {
      lastEyeMove = now;

      // Decision: Much lower saccade chance (20%) to reduce jitter
      bool isSaccade = random(100) < 20;
      eyeMoveSpeed =
          isSaccade ? 2 : 1; // Slower speed: 2 for saccade, 1 for drift

      // Decision: Stronger Center Bias (80%) for a "Neutral" look
      if (random(100) < 80) {
        targetPupilX = 0;
        targetPupilY = 0;
      } else {
        // Restricted Pupil Range (More subtle movement)
        targetPupilX = random(-6, 7);
        targetPupilY = random(-3, 4);
      }

      // Longer Interest Duration for "Steady" feel
      nextEyeMove = random(3000, 7000); // Slower updates (was 2000-6000)
    }
  }

  static unsigned long lastR = 0;
  if (now - lastR > 50) {
    lastR = now;
    if (currentEyeHeight < targetEyeHeight)
      currentEyeHeight = min(currentEyeHeight + 5, targetEyeHeight);
    else if (currentEyeHeight > targetEyeHeight)
      currentEyeHeight = max(currentEyeHeight - 5, targetEyeHeight);
    if (pupilX < targetPupilX)
      pupilX = min(pupilX + eyeMoveSpeed, targetPupilX);
    else if (pupilX > targetPupilX)
      pupilX = max(pupilX - eyeMoveSpeed, targetPupilX);
    if (pupilY < targetPupilY)
      pupilY = min(pupilY + eyeMoveSpeed, targetPupilY);
    else if (pupilY > targetPupilY)
      pupilY = max(pupilY - eyeMoveSpeed, targetPupilY);
    renderEmotionFrame(display, currentEmotion, currentFrame++);
  }

  if (currentEmotion != NEUTRAL && currentEmotion != SLEEP && !isTalkingNow &&
      millis() - lastEmotionUpdate > 6000) {
    setEmotion(NEUTRAL);
  }

  if (autoPilotActive) {
    autoPilotLoop();
  }

  handleServer();

  // 5-Hour Periodic API Key Check
  if (now - lastApiCheck > apiCheckInterval) {
    lastApiCheck = now;
    logLoc("System: 5-hour timer reached. Re-checking In-Build API keys...");
    globalConfig.restoreHardcoded(true);
    globalConfig.restoreHardcoded(false);
  }
}