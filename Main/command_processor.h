#ifndef COMMAND_PROCESSOR_H
#define COMMAND_PROCESSOR_H

#include "bitmaps.h"
#include "hardware_ctrl.h"
#include "logger.h"
#include <Arduino.h>

extern int robotSpeed;
extern void setEmotion(Emotion e);
extern void setHeadAngle(int angle);
extern bool autoPilotActive;
extern bool inhibitAutopilot; // Global flag
extern void stopTTS();        // Wrapper function to stop TTS

class CommandProcessor {
public:
  CommandProcessor() {}

  // Main command detection and execution function
  bool detectAndExecute(String msg) {
    msg.toLowerCase();
    msg.trim();

    // Movement Commands (10)
    if (containsAny(msg, {"আগে", "forward", "go forward"})) {
      moveRobot("FORWARD");
      logLoc("CMD: Forward");
      return true;
    }
    if (containsAny(msg, {"পিছে", "back", "go back", "reverse"})) {
      moveRobot("BACK");
      logLoc("CMD: Back");
      return true;
    }
    if (containsAny(msg, {"বামে", "left", "turn left"})) {
      moveRobot("LEFT");
      logLoc("CMD: Left");
      return true;
    }
    if (containsAny(msg, {"ডানে", "right", "turn right"})) {
      moveRobot("RIGHT");
      logLoc("CMD: Right");
      return true;
    }
    if (containsAny(msg, {"থামো", "থাম", "stop", "halt"})) {
      moveRobot("STOP");
      logLoc("CMD: Stop");
      return true;
    }
    if (containsAny(msg, {"ঘুরো", "spin", "rotate"})) {
      moveRobot("LEFT");
      delay(500);
      moveRobot("STOP");
      logLoc("CMD: Spin");
      return true;
    }
    if (containsAny(msg, {"ব্রেক", "brake", "emergency"})) {
      moveRobot("STOP");
      logLoc("CMD: Emergency Brake");
      return true;
    }

    // Emotion Commands (16)
    if (containsAny(msg, {"নিউট্রাল", "neutral", "normal face"})) {
      setEmotion(NEUTRAL);
      logLoc("CMD: Emotion - Neutral");
      return true;
    }
    if (containsAny(msg, {"ভালোবাসা", "love", "heart"})) {
      setEmotion(LOVE);
      logLoc("CMD: Emotion - Love");
      return true;
    }
    if (containsAny(msg, {"হাসো", "হাস", "laugh", "smile", "happy"})) {
      setEmotion(LAUGH);
      logLoc("CMD: Emotion - Laugh");
      return true;
    }
    if (containsAny(msg, {"ঘুমাও", "ঘুম", "sleep", "sleepy"})) {
      setEmotion(SLEEP);
      logLoc("CMD: Emotion - Sleep");
      return true;
    }
    if (containsAny(msg, {"চোখ মারো", "wink"})) {
      setEmotion(WINK);
      logLoc("CMD: Emotion - Wink");
      return true;
    }
    if (containsAny(msg, {"দুষ্টু", "evil", "mischievous"})) {
      setEmotion(MISCHIEVOUS);
      logLoc("CMD: Emotion - Evil");
      return true;
    }
    if (containsAny(msg, {"রাগ", "rage", "angry"})) {
      setEmotion(ANGRY_RAGE);
      logLoc("CMD: Emotion - Rage");
      return true;
    }
    if (containsAny(msg, {"শক", "shocked", "surprised"})) {
      setEmotion(SHOCKED);
      logLoc("CMD: Emotion - Shocked");
      return true;
    }
    if (containsAny(msg, {"দুঃখ", "দুখ", "sad"})) {
      setEmotion(SAD);
      logLoc("CMD: Emotion - Sad");
      return true;
    }
    if (containsAny(msg, {"মৃত", "dead"})) {
      setEmotion(DEAD);
      logLoc("CMD: Emotion - Dead");
      return true;
    }
    if (containsAny(msg, {"ঝিমঝিম", "dizzy", "confused"})) {
      setEmotion(DIZZY);
      logLoc("CMD: Emotion - Dizzy");
      return true;
    }
    if (containsAny(msg, {"পার্টি", "party", "celebrate"})) {
      setEmotion(PARTY);
      logLoc("CMD: Emotion - Party");
      return true;
    }
    if (containsAny(msg, {"সন্দেহ", "skeptical", "doubt"})) {
      setEmotion(SKEPTICAL);
      logLoc("CMD: Emotion - Skeptical");
      return true;
    }
    if (containsAny(msg, {"হতাশ", "frustrated"})) {
      setEmotion(FRUSTRATED);
      logLoc("CMD: Emotion - Frustrated");
      return true;
    }
    if (containsAny(msg, {"ফেরেশতা", "angel", "innocent"})) {
      setEmotion(ANGEL);
      logLoc("CMD: Emotion - Angel");
      return true;
    }
    if (containsAny(msg, {"কান্না", "crying", "cry"})) {
      setEmotion(CRYING);
      logLoc("CMD: Emotion - Crying");
      return true;
    }

    // Head Movement Commands (8)
    if (containsAny(msg, {"বামে দেখো", "look left"})) {
      setHeadAngle(135);
      logLoc("CMD: Look Left");
      return true;
    }
    if (containsAny(msg, {"ডানে দেখো", "look right"})) {
      setHeadAngle(45);
      logLoc("CMD: Look Right");
      return true;
    }
    if (containsAny(msg, {"সামনে দেখো", "look center", "look forward"})) {
      setHeadAngle(90);
      logLoc("CMD: Look Center");
      return true;
    }
    if (containsAny(msg, {"উপরে দেখো", "look up"})) {
      setHeadAngle(60);
      logLoc("CMD: Look Up");
      return true;
    }
    if (containsAny(msg, {"নিচে দেখো", "look down"})) {
      setHeadAngle(120);
      logLoc("CMD: Look Down");
      return true;
    }
    if (containsAny(msg, {"হ্যাঁ বলো", "nod yes", "nod"})) {
      nodYes();
      logLoc("CMD: Nod Yes");
      return true;
    }
    if (containsAny(msg, {"না বলো", "shake no", "shake head"})) {
      shakeNo();
      logLoc("CMD: Shake No");
      return true;
    }
    if (containsAny(msg, {"চারপাশে দেখো", "scan", "look around"})) {
      scanAround();
      logLoc("CMD: Scan Around");
      return true;
    }

    // Speed Control Commands (6)
    if (containsAny(msg, {"ধীরে", "slow", "speed slow"})) {
      robotSpeed = 100;
      logLoc("CMD: Speed Slow (100)");
      return true;
    }
    if (containsAny(msg, {"মাঝারি", "medium", "normal speed"})) {
      robotSpeed = 180;
      logLoc("CMD: Speed Medium (180)");
      return true;
    }
    if (containsAny(msg, {"দ্রুত", "fast", "speed fast"})) {
      robotSpeed = 230;
      logLoc("CMD: Speed Fast (230)");
      return true;
    }
    if (containsAny(msg, {"টার্বো", "turbo", "maximum"})) {
      robotSpeed = 255;
      logLoc("CMD: Speed Turbo (255)");
      return true;
    }

    // System Commands (10)
    if (containsAny(msg, {"অটো মোড", "auto mode", "autopilot"})) {
      moveRobot("AUTO");
      logLoc("CMD: Auto Mode Toggle");
      return true;
    }
    if (containsAny(msg, {"জেগে ওঠো", "wake up", "wake"})) {
      setEmotion(WAKE_UP);
      logLoc("CMD: Wake Up");
      return true;
    }
    if (containsAny(msg, {"স্লিপ মোড", "sleep mode"})) {
      setEmotion(SLEEP);
      moveRobot("STOP");
      logLoc("CMD: Sleep Mode");
      return true;
    }

    // Autopilot Mode Commands (5)
    if (containsAny(msg, {"অটো চালু", "auto on", "enable auto"})) {
      if (!autoPilotActive) {
        moveRobot("AUTO");
      }
      logLoc("CMD: Auto ON");
      return true;
    }
    if (containsAny(msg, {"অটো বন্ধ", "auto off", "disable auto"})) {
      if (autoPilotActive) {
        moveRobot("AUTO");
      }
      logLoc("CMD: Auto OFF");
      return true;
    }

    // Audio/TTS Commands (5)
    if (containsAny(msg,
                    {"চুপ করো", "চুপ", "stop talking", "shut up", "quiet"})) {
      stopTTS();
      logLoc("CMD: Stop Talking");
      return true;
    }

    // Special Action Commands (5+)
    if (containsAny(msg, {"নাচো", "নাচ", "dance"})) {
      performDance();
      logLoc("CMD: Dance");
      return true;
    }
    if (containsAny(msg, {"সেলিব্রেট", "celebrate", "victory"})) {
      setEmotion(PARTY);
      performCelebration();
      logLoc("CMD: Celebrate");
      return true;
    }
    if (containsAny(msg, {"হ্যালো বলো", "greet", "say hello"})) {
      performGreeting();
      logLoc("CMD: Greet");
      return true;
    }
    if (containsAny(msg, {"বিদায়", "goodbye", "bye"})) {
      performGoodbye();
      logLoc("CMD: Goodbye");
      return true;
    }
    if (containsAny(msg, {"সেলফি পোজ", "selfie", "photo pose"})) {
      performSelfiePose();
      logLoc("CMD: Selfie Pose");
      return true;
    }
    if (containsAny(msg, {"চিন্তা করো", "think", "thinking"})) {
      performThinking();
      logLoc("CMD: Thinking");
      return true;
    }

    // No command detected
    return false;
  }

private:
  // Helper function to check if message contains any of the keywords
  bool containsAny(String msg, std::initializer_list<const char *> keywords) {
    for (const char *keyword : keywords) {
      if (msg.indexOf(keyword) != -1) {
        return true;
      }
    }
    return false;
  }

