#ifndef HARDWARE_CTRL_H
#define HARDWARE_CTRL_H

#include "config.h"
#include "logger.h"
#include <Arduino.h>
#include <ESP32Servo.h>

extern void setAutoPilotEmotion(String state);
extern int targetPupilX;
extern Servo headServo;
extern int currentHeadAngle;
extern int robotSpeed;
bool autoPilotActive = false;

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

int robotSpeed = 255;

inline void setupHardware() {
  pinMode(MOTOR_A1, OUTPUT);
  pinMode(MOTOR_A2, OUTPUT);
  pinMode(MOTOR_B1, OUTPUT);
  pinMode(MOTOR_B2, OUTPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(IR_LEFT, INPUT_PULLUP);
  pinMode(IR_RIGHT, INPUT_PULLUP);
  headServo.attach(SERVO_PIN);
  headServo.write(90);
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
    analogWrite(MOTOR_A1, robotSpeed);
    analogWrite(MOTOR_A2, 0);
    analogWrite(MOTOR_B1, robotSpeed);
    analogWrite(MOTOR_B2, 0);
  } else if (dir == "BACK") {
    analogWrite(MOTOR_A1, 0);
    analogWrite(MOTOR_A2, robotSpeed);
    analogWrite(MOTOR_B1, 0);
    analogWrite(MOTOR_B2, robotSpeed);
  } else if (dir == "LEFT") {
    analogWrite(MOTOR_A1, 0);
    analogWrite(MOTOR_A2, robotSpeed);
    analogWrite(MOTOR_B1, robotSpeed);
    analogWrite(MOTOR_B2, 0);
  } else if (dir == "RIGHT") {
    analogWrite(MOTOR_A1, robotSpeed);
    analogWrite(MOTOR_A2, 0);
    analogWrite(MOTOR_B1, 0);
    analogWrite(MOTOR_B2, robotSpeed);
  } else {
    analogWrite(MOTOR_A1, 0);
    analogWrite(MOTOR_A2, 0);
    analogWrite(MOTOR_B1, 0);
    analogWrite(MOTOR_B2, 0);
  }
  updateHead();
}

static unsigned long pulseEndTime = 0;
static bool isPulsing = false;

inline void startPulseBody() {
  digitalWrite(MOTOR_A1, 1);
  digitalWrite(MOTOR_B1, 1);
  pulseEndTime = millis() + 15;
  isPulsing = true;
}

inline void updatePulseBody() {
  if (isPulsing && millis() >= pulseEndTime) {
    digitalWrite(MOTOR_A1, 0);
    digitalWrite(MOTOR_B1, 0);
    isPulsing = false;
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

inline void autoPilotLoop() {
  static unsigned long lastCheck = 0;
  if (millis() - lastCheck < 200)
    return;
  lastCheck = millis();

  long dist = readDistance();
  if (dist < 20) {
    moveRobot("BACK");
    delay(200);
    moveRobot("STOP");
  } else {
    moveRobot("FORWARD");
  }
}

#endif