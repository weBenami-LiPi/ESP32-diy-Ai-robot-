#include "soc/rtc_cntl_reg.h"
#include "soc/soc.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <SPIFFS.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>

#include "action_queue.h"
#include "audio_manager.h"
#include "bitmaps.h"
#include "body_sync.h"
#include "command_processor.h"
#include "config.h"
#include "gemini_manager.h"
#include "hardware_ctrl.h"
#include "logger.h"
#include "persistent_config.h"
#include "web_server_manager.h"

const char *ssid = "Glyph";
const char *password = "Mahdi&981";

#include <SPI.h>

Adafruit_SSD1306 display(128, 64, &SPI, OLED_DC, OLED_RESET, OLED_CS);
ConfigManager globalConfig;
GeminiManager gemini;
Servo headServo;
Servo tiltServo;
std::vector<RobotAction> globalQueue;
bool autoPilotActive = false;
int robotSpeed = 255;
int currentRampSpeed = 0;
int targetRampSpeed = 0;

int currentHeadAngle = 90;
int currentTiltAngle = 90;
bool isTalkingNow = false;
bool inhibitAutopilot = false; // Flag to pause ultrasonic reactions
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
  if (currentEmotion != e) {
    // Physical Jolt Effect (Expressive movement reactions)
    int pulseCount =
        (random(100) < 40) ? 2 : 1; // 40% chance of double jolt for added life

    if (e == LOVE || e == PARTY) {
      startPulseBody(true, 80, (e == PARTY) ? 2 : pulseCount); // Joyful jumps
    } else if (e == SHOCKED || e == ANGRY_RAGE) {
      startPulseBody(false, 100,
                     2); // Sharp intense recoils (always double for rage)
    } else if (e == SAD || e == CRYING) {
      startPulseBody(false, 50, 1); // Single sad flinch
    } else if (e == LAUGH) {
      startPulseBody(true, 40, 2); // Quick double "giggle" jolt
    } else if (e == SKEPTICAL || e == MISCHIEVOUS) {
      startPulseBody(true, 30, pulseCount); // Curious subtle lean
    }
  }
  currentEmotion = e;
  currentFrame = 0;
  lastEmotionUpdate = millis();
}

void setAutoPilotEmotion(String state) {
  if (state == "ACTIVE") {
    setEmotion(NEUTRAL);
  } else if (state == "OBSTACLE" || state == "CAUTION") {
    setEmotion(SHOCKED);
  } else if (state == "FOLLOW") {
    setEmotion(LOVE);
  } else if (state == "CURIOUS") {
    setEmotion(SKEPTICAL);
  } else if (state == "SCARED" || state == "HISS") {
    setEmotion(ANGRY_RAGE);
  }
}

// Wrapper function for TTS stop (used by command processor)
// Wrapper function for TTS stop (Wait, if TTS is removed, we just stub this)
void stopTTS() { /* TTS.stop() removed */ }

void handleActionQueue() {
  // TTS check removed as TTS is not active in this build

  while (!globalQueue.empty()) {
    RobotAction act = globalQueue.front();
    if (act.type == ACTION_SET_EMOTION) {
      setEmotion(act.emotionVal);
      globalQueue.erase(globalQueue.begin());
    } else if (act.type == ACTION_PLAY_AUDIO) {
      // TTS.playFile(act.payload); // Removed
      globalQueue.erase(globalQueue.begin());
    } else if (act.type == ACTION_SIMULATE_TALK) {
      // TTS.simulateTalk(act.durationMs); // Removed
      globalQueue.erase(globalQueue.begin());
    } else if (act.type == ACTION_ROBOT_CMD) {
      if (act.payload == "nod_yes_internal") {
        cmdProcessor.detectAndExecute("nod yes");
      } else if (act.payload == "shake_no_internal") {
        cmdProcessor.detectAndExecute("shake no");
      } else {
        moveRobot(act.payload);
      }
      globalQueue.erase(globalQueue.begin());
      // Don't break, allow other actions like emotion to happen simultaneously
    } else {
      globalQueue.erase(globalQueue.begin());
    }
  }
}

