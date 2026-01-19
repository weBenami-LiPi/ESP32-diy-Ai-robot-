#ifndef HARDWARE_CTRL_H
#define HARDWARE_CTRL_H

#include "config.h"
#include "logger.h"
#include <Arduino.h>
#include <ESP32Servo.h>

extern void setAutoPilotEmotion(String state);
extern int targetPupilX;
extern Servo headServo;
extern Servo tiltServo;
extern int currentHeadAngle;
extern int robotSpeed;
extern int currentRampSpeed;
extern int targetRampSpeed;
extern bool autoPilotActive;
extern int currentTiltAngle;
extern bool touchEnabled;
extern unsigned long persistenceEndTime; // Added extern for priority check

// Multi-Pulse State
static unsigned long pulseEndTime = 0;
static unsigned long pulseGapEndTime = 0;
static bool isPulsing = false;
static int remainingPulses = 0;
static bool currentPulseDir = true;
static int currentPulseDuration = 40;

// Smart Movement System
#include "movement_styles.h"

// Movement state
static int currentMovementSpeed = 180;
static int targetMovementSpeed = 180;
static MovementStyle currentMovementStyle = STYLE_CONFIDENT;
static unsigned long lastSpeedUpdate = 0;

// Safe servo write with angle limits
inline void safeServoWrite(int angle) {
  if (angle < SERVO_MIN_ANGLE)
    angle = SERVO_MIN_ANGLE;
  if (angle > SERVO_MAX_ANGLE)
    angle = SERVO_MAX_ANGLE;
  headServo.write(angle);
}

inline void updateHead() {
  int targetAngle = 90;
  if (targetPupilX < -2)
    targetAngle = 135;
  else if (targetPupilX > 2)
    targetAngle = 45;
  else
    targetAngle = 90;
  if (currentHeadAngle != targetAngle) {
    safeServoWrite(targetAngle);
    currentHeadAngle = targetAngle;
  }
}

inline void updateSpeedStep() {
  static unsigned long lastStep = 0;
  if (millis() - lastStep < 10)
    return;
  lastStep = millis();

  if (currentRampSpeed < targetRampSpeed) {
    currentRampSpeed = min(currentRampSpeed + 15, targetRampSpeed);
  } else if (currentRampSpeed > targetRampSpeed) {
    currentRampSpeed = max(currentRampSpeed - 25, targetRampSpeed);
  }

  analogWrite(MOTOR_ENA, currentRampSpeed);
  analogWrite(MOTOR_ENB, currentRampSpeed);
}

void setupHardware() {
  pinMode(MOTOR_ENA, OUTPUT);
  pinMode(MOTOR_IN1, OUTPUT);
  pinMode(MOTOR_IN2, OUTPUT);
  pinMode(MOTOR_IN3, OUTPUT);
  pinMode(MOTOR_IN4, OUTPUT);
  pinMode(MOTOR_ENB, OUTPUT);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(IR_FRONT, INPUT);
  pinMode(IR_BACK, INPUT);
  pinMode(ONBOARD_LED, OUTPUT);
  digitalWrite(ONBOARD_LED, LOW);

  headServo.attach(SERVO_PIN);
  safeServoWrite(90);

  // Stop motors initially
  digitalWrite(MOTOR_IN1, LOW);
  digitalWrite(MOTOR_IN2, LOW);
  digitalWrite(MOTOR_IN3, LOW);
  digitalWrite(MOTOR_IN4, LOW);
  analogWrite(MOTOR_ENA, 0);
  analogWrite(MOTOR_ENB, 0);
}

inline void moveRobot(String dir) {
  logLoc("Robot Move: " + dir);
  if (dir == "AUTO") {
    autoPilotActive = !autoPilotActive;
    if (!autoPilotActive)
      moveRobot("STOP");
    return;
  }

  if (dir == "FORWARD") {
    digitalWrite(MOTOR_IN1, HIGH);
    digitalWrite(MOTOR_IN2, LOW);
    digitalWrite(MOTOR_IN3, HIGH);
    digitalWrite(MOTOR_IN4, LOW);
  }

  updateHead();
}

inline void shakeNo() {
  Serial.println("Gesture: Shake NO");
  for (int i = 0; i < 3; i++) {
    safeServoWrite(135); // Left
    delay(150);
    safeServoWrite(45); // Right
    delay(150);
  }
  safeServoWrite(90); // Center
}

