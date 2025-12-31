#ifndef CONFIG_H
#define CONFIG_H

const char *ssid = "Glyph";
const char *password = "Mahdi&981";

#define OLED_MOSI 23
#define OLED_CLK 18
#define OLED_DC 16
#define OLED_CS 5
#define OLED_RESET 17

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define MOTOR_A1 27
#define MOTOR_A2 26
#define MOTOR_B1 25
#define MOTOR_B2 33
#define SERVO_PIN 13
#define TRIG_PIN 4
#define ECHO_PIN 34
#define IR_LEFT 35
#define IR_RIGHT 32

#define I2S_MIC_SERIAL_CLOCK 14
#define I2S_MIC_LEFT_RIGHT_CLOCK 15
#define I2S_MIC_SERIAL_DATA 22
#define SAMPLE_RATE_ASR 16000
#define SAMPLE_RATE_TTS 44100

#define I2S_SPEAKER_BCLK 19
#define I2S_SPEAKER_LRC 21
#define I2S_SPEAKER_DIN 2

extern bool autoPilotMode;

#define I2S_PORT_MIC I2S_NUM_0
#define I2S_PORT_SPK I2S_NUM_1

#endif