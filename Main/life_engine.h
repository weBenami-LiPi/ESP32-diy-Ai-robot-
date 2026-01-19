#ifndef LIFE_ENGINE_H
#define LIFE_ENGINE_H

#include "bitmaps.h"
#include "config.h"
#include "hardware_ctrl.h"
#include <Arduino.h>

// --- Constants / tuning ---
#define MOOD_DECAY_RATE 0.05  // How fast mood drifts to 50 (points per tick)
#define MOOD_PET_BONUS 1.5    // Points per tick while petting
#define MOOD_HIT_PENALTY 15.0 // Points deducted per hit
#define INTERACTION_TIMEOUT 10000 // Time before going back to IDLE
#define WARY_DURATION 20000       // 20s of "wary" eyes after being hit

enum RobotState {
  STATE_IDLE,
  STATE_INTERACTION, // Chatting / Listening
  STATE_SLEEP,
  STATE_REFLEX // Reacting to physical touch (Hit/Pet)
};

extern void setEmotion(Emotion e, bool isHighPriority);
extern void moveRobot(String cmd);
extern void setHeadAngleSmooth(int angle, int speed);
extern bool isTalkingNow;
extern unsigned long lastInteractionTime;

class LifeEngine {
public:
  float currentMood = 50.0; // 0=Angry, 50=Neutral, 100=Happy
  RobotState currentState = STATE_IDLE;

  // Sensor Tracking
  unsigned long leftHitStart = 0;
  unsigned long rightHitStart = 0;
  int hitCount = 0;
  unsigned long lastHitTime = 0;
  unsigned long lastWaryTime = 0; // Memory of abuse

  unsigned long lastHeartbeat = 0;
  unsigned long lastPresenceCheck = 0;
  unsigned long lastFidget = 0;

  void begin() {
    currentMood = 50.0;
    currentState = STATE_IDLE;
  }

  void update() {
    unsigned long now = millis();

    // 1. Mood Decay
    if (now % 100 == 0) {
      if (currentMood > 50.0)
        currentMood -= MOOD_DECAY_RATE;
      if (currentMood < 50.0)
        currentMood += MOOD_DECAY_RATE;
      if (currentMood > 100.0)
        currentMood = 100.0;
      if (currentMood < 0.0)
        currentMood = 0.0;
    }

    // 2. Heartbeat logic
    handleHeartbeat(now);

    // 3. Sensor Processing & State Transitions
    processPhysicalSensors(now);

    // 4. Presence/Gaze Awareness
    handleSocialAwareness(now);

    // 5. Idle Behavior / Fidgeting
    if (currentState == STATE_IDLE && !isSpeaking()) {
      performIdleBehavior(now);
    }
  }

  void handleHeartbeat(unsigned long now) {
    int pulseInterval = 2000;

    if (currentMood > 80)
      pulseInterval = 800;
    else if (currentMood < 30)
      pulseInterval = 400;
    else
      pulseInterval = 2500 - (currentMood * 15);

    if (now - lastHeartbeat > pulseInterval) {
      lastHeartbeat = now;
      startPulseBody(true, 30, 1);
    }
  }

  void handleSocialAwareness(unsigned long now) {
    if (now - lastPresenceCheck < 200)
      return;
    lastPresenceCheck = now;

    bool front = (digitalRead(IR_FRONT) == LOW);
    bool back = (digitalRead(IR_BACK) == LOW);

    if (front && !back && currentState == STATE_IDLE) {
      setHeadAngleSmooth(90, 1);
      targetPupilX = 0;
    } else if (!front && back && currentState == STATE_IDLE) {
      setHeadAngleSmooth(random(100) < 50 ? 45 : 135, 2);
    }
  }

  void processPhysicalSensors(unsigned long now) {
    bool front = (digitalRead(IR_FRONT) == LOW);
    bool back = (digitalRead(IR_BACK) == LOW);

    if (front || back) {
      lastInteractionTime = now;
      if (currentState == STATE_SLEEP) {
        setEmotion(WAKE_UP, true);
        currentState = STATE_IDLE;
        return;
      }
    }

    if (front) {
      if (leftHitStart == 0)
        leftHitStart = now;
    } else {
      detectTap(leftHitStart, now);
      leftHitStart = 0;
    }

    if (back) {
      if (rightHitStart == 0)
        rightHitStart = now;
    } else {
      detectTap(rightHitStart, now);
      rightHitStart = 0;
    }

    bool isPetting = false;
    if (leftHitStart > 0 && (now - leftHitStart > 800)) {
      modifyMood(MOOD_PET_BONUS);
      isPetting = true;
    }
    if (rightHitStart > 0 && (now - rightHitStart > 800)) {
      modifyMood(MOOD_PET_BONUS);
      isPetting = true;
    }

    if (isPetting) {
      // Only set petting emotion if no high-priority emotion is active
      if (now >= persistenceEndTime || persistenceEndTime == 0) {
        if (currentEmotion != LOVE && currentEmotion != ANGEL) {
          setEmotion(currentMood > 80 ? LOVE : ANGEL, true);
        }
      }
      currentState = STATE_REFLEX;
    } else if (currentState == STATE_REFLEX && !front && !back) {
      currentState = STATE_IDLE;
      // Only return to neutral if persistence has expired
      if (now >= persistenceEndTime || persistenceEndTime == 0) {
        setEmotion(NEUTRAL, false);
      }
    }
  }

  void detectTap(unsigned long startT, unsigned long now) {
    if (startT == 0)
      return;
    unsigned long duration = now - startT;

    if (duration < 500) {
      if (now - lastHitTime < 1000) {
        hitCount++;
      } else {
        hitCount = 1;
      }
      lastHitTime = now;

      if (hitCount >= 2) {
        modifyMood(-MOOD_HIT_PENALTY);
        setEmotion(ANGRY_RAGE, true);
        moveRobot("BACK");
        lastWaryTime = now; // Mark as wary
        delay(200);
        moveRobot("STOP");
        currentState = STATE_REFLEX;
      } else {
        setEmotion(SHOCKED, true);
      }
    }
  }

  void modifyMood(float delta) {
    currentMood += delta;
    if (currentMood > 100.0)
      currentMood = 100.0;
    if (currentMood < 0.0)
      currentMood = 0.0;
  }

  void performIdleBehavior(unsigned long now) {
    // --- WARY EYE MEMORY ---
    if (now - lastWaryTime < WARY_DURATION) {
      // Persist wary (squinted) eyes
      targetEyeHeight = 15; // Semi-narrowed
    }

    // --- FIDGETING ---
    if (now - lastFidget > random(3000, 8000)) {
      lastFidget = now;

      if (currentMood > 80) {
        setHeadAngleSmooth(90 + random(-10, 11), 3);
      } else if (currentMood < 30) {
        setHeadAngleSmooth(90, 1);
        if (random(100) < 30)
          targetEyeHeight = 10;
      } else if (now - lastWaryTime >= WARY_DURATION) {
        if (random(100) < 15) {
          setHeadAngleSmooth(random(70, 111), 1);
        }
      }
    }

    static unsigned long lastExpress = 0;
    if (now - lastExpress > 5000) {
      lastExpress = now;
      if (currentMood > 80 && random(100) < 30) {
        setEmotion(WINK, false);
      } else if (currentMood < 30 && random(100) < 40) {
        setEmotion(SKEPTICAL, false);
      }
    }
  }

  bool isSpeaking() { return isTalkingNow; }
};

extern LifeEngine Life;

#endif
