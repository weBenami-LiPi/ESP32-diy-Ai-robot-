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
#include "emotion_brain.h"
#include "emotion_reason.h"
#include "gemini_manager.h"
#include "hardware_ctrl.h"
#include "life_engine.h"
#include "logger.h"
#include "neural_brain.h"
#include "persistent_config.h"
#include "rover_navigation.h"
#include "smart_movement.h"
#include "web_server_manager.h"

const char *ssid = "Glyph";
const char *password = "Mahdi&981";

#include <SPI.h>

// Software SPI constructor is more robust on expansion boards
Adafruit_SSD1306 display(128, 64, OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET,
                         OLED_CS);
ConfigManager globalConfig;
GeminiManager gemini;
LifeEngine Life;
NeuralBrain Brain;
SmartMovement SmartMove;
EmotionBrain EmoBrain;
EmotionReasonSystem Reason;
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
unsigned long talkEndTime = 0;
bool inhibitAutopilot = false;
bool touchEnabled = true;
unsigned long persistenceEndTime = 0;
float tirednessFactor = 0.0;
Emotion currentEmotion = NEUTRAL;
Emotion nextEmotion = EMO_NONE; // Chain transition queue
int currentFrame = 0;
int pupilX = 0, pupilY = 0, targetPupilX = 0, targetPupilY = 0;
int currentEyeHeight = EYE_HEIGHT, targetEyeHeight = EYE_HEIGHT;
unsigned long blinkTimer = 0;
int nextBlink = 4000;
unsigned long lastEmotionUpdate = 0;
float emotionIntensity = 100.0; // Default max intensity on state change
unsigned long lastInteractionTime = 0;
const unsigned long apiCheckInterval = 5 * 60 * 60 * 1000; // 5 hours
unsigned long lastApiCheck = 0;
unsigned long currentSleepThreshold = 120000; // Default 2 mins (User Request)
unsigned long lastLoveTime = 0;               // Global for multi-file sync
unsigned long lastNeuralDecision = 0; // Global for startup delay control

