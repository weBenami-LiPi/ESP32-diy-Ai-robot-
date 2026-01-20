#ifndef MOVEMENT_STYLES_H
#define MOVEMENT_STYLES_H

#include <Arduino.h>

// Movement speed levels
enum MovementSpeed {
  SPEED_STEALTH = 0, // Very slow, cautious (50-80)
  SPEED_SLOW = 1,    // Slow, tired (100-150)
  SPEED_NORMAL = 2,  // Normal walking (180-220)
  SPEED_FAST = 3,    // Fast, energetic (230-255)
  SPEED_BURST = 4    // Sudden burst (255)
};

// Movement styles
enum MovementStyle {
  STYLE_CAT_PROWL,  // Slow, cautious, smooth like a hunting cat
  STYLE_CAT_POUNCE, // Fast sudden movement like cat pouncing
  STYLE_ENERGETIC,  // Bouncy, playful, quick movements
  STYLE_TIRED,      // Slow, lazy, minimal effort
  STYLE_CAUTIOUS,   // Careful, hesitant, stop-and-go
  STYLE_PLAYFUL,    // Random speed changes, unpredictable
  STYLE_CONFIDENT,  // Smooth, steady, purposeful
  STYLE_SCARED      // Jerky, fast, erratic
};

// Movement profile structure
struct MovementProfile {
  String name;
  MovementSpeed speed;
  int baseSpeed;         // Base motor speed (0-255)
  int accelRate;         // Acceleration rate (speed increase per step)
  int decelRate;         // Deceleration rate (speed decrease per step)
  int pauseDuration;     // Pause between movements (ms)
  bool smoothTransition; // Use smooth speed transitions
  float speedVariation;  // Random speed variation (0.0-1.0)
};

// Predefined movement profiles
const MovementProfile MOVEMENT_PROFILES[] = {
    // CAT_PROWL - Slow, cautious, smooth
    {
        "Cat Prowl", SPEED_STEALTH,
        70,   // Very slow base speed
        5,    // Slow acceleration
        8,    // Slow deceleration
        500,  // Long pauses to observe
        true, // Smooth transitions
        0.2   // Minimal variation
    },

    // CAT_POUNCE - Fast sudden movement
    {
        "Cat Pounce", SPEED_BURST,
        255,   // Maximum speed
        50,    // Very fast acceleration
        30,    // Fast deceleration
        100,   // Short pause before pounce
        false, // Sudden, not smooth
        0.1    // Minimal variation
    },

    // ENERGETIC - Bouncy, playful
    {
        "Energetic", SPEED_FAST,
        220,  // Fast base speed
        20,   // Quick acceleration
        15,   // Quick deceleration
        200,  // Short pauses
        true, // Smooth transitions
        0.4   // High variation for bounciness
    },

    // TIRED - Slow, lazy
    {
        "Tired", SPEED_SLOW,
        120,  // Slow base speed
        3,    // Very slow acceleration
        5,    // Slow deceleration
        800,  // Long pauses to rest
        true, // Smooth transitions
        0.1   // Minimal variation
    },

    // CAUTIOUS - Careful, hesitant
    {
        "Cautious", SPEED_SLOW,
        100,  // Slow base speed
        8,    // Moderate acceleration
        12,   // Quick deceleration (ready to stop)
        600,  // Frequent pauses to check
        true, // Smooth transitions
        0.3   // Some variation
    },

    // PLAYFUL - Random, unpredictable
    {
        "Playful", SPEED_NORMAL,
        180,  // Normal base speed
        15,   // Quick acceleration
        10,   // Quick deceleration
        300,  // Medium pauses
        true, // Smooth transitions
        0.6   // High variation for unpredictability
    },

    // CONFIDENT - Smooth, steady
    {
        "Confident", SPEED_NORMAL,
        200,  // Steady speed
        10,   // Smooth acceleration
        10,   // Smooth deceleration
        100,  // Minimal pauses
        true, // Very smooth transitions
        0.1   // Minimal variation
    },

    // SCARED - Jerky, fast, erratic
    {
        "Scared", SPEED_FAST,
        240,   // Fast speed
        40,    // Very fast acceleration
        35,    // Very fast deceleration
        50,    // Very short pauses
        false, // Jerky, not smooth
        0.5    // High variation for erratic behavior
    }};

// Get movement profile by style
inline MovementProfile getMovementProfile(MovementStyle style) {
  if (style >= 0 && style < 8) {
    return MOVEMENT_PROFILES[style];
  }
  // Default to confident
  return MOVEMENT_PROFILES[6];
}

// Map emotion to movement style
inline MovementStyle emotionToMovementStyle(Emotion emotion) {
  switch (emotion) {
  case LOVE:
  case ANGEL:
    return STYLE_PLAYFUL;

  case LAUGH:
  case PARTY:
    return STYLE_ENERGETIC;

  case SLEEP:
  case ANIM_TIRED:
    return STYLE_TIRED;

  case SAD:
  case CRYING:
    return STYLE_TIRED;

  case ANGRY_RAGE:
  case FRUSTRATED:
    return STYLE_ENERGETIC;

  case SHOCKED:
  case FEAR:
    return STYLE_SCARED;

  case MISCHIEVOUS:
    return STYLE_CAT_PROWL;

  case SKEPTICAL:
  case ANIM_SQUINT:
    return STYLE_CAUTIOUS;

  case DIZZY:
  case GLITCH:
    return STYLE_PLAYFUL;

  case HUNGRY:
    return STYLE_TIRED;

  case WAKE_UP:
    return STYLE_CAUTIOUS;

  default:
    return STYLE_CONFIDENT;
  }
}

#endif
