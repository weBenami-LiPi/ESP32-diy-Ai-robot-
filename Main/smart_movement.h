#ifndef SMART_MOVEMENT_H
#define SMART_MOVEMENT_H

#include "neural_brain.h"
#include <Arduino.h>

extern void moveRobot(String cmd);
extern void setHeadAngleSmooth(int angle, int speed);
extern int robotSpeed;

class SmartMovement {
private:
  enum MovementState {
    IDLE,
    TURNING,
    MOVING,
    WAITING,
    GESTURE_UP,
    GESTURE_DOWN
  };

  MovementState state = IDLE;
  BehaviorType currentBehavior = BEHAVIOR_REST;
  unsigned long stateTimer = 0;
  unsigned long stateDuration = 0;
  int targetTurnAngle = 90;
  int stepCount = 0;
  int currentDirection = 90; // 0-180, 90 = center
  int currentSpeed = 0;

  // Adaptive speed based on confidence
  void setAdaptiveSpeed(float confidence) {
    currentSpeed = (int)(confidence * robotSpeed);
    currentSpeed = constrain(currentSpeed, 100, 255);
  }

public:
  void begin() {
    currentDirection = 90;
    currentSpeed = robotSpeed;
    state = IDLE;
    currentBehavior = BEHAVIOR_REST;
  }

  // Execute behavior based on neural decision (Initiator)
  void executeBehavior(BehaviorDecision decision) {
    if (currentBehavior == decision.type && state != IDLE)
      return;

    currentBehavior = decision.type;
    setAdaptiveSpeed(decision.confidence);
    state = IDLE; // Trigger new logic
    Serial.print("SmartMove: Switching to ");
    Serial.println(currentBehavior);
  }

  // Main non-blocking update loop called from Main.ino
  void update() {
    unsigned long now = millis();

    // State machine for behaviors
    switch (currentBehavior) {
    case BEHAVIOR_EXPLORE:
      updateExplore(now);
      break;
    case BEHAVIOR_SOCIAL:
      updateApproach(now);
      break;
    case BEHAVIOR_ALERT:
      updateEvade(now);
      break;
    case BEHAVIOR_PLAY:
      updatePlay(now);
      break;
    case BEHAVIOR_REST:
    default:
      updateRest(now);
      break;
    }
  }

private:
  void updateExplore(unsigned long now) {
    if (state == IDLE) {
      if (random(100) < 30) {
        targetTurnAngle = random(60, 121);
        state = TURNING;
        stateTimer = now;
      } else {
        moveRobot("FORWARD");
        state = MOVING;
        stateTimer = now;
        stateDuration = random(1000, 2001);
      }
    } else if (state == TURNING) {
      setHeadAngleSmooth(targetTurnAngle, 2);
      currentDirection = targetTurnAngle;
      state = MOVING;
      stateTimer = now;
      stateDuration = random(1000, 2001);
      moveRobot("FORWARD");
    } else if (state == MOVING) {
      if (now - stateTimer > stateDuration) {
        moveRobot("STOP");
        state = WAITING;
        stateTimer = now;
        stateDuration = 500;
        setHeadAngleSmooth(random(70, 111), 2);
      }
    } else if (state == WAITING) {
      if (now - stateTimer > stateDuration) {
        state = IDLE;
      }
    }
  }

  void updateApproach(unsigned long now) {
    if (state == IDLE) {
      setHeadAngleSmooth(90, 1);
      currentDirection = 90;
      moveRobot("FORWARD");
      state = MOVING;
      stateTimer = now;
      stateDuration = 800;
    } else if (state == MOVING) {
      if (now - stateTimer > stateDuration) {
        moveRobot("STOP");
        setHeadAngleSmooth(random(85, 96), 1);
        state = WAITING;
        stateTimer = now;
        stateDuration = 1000;
      }
    } else if (state == WAITING) {
      if (now - stateTimer > stateDuration)
        state = IDLE;
    }
  }

  void updateEvade(unsigned long now) {
    if (state == IDLE) {
      moveRobot("BACK");
      state = MOVING;
      stateTimer = now;
      stateDuration = 500;
    } else if (state == MOVING) {
      if (now - stateTimer > stateDuration) {
        moveRobot("STOP");
        targetTurnAngle = (random(100) < 50 ? 45 : 135);
        state = TURNING;
        stateTimer = now;
      }
    } else if (state == TURNING) {
      setHeadAngleSmooth(targetTurnAngle, 2);
      moveRobot("FORWARD");
      state = WAITING;
      stateTimer = now;
      stateDuration = 800;
    } else if (state == WAITING) {
      if (now - stateTimer > stateDuration) {
        moveRobot("STOP");
        state = IDLE;
      }
    }
  }

  void updatePlay(unsigned long now) {
    if (state == IDLE) {
      moveRobot("LEFT");
      state = MOVING;
      stateTimer = now;
      stateDuration = 300;
    } else if (state == MOVING) {
      if (now - stateTimer > stateDuration) {
        moveRobot("STOP");
        state = GESTURE_UP;
        stepCount = 0;
        stateTimer = now;
      }
    } else if (state == GESTURE_UP) {
      if (now - stateTimer > 200) {
        setHeadAngleSmooth(100, 2);
        state = GESTURE_DOWN;
        stateTimer = now;
      }
    } else if (state == GESTURE_DOWN) {
      if (now - stateTimer > 200) {
        setHeadAngleSmooth(80, 2);
        stepCount++;
        stateTimer = now;
        if (stepCount >= 3) {
          setHeadAngleSmooth(90, 1);
          state = WAITING;
          stateDuration = 1000;
        } else {
          state = GESTURE_UP;
        }
      }
    } else if (state == WAITING) {
      if (now - stateTimer > stateDuration)
        state = IDLE;
    }
  }

  void updateRest(unsigned long now) {
    if (state == IDLE) {
      moveRobot("STOP");
      setHeadAngleSmooth(90, 1);
      state = WAITING;
      stateTimer = now;
      stateDuration = 2000;
    } else if (state == WAITING) {
      if (now - stateTimer > stateDuration)
        state = IDLE;
    }
  }

public:
  void returnToNeutral() {
    moveRobot("STOP");
    setHeadAngleSmooth(90, 1);
    currentDirection = 90;
    state = IDLE;
    currentBehavior = BEHAVIOR_REST;
  }
};

#endif
