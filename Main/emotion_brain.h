#ifndef EMOTION_BRAIN_H
#define EMOTION_BRAIN_H

#include "bitmaps.h"
#include "config.h"
#include "neural_brain.h"
#include <Arduino.h>

extern void setEmotion(Emotion e, bool isHighPriority);
extern Emotion currentEmotion;

class EmotionBrain {
private:
  Emotion lastEmotion = NEUTRAL;
  unsigned long lastEmotionChange = 0;

public:
  // Predict next emotion based on behavior
  Emotion predictEmotionForBehavior(BehaviorType behavior, float mood) {
    Emotion predicted = NEUTRAL;

    switch (behavior) {
    case BEHAVIOR_EXPLORE:
      predicted = (mood > 60) ? SKEPTICAL : NEUTRAL;
      break;

    case BEHAVIOR_SOCIAL:
      predicted = (mood > 70) ? LOVE : (mood > 50 ? WINK : NEUTRAL);
      break;

    case BEHAVIOR_REST:
      predicted = SLEEP;
      break;

    case BEHAVIOR_ALERT:
      predicted = (mood < 40) ? ANGRY_RAGE : SHOCKED;
      break;

    case BEHAVIOR_PLAY:
      predicted = (mood > 80) ? PARTY : LAUGH;
      break;
    }

    return predicted;
  }

  // Context-aware emotion selection
  Emotion reactToContext(bool personDetected, bool obstacle, float mood,
                         float energy) {
    // Priority-based emotion selection

    // High priority: Danger
    if (obstacle && energy > 20) {
      return FEAR;
    }

    // Medium priority: Low battery
    if (energy < 20) {
      return HUNGRY;
    }

    // Medium priority: Social interaction
    if (personDetected) {
      if (mood > 70)
        return LOVE;
      if (mood > 50)
        return WINK;
      if (mood < 30)
        return SKEPTICAL;
    }

    // Low priority: Mood-based
    if (mood > 80)
      return random(100) < 50 ? PARTY : LAUGH;
    if (mood < 20)
      return SAD;

    return NEUTRAL;
  }

  // Smooth emotion transition
  void transitionEmotion(Emotion newEmotion, bool force = false) {
    unsigned long now = millis();

    // Prevent rapid emotion changes (unless forced)
    if (!force && (now - lastEmotionChange < 2000)) {
      return;
    }

    // Set emotion with appropriate priority
    bool isHighPriority = (newEmotion == FEAR || newEmotion == HUNGRY ||
                           newEmotion == ANGRY_RAGE);

    setEmotion(newEmotion, isHighPriority);
    lastEmotion = newEmotion;
    lastEmotionChange = now;
  }

  // Get emotion intensity based on context
  float getEmotionIntensity(Emotion emo, float mood) {
    float intensity = 0.5; // Default 50%

    switch (emo) {
    case LOVE:
    case PARTY:
      intensity = mood / 100.0;
      break;
    case ANGRY_RAGE:
    case FEAR:
      intensity = (100 - mood) / 100.0;
      break;
    case SLEEP:
      intensity = 1.0; // Always full intensity
      break;
    default:
      intensity = 0.5;
    }

    return constrain(intensity, 0.3, 1.0);
  }

  // Select emotion based on context and mood - STRICKLY DETERMINISTIC (No
  // Internet)
  Emotion selectContextualEmotion(String context, float mood) {
    // Context-based emotion pool
    if (context == "OBSTACLE" || context == "CAUTION") {
      return (mood < 40) ? ANGRY_RAGE : SHOCKED;
    } else if (context == "CURIOUS" || context == "SCAN") {
      return SKEPTICAL;
    } else if (context == "SOCIAL" || context == "FOLLOW") {
      if (mood > 80) {
        // User Request: 10% probability for LOVE
        if (random(100) < 10)
          return LOVE;
        return WINK;
      }
      if (mood > 50)
        return WINK; // Wink is for friendly social
      return NEUTRAL;
    } else if (context == "HISS" || context == "SCARED") {
      return ANGRY_RAGE;
    } else if (context == "PLAY") {
      if (mood > 70)
        return PARTY; // Playing when happy
      return LAUGH;   // Playing when neutral
    }

    // Default mood-based fallback (if no context) - No randomness
    if (mood > 80)
      return WINK;
    if (mood < 30)
      return SAD;

    return NEUTRAL;
  }

  // Weighted autonomous emotion selector (Revised: Deterministic Logic)
  Emotion selectWeightedAutonomousEmotion(String context, float mood) {
    // Every emotion must have a REASON.
    // Logic: If mood is extremely high/low, reflect that. Otherwise stay
    // NEUTRAL.

    if (context != "") {
      return selectContextualEmotion(context, mood);
    }

    // Passive logic (when no active behavior context is provided)
    if (mood > 95) {
      if (random(100) < 10)
        return LOVE;
      return NEUTRAL;
    }
    if (mood < 10)
      return CRYING; // Extreme Sadness
    if (mood < 25)
      return SAD; // General Sadness

    return NEUTRAL; // Default to Normal (😐)
  }
};

#endif
