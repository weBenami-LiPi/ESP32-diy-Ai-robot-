# 🤖 Vextor AI (Vex) - Advanced ESP32 Identity Robot

![Vextor Status](https://img.shields.io/badge/Status-Active-brightgreen) ![ESP32](https://img.shields.io/badge/Platform-ESP32-blue) ![OpenAI](https://img.shields.io/badge/AI-OpenAI%20GPT-orange) ![ElevenLabs](https://img.shields.io/badge/Voice-ElevenLabs-blueviolet)

> **"I am Vextor. A robot created by Master Mahdi."**

Vextor (Vex) is a highly advanced, open-source AI desktop robot powered by the ESP32. It combines the intelligence of **OpenAI (ChatGPT)**, the realism of **ElevenLabs TTS**, and a unique, dynamic personality to create a truly living companion. 

Unlike standard AI assistants, Vextor has a **core identity**: he is a robot with attitude (inspired by *Madara Uchiha*), he speaks Bengali (and English), and he never breaks character.

---

## ✨ Key Features (A-Z)

### 🧠 **Artificial Intelligence & Persona**
- **Hardcoded Identity Protection**: Vextor follows a strict "Constitution" in his code. He knows his creator (Mahdi) and refuses to identify as an "AI Model."
- **Dynamic Personality**:
  - **Madara Mode**: Shows "massive attitude" with boys and is "charming/flirtatious" with girls.
  - **Concise & Witty**: Responses are short, sharp (1-2 sentences), and free of robotic fluff.
- **Contextual Memory**: Remembers the last 5 conversation cycles for continuous dialogue.

### 🗣️ **Voice & Speech System**
- **Premium Neural Voice**: Uses **ElevenLabs API** for cinema-grade, realistic speech.
- **Smart Web Fallback (Auto-Failover)**: 
  - If the ElevenLabs API limit is reached or keys are missing, Vextor **automatically switches** to the browser's built-in Text-to-Speech (Web Speech API).
  - Ensures the conversation *never* stops.
- **Real-Time Lip Sync**: Mouth animations synchronize perfectly with audio output (non-blocking streaming).

### 🎭 **Dynamic Emotions & Visuals**
- **OLED Expressions**: Features vivid, animated eyes on an SSD1306 display.
- **16+ Unique Emotions**:
  - `NEUTRAL` 😐 (Smoothed, calm gazing)
  - `LOVE` 😍 (Heart eyes pulse)
  - `SAD` 😢 & `CRYING` 😭 (Heavy-lidded eyes, realistic tears)
  - `ANGEL` 😇 (Glowing halo, happy smile)
  - `WINK` 😉, `FRUSTRATED` 😤, `PARTY` 🥳, and more.
- **Visual Polish**:
  - **Auto-Sleep**: Automatically falls asleep (Zzz animation) after 2 minutes of inactivity to save power. Wakes up instantly on interaction.
  - **Hidden Tags**: Automatically strips technical tags like `[LAUGH]` from voice and chat logs for a clean experience.
  - **Frozen Eyes**: Specific emotions (Sad, Crying) have "fixed" gazes for dramatic effect.

### 🎮 **Control & Web Interface**
- **Responsive WebUI**: A beautiful dark-mode interface hosted directly on the ESP32.
- **Features**:
  - **Live Chat**: Text with Vextor and see his responses with emojis.
  - **Manual Control**: Drive the robot (Forward, Backward, Left, Right).
  - **Emotion Board**: Manually trigger any emotion.
  - **Settings**: Configure API Keys, WiFi, and System Prompts on the fly.
  
### ⚙️ **Hardware & Autonomy**
- **Autopilot Mode**: Uses Ultrasonic and IR sensors to navigate and avoid obstacles autonomously.
- **Head Movement**: Servo-controlled head scanning for "studying" objects.
- **Motor Driver**: Dual DC motor control (L298N/L9110S compatible logic).

---

## 🛠️ Hardware Requirements
| Component | Function |
|-----------|----------|
| **ESP32 Dev Board** | The Brain |
| **0.96" OLED Display** | The Face (I2C) |
| **MAX98357A I2S Amp** | Audio Output |
| **INMP441 Microphone** | (Optional) Audio Input |
| **Servo Motor (SG90)** | Head Movement |
| **Motor Driver** | Wheel Control |
| **Ultrasonic Sensor (HC-SR04)** | Obstacle Detection |
| **IR Sensors** | Edge/Line Detection |
| **Speaker** | 4-8 Ohm, 3W |

---

## 🚀 Installation & Setup

### 1. Wiring
Follow the pin definitions in `config.h`:
- **OLED**: SDA (21), SCL (22) *(Check specific board mappings)*
- **I2S Speaker**: BCLK (19), LRC (21), DIN (2)
- **Servo**: Pin 13
- **Motors**: Pins 27, 26, 25, 33

### 2. Firmware Upload
1. Open the project in **Arduino IDE** or **VS Code (PlatformIO)**.
2. Install required libraries:
   - `ArduinoJson`
   - `Adafruit SSD1306` & `GFX`
   - `ESP8266Audio` (for I2S)
3. Select your ESP32 Board and **Upload**.
4. **Upload Filesystem Image (SPIFFS)**: Must output `index.html` and other assets.

### 3. Configuration
1. Connect to the WiFi Access Point: `Vextor` (or watch Serial for IP).
2. Go to the IP address (usually `192.168.4.1` or assigned local IP).
3. Navigate to **Settings** and enter your:
   - **OpenAI API Key**
   - **ElevenLabs API Key** & **Voice ID**
   - **WiFi Credentials**
4. Save & Restart.

---

## 📖 Usage
- **Talk**: Use the WebUI Chat to type to Vextor.
- **Interact**: Poke him, move him, or ask him to "look angry."
- **Watch**: Observe his natural eye movements; he looks around when bored and sleeps when ignored.
- **Failover**: If your paid voice credits run out, just keep talking! The browser voice will take over seamlessly.

---

## 🤖 Persona Guide
Vextor is **NOT** a helper bot. He is a character.
- **Don't ask:** "Write me a poem about flowers."
- **Do ask:** "Hey Vextor, what do you think of this human?"
- **Expect:** Sarcasm, wit, and occasional Bengali slang.

---

## ❤️ Credits
- **Creator**: Mahdi
- **System Architecture**: Antigravity Agent
- **Powered By**: ESP32, OpenAI, ElevenLabs

> *Project built with passion for bringing code to life.* 🚀
