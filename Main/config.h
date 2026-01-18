#ifndef CONFIG_H
#define CONFIG_H

// Power Source: 2x 18650 Li-ion (7700mAh)
// Driver: L298N Dual Bridge

extern const char *ssid;
extern const char *password;

// IPS/OLED Display (SPI 7-Pin Mode)
#define OLED_MOSI 23 // Connect to SDA/MOSI
#define OLED_CLK 18  // Connect to SCL/SCK
#define OLED_DC 2    // Data/Command pin
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
#define SERVO_PIN 13 // Head/Sensor Pan-Tilt Servo
#define TRIG_PIN 22  // Ultrasonic Trigger
#define ECHO_PIN 21  // Ultrasonic Echo

// Infrared Edge/Side Sensors (IR Proximity)
// These detect if someone touches the robot's side or if it's near a table
// edge.
#define IR_LEFT 34  // Connect to OUT of Left IR Module
#define IR_RIGHT 35 // Connect to OUT of Right IR Module

// INMP441 I2S Digital Microphone
#define I2S_SCK 5
#define I2S_WS 19
#define I2S_SD 12

extern bool autoPilotActive;

#endif