void setEmotion(Emotion e, bool isHighPriority) {
  // If a high-priority emotion (AI/User) is active, ignore routine autopilot
  // updates UNLESS we are trying to return to NEUTRAL
  static bool lastWasPriority = false;
  if (!isHighPriority && lastWasPriority && (millis() < persistenceEndTime) &&
      (e != NEUTRAL)) {
    Serial.println("Emotion blocked: Low priority emotion blocked by active "
                   "high-priority emotion");
    return;
  }

  // Clear pending movement actions if high priority emotion is set
  if (isHighPriority && e != NEUTRAL) {
    globalQueue.clear();
  }

  // Cooldown for LOVE emotion
  if (e == LOVE && (millis() - lastLoveTime < 30000) && !isHighPriority) {
    Serial.println("Emotion blocked: LOVE cooldown active (30s)");
    return;
  }
  if (e == LOVE)
    lastLoveTime = millis();

  if (currentEmotion != e) {
    int pulseCount = (random(100) < 40) ? 2 : 1;
    if (e == LOVE || e == PARTY) {
      startPulseBody(true, 80, (e == PARTY) ? 2 : pulseCount);
    } else if (e == SHOCKED || e == ANGRY_RAGE) {
      startPulseBody(false, 100, 2);
    } else if (e == SAD || e == CRYING) {
      startPulseBody(false, 50, 1);
    } else if (e == LAUGH) {
      startPulseBody(true, 40, 2);
    } else if (e == SKEPTICAL || e == MISCHIEVOUS) {
      startPulseBody(true, 30, pulseCount);
    }
  }

  // --- Dynamic Persistence Logic (Restored from Backup) ---
  unsigned long duration = 0;
  if (isHighPriority) {
    if (e == NEUTRAL) {
      duration =
          30000; // Shield: Keep 😐 for 30s against sensors (Backup value)
    } else {
      switch (e) {
      case LOVE:
        duration = 7500;
        break; // 😍
      case LAUGH:
        duration = 6000;
        break; // 😂
      case WINK:
        duration = 4000;
        break; // 😉
      case ANGRY_RAGE:
        duration = 15000;
        break; // 🤬
      case FRUSTRATED:
        duration = 8000;
        break; // 😤
      case SAD:
        duration = 10000;
        break; // 😢
      case CRYING:
        duration = 15000;
        break; // 😭
      case SHOCKED:
        duration = 4000;
        break; // 😮
      case SKEPTICAL:
        duration = 7000;
        break; // 🤨
      case MISCHIEVOUS:
        duration = 10000;
        break; // 😈
      case DIZZY:
        duration = 7000;
        break; // 😵
      case ANGEL:
        duration = 8000;
        break; // 😇
      case PARTY:
        duration = 12000;
        break; // 🥳
      case DEAD:
        duration = 6000;
        break; // 💀
      case SLEEP:
        duration = 1200000;
        break; // 😴
      default:
        duration = 5000;
        break;
      }
    }
    persistenceEndTime = millis() + duration;
    lastWasPriority = true;
    Serial.print("Emotion set: ");
    Serial.print(e);
    Serial.print(" (High Priority) - Persistence: ");
    Serial.print(duration / 1000.0);
    Serial.println("s");
  } else {
    lastWasPriority = false;
    if (millis() >= persistenceEndTime) {
      persistenceEndTime = 0;
    }

    // Non-priority updates (Autonomous/Rover activity)
    if (e != NEUTRAL) {
      duration = (e == LOVE) ? 7000 : 4000;
      persistenceEndTime = millis() + duration;
    }

    Serial.print("Emotion set: ");
    Serial.print(e);
    Serial.println(" (Low Priority)");
  }

  currentEmotion = e;
  currentFrame = 0;
  lastEmotionUpdate = millis();
  emotionIntensity = 100.0;

  if (e != SLEEP) {
    lastInteractionTime = millis();
  }
  targetEyeHeight = EYE_HEIGHT;

  // Execute synchronized emotion behavior (face + head + body)
  if (!isTalkingNow) {
    executeEmotionBehavior(e);
  }

  // Define Next Emotion for chaining (style.txt rules)
  if (isHighPriority || e != NEUTRAL) {
    switch (e) {
    case LOVE:
      nextEmotion = ANGEL;
      break;
    case ANGEL:
      nextEmotion = NEUTRAL;
      break;
    case LAUGH:
      nextEmotion = NEUTRAL;
      break;
    case WINK:
      nextEmotion = NEUTRAL;
      break;
    case MISCHIEVOUS:
      nextEmotion = NEUTRAL;
      break;
    case ANGRY_RAGE:
      nextEmotion = FRUSTRATED;
      break;
    case FRUSTRATED:
      nextEmotion = NEUTRAL;
      break;
    case SHOCKED:
      nextEmotion = NEUTRAL;
      break;
    case SAD:
      nextEmotion = NEUTRAL;
      break;
    case DEAD:
      nextEmotion = (random(100) < 50) ? CRYING : LAUGH;
      break;
    case DIZZY:
      nextEmotion = NEUTRAL;
      break;
    case PARTY:
      nextEmotion = LOVE;
      break;
    case SKEPTICAL:
      nextEmotion = NEUTRAL;
      break;
    case CRYING:
      nextEmotion = SAD; // 😭 > 😥 (SAD) > 😐
      break;
    case WAKE_UP:
      nextEmotion = NEUTRAL;
      break;
    case ANIM_SQUINT:
      nextEmotion = NEUTRAL;
      break;
    case ANIM_TIRED:
      nextEmotion = NEUTRAL;
      break;
    case FEAR:
      nextEmotion = NEUTRAL;
      break;
    case HUNGRY:
      nextEmotion = DIZZY;
      break;
    case SLEEP:
      nextEmotion = SLEEP; // Loop sleep
      break;
    case NEUTRAL:
      nextEmotion = EMO_NONE;
      break;
    default:
      nextEmotion = NEUTRAL;
      break;
    }
  } else {
    nextEmotion = EMO_NONE;
  }
}

void setAutoPilotEmotion(String state) {
  // DISABLED: No automatic emotion changes without reason
  // Robot will stay NEUTRAL in autonomous mode
  // Only user commands, AI responses, or sensor triggers will change emotions

  /*
  // Use weighted probability-based emotion selection
  // 50% NEUTRAL, 5% LOVE, 45% OTHER (context-based)
  Emotion selectedEmotion =
      EmoBrain.selectWeightedAutonomousEmotion(state, Life.currentMood);
  setEmotion(selectedEmotion, false);
  */

  // Stay in NEUTRAL - no random emotions
  // Emotions only change with explicit triggers
}

void stopTTS() {}

