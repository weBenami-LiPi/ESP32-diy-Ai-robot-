#ifndef BODY_SYNC_H
#define BODY_SYNC_H

#include "bitmaps.h"
#include "config.h"
#include "hardware_ctrl.h"
#include <Arduino.h>

extern void setEmotion(Emotion e, bool isHighPriority);

extern unsigned long lastInteractionTime;
extern Emotion currentEmotion;
extern int targetPupilX;
extern unsigned long persistenceEndTime;
extern float tirednessFactor;

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
    // If executing emotion behavior, don't interfere
    if (isBehaviorExecuting()) {
      return;
    }

    unsigned long now = millis();

    // Side Sensor Sync (IR) logic moved to LifeEngine

    if (!isSpeaking)
      return;

    long dist = readDistance();
    if (dist > 0 && dist < 25) {
      moveRobot("STOP");
    } else {
      // Body Pulse Sync
      int pulseInterval = random(300, 1500);
      if (currentEmotion == ANGRY_RAGE || currentEmotion == PARTY)
        pulseInterval = random(150, 600);
      else if (currentEmotion == SAD || currentEmotion == SLEEP)
        pulseInterval = random(2000, 4000);

      if (now - lastBodyPulse > pulseInterval) {
        lastBodyPulse = now;
        startPulseBody();
      }
    }

    // Head Movement Sync
    int headInterval = random(500, 1200);
    if (currentEmotion == ANGRY_RAGE)
      headInterval = random(200, 500);
    else if (currentEmotion == SAD)
      headInterval = random(1500, 3000);

    if (now - lastHeadNod > headInterval) {
      lastHeadNod = now;
      int baseAngle = 90;

      // Emotion-based vertical bias
      int verticalBias = 0;
      if (currentEmotion == SAD || currentEmotion == SLEEP)
        verticalBias = -20;
      else if (currentEmotion == ANGRY_RAGE || currentEmotion == SHOCKED)
        verticalBias = 10;

      if (targetPupilX < -2)
        baseAngle = 135;
      else if (targetPupilX > 2)
        baseAngle = 45;
      else
        baseAngle = 90;

      int nodRange = 15;
      if (currentEmotion == ANGRY_RAGE)
        nodRange = 30;
      else if (currentEmotion == SAD)
        nodRange = 5;

      int nodOffset = random(-nodRange, nodRange + 1);
      setHeadAngleSmooth(baseAngle + nodOffset + verticalBias, 1);
    }
  }
};

BodySyncManager BodySync;

#endif
