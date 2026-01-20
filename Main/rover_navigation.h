#ifndef ROVER_NAVIGATION_H
#define ROVER_NAVIGATION_H

#include "config.h"
#include "emotion_reason.h"
#include "hardware_ctrl.h"
#include <Arduino.h>


// ============ ROVER STATE MACHINE ============
// Inspired by NASA Mars Rover navigation logic

enum RoverState {
  ROVER_FORWARD,   // Moving forward, scanning for obstacles
  ROVER_STOPPED,   // Obstacle or cliff detected
  ROVER_SCANNING,  // Radar scanning left/right
  ROVER_DECIDING,  // Analyzing which direction is best
  ROVER_AVOIDING,  // Executing avoidance maneuver
  ROVER_CLIFF_BACK // Cliff detected, backing up
};

// ============ TRAVERSABILITY INDEX ============
// Higher = safer to traverse, Lower = dangerous
struct TraversabilityData {
  int leftDistance;
  int rightDistance;
  int frontDistance;
  bool leftCliff;
  bool rightCliff;
  float traversabilityIndex; // 0.0 (dangerous) to 1.0 (safe)
};

// ============ ROVER NAVIGATION CLASS ============
class RoverNavigation {
private:
  RoverState currentState = ROVER_FORWARD;
  TraversabilityData terrain;

  // Non-blocking timing
  unsigned long lastStateChange = 0;
  unsigned long scanStartTime = 0;
  unsigned long avoidStartTime = 0;

  // Sensor fusion: Median filter for ultrasonic
  int distanceBuffer[5] = {100, 100, 100, 100, 100};
  int bufferIndex = 0;

  // Thresholds
  const int OBSTACLE_THRESHOLD = 25; // cm
  const int CLIFF_DETECTED_HIGH = 1; // IR returns HIGH when no ground
  const int SCAN_DURATION = 400;     // ms per scan direction
  const int AVOID_DURATION = 600;    // ms for turn duration
  const int BACKUP_DURATION = 500;   // ms for backup

  // Decision tracking
  int bestDirection = 0; // -1 = left, 0 = forward, 1 = right

  // ============ SENSOR FUSION ============
  int getFilteredDistance() {
    // Read raw distance
    int raw = readDistance();

    // Add to circular buffer
    distanceBuffer[bufferIndex] = raw;
    bufferIndex = (bufferIndex + 1) % 5;

    // Sort and return median
    int sorted[5];
    memcpy(sorted, distanceBuffer, sizeof(sorted));
    for (int i = 0; i < 4; i++) {
      for (int j = i + 1; j < 5; j++) {
        if (sorted[i] > sorted[j]) {
          int temp = sorted[i];
          sorted[i] = sorted[j];
          sorted[j] = temp;
        }
      }
    }
    return sorted[2]; // Median
  }

  // ============ CLIFF DETECTION ============
  bool isCliffDetected() {
    bool frontCliff = (digitalRead(IR_FRONT) == HIGH);
    bool backCliff = (digitalRead(IR_BACK) == HIGH);
    terrain.leftCliff = frontCliff;
    terrain.rightCliff = backCliff;
    return frontCliff || backCliff;
  }

  // ============ TRAVERSABILITY CALCULATION ============
  void calculateTraversability() {
    terrain.frontDistance = getFilteredDistance();

    // Normalize distance to 0-1 range (0-100cm)
    float distScore = constrain(terrain.frontDistance / 100.0, 0.0, 1.0);

    // Cliff penalty
    float cliffPenalty = (terrain.leftCliff || terrain.rightCliff) ? 0.5 : 0.0;

    // Calculate traversability index
    terrain.traversabilityIndex = distScore - cliffPenalty;
    terrain.traversabilityIndex =
        constrain(terrain.traversabilityIndex, 0.0, 1.0);
  }

  // ============ STATE HANDLERS ============
  void handleForward() {
    calculateTraversability();

    // Hazard Detection (VETO logic from NASA rovers)
    if (terrain.traversabilityIndex < 0.25 ||
        terrain.frontDistance < OBSTACLE_THRESHOLD) {
      moveRobot("STOP");
      currentState = ROVER_STOPPED;
      lastStateChange = millis();

      // Emotional Reactivity: Ultra-Close Obstacle -> FEAR, Moderate -> SHOCKED
      if (terrain.frontDistance < 12) {
        Reason.addStimulus(STIM_CLIFF, "Dangerous obstacle detected!", 30.0);
        setEmotion(FEAR, true);
        Serial.println("[ROVER] DANGER! Very close obstacle.");
      } else {
        Reason.addStimulus(STIM_COLLISION, "Obstacle in path", 15.0);
        setEmotion(SHOCKED, true);
        Serial.println("[ROVER] Obstacle detected! Stopping.");
      }
      return;
    }

    // Cliff Detection (highest priority)
    if (isCliffDetected()) {
      moveRobot("STOP");
      currentState = ROVER_CLIFF_BACK;
      lastStateChange = millis();
      Reason.addStimulus(STIM_CLIFF, "Emergency: Cliff detected!", 50.0);
      setEmotion(FEAR, true);
      Serial.println("[ROVER] CLIFF! Emergency backup.");
      return;
    }

    // All clear, move forward
    moveRobot("FORWARD");
  }

  void handleStopped() {
    // Transition to scanning after brief pause
    if (millis() - lastStateChange > 200) {
      currentState = ROVER_SCANNING;
      scanStartTime = millis();
      setHeadAngle(45); // Look right first
      Serial.println("[ROVER] Scanning right...");
    }
  }

