#ifndef EMOTION_BEHAVIORS_H
#define EMOTION_BEHAVIORS_H

#include "bitmaps.h"
#include <Arduino.h>

// Emotion behavior structure
struct EmotionBehavior {
  String name;
  String emoji;

  // Head movement
  int headAngleMin;
  int headAngleMax;
  int headSpeed;      // 1-5 (slow to fast)
  String headPattern; // "static", "scan", "shake", "nod", "circle"

  // Body movement
  String bodyAction; // "idle", "pulse", "shake", "spin", "forward", "back"
  int pulseCount;
  int moveDuration; // milliseconds

  // Timing
  int behaviorDuration; // Total duration in ms
  bool isLooping;       // Continuous or one-shot
};

// Behavior definitions for all emotions
const EmotionBehavior EMOTION_BEHAVIORS[] = {
    // NEUTRAL - 😐
    {"NEUTRAL", "😐", 85, 95, 1, "scan", "idle", 0, 0, 0, true},
    // LOVE - 😍
    {"LOVE", "😍", 80, 100, 2, "nod", "pulse", 2, 40, 2500, false},
    // LAUGH - 😂
    {"LAUGH", "😂", 85, 95, 5, "shake", "shake", 4, 30, 2500, false},
    // SLEEP - 😴
    {"SLEEP", "😴", 70, 70, 1, "static", "idle", 0, 0, 0, true},
    // WINK - 😉
    {"WINK", "😉", 95, 105, 3, "nod", "pulse", 1, 40, 1000, false},
    // MISCHIEVOUS - 😈
    {"MISCHIEVOUS", "😈", 60, 120, 2, "scan", "spin", 1, 1500, 3000, false},
    // ANGRY_RAGE - 🤬
    {"ANGRY_RAGE", "🤬", 70, 110, 5, "shake", "shake", 6, 50, 3000, false},
    // SHOCKED - 😮
    {"SHOCKED", "😮", 110, 120, 5, "static", "back", 1, 100, 1000, false},
    // SAD - 😢
    {"SAD", "😢", 70, 75, 1, "static", "idle", 0, 0, 0, true},
    // DEAD - 💀
    {"DEAD", "💀", 60, 120, 3, "glitch", "glitch", 1, 100, 0, true},
    // DIZZY - 😵
    {"DIZZY", "😵", 75, 105, 3, "circle", "spin", 1, 2000, 3000, false},
    // PARTY - 🥳
    {"PARTY", "🥳", 45, 135, 4, "scan", "party", 3, 1000, 4000, false},
    // SKEPTICAL - 🤨
    {"SKEPTICAL", "🤨", 95, 100, 2, "static", "back", 1, 50, 2000, false},
    // FRUSTRATED - 😤
    {"FRUSTRATED", "😤", 80, 100, 4, "shake", "shake", 4, 40, 2000, false},
    // ANGEL - 😇
    {"ANGEL", "😇", 95, 105, 2, "nod", "forward", 1, 200, 3000, false},
    // CRYING - 😭
    {"CRYING", "😭", 70, 80, 2, "shake", "shake", 3, 50, 0, true},
    // WAKE_UP - 😲
    {"WAKE_UP", "😲", 60, 120, 4, "scan", "pulse", 2, 60, 2000, false},
    // GLITCH - 🧩
    {"GLITCH", "🧩", 50, 130, 5, "glitch", "glitch", 3, 80, 2000, false},
    // ANIM_SCAN - 🔍
    {"ANIM_SCAN", "🔍", 45, 135, 2, "scan", "spin", 1, 3000, 4000, false},
    // ANIM_LOADING - ⏳
    {"ANIM_LOADING", "⏳", 90, 90, 1, "static", "idle", 0, 0, 0, true},
    // ANIM_SQUINT - 😑
    {"ANIM_SQUINT", "😑", 95, 105, 1, "static", "idle", 0, 0, 0, true},
    // ANIM_TIRED - 🥱
    {"ANIM_TIRED", "🥱", 75, 80, 1, "static", "pulse", 1, 100, 0, true},
    // FEAR - 😱
    {"FEAR", "😱", 110, 130, 5, "shake", "back", 1, 150, 1000, false},
    // HUNGRY - 😫
    {"HUNGRY", "😫", 75, 85, 1, "scan", "forward", 1, 100, 0, true}};

// Get behavior for emotion
inline EmotionBehavior getBehaviorForEmotion(Emotion emo) {
  if (emo >= 0 && emo < 24) {
    return EMOTION_BEHAVIORS[emo];
  }
  // Default to NEUTRAL
  return EMOTION_BEHAVIORS[0];
}

#endif