  // Head movement actions
  void nodYes() {
    inhibitAutopilot = true;
    setEmotion(LOVE);
    // Double-bounce nod
    setHeadAngle(100);
    delay(150);
    setHeadAngle(80);
    delay(150);
    setHeadAngle(110);
    delay(250);
    setHeadAngle(70);
    delay(250);
    setHeadAngle(90);
    inhibitAutopilot = false;
  }

  void shakeNo() {
    inhibitAutopilot = true;
    setEmotion(SAD);
    for (int i = 0; i < 3; i++) {
      setHeadAngle(140);
      delay(120);
      setHeadAngle(40);
      delay(120);
      // Slight motor jitter to show disagreement
      if (i % 2 == 0) {
        startPulseBody(true, 30, 1);
      } else {
        startPulseBody(false, 30, 1);
      }
    }
    setHeadAngle(90);
    inhibitAutopilot = false;
  }

  void scanAround() {
    setHeadAngle(45);
    delay(500);
    setHeadAngle(90);
    delay(300);
    setHeadAngle(135);
    delay(500);
    setHeadAngle(90);
  }

  // Special actions
  void performDance() {
    setEmotion(PARTY);
    // Shuffle
    for (int i = 0; i < 3; i++) {
      moveRobot("LEFT");
      delay(200);
      moveRobot("RIGHT");
      delay(200);
    }
    // Full spin
    moveRobot("LEFT");
    delay(1500);
    moveRobot("STOP");
    // Victory pulse
    startPulseBody(true, 100, 2);
    setEmotion(NEUTRAL);
  }