void setup() {
  // 1. Brownout Protection (Keep disabled if power is unstable)
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

  Serial.begin(115200);

  // 2. Init Display First (Low Power)
  SPI.begin(OLED_CLK, -1, OLED_MOSI, OLED_CS);
  display.begin(SSD1306_SWITCHCAPVCC);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Booting System...");
  display.display();
  delay(100);

  // 3. Init Config System
  display.println("- Config...");
  display.display();
  globalConfig.begin();
  delay(50);

  // 4. Init File System
  display.println("- SPIFFS...");
  display.display();
  SPIFFS.begin(true);
  delay(50);

  // 5. Init Hardware (Motors/Servos) - POTENTIAL SPIKE SOURCE
  display.println("- Hardware...");
  display.display();
  setupHardware(); // Output should now be safe (Disabled -> Zeroed -> Enabled)
  delay(200);      // Allow capacitance to recharge

  delay(100);

  // 7. Init WiFi - HIGH POWER
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Connecting WiFi:");
  display.println(ssid);
  display.display();

  WiFi.begin(ssid, password);

  int retries = 0;
  while (WiFi.status() != WL_CONNECTED && retries < 20) {
    delay(500);
    Serial.print(".");
    display.print(".");
    display.display();
    retries++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConnected!");
    logLoc("Connected to WiFi: " + String(ssid));

    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Online!");
    display.println(WiFi.localIP());
    display.display();
  } else {
    display.println("\nWiFi Failed!");
    display.display();
  }

  delay(1000);

  setupWebServer();

  // Boot-up API Status Check & Greeting
  globalConfig.restoreHardcoded(true);
  globalConfig.restoreHardcoded(false);

  if (WiFi.status() == WL_CONNECTED) {
    // TTS Greeting removed
  }
}

void loop() {
  if (Serial.available() > 0) {
    String msg = Serial.readStringUntil('\n');
    msg.trim();
    if (msg.length() > 0) {
      logChat("Serial User: " + msg);
      setEmotion(NEUTRAL);

      // COMMAND DETECTION: Check if message is a direct command
      if (cmdProcessor.detectAndExecute(msg)) {
        Serial.println("[Command Executed]");
        // Command executed silently - no AI processing
      } else {
        // NOT A COMMAND: Proceed with normal AI conversation
        String reply = gemini.askGemini(msg, globalConfig.config.system_prompt);
        logChat("Vextor: " + reply);
        Serial.println("Vextor: " + reply);
        processAndSpeak(reply);
      }
    }
  }

  // handleActionQueue updates emotions and movement commands
  handleActionQueue();
  updatePulseBody();
  bool talk = false; // TTS replaced by web fallbacks
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

  if (now - lastInteractionTime > 120000 &&
      currentEmotion != SLEEP) { // 2 minutes
    setEmotion(SLEEP);
  }

  // Auto-transition from WAKE_UP to NEUTRAL
  if (currentEmotion == WAKE_UP && (now - lastEmotionUpdate > 1500)) {
    setEmotion(NEUTRAL);
  }

  // Natural Curiosity & Saccade System
  static unsigned long lastEyeMove = 0;
  static unsigned long nextEyeMove = 80000; // 40x slower than 2000
  static int eyeMoveSpeed = 2;

  if (isTalkingNow || currentEmotion != NEUTRAL || autoPilotActive) {
    targetPupilX = 0;
    targetPupilY = 0;
    eyeMoveSpeed = 2; // Fast focus
  } else if (!autoPilotActive && (now - lastInteractionTime > 3000)) {
    // Only look around if idle for > 3 seconds and NOT in autopilot
    if (now - lastEyeMove > nextEyeMove) {
      lastEyeMove = now;

      // Decision: Extreme low saccade chance (less than 1%) for natural feel
      bool isSaccade = random(1000) < 5;
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
      // 40x Slower updates (was 3000-7000ms -> approx 120,000-280,000ms)
      nextEyeMove = random(120000, 280000);
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

  if (currentEmotion != NEUTRAL && currentEmotion != SLEEP && !isTalkingNow) {
    if (currentEmotion == WINK && currentFrame >= 6) {
      setEmotion(NEUTRAL);
    } else if (millis() - lastEmotionUpdate > 6000) {
      setEmotion(NEUTRAL);
    }
  }

  if (autoPilotActive && !inhibitAutopilot) {
    autoPilotLoop();
  }

  updateSpeedStep();
  handleServer();

  // 5-Hour Periodic API Key Check
  if (now - lastApiCheck > apiCheckInterval) {
    lastApiCheck = now;
    logLoc("System: 5-hour timer reached. Re-checking In-Build API keys...");
    globalConfig.restoreHardcoded(true);
    globalConfig.restoreHardcoded(false);
  }
}