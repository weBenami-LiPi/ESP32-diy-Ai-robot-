# 🤖 Vextor AI (Vex) - Advanced ESP32 Identity Robot

![Vextor Hero](docs/images/vextor_hero.png)

![Vextor Status](https://img.shields.io/badge/Status-Active-brightgreen) ![ESP32](https://img.shields.io/badge/Platform-ESP32-blue) ![Gemini AI](https://img.shields.io/badge/AI-Google%20Gemini-orange) ![WebUI](https://img.shields.io/badge/Interface-Premium%20Web-blueviolet)

> **"Meow! I am Vextor. A curious and expressive AI cat robot created by Master Mahdi."**

Vextor (Vex) is a professional-grade, open-source AI companion powered by the **ESP32**. It leverages **Google Gemini AI** for advanced reasoning, **Web Speech API** for natural voice interaction, and a custom **Procedural Graphics Engine** for lifelike emotional expressions.

---

## 🧠 System Intelligence & Logic

Vextor's brain operates on a sophisticated state machine, balancing autonomous curiosity with real-time user interaction.

```mermaid
---
config:
  theme: dark
  themeVariables:
    primaryColor: '#BB86FC'
    primaryTextColor: '#fff'
    primaryBorderColor: '#03DAC6'
    lineColor: '#03DAC6'
    secondaryColor: '#03DAC6'
    tertiaryColor: '#CF6679'
---
stateDiagram-v2
    direction LR
    
    [*] --> Boot: Power On
    Boot --> WiFiSetup: Initialize System
    WiFiSetup --> Idle: WiFi Connected
    WiFiSetup --> APMode: No Network
    APMode --> Idle: User Config
    
    Idle --> Autopilot: No User (30s)
    Idle --> Listening: Voice/Touch
    
    Autopilot --> Scanning: Look Around
    Scanning --> ObstacleFound: Sensor Alert
    Scanning --> Autopilot: Path Clear
    ObstacleFound --> Reacting: Avoid/Curious
    Reacting --> Autopilot: Done
    Autopilot --> Idle: User Detected
    
    Listening --> Processing: Capture Input
    Processing --> AIThinking: Send to Gemini
    AIThinking --> AIThinking: Cloud Processing
    AIThinking --> ResponseReady: JSON Received
    AIThinking --> ErrorState: API Timeout
    ResponseReady --> Executing: Parse Tags
    Executing --> EmotionUpdate: [EMOTION]
    Executing --> HeadMove: [YES]/[NO]
    Executing --> SpeakOut: TTS Output
    EmotionUpdate --> Executing: Update Eyes
    HeadMove --> Executing: Servo Move
    SpeakOut --> Listening: Continue
    SpeakOut --> Idle: End Chat
    
    ErrorState --> Idle: Fallback
```

---

## 🏗️ Interactive Architecture

The following diagram illustrates how the Premium Web Dashboard, Cloud AI, and ESP32 Hardware layers interact seamlessly.

```mermaid
%%{init: {'theme':'dark', 'themeVariables': { 'primaryColor':'#BB86FC','primaryTextColor':'#fff','primaryBorderColor':'#03DAC6','lineColor':'#03DAC6','secondaryColor':'#3700B3','tertiaryColor':'#CF6679'}}}%%
graph TB
    %% User Interface Layer
    subgraph UI["🌐 User Interface Layer"]
        User([👤 User])
        Browser[🖥️ Web Browser]
        Dashboard[📱 Premium Dashboard]
        VoiceInput[🎤 Voice Input]
    end
    
    %% Network Layer
    subgraph NET["📡 Network Layer"]
        WiFi[WiFi AP/Client]
        WS[⚡ WebSocket]
        HTTP[🌍 HTTP Server]
    end
    
    %% Application Layer
    subgraph APP["⚙️ Application Layer"]
        Router[Request Router]
        APIManager[🔗 Gemini API]
        SessionMgr[💾 Session Mgr]
    end
    
    %% Logic Layer
    subgraph LOGIC["🧠 Business Logic Layer"]
        BehaviorEngine[Behavior Engine]
        EmotionCtrl[😊 Emotion Ctrl]
        ChatMemory[💬 Chat Memory]
        AutopilotAI[🤖 Autopilot]
        TagParser[🏷️ Tag Parser]
    end
    
    %% Hardware Abstraction
    subgraph HAL["🔧 Hardware Abstraction"]
        MotorCtrl[🚗 Motor Ctrl]
        ServoCtrl[↔️ Servo Ctrl]
        DisplayCtrl[📺 Display Ctrl]
        SensorMgr[📡 Sensor Mgr]
    end
    
    %% Physical Hardware
    subgraph HW["⚡ Hardware Layer"]
        Motors[4x DC Motors]
        Servo[Pan-Tilt Servo]
        OLED[128x64 OLED]
        Ultrasonic[HC-SR04]
        IR[2x IR Sensors]
    end
    
    %% Cloud
    Cloud[☁️ Google Gemini AI]
    
    %% Connections with labels
    User -->|Voice/Text| Browser
    Browser -->|WebUI| Dashboard
    Browser -->|Audio| VoiceInput
    Dashboard <-->|Real-time| WS
    VoiceInput -->|Stream| WS
    WS <-->|Network| WiFi
    HTTP <-->|Config| WiFi
    
    WiFi -->|Route| Router
    Router <-->|API Call| APIManager
    Router -->|Sessions| SessionMgr
    
    APIManager <-->|HTTPS| Cloud
    APIManager -->|Response| BehaviorEngine
    SessionMgr -->|Context| BehaviorEngine
    
    BehaviorEngine <-->|State| EmotionCtrl
    BehaviorEngine <-->|History| ChatMemory
    BehaviorEngine <-->|Auto Mode| AutopilotAI
    BehaviorEngine -->|Parse| TagParser
    
    TagParser -->|"EMOTION Tag"| EmotionCtrl
    TagParser -->|"MOVE Tag"| MotorCtrl
    TagParser -->|"HEAD Tag"| ServoCtrl
    
    SensorMgr -->|Distance| AutopilotAI
    SensorMgr -->|Proximity| BehaviorEngine
    
    EmotionCtrl -->|Graphics| DisplayCtrl
    AutopilotAI -->|Navigate| MotorCtrl
    
    MotorCtrl -->|PWM| Motors
    ServoCtrl -->|Signal| Servo
    DisplayCtrl -->|SPI| OLED
    SensorMgr -->|Read| Ultrasonic
    SensorMgr -->|Read| IR
    
    %% Feedback
    DisplayCtrl -.->|Status| WS
    MotorCtrl -.->|State| WS
    SensorMgr -.->|Data| WS
    
    %% Styling
    classDef uiStyle fill:#BB86FC,stroke:#03DAC6,stroke-width:2px,color:#fff
    classDef netStyle fill:#3700B3,stroke:#03DAC6,stroke-width:2px,color:#fff
    classDef appStyle fill:#6200EE,stroke:#03DAC6,stroke-width:2px,color:#fff
    classDef logicStyle fill:#018786,stroke:#03DAC6,stroke-width:2px,color:#fff
    classDef halStyle fill:#CF6679,stroke:#03DAC6,stroke-width:2px,color:#fff
    classDef hwStyle fill:#B00020,stroke:#03DAC6,stroke-width:2px,color:#fff
    classDef cloudStyle fill:#03DAC6,stroke:#BB86FC,stroke-width:3px,color:#000
    
    class User,Browser,Dashboard,VoiceInput uiStyle
    class WiFi,WS,HTTP netStyle
    class Router,APIManager,SessionMgr appStyle
    class BehaviorEngine,EmotionCtrl,ChatMemory,AutopilotAI,TagParser logicStyle
    class MotorCtrl,ServoCtrl,DisplayCtrl,SensorMgr halStyle
    class Motors,Servo,OLED,Ultrasonic,IR hwStyle
    class Cloud cloudStyle
```

---

## 📱 Premium Control Interface

![Vextor UI Mockup](docs/images/vextor_ui.png)

Vextor features a high-performance web dashboard with real-time feedback, system diagnostics, and emotional monitoring.

---

## ⚡ Optimized Hardware Architecture

Vextor's hardware is designed for maximum efficiency and stability. Below are the **optimized pin configurations** categorized by component.

### 📺 1. Visual Interface (OLED/IPS)
Optimized for high-speed SPI communication to ensure fluid 60FPS animations.

| OLED Pin | ESP32 GPIO | Role | Description |
| :--- | :--- | :--- | :--- |
| **D0 (SCL/CLK)** | **GPIO 18** | SPI_SCK | Serial Clock |
| **D1 (SDA/MOSI)**| **GPIO 23** | SPI_MOSI| Master Out Slave In |
| **RES (RESET)** | **GPIO 4** | RESET | Hardware Reset |
| **DC (A0)** | **GPIO 2** | DATA/CMD| Data/Command Toggle |
| **CS** | **GPIO 15** | CHIP_SEL| Chip Select |
| **VCC** | 3.3V | POWER | Integrated Power |
| **GND** | GND | GROUND | Common Ground |

### ⚙️ 2. Locomotion System (4WD + Head)
Features a dual-bridge driver with PWM ramping for **optimized power delivery** and silent movement.

| Component | ESP32 GPIO | Driver Pin | Function |
| :--- | :--- | :--- | :--- |
| **Left Motors (PWM)** | **GPIO 32** | ENA | Speed Control (Left) |
| **Motor L1** | **GPIO 33** | IN1 | Direction Control |
| **Motor L2** | **GPIO 25** | IN2 | Direction Control |
| **Motor R1** | **GPIO 26** | IN3 | Direction Control |
| **Motor R2** | **GPIO 27** | IN4 | Direction Control |
| **Right Motors (PWM)**| **GPIO 14** | ENB | Speed Control (Right) |
| **Head Servo** | **GPIO 13** | Signal | Pan-Tilt Navigation |

### 📡 3. Sensory Array & Interaction
Optimized for zero-latency environmental awareness and human interaction.

| Sensor Type | Pin | GPIO | Optimized Role |
| :--- | :--- | :--- | :--- |
| **Ultrasonic Trig** | TRIG | **GPIO 22** | Distance Pulse |
| **Ultrasonic Echo** | ECHO | **GPIO 21** | Return Timing |
| **Left IR Sensor** | OUT | **GPIO 34** | Proximity Detection |
| **Right IR Sensor** | OUT | **GPIO 35** | Proximity Detection |
| **I2S Mic SCK** | SCK | **GPIO 5** | Digital Audio Clock |
| **I2S Mic WS** | WS | **GPIO 19** | Word Select |
| **I2S Mic SD** | SD | **GPIO 12** | Serial Data Out |

---

## 🎭 Advanced Behavioral Intelligence

### 🧬 Procedural Emotion Engine
Vextor no longer uses static images. His eyes are rendered in real-time using **mathematical primitives**, allowing for:
- **Dynamic Blinking**: Natural eye intervals that adapt to conversation.
- **Micro-Saccades**: Subtle eye movements that simulate biological life.
- **Emotion Morphing**: Smooth transitions between Neutral, Love, Angry, Shocked, and more.

### 🧠 Gemini AI Persona
Powered by the **Google Gemini Pro** model, Vextor possesses:
- **Cat Identity**: A consistent, playful persona that never breaks character.
- **Bilingual Mastery**: Seamlessly interacts in both **Bengali and English**.
- **Context Awareness**: Remembers recent interactions for flowing conversations.

---

## 🛠️ Power Management Strategy
For an **optimized build**, follow this high-performance power routing:

1.  **Dual 18650 Cells**: Provides stable 7.4V - 8.4V power.
2.  **Motor Driver (L298N)**: Connect Battery (+) to 12V terminal.
3.  **ESP32 Power**: Use the L298N's 5V output to the ESP32 `VIN` pin.
4.  **Common Ground**: **CRITICAL** - All GND pins must be connected together to prevent signal noise.

---

## 🚀 Optimized Installation

1.  **Clone & Open**: Open the `Main.ino` in Arduino IDE.
2.  **Install Dependencies**:
    - `Adafruit_SSD1306` & `Adafruit_GFX` (Visuals)
    - `ESP32Servo` (Movement)
    - `ArduinoJson` (Memory)
3.  **Partition Scheme**: Select **Huge APP (3MB No OTA)** in Tools menu.
4.  **Upload & Flash**: Connect ESP32 and hit Upload. Don't forget to upload the `data` folder using the **SPIFFS Upload Tool**.

---

## 🤝 Project Credits
- **Lead Developer**: Master Mahdi
- **AI Architect**: Antigravity Agent
- **Community**: Built with passion and open-source love.

> *"Vextor is not just a robot; he is an optimized companion with a digital soul."* 🚀
