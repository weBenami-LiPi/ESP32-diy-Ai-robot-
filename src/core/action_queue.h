#ifndef ACTION_QUEUE_H
#define ACTION_QUEUE_H

#include "../vision/bitmaps.h" // For Emotion enum
#include <Arduino.h>
#include <vector>

enum ActionType {
  ACTION_PLAY_AUDIO,
  ACTION_SET_EMOTION,
  ACTION_DELAY,
  ACTION_SIMULATE_TALK,
  ACTION_ROBOT_CMD
};

struct RobotAction {
  ActionType type;
  String payload; // Filename for audio, or unused for emotion
  Emotion emotionVal;
  int durationMs; // For delay or estimated duration
};

extern std::vector<RobotAction> globalQueue;

#endif