inline void nodYes() {
  Serial.println("Gesture: Nod YES");
  // Simulate Nod with quick Body Jolt (Forward/Back)
  digitalWrite(MOTOR_IN1, HIGH);
  digitalWrite(MOTOR_IN2, LOW);
  digitalWrite(MOTOR_IN3, HIGH);
  digitalWrite(MOTOR_IN4, LOW);
  analogWrite(MOTOR_ENA, 100);
  analogWrite(MOTOR_ENB, 100);
  delay(100);

  digitalWrite(MOTOR_IN1, LOW);
  digitalWrite(MOTOR_IN2, HIGH);
  digitalWrite(MOTOR_IN3, LOW);
  digitalWrite(MOTOR_IN4, HIGH);
  delay(100);

  moveRobot("STOP");
}

inline void startPulseBody(bool forward = true, int durationMs = 40,
                           int count = 1) {
  if (count <= 0)
    return;

  currentPulseDir = forward;
  currentPulseDuration = durationMs;
  remainingPulses = count;

  if (forward) {
    digitalWrite(MOTOR_IN1, HIGH);
    digitalWrite(MOTOR_IN2, LOW);
    digitalWrite(MOTOR_IN3, HIGH);
    digitalWrite(MOTOR_IN4, LOW);
  } else {
    digitalWrite(MOTOR_IN1, LOW);
    digitalWrite(MOTOR_IN2, HIGH);
    digitalWrite(MOTOR_IN3, LOW);
    digitalWrite(MOTOR_IN4, HIGH);
  }

  analogWrite(MOTOR_ENA, robotSpeed);
  analogWrite(MOTOR_ENB, robotSpeed);

  pulseEndTime = millis() + durationMs;
  isPulsing = true;
  remainingPulses--;
}

inline void updatePulseBody() {
  unsigned long now = millis();

  if (isPulsing && now >= pulseEndTime) {
    // End of current pulse
    analogWrite(MOTOR_ENA, 0);
    analogWrite(MOTOR_ENB, 0);
    isPulsing = false;

    if (remainingPulses > 0) {
      pulseGapEndTime = now + 60; // 60ms gap between pulses
    }
  }

  if (!isPulsing && remainingPulses > 0 && now >= pulseGapEndTime) {
    // Start next pulse in sequence
    if (currentPulseDir) {
      digitalWrite(MOTOR_IN1, HIGH);
      digitalWrite(MOTOR_IN2, LOW);
      digitalWrite(MOTOR_IN3, HIGH);
      digitalWrite(MOTOR_IN4, LOW);
    } else {
      digitalWrite(MOTOR_IN1, LOW);
      digitalWrite(MOTOR_IN2, HIGH);
      digitalWrite(MOTOR_IN3, LOW);
      digitalWrite(MOTOR_IN4, HIGH);
    }
    analogWrite(MOTOR_ENA, robotSpeed);
    analogWrite(MOTOR_ENB, robotSpeed);

    pulseEndTime = now + currentPulseDuration;
    isPulsing = true;
    remainingPulses--;
  }
}

inline void setHeadTiltSmooth(int angle) {
  static int cur = 90;
  if (angle > cur)
    cur++;
  else if (angle < cur)
    cur--;
  safeServoWrite(cur);
}

inline float easeInOutCubic(float t) {
  return t < 0.5 ? 4 * t * t * t : 1 - pow(-2 * t + 2, 3) / 2;
}

inline void setHeadAngleSmooth(int targetAngle, int speed = 2) {
  static float currentPos = 90;
  static unsigned long lastUpdate = 0;

  if (abs(currentPos - targetAngle) < 1)
    return;

  if (millis() - lastUpdate > 20) {
    lastUpdate = millis();
    float step = (targetAngle > currentPos) ? speed : -speed;
    currentPos += step;
    safeServoWrite((int)currentPos);
    currentHeadAngle = (int)currentPos;
  }
}

inline void setHeadAngle(int angle) {
  if (angle >= SERVO_MIN_ANGLE && angle <= SERVO_MAX_ANGLE) {
    safeServoWrite(angle);
    currentHeadAngle = angle;
  }
}

inline long readDistance() {
  digitalWrite(TRIG_PIN, 0);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, 1);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, 0);
  long d = pulseIn(ECHO_PIN, 1, 15000);
  return (d == 0) ? 999 : d * 0.034 / 2;
}

extern int detectSoundDirection();