  void performCelebration() {
    setEmotion(PARTY);
    // Hopping effect
    for (int i = 0; i < 3; i++) {
      startPulseBody(true, 50, 1);
      delay(100);
      startPulseBody(false, 50, 1);
      delay(100);
    }
    moveRobot("STOP");
    for (int i = 0; i < 4; i++) {
      setHeadAngle(60);
      delay(100);
      setHeadAngle(120);
      delay(100);
    }
    setHeadAngle(90);
  }

  void performGreeting() {
    setEmotion(LOVE);
    setHeadAngle(70);
    delay(300);
    setEmotion(WINK);
    delay(500);
    nodYes();
    setEmotion(NEUTRAL);
  }

  void performGoodbye() {
    setEmotion(SAD);
    setHeadAngle(45);
    delay(800);
    setHeadAngle(90);
    setEmotion(WINK);
    delay(600);
    setEmotion(NEUTRAL);
  }

  void performSelfiePose() {
    setEmotion(LOVE);
    setHeadAngle(120);
    delay(1500);
    setEmotion(LAUGH);
    setHeadAngle(60);
    delay(1500);
    setEmotion(WINK);
    setHeadAngle(90);
    delay(1500);
    setEmotion(NEUTRAL);
  }

  void performThinking() {
    setEmotion(SKEPTICAL);
    setHeadAngle(60);
    for (int i = 0; i < 3; i++) {
      startPulseBody(true, 30, 1);
      delay(500);
      startPulseBody(false, 30, 1);
      delay(500);
    }
    setHeadAngle(90);
    setEmotion(NEUTRAL);
  }
};

// Global instance
CommandProcessor cmdProcessor;

#endif