// Non-blocking Action Queue Processor
void processQueue() {
  static unsigned long lastActionTime = 0;
  static unsigned long actionDelay = 0;

  if (globalQueue.empty())
    return;

  unsigned long now = millis();
  if (now - lastActionTime < actionDelay)
    return;

  RobotAction act = globalQueue.front();
  globalQueue.erase(globalQueue.begin());

  lastActionTime = now;
  actionDelay = act.durationMs;

  switch (act.type) {
  case ACTION_SET_EMOTION:
    setEmotion(act.emotionVal, true);
    break;

  case ACTION_ROBOT_CMD:
    if (act.payload.startsWith("head:")) {
      int angle = act.payload.substring(5).toInt();
      setHeadAngle(angle);
    } else if (act.payload.startsWith("move:")) {
      String dir = act.payload.substring(5);
      moveRobot(dir);
    } else if (act.payload.startsWith("pulse:")) {
      String params = act.payload.substring(6);
      int commaIdx = params.indexOf(',');
      if (commaIdx != -1) {
        bool forward = (params.substring(0, commaIdx) == "F");
        int count = params.substring(commaIdx + 1).toInt();
        startPulseBody(forward, 40, count);
      }
    } else {
      moveRobot(act.payload);
    }
    break;

  case ACTION_DELAY:
    // Just the durationMs in actionDelay is enough
    break;

  case ACTION_SIMULATE_TALK:
    // Duration based on text length (simplified)
    break;
  }
}

void setup() {
  // 1. Brownout Protection (Keep disabled if power is unstable)
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

  Serial.begin(115200);

  // 2. Init Display (With Manual Reset & Stage Logging)
  Serial.println("\n--- VEXTOR BOOT SEQUENCE ---");
  Serial.println("Display Boot: Stage 1 - Stabilizing Power...");
  delay(1500);

  Serial.println("Display Boot: Stage 2 - Pin Configuration...");
  pinMode(OLED_DC, OUTPUT);
  pinMode(OLED_RESET, OUTPUT);
  pinMode(OLED_CS, OUTPUT);

  // Force CS LOW to select the chip
  digitalWrite(OLED_CS, LOW);
  Serial.println(" -> CS Pin 15: LOW");
  Serial.println(" -> DC Pin 5: OUTPUT");

  Serial.println("Display Boot: Stage 3 - Hardware Reset Pulse...");
  digitalWrite(OLED_RESET, HIGH);
  delay(50);
  digitalWrite(OLED_RESET, LOW);
  delay(150);
  digitalWrite(OLED_RESET, HIGH);
  delay(150);

  Serial.println("Display Boot: Stage 4 - display.begin()...");
  if (!display.begin(SSD1306_SWITCHCAPVCC)) {
    Serial.println("CRITICAL: OLED Init Failed!");
    Serial.println("Troubleshooting Tips:");
    Serial.println("1. Check DC Wire  -> Must be on GPIO 5 (D5)");
    Serial.println("2. Check RES Wire -> Must be on GPIO 4 (D4)");
    Serial.println("3. Check CS Wire  -> Must be on GPIO 15 (D15)");
    Serial.println("4. Check MOSI/SDA -> GPIO 23, CLK/SCL -> GPIO 18");
    Serial.println(
        "5. If it's a 1.3 inch display, it might need SH1106 driver.");
  } else {
    Serial.println("SUCCESS: OLED Connected via Software SPI.");
  }

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

  // 5. Init Hardware
  display.println("- Hardware...");
  display.display();
  setupHardware();
  delay(200);

  // Init Life Engine
  Life.begin();
  Reason.begin();

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

  // Clear boot text and show Normal eyes immediately with 10s Startup Lock
  lastInteractionTime = millis();
  currentEmotion = NEUTRAL;
  persistenceEndTime = millis() + 10000;
  lastLoveTime = millis();
  lastNeuralDecision = millis();
  lastEmotionUpdate = millis();

  // Initialize Neural Brain System
  Brain.begin();
  SmartMove.begin();
  Serial.println(F("Neural Brain System: ONLINE (10s Startup Lock)"));
  Brain.printWeights();
}