inline void autoPilotLoop() {
  static unsigned long lastCheck = 0;
  if (millis() - lastCheck < 300)
    return;
  lastCheck = millis();

  long dist = readDistance();

  if (dist < 10) {
    // Stage 1: SCARED/HISS (Very Close)
    setAutoPilotEmotion("HISS");
    // "Hiss" effect: quick jitter
    for (int i = 0; i < 3; i++) {
      moveRobot("LEFT");
      delay(500);
      moveRobot("RIGHT");
      delay(50);
    }
    moveRobot("BACK");
    delay(600);
    moveRobot("STOP");
  } else if (dist < 25) {
    // Stage 2: CAUTION
    setAutoPilotEmotion("CAUTION");
    moveRobot("BACK");
    delay(400);
    moveRobot("LEFT");
    delay(500);
    moveRobot("STOP");
  } else {
    // IR Sensor Front/Back Avoidance
    bool irF = (digitalRead(IR_FRONT) == LOW);
    bool irB = (digitalRead(IR_BACK) == LOW);

    if (irF) {
      // Front blocked, move back
      moveRobot("BACK");
      delay(600);
      moveRobot("STOP");
    } else if (irB) {
      // Back blocked, move forward
      moveRobot("FORWARD");
      delay(600);
      moveRobot("STOP");
    } else if (dist < 50) {
      // Stage 3: CURIOUS
      setAutoPilotEmotion("CURIOUS");
      moveRobot("STOP");
      // Tilt head to scan
      setHeadAngle(45);
      delay(400);
      setHeadAngle(135);
      delay(400);
      setHeadAngle(90);
    } else {
      // Stage 4: ACTIVE / Normal
      int soundDir = detectSoundDirection();
      if (soundDir == 1) { // Right
        setAutoPilotEmotion("FOLLOW");
        moveRobot("RIGHT");
        delay(300);
        moveRobot("STOP");
      } else if (soundDir == -1) { // Left
        setAutoPilotEmotion("FOLLOW");
        moveRobot("LEFT");
        delay(300);
        moveRobot("STOP");
      } else {
        setAutoPilotEmotion("ACTIVE");
        moveRobot("FORWARD");
      }
    }
  }
}

inline void executeCombo(String name) {
  if (name == "COMBO_GREETING") {
    setEmotion(LOVE);
    setHeadAngle(130);
    delay(200);
    setHeadAngle(50);
    delay(200);
    setHeadAngle(90);
    startPulseBody(true, 80, 2);
  } else if (name == "COMBO_THINKING") {
    setEmotion(ANIM_LOADING);
    setHeadAngle(120);
    delay(500);
    setEmotion(ANIM_SQUINT);
  } else if (name == "COMBO_SCARED_RETREAT") {
    setEmotion(GLITCH);
    moveRobot("BACK");
    delay(600);
    moveRobot("STOP");
    setHeadAngle(160); // Changed from 180 to safe maximum
  } else if (name == "COMBO_BORED") {
    setEmotion(SKEPTICAL);
    setHeadAngle(70);
    delay(400);
    setHeadAngle(110);
    delay(400);
    setHeadAngle(90);
  }
}

inline void checkTouchInteraction() {
  if (!touchEnabled)
    return; // Safety exit if sensors are ghost-triggering

  if (millis() < persistenceEndTime)
    return; // Shield: Don't let sensors override active User/AI commands

  bool front = (digitalRead(IR_FRONT) == LOW);
  bool back = (digitalRead(IR_BACK) == LOW);
  static unsigned long doubleHitStart = 0;
  static bool isStuck = false;
  static bool triggeredOnce = false;

  if (front && back) {
    if (doubleHitStart == 0)
      doubleHitStart = millis();

    unsigned long duration = millis() - doubleHitStart;
    if (duration > 5000) {
      isStuck = true; // Block further triggers until released
      triggeredOnce = false;
    } else if (duration > 2000 && !isStuck && !triggeredOnce) {
      setEmotion(LOVE, true); // High priority trigger once
      triggeredOnce = true;
    }
  } else {
    doubleHitStart = 0;
    isStuck = false;
    triggeredOnce = false;
  }
}

// ============ EMOTION BEHAVIOR SYSTEM ============
#include "emotion_behaviors.h"

// Behavior execution state
static bool isExecutingBehavior = false;
static unsigned long behaviorEndTime = 0;

// Head movement patterns
inline void headScan(int minAngle, int maxAngle, int speed) {
  int currentAngle = currentHeadAngle;
  int step = (maxAngle > minAngle) ? speed : -speed;

  for (int angle = minAngle; angle <= maxAngle; angle += abs(step)) {
    setHeadAngleSmooth(angle, speed);
    delay(30);
  }
}

