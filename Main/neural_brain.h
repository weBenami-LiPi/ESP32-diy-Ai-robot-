#ifndef NEURAL_BRAIN_H
#define NEURAL_BRAIN_H

#include <Arduino.h>

// Behavior types that the robot can choose
enum BehaviorType {
  BEHAVIOR_EXPLORE, // Curious wandering
  BEHAVIOR_SOCIAL,  // Seek interaction
  BEHAVIOR_REST,    // Energy conservation
  BEHAVIOR_ALERT,   // Danger response
  BEHAVIOR_PLAY     // Entertainment mode
};

// Decision output structure
struct BehaviorDecision {
  BehaviorType type;
  float confidence; // 0.0 to 1.0
  int priority;     // 0-10
};

class NeuralBrain {
private:
  // Input neurons (sensor data & internal state)
  float sensorFront = 0.0;       // 0.0 = nothing, 1.0 = detected
  float sensorBack = 0.0;        // 0.0 = nothing, 1.0 = touch
  float obstacleProximity = 0.0; // 0.0 = far, 1.0 = very close
  float batteryLevel = 100.0;    // 0-100%

  // Internal state
  float moodLevel = 50.0;      // 0-100 (from Life Engine)
  float energyLevel = 100.0;   // 0-100
  float boredomLevel = 0.0;    // 0-100
  float timeSinceActive = 0.0; // seconds

  // Behavior weights (learned over time)
  float exploreWeight = 0.5;
  float socialWeight = 0.6;
  float restWeight = 0.3;
  float alertWeight = 0.8;
  float playWeight = 0.4;

  // Memory
  unsigned long lastDecisionTime = 0;
  BehaviorType lastBehavior = BEHAVIOR_EXPLORE;
  float lastOutcome = 0.5; // Success rate of last behavior

  // Calculate score for each behavior type
  float calculateBehaviorScore(BehaviorType type) {
    float score = 0.0;

    switch (type) {
    case BEHAVIOR_EXPLORE:
      // Explore when bored and energetic, and no immediate obstacle
      score += boredomLevel * 0.5;
      score += energyLevel * 0.4;
      score -= obstacleProximity * 0.8; // High penalty for close obstacles
      score *= exploreWeight;
      break;

    case BEHAVIOR_SOCIAL:
      // Social when someone is in front or back
      score += sensorFront * 0.6;
      score += sensorBack * 0.4;
      score += moodLevel * 0.3;
      score -= (100 - energyLevel) * 0.2;
      score *= socialWeight;
      break;

    case BEHAVIOR_REST:
      // Rest when energy is low or very bored (nothing to do)
      score += (100 - energyLevel) * 0.7;
      score += (boredomLevel > 80 ? 0.4 : 0.0);
      score *= restWeight;
      break;

    case BEHAVIOR_ALERT:
      // Alert when an obstacle is suddenly close
      score += obstacleProximity * 0.9;
      score += (sensorFront > 0.8 ? 0.3 : 0.0);
      score *= alertWeight;
      break;

    case BEHAVIOR_PLAY:
      // Play when mood and energy are good, and someone is present
      score += moodLevel * 0.5;
      score += energyLevel * 0.3;
      score += sensorFront * 0.4;
      score *= playWeight;
      break;
    }

    return constrain(score, 0.0, 100.0);
  }

  // Normalize all weights to sum to 1.0
  void normalizeWeights() {
    float sum =
        exploreWeight + socialWeight + restWeight + alertWeight + playWeight;
    if (sum > 0) {
      exploreWeight /= sum;
      socialWeight /= sum;
      restWeight /= sum;
      alertWeight /= sum;
      playWeight /= sum;
    }
  }

public:
  void begin() {
    lastDecisionTime = millis();
    normalizeWeights();
  }

  // Update input neurons from sensors
  void updateInputs(bool frontDetected, bool backDetected, int distance,
                    float mood, float battery) {
    sensorFront = frontDetected ? 1.0 : 0.0;
    sensorBack = backDetected ? 1.0 : 0.0;
    obstacleProximity =
        map(distance, 0, 100, 100, 0) / 100.0; // Closer = higher
    moodLevel = mood;
    batteryLevel = battery;

    // Update energy based on battery
    energyLevel = battery;

    // Update boredom based on time
    unsigned long now = millis();
    timeSinceActive = (now - lastDecisionTime) / 1000.0; // Convert to seconds
    boredomLevel =
        constrain(timeSinceActive / 60.0 * 100.0, 0, 100); // Max at 1 min
  }

  // Make intelligent decision
  BehaviorDecision makeDecision() {
    BehaviorDecision decision;
    float maxScore = -1.0;
    BehaviorType bestBehavior = BEHAVIOR_EXPLORE;

    // Calculate scores for all behaviors
    for (int i = 0; i < 5; i++) {
      BehaviorType type = (BehaviorType)i;
      float score = calculateBehaviorScore(type);

      if (score > maxScore) {
        maxScore = score;
        bestBehavior = type;
      }
    }

    decision.type = bestBehavior;
    decision.confidence = maxScore / 100.0;
    decision.priority = (int)(maxScore / 10.0);

    lastBehavior = bestBehavior;
    lastDecisionTime = millis();

    return decision;
  }

  // Simple reinforcement learning
  void provideFeedback(float reward) {
    // Reward: 1.0 = success, 0.0 = failure, 0.5 = neutral
    lastOutcome = reward;

    // Update weight for last behavior
    float adjustment = (reward - 0.5) * 0.1; // ±10% max

    switch (lastBehavior) {
    case BEHAVIOR_EXPLORE:
      exploreWeight += adjustment;
      break;
    case BEHAVIOR_SOCIAL:
      socialWeight += adjustment;
      break;
    case BEHAVIOR_REST:
      restWeight += adjustment;
      break;
    case BEHAVIOR_ALERT:
      alertWeight += adjustment;
      break;
    case BEHAVIOR_PLAY:
      playWeight += adjustment;
      break;
    }

    // Keep weights positive and normalized
    exploreWeight = constrain(exploreWeight, 0.1, 1.0);
    socialWeight = constrain(socialWeight, 0.1, 1.0);
    restWeight = constrain(restWeight, 0.1, 1.0);
    alertWeight = constrain(alertWeight, 0.1, 1.0);
    playWeight = constrain(playWeight, 0.1, 1.0);

    normalizeWeights();
  }

  // Get current behavior weights (for debugging)
  void printWeights() {
    Serial.println("=== Neural Brain Weights ===");
    Serial.print("Explore: ");
    Serial.println(exploreWeight, 3);
    Serial.print("Social: ");
    Serial.println(socialWeight, 3);
    Serial.print("Rest: ");
    Serial.println(restWeight, 3);
    Serial.print("Alert: ");
    Serial.println(alertWeight, 3);
    Serial.print("Play: ");
    Serial.println(playWeight, 3);
  }

  // Get behavior name as string
  String getBehaviorName(BehaviorType type) {
    switch (type) {
    case BEHAVIOR_EXPLORE:
      return "EXPLORE";
    case BEHAVIOR_SOCIAL:
      return "SOCIAL";
    case BEHAVIOR_REST:
      return "REST";
    case BEHAVIOR_ALERT:
      return "ALERT";
    case BEHAVIOR_PLAY:
      return "PLAY";
    default:
      return "UNKNOWN";
    }
  }
};

#endif
