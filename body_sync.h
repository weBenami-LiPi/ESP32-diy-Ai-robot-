#ifndef BODY_SYNC_H
#define BODY_SYNC_H

#include "config.h"
#include "hardware_ctrl.h"
#include <Arduino.h>

extern unsigned long lastInteractionTime;
extern Emotion currentEmotion;
extern int targetPupilX;

class BodySyncManager {
public:
  bool isSpeaking = false;
  unsigned long lastBodyPulse = 0;
  unsigned long lastHeadNod = 0;

  void setSpeaking(bool state) {
    isSpeaking = state;
    if (!state) {
      moveRobot("STOP");
      setHeadAngle(90);
    }
  }

  void update() {
    unsigned long now = millis();

    // Side Sensor Sync (IR)
    static unsigned long lastIR = 0;
    static unsigned long leftHitStart = 0;
    static unsigned long rightHitStart = 0;

    if (now - lastIR > 100) {
      lastIR = now;
      bool left = (digitalRead(IR_LEFT) == LOW);
      bool right = (digitalRead(IR_RIGHT) == LOW);

      // Manage continuous hit timers
      if (left) {
        if (leftHitStart == 0)
          leftHitStart = now;
      } else {
        leftHitStart = 0;
      }
      if (right) {
        if (rightHitStart == 0)
          rightHitStart = now;
      } else {
        rightHitStart = 0;
      }

      // Only trigger if NO emotion is currently active (allow manual/AI emotion
      // to persist) AND not speaking
      if (!isSpeaking && currentEmotion == NEUTRAL) {
        bool leftStuck = (leftHitStart > 0 && (now - leftHitStart > 5000));
        bool rightStuck = (rightHitStart > 0 && (now - rightHitStart > 5000));

        if (left && right && !leftStuck && !rightStuck) {
          setEmotion(SHOCKED);
          targetPupilX = 0;
        } else if (left && !leftStuck) {
          targetPupilX = -12;
          lastInteractionTime = now; // Linger
        } else if (right && !rightStuck) {
          targetPupilX = 12;
          lastInteractionTime = now; // Linger
        } else {
          // No hit or sensor timed out, return to center quickly
          if (now - lastInteractionTime > 500)
            targetPupilX = 0;
        }
      }
    }

    if (!isSpeaking)
      return;

    long dist = readDistance();
    if (dist > 0 && dist < 25) {
      moveRobot("STOP");
    } else {
      if (now - lastBodyPulse > random(300, 1500)) {
        lastBodyPulse = now;
        startPulseBody();
      }
    }
    if (now - lastHeadNod > random(500, 1200)) {
      lastHeadNod = now;
      int baseAngle = 90;
      if (targetPupilX < -2)
        baseAngle = 135;
      else if (targetPupilX > 2)
        baseAngle = 45;
      else
        baseAngle = 90;
      int nodOffset = random(-15, 15);
      setHeadAngle(baseAngle + nodOffset);
    }
  }
};

BodySyncManager BodySync;

#endif