inline void headShake(int count, int range) {
  int center = 90;
  for (int i = 0; i < count; i++) {
    setHeadAngle(center - range);
    delay(100);
    setHeadAngle(center + range);
    delay(100);
  }
  setHeadAngle(center);
}

inline void headNod(int count, int range) {
  int center = 90;
  for (int i = 0; i < count; i++) {
    setHeadAngle(center + range);
    delay(150);
    setHeadAngle(center - range);
    delay(150);
  }
  setHeadAngle(center);
}

inline void headCircle(int radius) {
  int center = 90;
  int angles[] = {center, center + radius, center, center - radius};
  for (int i = 0; i < 4; i++) {
    setHeadAngle(angles[i]);
    delay(200);
  }
  setHeadAngle(center);
}

inline void headGlitch() {
  // Random jerky movements
  for (int i = 0; i < 5; i++) {
    int randomAngle = random(SERVO_MIN_ANGLE, SERVO_MAX_ANGLE);
    setHeadAngle(randomAngle);
    delay(random(50, 150));
  }
  setHeadAngle(90);
}

// Body movement patterns
inline void bodyShake(int count, int duration) {
  for (int i = 0; i < count; i++) {
    // Quick forward-back shake
    digitalWrite(MOTOR_IN1, HIGH);
    digitalWrite(MOTOR_IN2, LOW);
    digitalWrite(MOTOR_IN3, HIGH);
    digitalWrite(MOTOR_IN4, LOW);
    analogWrite(MOTOR_ENA, robotSpeed);
    analogWrite(MOTOR_ENB, robotSpeed);
    delay(duration);

    digitalWrite(MOTOR_IN1, LOW);
    digitalWrite(MOTOR_IN2, HIGH);
    digitalWrite(MOTOR_IN3, LOW);
    digitalWrite(MOTOR_IN4, HIGH);
    delay(duration);
  }
  moveRobot("STOP");
}

inline void bodySpin(String direction, int duration) {
  if (direction == "left") {
    digitalWrite(MOTOR_IN1, LOW);
    digitalWrite(MOTOR_IN2, HIGH);
    digitalWrite(MOTOR_IN3, HIGH);
    digitalWrite(MOTOR_IN4, LOW);
  } else {
    digitalWrite(MOTOR_IN1, HIGH);
    digitalWrite(MOTOR_IN2, LOW);
    digitalWrite(MOTOR_IN3, LOW);
    digitalWrite(MOTOR_IN4, HIGH);
  }
  analogWrite(MOTOR_ENA, robotSpeed);
  analogWrite(MOTOR_ENB, robotSpeed);
  delay(duration);
  moveRobot("STOP");
}

inline void bodyPulse(int count, bool forward) {
  startPulseBody(forward, 40, count);
}

inline void bodyParty() {
  // Spin right
  bodySpin("right", 500);
  delay(100);
  // Forward pulse
  bodyPulse(2, true);
  delay(100);
  // Spin left
  bodySpin("left", 500);
}

inline void bodyGlitch() {
  // Random jerky movements
  String movements[] = {"FORWARD", "BACK", "LEFT", "RIGHT"};
  for (int i = 0; i < 3; i++) {
    int randomMove = random(0, 4);
    moveRobot(movements[randomMove]);
    delay(random(50, 150));
    moveRobot("STOP");
    delay(50);
  }
}