void loop() {
  if (Serial.available() > 0) {
    String msg = Serial.readStringUntil('\n');
    msg.trim();
    if (msg.length() > 0) {
      logChat("Serial User: " + msg);
      autoPilotActive = false; // Override autopilot on interaction
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

  unsigned long now = millis();

  // processQueue updates emotions and movement commands
  processQueue();
  Life.update();   // Update Mood and State Machine
  Reason.update(); // Update Emotion Resonance/Decay
  updatePulseBody();
  isTalkingNow = (millis() < talkEndTime);
  BodySync.setSpeaking(isTalkingNow);
  BodySync.update();
  checkTouchInteraction(); // Check for petting/hits

  // EMOTION REASONING & CHAINING (style.txt rules)
  static unsigned long lastReasonCheck = 0;
  if (now - lastReasonCheck > 5000) {
    lastReasonCheck = now;
    if (millis() >= persistenceEndTime || persistenceEndTime == 0) {
      if (nextEmotion != EMO_NONE) {
        // Always allow chain transitions (even in manual mode) to complete
        // interaction-based sequences
        setEmotion(nextEmotion, false);
      } else if (autoPilotActive && random(100) < 10) {
        // Only trigger new random emotions if Autopilot is ON
        Emotion best = Reason.getBestEmotion();
        if (best != NEUTRAL && best != currentEmotion) {
          setEmotion(best, false);
        }
      } else if (autoPilotActive) {
        // Maintain Neutral if in Autopilot
        setEmotion(NEUTRAL, false);
      }
    }
  }

  // --- Circadian Rhythm / Energy System ---
  static unsigned long lastEnergyUpdate = 0;
  if (now - lastEnergyUpdate > 60000) { // Every minute
    lastEnergyUpdate = now;
    tirednessFactor += 0.01; // Increase tiredness slowly
    if (tirednessFactor > 1.0)
      tirednessFactor = 1.0;
  }

  // --- Advanced Procedural Blinking ---
  // Disable blinking for specific emotions that have their own eye logic or
  // should stay open
  bool canBlink = !(currentEmotion == LOVE || currentEmotion == ANGEL ||
                    currentEmotion == LAUGH || currentEmotion == SLEEP ||
                    currentEmotion == ANGRY_RAGE || currentEmotion == DEAD ||
                    currentEmotion == DIZZY);

  int minBlink =
      (currentEmotion == ANIM_TIRED) ? 3000 : (isTalkingNow ? 10000 : 7000);
  int maxBlink =
      (currentEmotion == ANIM_TIRED) ? 6000 : (isTalkingNow ? 15000 : 11000);
  static bool isDoubleBlinking = false;

  if (canBlink && (now - blinkTimer > nextBlink)) {
    blinkTimer = now;
    targetEyeHeight = 2; // Close eyes

    if (!isDoubleBlinking && random(100) < 15) {
      isDoubleBlinking = true;
      nextBlink = 200;
    } else {
      isDoubleBlinking = false;
      nextBlink = random(minBlink, maxBlink);
    }
  } else if (currentEyeHeight <= 4) {
    targetEyeHeight = EYE_HEIGHT; // Open eyes
  }

  if (isTalkingNow || (currentEmotion != NEUTRAL && currentEmotion != SLEEP) ||
      autoPilotActive) {
    lastInteractionTime = now;
  }

  // Explicit wake-up if something starts happening while asleep
  if (currentEmotion == SLEEP && (isTalkingNow || autoPilotActive)) {
    setEmotion(NEUTRAL);
  }

  if (now - lastInteractionTime > currentSleepThreshold &&
      currentEmotion != SLEEP) {
    setEmotion(SLEEP);
    // Recalculate next random sleep threshold (approx 2 minutes: 110s-130s)
    currentSleepThreshold = random(110000, 130001);
  }

  // Auto-transition from WAKE_UP to NEUTRAL
  if (currentEmotion == WAKE_UP && (now - lastEmotionUpdate > 1500)) {
    setEmotion(NEUTRAL);
  }

  // --- Advanced Natural Curiosity & Saccade System ---
  static unsigned long lastEyeMove = 0;
  static unsigned long nextEyeMove = 2000;
  static int eyeMoveSpeed = 2;

  if (isTalkingNow || currentEmotion != NEUTRAL || autoPilotActive) {
    // Focused state
    int focusInterval = (currentEmotion == ANIM_SCAN) ? 500 : 2000;
    if (now - lastEyeMove > focusInterval) {
      lastEyeMove = now;
      targetPupilX = (currentEmotion == ANIM_SCAN) ? random(-10, 11) : 0;
      targetPupilY = 0;
      eyeMoveSpeed = 2;
    }
  } else if (now - lastInteractionTime > 3000) {
    // Idle/Curious drift
    if (now - lastEyeMove > nextEyeMove) {
      lastEyeMove = now;

      // Frequent saccades (jitters) for high "life"
      bool isSaccade = random(100) < 30; // 30% chance of saccade
      eyeMoveSpeed = isSaccade ? 3 : 1;

      if (random(100) < 60) { // 60% center bias
        targetPupilX = 0;
        targetPupilY = 0;
      } else {
        targetPupilX = random(-12, 13);
        targetPupilY = random(-4, 5);
      }
      nextEyeMove = isSaccade ? random(100, 500) : random(2000, 5000);
    }
  }

  // --- Breathing Micro-movements (Servos) ---
  static unsigned long lastBreathing = 0;
  if (now - lastBreathing > 1500 && autoPilotActive && !isTalkingNow) {
    lastBreathing = now;
    int breathOffset = (now / 1500 % 2 == 0) ? 2 : -2;
    setHeadAngle(currentHeadAngle + breathOffset);
  }

  static unsigned long lastR = 0;
  if (now - lastR > 30) {
    lastR = now;
    // Faster transition speed (from 5 to 15) for ~0.2s blink
    if (currentEyeHeight < targetEyeHeight)
      currentEyeHeight = min(currentEyeHeight + 15, targetEyeHeight);
    else if (currentEyeHeight > targetEyeHeight)
      currentEyeHeight = max(currentEyeHeight - 15, targetEyeHeight);
    if (pupilX < targetPupilX)
      pupilX = min(pupilX + eyeMoveSpeed, targetPupilX);
    else if (pupilX > targetPupilX)
      pupilX = max(pupilX - eyeMoveSpeed, targetPupilX);
    if (pupilY < targetPupilY)
      pupilY = min(pupilY + eyeMoveSpeed, targetPupilY);
    else if (pupilY > targetPupilY)
      pupilY = max(pupilY - eyeMoveSpeed, targetPupilY);

    // Check for Sleep override based on Energy
    if (tirednessFactor > 0.9 && currentEmotion != SLEEP) {
      setEmotion(ANIM_TIRED);
    }

    renderEmotionFrame(display, currentEmotion, currentFrame++);
  }

  if (currentEmotion != NEUTRAL && currentEmotion != SLEEP && !isTalkingNow) {
    if (currentEmotion == WINK && currentFrame >= 6) {
      setEmotion(NEUTRAL);
    }
    // Transition back to NEUTRAL if high-priority emotion expires
    if (millis() >= persistenceEndTime && persistenceEndTime != 0) {
      persistenceEndTime = 0;
      setEmotion(NEUTRAL, false);
    }
  }

  // === SERVO SCANNING & OBSTACLE AVOIDANCE (NON-BLOCKING) ===
  static unsigned long lastScan = 0;
  static int scanAngle = 90;
  static bool scanningForward = true;
  static int scanState = 0; // 0:Idle, 1:WaitStabilize, 2:AvoidanceSequence
  static unsigned long scanTimer = 0;

  if (autoPilotActive && !inhibitAutopilot) {
    // Perform servo scanning every 500ms
    if (now - lastScan > 500) {
      lastScan = now;

      // Sweep servo from 0 to 160 degrees
      if (scanningForward) {
        scanAngle += 20;
        if (scanAngle >= SERVO_MAX_ANGLE) {
          scanAngle = SERVO_MAX_ANGLE;
          scanningForward = false;
        }
      } else {
        scanAngle -= 20;
        if (scanAngle <= SERVO_MIN_ANGLE) {
          scanAngle = SERVO_MIN_ANGLE;
          scanningForward = true;
        }
      }

      setHeadAngle(scanAngle);
      // delay(50); // Removed blocking delay
      scanState = 1;
      scanTimer = now;
      return; // Yield to allow other tasks (display, wifi) to run
    } else if (scanState == 1 && (now - scanTimer > 50)) {
      // Resume after stabilization
      // ... continue logic

      // Check obstacles at current angle
      bool frontClear = isPathClearFront();
      bool irFront = (digitalRead(IR_FRONT) == LOW);
      bool irBack = (digitalRead(IR_BACK) == LOW);

      if (irFront || irBack ||
          (!frontClear && scanAngle > 60 && scanAngle < 120)) {
        // Obstacle Triggered!
        if (irFront) {
          moveRobot("BACK");
          scanTimer = now + 500;
          scanState = 2;
        } else if (irBack) {
          moveRobot("FORWARD");
          scanTimer = now + 400;
          scanState = 2;
        } else {
          moveRobot("STOP");
          // Turn random direction
          if (random(100) < 50)
            moveRobot("LEFT");
          else
            moveRobot("RIGHT");
          scanTimer = now + 600;
          scanState = 2;
        }
      } else {
        // Clear - continue
        scanState = 0;
      }
    } else if (scanState == 2) {
      // AVOIDANCE EXECUTION (Simple timer based)
      if (now > scanTimer) {
        moveRobot("STOP");
        setHeadAngle(90);
        scanState = 0;
      }
    }

    if (isTalkingNow) {
      moveRobot("STOP"); // Stop and listen/talk
    } else {
      // Continue normal navigation if path is clear
      if (isPathClearFront()) {
        Rover.update(); // NASA-style rover navigation
        Rover.checkFailSafe();
      }
    }
  }

  updateSpeedStep();
  handleServer();

  // === NEURAL BRAIN AUTONOMOUS BEHAVIOR ===
  // Only execute if Auto Pilot is active
  if (now - lastNeuralDecision > 5000 && autoPilotActive) { // Every 5 seconds
    lastNeuralDecision = now;

    // Update neural brain inputs
    bool frontDetected = (digitalRead(IR_FRONT) == LOW);
    bool backDetected = (digitalRead(IR_BACK) == LOW);
    int distance = 50; // Placeholder - add actual distance sensor if available
    Brain.updateInputs(frontDetected, backDetected, distance, Life.currentMood,
                       100);

    // Make intelligent decision
    BehaviorDecision decision = Brain.makeDecision();

    // Log decision
    Serial.print("Neural Decision: ");
    Serial.print(Brain.getBehaviorName(decision.type));
    Serial.print(" (Confidence: ");
    Serial.print(decision.confidence * 100);
    Serial.println("%)");

    // NEURAL SMART EMOTIONS: Intelligent emotion changes based on neural
    // decisions Only change emotions if:
    // 1. High confidence (>70%)
    // 2. Not overriding user/AI set emotions (check persistence time)
    // 3. Enough time passed since last neural emotion change (cooldown)

    static unsigned long lastNeuralEmotionChange = 0;
    const unsigned long NEURAL_EMOTION_COOLDOWN = 10000; // 10 seconds cooldown

    // Only change emotion if confident AND not interrupting user/AI emotions
    if (decision.confidence > 0.7 && millis() > persistenceEndTime &&
        (millis() - lastNeuralEmotionChange > NEURAL_EMOTION_COOLDOWN)) {

      // Re-enabled: Intelligent emotion changes based on neural decisions
      Emotion smartEmotion = NEUTRAL;

      // Smart behavior-to-emotion mapping
      switch (decision.type) {
      case BEHAVIOR_ALERT:
        // High alert - show concerned/cautious emotion
        smartEmotion = (Life.currentMood < 50) ? SHOCKED : SKEPTICAL;
        break;

      case BEHAVIOR_EXPLORE:
        // Exploring - show curious/interested emotion
        smartEmotion = ANIM_SCAN;
        break;

      case BEHAVIOR_SOCIAL:
        // Social interaction - show friendly emotion
        smartEmotion = (Life.currentMood > 70) ? LOVE : WINK;
        break;

      case BEHAVIOR_PLAY:
        // Playing - show playful emotion
        smartEmotion = PARTY;
        break;

      case BEHAVIOR_REST:
        // Resting - show Angel if mood is high, else tired/neutral
        if (Life.currentMood > 60) {
          smartEmotion = ANGEL;
        } else {
          smartEmotion = (tirednessFactor > 0.7) ? ANIM_TIRED : NEUTRAL;
        }
        break;

      default:
        smartEmotion = NEUTRAL;
        break;
      }

      // Only change if it's a meaningful emotion (not just staying NEUTRAL)
      if (smartEmotion != NEUTRAL || currentEmotion != NEUTRAL) {
        setEmotion(smartEmotion, false); // Low priority - user can override
        lastNeuralEmotionChange = millis();
        Serial.print("Smart Emotion: ");
        Serial.println(smartEmotion);
      }
    }

    // Initiate smart behavior based on decision
    if (!isTalkingNow && decision.confidence > 0.5) {
      SmartMove.executeBehavior(decision);
    }
  }

  // --- Process Non-blocking Behaviors & Actions ---
  if (!isTalkingNow) {
    SmartMove.update();
  }

  // --- Process Action Queue ---
  processQueue();

  // 5-Hour Periodic API Key Check
  if (now - lastApiCheck > apiCheckInterval) {
    lastApiCheck = now;
    logLoc("System: 5-hour timer reached. Re-checking In-Build API keys...");
    globalConfig.restoreHardcoded(true);
    globalConfig.restoreHardcoded(false);
  }
}