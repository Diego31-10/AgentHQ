# 🤖 AgentCity
 ### Physical Visualization of Autonomous AI Agents

![ESP32](https://img.shields.io/badge/ESP32-IoT-blue)
![AI Agents](https://img.shields.io/badge/AI-Multi--Agent-green)
![Python](https://img.shields.io/badge/Python-Backend-yellow)
![Status](https://img.shields.io/badge/Status-Active-brightgreen)
![License](https://img.shields.io/badge/License-MIT-lightgrey)

---

# 🌐 Overview

**AgentCity Lab** is an **AI + IoT project** that physically visualizes how autonomous AI agents collaborate to solve tasks.

Instead of AI agents operating invisibly in software, this system creates a **miniature office environment** where each agent is represented by a **physical avatar** controlled by an **ESP32 microcontroller**.

Agents move between locations in their office depending on what they are doing:

- receiving information
- processing tasks
- communicating results

The result is a **physical simulation of multi-agent systems** where users can **see AI collaboration in real time**.

---

# 🎯 Project Goal

The goal of this project is to make **AI agent workflows visible and intuitive**.

By combining **AI agents, IoT hardware, and physical movement**, the system demonstrates how autonomous systems:

1. receive tasks
2. collaborate
3. process information
4. produce results

All represented physically in a miniature **AI office environment**.

---

# 🧠 Core Concepts

This project demonstrates concepts from multiple technical domains:

| Domain | Concepts |
|------|------|
Artificial Intelligence | Multi-Agent Systems |
IoT | ESP32 device control |
Automation | Task delegation |
Human-Computer Interaction | Voice + messaging interfaces |
Embedded Systems | Real-time hardware control |
System Architecture | Distributed agent workflow |

---

# 🏢 Physical Office Concept

Each AI agent has its own **mini office** containing three work areas.

```

[ Computer ] ----- [ Sofa ] ----- [ Phone ]
0%              50%            100%

WORKING           IDLE       COMMUNICATION

```

| Location | Meaning |
|------|------|
Computer | Agent is working |
Sofa | Agent is idle |
Phone | Agent is communicating |

A **servo motor moves the agent avatar** between these locations.

---

# 🎛 Agent State Model

Every agent operates as an independent **state machine**.

State cycle:

```

IDLE
↓
COMMUNICATION
↓
WORKING
↓
COMMUNICATION
↓
IDLE

```

Meaning:

| State | Description |
|------|------|
IDLE | Waiting for tasks |
COMMUNICATION | Receiving or sending information |
WORKING | Processing or executing tasks |

Each agent executes this cycle **independently** when participating in a workflow.

---

# 💡 Visual Feedback System

Each agent communicates its state using hardware indicators.

| Component | Purpose |
|------|------|
Servo | Moves agent avatar |
RGB LED | Displays current state |
LCD | Shows current task |

### LED Status Colors

| State | Color |
|------|------|
Idle | 🟡 Yellow |
Communication | 🔵 Blue |
Working | 🟢 Green |
Error | 🔴 Red |

---

# ⚙️ System Architecture

```

User Interaction
│
├── Telegram Command
└── Voice Command
│
▼
Speech-to-Text
│
▼
AI Agent System
│
▼
Task Delegation
│
▼
Backend API
│
▼
Agent State JSON
│
▼
ESP32
│
▼
Servos + LCD + RGB LEDs
│
▼
Physical AI Office

```

The backend sends agent state updates to the ESP32, which then controls the hardware.

---

# 🤖 AI Agents

The system contains **three specialized agents**.

---

## 🧠 Planner Agent

Responsibilities:

- receive user tasks
- analyze requests
- create execution plans
- delegate tasks

Examples:

- interpreting user commands
- planning workflows
- coordinating agents

---

## 🔎 Research Agent

Responsibilities:

- gather information
- analyze external data
- prepare research results

Examples:

- reading calendars
- retrieving information
- analyzing schedules

---

## ⚙️ Executor Agent

Responsibilities:

- perform real actions
- execute automation tasks
- return final results

Examples:

- creating meetings
- sending notifications
- triggering automations

---

# 🔄 Multi-Agent Workflow

The agents collaborate sequentially.

```

User
│
▼
Planner
│
▼
Researcher
│
▼
Executor
│
▼
User Response

```

Each agent executes its own **state cycle** during the process.

---

# 📡 Detailed Workflow Example

## Scenario

User command:

```

Organize my meetings tomorrow

```

---

# Step 1 — User Sends Task

System state:

| Agent | State |
|------|------|
Planner | IDLE |
Researcher | IDLE |
Executor | IDLE |

---

# Step 2 — Planner Receives Request

Planner moves to **Phone**.

State:

```

COMMUNICATION

```

Hardware:

- Servo → Phone
- LED → Blue
- LCD → "Receiving task"

---

# Step 3 — Planner Creates Plan

Planner moves to **Computer**.

State:

```

WORKING

```

Hardware:

- Servo → Computer
- LED → Green
- LCD → "Creating plan"

---

# Step 4 — Planner Sends Plan

Planner returns to **Phone**.

State:

```

COMMUNICATION

```

Hardware:

- Servo → Phone
- LED → Blue
- LCD → "Sending plan"

Planner then returns to **IDLE**.

---

# Step 5 — Researcher Receives Plan

Researcher moves to **Phone**.

State:

```

COMMUNICATION

```

Hardware:

- Servo → Phone
- LED → Blue
- LCD → "Receiving plan"

---

# Step 6 — Researcher Performs Analysis

Researcher moves to **Computer**.

State:

```

WORKING

```

Hardware:

- Servo → Computer
- LED → Green
- LCD → "Running research"

---

# Step 7 — Researcher Sends Results

Researcher returns to **Phone**.

State:

```

COMMUNICATION

```

Hardware:

- Servo → Phone
- LED → Blue
- LCD → "Sending research"

Researcher returns to **IDLE**.

---

# Step 8 — Executor Receives Data

Executor moves to **Phone**.

State:

```

COMMUNICATION

```

Hardware:

- Servo → Phone
- LED → Blue
- LCD → "Receiving data"

---

# Step 9 — Executor Executes Task

Executor moves to **Computer**.

State:

```

WORKING

```

Hardware:

- Servo → Computer
- LED → Green
- LCD → "Executing actions"

Examples:

- creating calendar events
- scheduling meetings

---

# Step 10 — Executor Sends Result

Executor returns to **Phone**.

State:

```

COMMUNICATION

```

Hardware:

- Servo → Phone
- LED → Blue
- LCD → "Sending response"

The user receives the final result.

Executor returns to **IDLE**.

---

# ✅ Task Completion Signal

When the workflow finishes:

All agents perform a **completion signal**.

| Component | Behavior |
|------|------|
LED | Green blinking |
LCD | Displays "DONE" |
Servo | Returns to idle position |

Example display:

```

DONE
Ready for tasks

```

---

# 🎤 Voice Interaction

Users can interact with the system using voice commands.

Example:

```

"Ok agents, organize my meetings tomorrow"

```

Voice flow:

```

User speech
│
▼
Microphone input
│
▼
Speech-to-text
│
▼
AI agents analyze request
│
▼
Agents execute workflow
│
▼
Physical office reacts

```

---

# 💬 Telegram Integration

The system also supports remote interaction via Telegram.

Example command:

```

/task organize my meetings tomorrow

````

The request enters the same **agent workflow pipeline**.

---

# 📡 Backend Communication Example

Backend sends agent states to the ESP32.

Example JSON:

```json
{
  "planner": {
    "state": "communication",
    "task": "receiving request"
  },
  "researcher": {
    "state": "idle",
    "task": "waiting"
  },
  "executor": {
    "state": "idle",
    "task": "waiting"
  }
}
````

ESP32 interprets the state and updates:

* servo position
* LED color
* LCD text

---

# 🔧 Hardware Components

| Component         | Purpose                 |
| ----------------- | ----------------------- |
| ESP32             | Main controller         |
| Servo Motors      | Move agents             |
| LCD Displays      | Display tasks           |
| RGB LEDs          | State indicators        |
| Microphone Module | Voice input             |
| Custom PCB        | Circuit integration     |
| Physical Model    | Mini office environment |

---

# 🧰 Software Stack

| Technology     | Purpose             |
| -------------- | ------------------- |
| Python         | Backend logic       |
| OpenClaw       | Agent framework     |
| LLM APIs       | Reasoning           |
| ESP32 Arduino  | Hardware control    |
| REST API       | Communication layer |
| Telegram Bot   | Messaging interface |
| Speech-to-Text | Voice commands      |

---

# 📂 Repository Structure

```
AgentCity-lab
│
├── hardware
│   ├── circuits
│   ├── pcb
│   └── esp32
│
├── backend
│   ├── agents
│   ├── api
│   └── automation
│
├── speech
│   └── speech_to_text
│
├── docs
│   └── diagrams
│
└── README.md
```

---

## 👥 Meet the Team

---

### 🧠 AI & Software Engineering
**Diego Torres** | [GitHub](https://github.com/Diego31-10)

**Core Responsibilities:**
* **AI Architecture:** Designing and optimizing neural network structures.
* **Backend Development:** Building robust server-side infrastructure and APIs.
* **Speech Processing:** Developing advanced audio-to-text and NLP pipelines.
* **Automation Logic:** Engineering intelligent, event-driven workflows.
* **LLM Integration:** Implementing and fine-tuning Large Language Models.

---

### ⚙️ Hardware & Electronics
**Juan José Medina** | [GitHub](https://github.com/JuanjoMedina23)

**Core Responsibilities:**
* **Firmware Engineering:** Advanced ESP32 programming and IoT connectivity.
* **Circuit Design:** Schematics and electronic system analysis.
* **PCB Development:** Custom printed circuit board design and routing.
* **Physical Prototyping:** Structural model construction and hardware assembly.

---

# 🚀 Future Improvements

Potential future features:

* real-time web dashboard
* computer vision monitoring
* additional AI agents
* voice response (text-to-speech)
* smart home integrations
* distributed IoT nodes

---

# 📚 Educational Value

This project demonstrates real-world concepts including:

* multi-agent AI systems
* IoT hardware integration
* automation pipelines
* embedded programming
* human-AI interaction

---

## 📜 License

This project is licensed under the **MIT License**.  
For more details, please see [LICENSE](./LICENSE).

---

# ⭐ Support the Project

If you find this project interesting, consider giving the repository a **star ⭐**.