// Main emotion behavior executor
inline void executeEmotionBehavior(Emotion emotion) {
  if (isTalkingNow)
    return; // Don't interrupt talking

  EmotionBehavior behavior = getBehaviorForEmotion(emotion);

  // Movement sync probability check
  bool shouldDoPhysicalMovement = true;
  if (autoPilotActive) {
    // Autonomous mode: 20% physical movements
    shouldDoPhysicalMovement = (random(100) < 20);
  } else {
    // Normal mode: 60% physical movements
    shouldDoPhysicalMovement = (random(100) < 60);
  }

  // Mark behavior as executing
  isExecutingBehavior = true;
  behaviorEndTime = millis() + behavior.behaviorDuration;

  // Execute head movement pattern
  if (shouldDoPhysicalMovement) {
    if (behavior.headPattern == "scan") {
      headScan(behavior.headAngleMin, behavior.headAngleMax,
               behavior.headSpeed);
    } else if (behavior.headPattern == "shake") {
      int range = (behavior.headAngleMax - behavior.headAngleMin) / 2;
      headShake(3, range);
    } else if (behavior.headPattern == "nod") {
      int range = (behavior.headAngleMax - behavior.headAngleMin) / 2;
      headNod(2, range);
    } else if (behavior.headPattern == "circle") {
      headCircle(15);
    } else if (behavior.headPattern == "glitch") {
      headGlitch();
    } else if (behavior.headPattern == "static") {
      setHeadAngle(behavior.headAngleMin);
    }
  } else {
    // Just set head to center position
    setHeadAngle((behavior.headAngleMin + behavior.headAngleMax) / 2);
  }

  // Execute body movement pattern
  if (shouldDoPhysicalMovement) {
    if (behavior.bodyAction == "pulse") {
      bodyPulse(behavior.pulseCount, true);
    } else if (behavior.bodyAction == "shake") {
      bodyShake(behavior.pulseCount,
                behavior.moveDuration / behavior.pulseCount);
    } else if (behavior.bodyAction == "spin") {
      bodySpin("right", behavior.moveDuration);
    } else if (behavior.bodyAction == "forward") {
      digitalWrite(MOTOR_IN1, HIGH);
      digitalWrite(MOTOR_IN2, LOW);
      digitalWrite(MOTOR_IN3, HIGH);
      digitalWrite(MOTOR_IN4, LOW);
      analogWrite(MOTOR_ENA, robotSpeed);
      analogWrite(MOTOR_ENB, robotSpeed);
      delay(behavior.moveDuration);
      moveRobot("STOP");
    } else if (behavior.bodyAction == "back") {
      digitalWrite(MOTOR_IN1, LOW);
      digitalWrite(MOTOR_IN2, HIGH);
      digitalWrite(MOTOR_IN3, LOW);
      digitalWrite(MOTOR_IN4, HIGH);
      analogWrite(MOTOR_ENA, robotSpeed);
      analogWrite(MOTOR_ENB, robotSpeed);
      delay(behavior.moveDuration);
      moveRobot("STOP");
    } else if (behavior.bodyAction == "party") {
      bodyParty();
    } else if (behavior.bodyAction == "glitch") {
      bodyGlitch();
    }
    // "idle" does nothing
  }

  // Reset behavior state
  if (!behavior.isLooping) {
    isExecutingBehavior = false;
  }
}

// Check if behavior is executing (for BodySync to avoid interference)
inline bool isBehaviorExecuting() {
  if (isExecutingBehavior && millis() < behaviorEndTime) {
    return true;
  }
  isExecutingBehavior = false;
  return false;
}

// ============ SMART MOVEMENT SYSTEM ============

// Update movement speed with smooth transitions
inline void updateSmartMovementSpeed() {
  unsigned long now = millis();
  if (now - lastSpeedUpdate < 20)
    return; // Update every 20ms
  lastSpeedUpdate = now;

  MovementProfile profile = getMovementProfile(currentMovementStyle);

  // Smooth acceleration/deceleration
  if (currentMovementSpeed < targetMovementSpeed) {
    currentMovementSpeed =
        min(currentMovementSpeed + profile.accelRate, targetMovementSpeed);
  } else if (currentMovementSpeed > targetMovementSpeed) {
    currentMovementSpeed =
        max(currentMovementSpeed - profile.decelRate, targetMovementSpeed);
  }

  // Apply speed variation for natural movement
  if (profile.speedVariation > 0) {
    int variation = (int)(profile.speedVariation * 20);
    int randomOffset = random(-variation, variation + 1);
    int finalSpeed = constrain(currentMovementSpeed + randomOffset, 50, 255);
    analogWrite(MOTOR_ENA, finalSpeed);
    analogWrite(MOTOR_ENB, finalSpeed);
  } else {
    analogWrite(MOTOR_ENA, currentMovementSpeed);
    analogWrite(MOTOR_ENB, currentMovementSpeed);
  }
}

// Set movement style
inline void setMovementStyle(MovementStyle style) {
  currentMovementStyle = style;
  MovementProfile profile = getMovementProfile(style);
  targetMovementSpeed = profile.baseSpeed;
}

