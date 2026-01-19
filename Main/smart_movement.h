#ifndef SMART_MOVEMENT_H
#define SMART_MOVEMENT_H

#include "neural_brain.h"
#include <Arduino.h>


extern void moveRobot(String cmd);
extern void setHeadAngleSmooth(int angle, int speed);
extern int robotSpeed;

class SmartMovement {
private:
  int currentDirection = 90; // 0-180, 90 = center
  int currentSpeed = 0;
  unsigned long lastMoveTime = 0;

  // Smooth turning
  void smoothTurn(int targetAngle, int duration) {
    int steps = 10;
    int currentAngle = currentDirection;
    int angleStep = (targetAngle - currentAngle) / steps;

    for (int i = 0; i < steps; i++) {
      currentAngle += angleStep;
      setHeadAngleSmooth(currentAngle, 1);
      delay(duration / steps);
    }
    currentDirection = targetAngle;
  }

  // Adaptive speed based on confidence
  void setAdaptiveSpeed(float confidence) {
    currentSpeed = (int)(confidence * robotSpeed);
    currentSpeed = constrain(currentSpeed, 100, 255);
  }

public:
  void begin() {
    currentDirection = 90;
    currentSpeed = robotSpeed;
  }

  // Execute behavior based on neural decision
  void executeBehavior(BehaviorDecision decision) {
    setAdaptiveSpeed(decision.confidence);

    switch (decision.type) {
    case BEHAVIOR_EXPLORE:
      exploreMode();
      break;

    case BEHAVIOR_SOCIAL:
      approachMode();
      break;

    case BEHAVIOR_REST:
      restMode();
      break;

    case BEHAVIOR_ALERT:
      evadeMode();
      break;

    case BEHAVIOR_PLAY:
      playMode();
      break;
    }
  }

  // Exploration: Random but purposeful wandering
  void exploreMode() {
    Serial.println("🔍 Explore Mode");

    // Random direction change
    if (random(100) < 30) {
      int newDirection = random(60, 121); // 60-120 degrees
      smoothTurn(newDirection, 500);
    }

    // Move forward with pauses to "observe"
    moveRobot("FORWARD");
    delay(random(1000, 2000));
    moveRobot("STOP");

    // Look around curiously
    setHeadAngleSmooth(random(70, 111), 2);
    delay(500);
  }

  // Social: Friendly approach to person
  void approachMode() {
    Serial.println("💕 Social Mode - Approaching");

    // Slow, non-threatening approach
    setAdaptiveSpeed(0.5); // 50% speed

    // Face forward
    smoothTurn(90, 300);

    // Move forward slowly
    moveRobot("FORWARD");
    delay(800);
    moveRobot("STOP");

    // Tilt head slightly (friendly gesture)
    setHeadAngleSmooth(random(85, 96), 1);
  }

  // Rest: Minimal movement, energy conservation
  void restMode() {
    Serial.println("😴 Rest Mode");

    // Stop all movement
    moveRobot("STOP");

    // Lower head position
    setHeadAngleSmooth(90, 1);

    // Minimal activity
    delay(2000);
  }

  // Alert: Quick evasion from danger
  void evadeMode() {
    Serial.println("⚠️ Alert Mode - Evading");

    // Quick backward movement
    setAdaptiveSpeed(0.8); // 80% speed
    moveRobot("BACK");
    delay(500);

    // Turn away
    int escapeAngle = random(100) < 50 ? 45 : 135;
    smoothTurn(escapeAngle, 200);

    // Move to safe distance
    moveRobot("FORWARD");
    delay(800);
    moveRobot("STOP");
  }

  // Play: Entertaining movements
  void playMode() {
    Serial.println("🎮 Play Mode");

    // Playful spin
    moveRobot("LEFT");
    delay(300);
    moveRobot("STOP");

    // Head bobbing
    for (int i = 0; i < 3; i++) {
      setHeadAngleSmooth(100, 2);
      delay(200);
      setHeadAngleSmooth(80, 2);
      delay(200);
    }
    setHeadAngleSmooth(90, 1);
  }

  // Smooth navigation with obstacle avoidance
  void navigateSmooth(int targetX, int targetY) {
    // Calculate direction to target
    // Simplified: just move forward for now
    moveRobot("FORWARD");
    delay(500);
    moveRobot("STOP");
  }

  // Return to neutral position
  void returnToNeutral() {
    moveRobot("STOP");
    setHeadAngleSmooth(90, 1);
    currentDirection = 90;
  }
};

#endif
