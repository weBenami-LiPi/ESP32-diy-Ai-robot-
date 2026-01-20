#ifndef CONFIG_H
#define CONFIG_H

// Power Source: 2x 18650 Li-ion (7700mAh)
// Driver: L298N Dual Bridge

extern const char *ssid;
extern const char *password;

// IPS/OLED Display (SPI 7-Pin Mode)
#define OLED_MOSI 23 // Connect to SDA/MOSI
#define OLED_CLK 18  // Connect to SCL/SCK
#define OLED_DC 5    // Data/Command pin (MOVED from 2 to 5)
#define OLED_CS 15   // Chip Select
#define OLED_RESET 4 // Reset pin

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// L298N Motor Driver (4WD Control)
#define MOTOR_ENA 32 // PWM Speed for Left Motors
#define MOTOR_IN1 33 // Direction pin
#define MOTOR_IN2 25 // Direction pin
#define MOTOR_IN3 26 // Direction pin
#define MOTOR_IN4 27 // Direction pin
#define MOTOR_ENB 14 // PWM Speed for Right Motors

// Servo & Distance Sensor
#define SERVO_PIN 13      // Head/Sensor Pan-Tilt Servo
#define SERVO_MIN_ANGLE 0 // Minimum safe angle for SG90
#define SERVO_MAX_ANGLE                                                        \
  160 // Maximum safe angle (reduced from 180 to prevent damage)

// Front Ultrasonic Sensor (HC-SR04)
#define TRIG_PIN_FRONT 22 // Trigger pin for front sensor
#define ECHO_PIN_FRONT 21 // Echo pin for front sensor

// Back Ultrasonic Sensor (HC-SR04)
#define TRIG_PIN_BACK 12 // Trigger pin for back sensor
#define ECHO_PIN_BACK 2  // Echo pin for back sensor

// Obstacle Detection Thresholds (in cm)
#define SAFE_DISTANCE_FRONT 15 // Stop if obstacle within 15cm ahead
#define SAFE_DISTANCE_BACK 10  // Avoid backing into obstacles within 10cm

// Infrared Front/Back Sensors (IR Proximity)
// These detect if someone is in front or behind the robot.
#define IR_FRONT 34 // Connect to OUT of Front IR Module
#define IR_BACK 35  // Connect to OUT of Back IR Module

// INMP441 I2S Digital Microphone
#define I2S_SCK 16 // MOVED from 5 to 16 (RX2)
#define I2S_WS 19
#define I2S_SD 17 // MOVED from 12 to 17 (TX2)

extern bool autoPilotActive;

// Autonomous Mood Probabilities (%)
#define AUTONOMOUS_NEUTRAL_WEIGHT 85 // Normal mood - Increased to 85%
#define AUTONOMOUS_LOVE_WEIGHT 5     // Affectionate mood - 5%
#define AUTONOMOUS_OTHER_WEIGHT 10   // Other emotions - 10%

// Dynamic Emotion State
// TODO: Task List
// - [x] Update Web Server Logic
// - [x] Add LED toggle in `web_server_manager.h`
// - [x] Verification
//   - [x] Create walkthrough.mdp
extern float emotionIntensity;          // 0.0 to 100.0
extern unsigned long lastEmotionUpdate; // Timestamp of last decay check
extern int targetPupilX, targetPupilY;
extern int targetEyeHeight;
extern bool isTalkingNow;

#define ONBOARD_LED 2 // Built-in Blue LED on most ESP32 boards

#endif