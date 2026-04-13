<div align="center">

# 🦞 AgentCity

**A multi-agent AI system where three specialized agents collaborate in real time**

[![OpenRouter](https://img.shields.io/badge/LLM-OpenRouter-7c3aed?style=flat-square)](https://openrouter.ai)
[![Claude](https://img.shields.io/badge/Model-Claude_Haiku-06b6d4?style=flat-square)](https://anthropic.com)
[![Node.js](https://img.shields.io/badge/Runtime-Node.js-339933?style=flat-square)](https://nodejs.org)
[![License](https://img.shields.io/badge/License-MIT-lightgrey?style=flat-square)](./LICENSE)

*What if you could watch AI agents think, collaborate, and act — in real time?*

</div>

---

## 🌆 The Idea

AI agents are invisible — they live in terminals, logs, and API responses. You send a message, something happens, you get a reply.

**AgentCity makes that invisible process visible.**

Three specialized agents work together to process any task. While they work, you watch them live in a full-screen terminal dashboard. Each agent has a visual representation that changes based on what they're doing: resting on the sofa, working at the computer, or on the phone communicating with another agent.

---

## 🦞 The Residents

AgentCity has three permanent residents, each one an AI agent with its own personality and role:

<div align="center">

| Name | Role | Personality |
|------|------|-------------|
| **Xocas** | Planner | The strategist. Receives every task first, breaks it down, and decides who does what. Never acts without a plan. |
| **Momo** | Researcher | The curious one. Takes Xocas's plan and enriches it with context, data, and insights. |
| **Llados** | Executor | The doer. Takes Momo's research and produces the final output — documents, reports, action plans. |

</div>

---

## 🏢 Their Office

```
[ 💻 Computer ] ──────── [ 🛋️ Sofa ] ──────── [ 📞 Phone ]
   WORKING                   IDLE              COMMUNICATION
```

Each agent moves between these three positions depending on what they're doing at that moment — visible in real time in the terminal dashboard.

---

## 🎨 Agent States (Live TUI)

```
╔══════════════════════════════════════════════════════════════════════════╗
║              === AGENTCITY  -  Multi-Agent Dashboard ===                 ║
╚══════════════════════════════════════════════════════════════════════════╝
┌─ XOCAS - Planner ──────┐  ┌─ MOMO - Researcher ────┐  ┌─ LLADOS - Executor ────┐
│   .--.                  │  │   .--.                  │  │   .--.                  │
│  (^  ^)                 │  │  (x  x)  zzz            │  │  (o  o) )))            │
│   \--/                  │  │   \--/                  │  │   \--/                  │
│  ->|  |  [=]            │  │  _/|  |\_               │  │  /|  |c                │
│  [ WORKING - At PC ]    │  │  [ IDLE - On sofa ]     │  │  [ COMM - On phone ]   │
└─────────────────────────┘  └─────────────────────────┘  └─────────────────────────┘
```

### State Colors

| Color | State | Meaning |
|-------|-------|---------|
| 🟡 Yellow | `IDLE` | Agent is resting, waiting for work |
| 🟢 Green | `WORKING` | Agent is processing — reasoning, writing, executing |
| 🔵 Cyan | `COMMUNICATING` | Agent is sending or receiving from another agent |

---

## ⚙️ System Architecture

```
User Input (TUI)
      │
      ▼
  Orchestrator
      │
      ├── Xocas  (Planner)    ← claude-haiku-4-5 via OpenRouter
      ├── Momo   (Researcher) ← claude-haiku-4-5 via OpenRouter
      └── Llados (Executor)   ← claude-haiku-4-5 via OpenRouter
      │
      ├── State Manager  ──► REST API :5001  ──► (ESP32 ready)
      │
      └── Pipeline: Xocas → Momo → Llados → File Output
```

---

## 🔄 Workflow Example

**User types in TUI:** `organize my meetings for tomorrow`

| Step | Agent | State | Visual |
|------|-------|-------|--------|
| 1 | All | `IDLE` | All agents on sofa, yellow |
| 2 | Xocas | `COMMUNICATING` | On the phone, receiving task |
| 3 | Xocas | `WORKING` | At the computer, creating plan |
| 4 | Xocas | `COMMUNICATING` | On the phone, sending plan → `IDLE` |
| 5 | Momo | `COMMUNICATING` | On the phone, receiving plan |
| 6 | Momo | `WORKING` | At the computer, running research |
| 7 | Momo | `COMMUNICATING` | On the phone, sending research → `IDLE` |
| 8 | Llados | `COMMUNICATING` | On the phone, receiving data |
| 9 | Llados | `WORKING` | At the computer, executing |
| 10 | Llados | `COMMUNICATING` | On the phone, delivering result → `IDLE` |
| ✅ | All | `IDLE` | Pipeline complete |

---

## 📄 File Output

Llados generates output files in multiple formats. AgentCity auto-detects the format from your task description:

| What you say | Output format |
|---|---|
| "...Word document..." | `.docx` |
| "...PDF..." | `.pdf` |
| "...markdown..." | `.md` |
| "...text file..." | `.txt` |
| "...file..." (generic) | `.docx` (default) |

Files are saved directly to your Desktop.

---

## 🌐 REST API (ESP32 Ready)

AgentCity exposes a live state API on port `5001`, designed for future integration with physical hardware (servos, LEDs, LCD screens):

```
GET /states   → real-time state of all 3 agents
GET /health   → health check
```

Example response:
```json
{
  "xocas":  { "state": "WORKING",        "task": "Creating plan..." },
  "momo":   { "state": "IDLE",           "task": "Waiting..." },
  "llados": { "state": "COMMUNICATING",  "task": "Sending response..." }
}
```

---

## 🛠️ Tech Stack

| Component | Technology |
|---|---|
| Runtime | Node.js (ES modules) |
| LLM | [OpenRouter](https://openrouter.ai) → `anthropic/claude-haiku-4-5` |
| Terminal UI | [blessed](https://github.com/chjj/blessed) |
| REST API | Express |
| File generation | docx, pdfkit |
| HTTP client | OpenAI-compatible SDK |

---

## 🚀 Setup

```bash
# 1. Clone the repo
git clone https://github.com/Diego31-10/agentcity.git
cd agentcity

# 2. Install dependencies
npm install

# 3. Add your OpenRouter API key
# Create a .env file:
echo "OPENROUTER_API_KEY=sk-or-v1-..." > .env

# 4. Run
node src/main.js
```

Or open in a new terminal window (Windows):
```powershell
.\launch.ps1
```

The task input happens **inside the TUI** — type when prompted and press Enter.

---

## 📁 Project Structure

```
agentcity/
├── src/
│   ├── main.js          # Entry point — TUI, pipeline, file saving
│   ├── agents.js        # Xocas, Momo, Llados agent logic
│   ├── claudeClient.js  # OpenRouter API client
│   ├── stateManager.js  # Real-time agent state tracking
│   ├── tui.js           # Terminal UI (blessed)
│   ├── fileWriter.js    # Multi-format file output (.docx, .pdf, .md, .txt)
│   └── api.js           # REST API (port 5001)
├── launch.ps1           # Windows launcher (opens in new terminal)
├── package.json
└── README.md
```

---

## 🔮 What's Next

- Physical city: ESP32 + servo motors + RGB LEDs + LCD displays per agent
- Web dashboard mirroring the terminal state in real time
- Llados with real tool access (calendar, email, web search)
- Voice input/output integration
- More agents, more offices — the city grows

---

## 🌍 Why This Project Exists

Most people have no intuition for what multi-agent AI systems actually do. They know "AI" does something, but the process — the delegation, the specialization, the back-and-forth between agents — is completely opaque.

AgentCity tries to fix that. Not with a diagram, but with movement you can see. When you watch three agents shift states in real time in response to a single message, something clicks that a static API response never achieves.

---

## 📜 License

MIT — see [LICENSE](./LICENSE)

---

<div align="center">

*Built with 🦞 and ❤️*

**AgentCity** — *Where AI agents get to work*

</div>
