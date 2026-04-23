<div align="center">

# 🍼 E-Nanny: Smart Baby Monitor

**An intelligent IoT baby monitoring system built with STM32F401 + ESP32S3**

[![Language: C](https://img.shields.io/badge/Language-C%20%2F%20C%2B%2B-blue?style=flat-square&logo=c)](https://en.wikipedia.org/wiki/C_(programming_language))
[![Platform: STM32](https://img.shields.io/badge/MCU-STM32F401CCU6-03234B?style=flat-square&logo=stmicroelectronics)](https://www.st.com/en/microcontrollers-microprocessors/stm32f401.html)
[![Platform: ESP32](https://img.shields.io/badge/Gateway-ESP32S3-E7352C?style=flat-square&logo=espressif)](https://www.espressif.com/)
[![Cloud: ThingsBoard](https://img.shields.io/badge/Cloud-ThingsBoard-25AE88?style=flat-square)](https://thingsboard.io/)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow?style=flat-square)](LICENSE)

> A real-time embedded system that **detects baby crying using on-device AI**, monitors environment, soothes the baby automatically with lullabies and cradle rocking — all viewable from a live IoT dashboard.

---

![System Demo](docs/images/demo_banner.png)
<!-- Replace with your actual demo photo/GIF -->

</div>

---

## ✨ Features

- 🤖 **On-Device AI Cry Detection** — TinyML model running on ESP32S3, classifies baby cry in real-time without sending audio to the cloud
- 📷 **Live Camera Feed** — OV2640 camera streams video via ESP32S3 for remote monitoring
- 🌡️ **Environment Sensing** — DHT22 measures temperature & humidity inside the cradle
- 🎵 **Auto Lullaby Playback** — DFPlayer Mini plays soothing music when crying is detected
- 🔄 **Cradle Auto-Rocking** — Servo SG90 gently rocks the cradle in response to baby state
- 📊 **IoT Dashboard** — ThingsBoard dashboard displays real-time sensor data & baby status
- ⚡ **Non-blocking Firmware** — STM32 firmware uses HAL_GetTick() + State Machine (no HAL_Delay)
- 🔗 **Dual-MCU Architecture** — STM32 handles real-time peripherals; ESP32S3 handles AI + networking

---

## 🏗️ System Architecture

```
┌─────────────────────────────────┐         ┌──────────────────────────────────────┐
│     STM32F401CCU6 (Core MCU)    │         │        ESP32S3 (IoT Gateway)         │
│                                 │         │                                      │
│  ┌──────────┐  ┌─────────────┐  │  UART   │  ┌──────────┐   ┌────────────────┐  │
│  │  DHT22   │  │ DFPlayer    │  │◄───────►│  │ AI Engine│   │ Camera Module  │  │
│  │ Temp/Hum │  │ (Lullaby)   │  │         │  │(Cry Detect│   │ (OV2640)       │  │
│  └──────────┘  └─────────────┘  │         │  └──────────┘   └────────────────┘  │
│                                 │         │                                      │
│  ┌──────────┐  ┌─────────────┐  │         │  ┌──────────┐   ┌────────────────┐  │
│  │ Servo    │  │  State      │  │         │  │ Audio    │   │ Cloud Client   │  │
│  │ SG90     │  │  Machine    │  │         │  │ Mic      │   │ (MQTT/WiFi)    │  │
│  │(Rocking) │  │ (Non-block) │  │         │  └──────────┘   └───────┬────────┘  │
│  └──────────┘  └─────────────┘  │         │                         │           │
└─────────────────────────────────┘         └─────────────────────────┼───────────┘
                                                                       │ MQTT
                                                                       ▼
                                                          ┌────────────────────┐
                                                          │   ThingsBoard IoT  │
                                                          │   Cloud Dashboard  │
                                                          │  (Temp/Hum/Status) │
                                                          └────────────────────┘
```

### Communication Protocol (STM32 ↔ ESP32)

| From | To | Protocol | Data |
|------|----|----------|------|
| STM32 | ESP32S3 | UART 115200 baud | Temperature, Humidity (framed packet) |
| ESP32S3 | STM32 | UART 115200 baud | Commands: play music (0x01), rock cradle (0x04) |
| ESP32S3 | ThingsBoard | MQTT over WiFi | Telemetry + Baby Status string |
| ESP32S3 | OV2640 | DVP + I2C | Camera frame capture |

---

## 📦 Hardware Bill of Materials

| Component | Role | Interface | Notes |
|-----------|------|-----------|-------|
| STM32F401CCU6 | Main MCU | — | ARM Cortex-M4, 84 MHz, 256KB Flash |
| Seeed XIAO ESP32S3 Sense | AI + IoT Gateway | UART | Built-in OV2640 camera + INMP441 mic |
| DHT22 (AM2302) | Temp & Humidity Sensor | 1-Wire GPIO | ±0.5°C / ±2% RH accuracy |
| DFPlayer Mini | MP3/WAV Player | UART 9600 baud | MicroSD card, 3.2W amplifier |
| Servo SG90 | Cradle Rocking Motor | PWM (TIM) | 4.8–6V, 180° rotation |

---

## 📁 Project Structure

```
E-Nanny-SmartBabyMonitor/
│
├── 📁 stm32-firmware/              # STM32CubeIDE Project (STM32F401CCU6)
│   └── Core/
│       ├── App/
│       │   ├── ChuongTrinhChinh.c  # Main application loop (non-blocking)
│       │   └── Libraries/
│       │       ├── DHT22/          # DHT22 driver (HAL_GetTick based)
│       │       └── DFPlayer/       # DFPlayer Mini UART driver
│       └── Src/
│           └── main.c              # HAL init + peripheral config (CubeMX generated)
│
├── 📁 esp32-firmware/              # PlatformIO Project (ESP32S3)
│   └── src/
│       ├── main.cpp                # Setup + main loop
│       ├── AiEngine.cpp/h          # TinyML cry detection inference
│       ├── AudioMic.cpp/h          # I2S microphone sampling (INMP441)
│       ├── CameraModule.cpp/h      # OV2640 init + frame capture
│       ├── CloudClient.cpp/h       # WiFi + MQTT (ThingsBoard)
│       └── Stm32Controller.cpp/h   # UART bridge to STM32
│
└── 📁 docs/
    ├── images/                     # Photos, screenshots, demo GIF
    └── hardware/                   # Wiring diagrams
```

---

## 🔧 Getting Started

### Prerequisites

**STM32 Firmware:**
- [STM32CubeIDE](https://www.st.com/en/development-tools/stm32cubeide.html) v1.13+
- ST-Link V2 programmer

**ESP32S3 Firmware:**
- [Visual Studio Code](https://code.visualstudio.com/) + [PlatformIO](https://platformio.org/) extension

### 1. Clone the Repository

```bash
git clone https://github.com/YOUR_USERNAME/E-Nanny-SmartBabyMonitor.git
cd E-Nanny-SmartBabyMonitor
```

### 2. Configure WiFi & ThingsBoard credentials

Edit `esp32-firmware/src/CloudClient.cpp`:

```cpp
const char* ssid     = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
const char* tb_token = "YOUR_THINGSBOARD_DEVICE_TOKEN";
```

> ⚠️ **Never commit real credentials to Git!** Use a `secrets.h` file that is listed in `.gitignore`.

### 3. Flash STM32 Firmware

1. Open `stm32-firmware/` folder in **STM32CubeIDE**
2. Connect ST-Link V2 to the STM32F401CCU6 board
3. `Project → Build All` → `Run → Debug`

### 4. Flash ESP32S3 Firmware

```bash
cd esp32-firmware
pio run --target upload
pio device monitor   # View serial output at 115200 baud
```

---

## 💡 Software Design Highlights

### Non-Blocking STM32 Architecture
The STM32 firmware uses a **State Machine + HAL_GetTick()** pattern — no `HAL_Delay()` blocking calls anywhere. All sensors and actuators are handled concurrently in a cooperative scheduling loop:

```c
void ChuongTrinh_Loop(void) {
    DHT22_Task();      // Non-blocking: reads every 2000ms
    DFPlayer_Task();   // Non-blocking: manages play state
    Servo_Task();      // Non-blocking: PWM rocking pattern
    UART_RxTask();     // Non-blocking: state machine parser
}
```

### UART Framed Protocol (STM32 ↔ ESP32)
Commands use a fixed-format frame to prevent packet loss/corruption:
```
[0xAA] [CMD] [DATA] [CHECKSUM]
```

### On-Device AI — No Cloud Audio
The baby cry classifier runs **entirely on the ESP32S3 edge device** using TinyML. Raw PCM audio from the I2S microphone is windowed, feature-extracted, and fed into a neural network locally — ensuring **< 500ms latency** and **zero audio data leaving the device**.

---

## 📸 Demo & Screenshots

| ThingsBoard Dashboard | Hardware Assembly |
|-----------|----------|
| ![Dashboard](docs/images/dashboard.png) | ![Hardware](docs/images/hardware.jpg) |

> 📹 Demo video: [Watch on YouTube](https://youtube.com/YOUR_VIDEO_LINK)

---

## 🗺️ Roadmap

- [x] DHT22 temperature & humidity sensing
- [x] DFPlayer Mini lullaby auto-playback
- [x] Servo SG90 cradle auto-rocking
- [x] Baby cry detection (TinyML on ESP32S3)
- [x] ThingsBoard MQTT IoT dashboard
- [x] OV2640 live camera feed
- [ ] Mobile push notifications (React Native)
- [ ] OTA firmware update for ESP32S3
- [ ] Improved AI model (larger dataset, better accuracy)

---

## 📄 License

This project is licensed under the MIT License — see [LICENSE](LICENSE) for details.

---

## 👤 Author

**Vinh Gia Vi**
- GitHub: [@YOUR_USERNAME](https://github.com/YOUR_USERNAME)
- Email: voquangvinh.ptit@gmail.com
- University: Posts and Telecommunications Institute of Technology (PTIT)

---

<div align="center">

⭐ If you find this project useful or interesting, a star would mean a lot! ⭐

</div>