  void handleScanning() {
    unsigned long elapsed = millis() - scanStartTime;

    if (elapsed < SCAN_DURATION) {
      // Looking right
      terrain.rightDistance = getFilteredDistance();
    } else if (elapsed < SCAN_DURATION * 2) {
      // Look left
      if (elapsed < SCAN_DURATION + 50) {
        setHeadAngle(135);
        Serial.println("[ROVER] Scanning left...");
      }
      terrain.leftDistance = getFilteredDistance();
    } else {
      // Return to center and decide
      setHeadAngle(90);
      currentState = ROVER_DECIDING;
      lastStateChange = millis();
    }
  }

  void handleDeciding() {
    if (millis() - lastStateChange < 100)
      return; // Brief pause

    Serial.print("[ROVER] Left: ");
    Serial.print(terrain.leftDistance);
    Serial.print("cm, Right: ");
    Serial.print(terrain.rightDistance);
    Serial.println("cm");

    // Choose direction with more space
    if (terrain.leftDistance > terrain.rightDistance + 10) {
      bestDirection = -1; // Turn left
      Serial.println("[ROVER] Decision: Turn LEFT");
    } else if (terrain.rightDistance > terrain.leftDistance + 10) {
      bestDirection = 1; // Turn right
      Serial.println("[ROVER] Decision: Turn RIGHT");
    } else {
      // Similar distances, pick random
      bestDirection = (random(100) < 50) ? -1 : 1;
      Serial.println("[ROVER] Decision: Random turn");
    }

    currentState = ROVER_AVOIDING;
    avoidStartTime = millis();
  }

  void handleAvoiding() {
    if (millis() - avoidStartTime < AVOID_DURATION) {
      // Execute turn
      if (bestDirection < 0) {
        moveRobot("LEFT");
      } else {
        moveRobot("RIGHT");
      }
    } else {
      // Done avoiding
      moveRobot("STOP");
      currentState = ROVER_FORWARD;
      lastStateChange = millis();
      Serial.println("[ROVER] Avoidance complete, resuming forward.");
    }
  }

  void handleCliffBackup() {
    if (millis() - lastStateChange < BACKUP_DURATION) {
      moveRobot("BACK");
    } else {
      moveRobot("STOP");
      currentState = ROVER_SCANNING;
      scanStartTime = millis();
      setHeadAngle(45);
      Serial.println("[ROVER] Backed up, now scanning.");
    }
  }

public:
  void begin() {
    currentState = ROVER_FORWARD;
    lastStateChange = millis();
    Serial.println("[ROVER] Navigation system initialized.");
  }

  RoverState getState() { return currentState; }

  String getStateName() {
    switch (currentState) {
    case ROVER_FORWARD:
      return "FORWARD";
    case ROVER_STOPPED:
      return "STOPPED";
    case ROVER_SCANNING:
      return "SCANNING";
    case ROVER_DECIDING:
      return "DECIDING";
    case ROVER_AVOIDING:
      return "AVOIDING";
    case ROVER_CLIFF_BACK:
      return "CLIFF_BACK";
    default:
      return "UNKNOWN";
    }
  }

  TraversabilityData getTerrainData() { return terrain; }

  // ============ MAIN UPDATE LOOP ============
  // Call this in loop() instead of autoPilotLoop() for rover mode
  void update() {
    // Activity-based emotion mapping
    static unsigned long lastActivityEmotion = 0;
    if (millis() - lastActivityEmotion > 3000) { // Every 3 seconds
      lastActivityEmotion = millis();

      switch (currentState) {
      case ROVER_FORWARD:
        // Moving forward - stay neutral or show curiosity
        if (random(100) < 20) {
          setEmotion(ANIM_SCAN, false); // Curious scanning look
        }
        break;
      case ROVER_SCANNING:
        // Scanning - show curious/skeptical look
        setEmotion(SKEPTICAL, false);
        break;
      case ROVER_STOPPED:
      case ROVER_DECIDING:
        // Thinking - show processing
        setEmotion(ANIM_LOADING, false);
        break;
      case ROVER_CLIFF_BACK:
        // Danger! - show fear/shock
        setEmotion(FEAR, false);
        break;
      case ROVER_AVOIDING:
        // Avoiding - show alert
        setEmotion(SHOCKED, false);
        break;
      }
    }

    switch (currentState) {
    case ROVER_FORWARD:
      handleForward();
      break;
    case ROVER_STOPPED:
      handleStopped();
      break;
    case ROVER_SCANNING:
      handleScanning();
      break;
    case ROVER_DECIDING:
      handleDeciding();
      break;
    case ROVER_AVOIDING:
      handleAvoiding();
      break;
    case ROVER_CLIFF_BACK:
      handleCliffBackup();
      break;
    }
  }

  // ============ FAIL-SAFE ============
  // If ultrasonic hasn't updated in 5 seconds, stop
  unsigned long lastValidReading = 0;

  void checkFailSafe() {
    if (terrain.frontDistance > 0 && terrain.frontDistance < 400) {
      lastValidReading = millis();
    }

    if (millis() - lastValidReading > 5000) {
      moveRobot("STOP");
      currentState = ROVER_STOPPED;
      Serial.println("[ROVER] FAIL-SAFE: Sensor timeout!");
    }
  }
};

// Global instance
RoverNavigation Rover;

#endif