// Move with style
inline void moveWithStyle(String direction, MovementStyle style) {
  setMovementStyle(style);
  MovementProfile profile = getMovementProfile(style);

  // Set direction
  if (direction == "FORWARD") {
    digitalWrite(MOTOR_IN1, HIGH);
    digitalWrite(MOTOR_IN2, LOW);
    digitalWrite(MOTOR_IN3, HIGH);
    digitalWrite(MOTOR_IN4, LOW);
  } else if (direction == "BACK") {
    digitalWrite(MOTOR_IN1, LOW);
    digitalWrite(MOTOR_IN2, HIGH);
    digitalWrite(MOTOR_IN3, LOW);
    digitalWrite(MOTOR_IN4, HIGH);
  } else if (direction == "LEFT") {
    digitalWrite(MOTOR_IN1, LOW);
    digitalWrite(MOTOR_IN2, HIGH);
    digitalWrite(MOTOR_IN3, HIGH);
    digitalWrite(MOTOR_IN4, LOW);
  } else if (direction == "RIGHT") {
    digitalWrite(MOTOR_IN1, HIGH);
    digitalWrite(MOTOR_IN2, LOW);
    digitalWrite(MOTOR_IN3, LOW);
    digitalWrite(MOTOR_IN4, HIGH);
  }

  // Apply speed with smooth transition
  if (profile.smoothTransition) {
    // Gradual acceleration
    for (int i = 0; i < 10; i++) {
      updateSmartMovementSpeed();
      delay(profile.accelRate * 2);
    }
  } else {
    // Instant speed (for sudden movements like pounce)
    currentMovementSpeed = profile.baseSpeed;
    analogWrite(MOTOR_ENA, currentMovementSpeed);
    analogWrite(MOTOR_ENB, currentMovementSpeed);
  }
}

// Cat-like prowl movement
inline void catProwl(int duration = 2000) {
  setMovementStyle(STYLE_CAT_PROWL);
  MovementProfile profile = getMovementProfile(STYLE_CAT_PROWL);

  unsigned long startTime = millis();
  while (millis() - startTime < duration) {
    // Move forward slowly
    moveWithStyle("FORWARD", STYLE_CAT_PROWL);
    updateSmartMovementSpeed();
    delay(50);

    // Occasional pause to observe
    if (random(100) < 20) {
      moveRobot("STOP");
      delay(profile.pauseDuration);
    }
  }
  moveRobot("STOP");
}

// Cat-like pounce movement
inline void catPounce() {
  // Brief pause before pounce (crouch)
  moveRobot("STOP");
  setHeadAngle(70); // Look down
  delay(300);

  // Sudden fast forward movement
  setMovementStyle(STYLE_CAT_POUNCE);
  digitalWrite(MOTOR_IN1, HIGH);
  digitalWrite(MOTOR_IN2, LOW);
  digitalWrite(MOTOR_IN3, HIGH);
  digitalWrite(MOTOR_IN4, LOW);
  analogWrite(MOTOR_ENA, 255);
  analogWrite(MOTOR_ENB, 255);
  delay(400);

  // Quick stop
  moveRobot("STOP");
  setHeadAngle(90);
}

// Playful bouncy movement
inline void playfulBounce(int count = 3) {
  setMovementStyle(STYLE_PLAYFUL);

  for (int i = 0; i < count; i++) {
    // Quick forward
    moveWithStyle("FORWARD", STYLE_PLAYFUL);
    delay(200);

    // Quick back
    moveWithStyle("BACK", STYLE_PLAYFUL);
    delay(150);

    // Pause
    moveRobot("STOP");
    delay(100);
  }
  moveRobot("STOP");
}

// Tired lazy movement
inline void tiredWalk(int duration = 1500) {
  setMovementStyle(STYLE_TIRED);
  MovementProfile profile = getMovementProfile(STYLE_TIRED);

  unsigned long startTime = millis();
  while (millis() - startTime < duration) {
    moveWithStyle("FORWARD", STYLE_TIRED);
    updateSmartMovementSpeed();
    delay(100);

    // Frequent pauses to rest
    if (random(100) < 30) {
      moveRobot("STOP");
      delay(profile.pauseDuration);
    }
  }
  moveRobot("STOP");
}

// Scared erratic movement
inline void scaredRetreat() {
  setMovementStyle(STYLE_SCARED);

  // Jerky backward movement
  for (int i = 0; i < 5; i++) {
    moveWithStyle("BACK", STYLE_SCARED);
    delay(random(50, 150));
    moveRobot("STOP");
    delay(random(30, 80));
  }

  // Quick spin to escape
  bodySpin("left", 300);
  moveRobot("STOP");
}

// Confident purposeful movement
inline void confidentWalk(int duration = 2000) {
  setMovementStyle(STYLE_CONFIDENT);

  unsigned long startTime = millis();
  while (millis() - startTime < duration) {
    moveWithStyle("FORWARD", STYLE_CONFIDENT);
    updateSmartMovementSpeed();
    delay(50);
  }
  moveRobot("STOP");
}

#endif
