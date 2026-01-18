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

// Multi-Pulse State
static unsigned long pulseEndTime = 0;
static unsigned long pulseGapEndTime = 0;
static bool isPulsing = false;
static int remainingPulses = 0;
static bool currentPulseDir = true;
static int currentPulseDuration = 40;

inline void updateHead() {
  int targetAngle = 90;
  if (targetPupilX < -2)
    targetAngle = 135;
  else if (targetPupilX > 2)
    targetAngle = 45;
  else
    targetAngle = 90;
  if (currentHeadAngle != targetAngle) {
    headServo.write(targetAngle);
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

  headServo.attach(SERVO_PIN);
  headServo.write(90);

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
    targetRampSpeed = robotSpeed;
    Serial.println("Manual Drive: FORWARD");
  } else if (dir == "BACK") {
    digitalWrite(MOTOR_IN1, LOW);
    digitalWrite(MOTOR_IN2, HIGH);
    digitalWrite(MOTOR_IN3, LOW);
    digitalWrite(MOTOR_IN4, HIGH);
    targetRampSpeed = robotSpeed;
    Serial.println("Manual Drive: BACK");
  } else if (dir == "LEFT") {
    digitalWrite(MOTOR_IN1, LOW);
    digitalWrite(MOTOR_IN2, HIGH);
    digitalWrite(MOTOR_IN3, HIGH);
    digitalWrite(MOTOR_IN4, LOW);
    targetRampSpeed = robotSpeed;
    Serial.println("Manual Drive: LEFT");
  } else if (dir == "RIGHT") {
    digitalWrite(MOTOR_IN1, HIGH);
    digitalWrite(MOTOR_IN2, LOW);
    digitalWrite(MOTOR_IN3, LOW);
    digitalWrite(MOTOR_IN4, HIGH);
    targetRampSpeed = robotSpeed;
    Serial.println("Manual Drive: RIGHT");
  } else if (dir == "STOP") {
    digitalWrite(MOTOR_IN1, LOW);
    digitalWrite(MOTOR_IN2, LOW);
    digitalWrite(MOTOR_IN3, LOW);
    digitalWrite(MOTOR_IN4, LOW);
    targetRampSpeed = 0;
    currentRampSpeed = 0;
    Serial.println("Manual Drive: STOP");
  }

  updateHead();
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
  headServo.write(cur);
}

inline void setHeadAngle(int angle) {
  if (angle >= 0 && angle <= 180) {
    headServo.write(angle);
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
      delay(50);
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

#endif