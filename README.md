<div align="center">

# 🦞 AgentHQ

**A multi-agent AI system where three specialized agents collaborate — visualized in real-time on Terminal UI and Physical Hardware**

[![OpenRouter](https://img.shields.io/badge/LLM-OpenRouter-7c3aed?style=flat-square)](https://openrouter.ai)
[![Claude](https://img.shields.io/badge/Model-Claude_Haiku-06b6d4?style=flat-square)](https://anthropic.com)
[![Node.js](https://img.shields.io/badge/Runtime-Node.js-339933?style=flat-square)](https://nodejs.org)
[![ESP32](https://img.shields.io/badge/Hardware-ESP32-FF7043?style=flat-square)](https://www.espressif.com/)
[![License](https://img.shields.io/badge/License-MIT-lightgrey?style=flat-square)](./LICENSE)

*What if you could watch AI agents think, collaborate, and act — in real time?*

</div>

---

## 🌆 What is AgentHQ?

AgentHQ makes the invisible process of AI agents **visible**. Three specialized agents work together to process any task, while you watch them live in a **full-screen terminal dashboard** and/or on a **physical circuit** with moving servos, color-changing LEDs, and LCD displays.

**Two ways to experience it:**
- 💻 **Terminal UI** — Full-screen dashboard in your terminal
- 🎭 **Physical Circuit** — ESP32 with servos, LEDs, and LCD displays

---

## 🦞 The Agents

| Agent | Role | Description |
|-------|------|-------------|
| **Xocas** | Planner | Receives every task, breaks it down, decides who does what |
| **Momo** | Researcher | Enriches Xocas's plan with context, data, and insights |
| **Llados** | Executor | Produces the final output — documents, reports, action plans |

Each agent moves between three states, visible in real time:

| Color | State | Meaning |
|-------|-------|---------|
| 🟡 Yellow | `IDLE` | Resting on sofa, waiting for work |
| 🟢 Green | `WORKING` | At the computer, processing |
| 🔵 Cyan | `COMMUNICATING` | On the phone, talking to another agent |

---

## ⚙️ Architecture

```
User Input → Orchestrator → Xocas (Plan) → Momo (Research) → Llados (Output)
                                ↓
                         State Manager
                        ↙             ↘
               TUI Display        REST API :5001
                                        ↓
                                  ESP32 Hardware
```

---

## 🎬 Demo

<div align="center">
  <video src="https://github.com/user-attachments/assets/6523803f-3093-4b0b-8597-d5d399b60ef5" width="100%" controls>
    Tu navegador no soporta videos de HTML5.
  </video>
</div>

---
## 🚀 Quick Start

```bash
git clone https://github.com/Diego31-10/agenthq.git
cd agenthq
npm install
echo "OPENROUTER_API_KEY=sk-or-v1-..." > .env
node src/main.js
```

**Windows:** Use `.\launch.ps1` to open in a new terminal window.

---

## 🎭 Hardware Setup (ESP32)

> Skip this section if you only want the Terminal UI.

1. Install **Arduino IDE** or **PlatformIO**
2. Add ESP32 board: Board Manager → "esp32 by Espressif Systems"
3. Install libraries: `Adafruit PWM Servo Driver`, `LiquidCrystal_I2C`, `ArduinoJson`
4. Open `hardware/agentcity_esp32/agenthq_esp32/agenthq_esp32.ino`
5. Configure WiFi & API URL (lines 30–35):
   ```cpp
   const char* WIFI_SSID     = "YOUR_SSID";
   const char* WIFI_PASSWORD = "YOUR_PASSWORD";
   const char* API_URL       = "http://YOUR_PC_IP:5001/states";
   ```
6. Select board `ESP32 Dev Module`, choose port, click **Upload**

**Wiring summary** (see `circuits/` for full diagrams):
- PCA9685 → ESP32 I2C (SDA=21, SCL=22)
- 3x SG90 Servos → PCA9685 channels 0, 1, 2
- 3x RGB LEDs → GPIO pins
- 3x LCD 16x2 → I2C addresses 0x27, 0x26, 0x25
- Buzzer → GPIO 2

**Components:** 3x SG90 servos · 3x RGB LEDs · 3x 16x2 LCDs · PCA9685 · Buzzer · ESP32 WROOM-32

---

## 📄 File Output

Llados auto-detects the desired format from your task and saves the file to your Desktop:

| You say | Output |
|---------|--------|
| "...Word document..." | `.docx` |
| "...PDF..." | `.pdf` |
| "...markdown..." | `.md` |
| "...text file..." | `.txt` |
| Generic "...file..." | `.docx` (default) |

---

## 📁 Project Structure

```
agenthq/
├── src/
│   ├── main.js          # Entry point
│   ├── agents.js        # Agent logic (Xocas, Momo, Llados)
│   ├── claudeClient.js  # OpenRouter API client
│   ├── stateManager.js  # Real-time state tracking
│   ├── tui.js           # Terminal UI (blessed)
│   ├── fileWriter.js    # File output (.docx, .pdf, .md, .txt)
│   └── api.js           # REST API (port 5001)
├── hardware/
│   └── agentcity_esp32/agenthq_esp32/agenthq_esp32.ino
├── circuits/            # Pinout diagrams
├── launch.ps1           # Windows launcher
├── .env.example
└── README.md
```

---

## 🛠️ Tech Stack

| Layer | Technology |
|-------|-----------|
| Runtime | Node.js (ES modules) |
| LLM | OpenRouter → `anthropic/claude-haiku-4-5` |
| Terminal UI | blessed |
| REST API | Express |
| File generation | docx, pdfkit |
| Hardware | ESP32 + PCA9685 + ArduinoJson |

---

## 🔮 What's Next

- Web dashboard mirroring TUI and hardware state
- Real tool access for Llados (calendar, email, web search)
- Voice input/output
- Support for more agents and IoT monitoring

---

## 📜 License

MIT — see [LICENSE](./LICENSE)

---

<div align="center">

*Built with 🦞 and ❤️*

**AgentHQ** — *Where AI agents get to work*

</div>
