#ifndef EMOTION_REASON_H
#define EMOTION_REASON_H

#include "bitmaps.h"
#include <Arduino.h>

enum StimulusType {
  STIM_TOUCHED,   // Being petted
  STIM_VOICE,     // Hearing sound
  STIM_COLLISION, // Bumping into something
  STIM_CLIFF,     // Detecting a cliff
  STIM_CHAT_POS,  // Positive chat interaction
  STIM_CHAT_NEG,  // Negative chat interaction
  STIM_CHAT_SURP  // Surprising chat interaction
};

struct EmotionResonance {
  float joy = 0.0;      // 0-100
  float sorrow = 0.0;   // 0-100
  float fear = 0.0;     // 0-100
  float anger = 0.0;    // 0-100
  float surprise = 0.0; // 0-100
};

class EmotionReasonSystem {
private:
  EmotionResonance resonance;
  String lastReason = "System started";
  unsigned long lastDecayTime = 0;
  const float DECAY_RATE = 0.5; // Points per second

public:
  void begin() { lastDecayTime = millis(); }

  void addStimulus(StimulusType type, String reason, float intensity = 20.0) {
    lastReason = reason;

    switch (type) {
    case STIM_TOUCHED:
      resonance.joy += intensity;
      resonance.anger -= intensity * 0.5;
      break;
    case STIM_CHAT_POS:
      resonance.joy += intensity;
      break;
    case STIM_CHAT_NEG:
      resonance.sorrow += intensity;
      resonance.joy -= intensity * 0.3;
      break;
    case STIM_COLLISION:
      resonance.surprise += intensity;
      resonance.fear += intensity * 0.5;
      break;
    case STIM_CLIFF:
      resonance.fear += intensity * 2.0;
      resonance.surprise += intensity;
      break;
    case STIM_CHAT_SURP:
      resonance.surprise += intensity;
      break;
    case STIM_VOICE:
      resonance.surprise += intensity * 0.2;
      break;
    }

    clampResonance();
    Serial.print("[REASON] ");
    Serial.println(reason);
  }

  void update() {
    unsigned long now = millis();
    float elapsedSeconds = (now - lastDecayTime) / 1000.0;

    if (elapsedSeconds >= 1.0) {
      lastDecayTime = now;
      decayResonance(DECAY_RATE * elapsedSeconds);
    }
  }

  Emotion getBestEmotion() {
    float maxVal = 5.0; // Threshold to trigger anything other than NEUTRAL
    Emotion best = NEUTRAL;

    if (resonance.fear > maxVal && resonance.fear >= resonance.surprise) {
      best = FEAR;
      maxVal = resonance.fear;
    }
    if (resonance.surprise > maxVal && resonance.surprise > maxVal) {
      best = SHOCKED;
      maxVal = resonance.surprise;
    }
    if (resonance.anger > maxVal && resonance.anger > maxVal) {
      best = ANGRY_RAGE;
      maxVal = resonance.anger;
    }
    if (resonance.joy > maxVal && resonance.joy > maxVal) {
      if (resonance.joy > 60)
        best = LOVE;
      else if (resonance.joy > 40)
        best = ANGEL;
      else
        best = LAUGH;
      maxVal = resonance.joy;
    }
    if (resonance.sorrow > maxVal && resonance.sorrow > maxVal) {
      best = SAD;
      maxVal = resonance.sorrow;
    }

    return best;
  }

  String getLastReason() { return lastReason; }

private:
  void clampResonance() {
    resonance.joy = constrain(resonance.joy, 0, 100);
    resonance.sorrow = constrain(resonance.sorrow, 0, 100);
    resonance.fear = constrain(resonance.fear, 0, 100);
    resonance.anger = constrain(resonance.anger, 0, 100);
    resonance.surprise = constrain(resonance.surprise, 0, 100);
  }

  void decayResonance(float amount) {
    resonance.joy -= amount;
    resonance.sorrow -= amount;
    resonance.fear -= amount;
    resonance.anger -= amount;
    resonance.surprise -= amount;
    clampResonance();
  }
};

extern EmotionReasonSystem Reason;

#endif
