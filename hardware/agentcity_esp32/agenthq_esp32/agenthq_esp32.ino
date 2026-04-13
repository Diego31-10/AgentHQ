/*
 * AgentHQ - ESP32 Controller
 *
 * Reads agent states from the AgentHQ REST API and controls:
 *   - 3x SG90 servos via PCA9685 (I2C, addr 0x40)
 *   - 3x RGB LEDs (common cathode)
 *   - 3x LCD 16x2 via I2C
 * 
 * Agent states:
 *   IDLE          -> Servo: 90° (sofa)  | LED: Yellow | LCD: task text
 *   WORKING       -> Servo: 0°  (PC)    | LED: Green  | LCD: task text
 *   COMMUNICATING -> Servo: 180° (phone)| LED: Blue   | LCD: task text
 * 
 * Dependencies (install via Arduino Library Manager):
 *   - Adafruit PWM Servo Driver Library (for PCA9685)
 *   - LiquidCrystal_I2C
 *   - ArduinoJson
 *   - WiFi (built-in ESP32)
 *   - HTTPClient (built-in ESP32)
 */

#include <Wire.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Adafruit_PWMServoDriver.h>
#include <LiquidCrystal_I2C.h>

// ─── WiFi Config ────────────────────────────────────────────────
const char* WIFI_SSID     = "DIEGO";
const char* WIFI_PASSWORD = "TpDeDatv$039";

// ─── AgentHQ API ──────────────────────────────────────────────
// Replace with your PC's local IP where AgentHQ is running
const char* API_URL = "http://192.168.1.9:5001/states";

// How often to poll the API (milliseconds)
const unsigned long POLL_INTERVAL = 500;

// ─── PCA9685 Servo Driver ────────────────────────────────────────
Adafruit_PWMServoDriver pca = Adafruit_PWMServoDriver(0x40);

// Servo channels on PCA9685
#define SERVO_XOCAS   0
#define SERVO_MOMO    1
#define SERVO_LLADOS  2

// Servo pulse range (SG90: ~150-600)
#define SERVO_MIN  150   // 0°   = WORKING (at PC)
#define SERVO_MID  375   // 90°  = IDLE    (on sofa)
#define SERVO_MAX  550   // ~180° = COMMUNICATING (on phone) (calibrated for your SG90)

// ─── RGB LED Pins ────────────────────────────────────────────────
// Each agent has 3 pins: R, G, B
struct RGBPin { int r, g, b; };

// XOCAS OK
RGBPin LED_XOCAS = {25, 26, 27};

// MOMO (cableado final): usa el LED que estaba en LLADOS (R/G invertidos)
RGBPin LED_MOMO = {33, 32, 23};  // R=GPIO33, G=GPIO32, B=GPIO23

// LLADOS (cableado final): usa el LED que estaba en MOMO (R/G invertidos)
RGBPin LED_LLADOS = {12, 14, 13}; // R=GPIO12, G=GPIO14, B=GPIO13

// ─── Buzzer / Status Indicator ───────────────────────────────────
// ESP32 Arduino core v3.x: ledcSetup/ledcAttachPin ya no existen.
// Usamos tone() para beeps (más compatible).
#define BUZZER_PIN 2

// ─── LCD Displays (I2C) ──────────────────────────────────────────
// Each LCD needs a unique I2C address. Set jumpers on I2C modules:
//   Xocas:  0x27
//   Momo:   0x26
//   Llados: 0x25
LiquidCrystal_I2C lcd_xocas (0x27, 16, 2);
LiquidCrystal_I2C lcd_momo  (0x26, 16, 2);
LiquidCrystal_I2C lcd_llados(0x25, 16, 2);

// ─── State tracking ──────────────────────────────────────────────
struct AgentState {
  String state;
  String task;
};

AgentState prev_xocas  = {"", ""};
AgentState prev_momo   = {"", ""};
AgentState prev_llados = {"", ""};

// Current (for LCD scrolling)
AgentState cur_xocas  = {"IDLE", "Waiting..."};
AgentState cur_momo   = {"IDLE", "Waiting..."};
AgentState cur_llados = {"IDLE", "Waiting..."};

unsigned long lastPoll = 0;

// ─── LCD Scrolling ───────────────────────────────────────────────
// Option 3: Line 1 = "AGENT STATE" (fixed). Line 2 = task (scroll if >16).
const unsigned long SCROLL_STEP_MS  = 250;
const unsigned long SCROLL_PAUSE_MS = 800;  // pause at start/end

// LCD debounce (avoid flicker when tasks change too fast)
const unsigned long LCD_TASK_DEBOUNCE_MS = 700;

struct ScrollState {
  String text;
  int pos;
  bool forward;
  unsigned long nextAt;
};

ScrollState sc_xocas  = {"", 0, true, 0};
ScrollState sc_momo   = {"", 0, true, 0};
ScrollState sc_llados = {"", 0, true, 0};

unsigned long lastLcdUpdate_xocas  = 0;
unsigned long lastLcdUpdate_momo   = 0;
unsigned long lastLcdUpdate_llados = 0;

// Offline/backoff tracking
unsigned long lastOkPollAt = 0;
unsigned long pollIntervalMs = POLL_INTERVAL;
const unsigned long POLL_INTERVAL_MIN = POLL_INTERVAL;
const unsigned long POLL_INTERVAL_MAX = 5000;
const unsigned long OFFLINE_AFTER_MS  = 3000;
bool offlineMode = false;

// ─── Setup ───────────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);
  Serial.println("[AgentHQ] Booting...");

  // Init LED pins
  initLEDs(LED_XOCAS);
  initLEDs(LED_MOMO);
  initLEDs(LED_LLADOS);

  // Init buzzer
  pinMode(BUZZER_PIN, OUTPUT);
  noTone(BUZZER_PIN);

  // Init PCA9685
  pca.begin();
  pca.setPWMFreq(50);  // 50Hz for servos

  // Init LCDs
  lcd_xocas.init();  lcd_xocas.backlight();
  lcd_momo.init();   lcd_momo.backlight();
  lcd_llados.init(); lcd_llados.backlight();

  printLCD(lcd_xocas,  "XOCAS",    "Booting...");
  printLCD(lcd_momo,   "MOMO",     "Booting...");
  printLCD(lcd_llados, "LLADOS",   "Booting...");

  // Set all agents to IDLE on startup
  applyState(SERVO_XOCAS,  LED_XOCAS,  lcd_xocas,  "IDLE", "Waiting...");
  applyState(SERVO_MOMO,   LED_MOMO,   lcd_momo,   "IDLE", "Waiting...");
  applyState(SERVO_LLADOS, LED_LLADOS, lcd_llados, "IDLE", "Waiting...");

  lastOkPollAt = millis();
  offlineMode = false;

  // Connect to WiFi
  connectWiFi();
}

// ─── Loop ────────────────────────────────────────────────────────
void setOfflineUI() {
  // Visual fallback when API/WiFi is down
  // Keep servos at IDLE, LEDs red, LCD shows OFFLINE
  pca.setPWM(SERVO_XOCAS, 0, SERVO_MID);
  pca.setPWM(SERVO_MOMO, 0, SERVO_MID);
  pca.setPWM(SERVO_LLADOS, 0, SERVO_MID);

  setLED(LED_XOCAS, 255, 0, 0);
  setLED(LED_MOMO, 255, 0, 0);
  setLED(LED_LLADOS, 255, 0, 0);

  // Show once (avoid flicker)
  renderAgentLCD(lcd_xocas, "XOCAS", "OFFLINE", sc_xocas, "No API / WiFi");
  renderAgentLCD(lcd_momo, "MOMO", "OFFLINE", sc_momo, "No API / WiFi");
  renderAgentLCD(lcd_llados, "LLADOS", "OFFLINE", sc_llados, "No API / WiFi");
}

void maybeApplyDebouncedLCD(const String& agent, const String& state, const String& task) {
  unsigned long now = millis();

  if (agent == "XOCAS") {
    if (now - lastLcdUpdate_xocas >= LCD_TASK_DEBOUNCE_MS || cur_xocas.task.length() == 0) {
      cur_xocas.state = state;
      cur_xocas.task = task;
      scrollReset(sc_xocas, task);
      lastLcdUpdate_xocas = now;
    }
  } else if (agent == "MOMO") {
    if (now - lastLcdUpdate_momo >= LCD_TASK_DEBOUNCE_MS || cur_momo.task.length() == 0) {
      cur_momo.state = state;
      cur_momo.task = task;
      scrollReset(sc_momo, task);
      lastLcdUpdate_momo = now;
    }
  } else {
    if (now - lastLcdUpdate_llados >= LCD_TASK_DEBOUNCE_MS || cur_llados.task.length() == 0) {
      cur_llados.state = state;
      cur_llados.task = task;
      scrollReset(sc_llados, task);
      lastLcdUpdate_llados = now;
    }
  }
}

void loop() {
  unsigned long now = millis();

  // Adaptive polling / offline detection
  if (now - lastPoll >= pollIntervalMs) {
    lastPoll = now;

    bool ok = false;

    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("[WiFi] Disconnected. Reconnecting...");
      connectWiFi();
    }

    if (WiFi.status() == WL_CONNECTED) {
      AgentState xocas, momo, llados;
      if (fetchStates(xocas, momo, llados)) {
        ok = true;

        // Back to fast polling when OK
        pollIntervalMs = POLL_INTERVAL_MIN;
        lastOkPollAt = now;
        offlineMode = false;

        // Apply to servos+LEDs when changes
        if (xocas.state != prev_xocas.state || xocas.task != prev_xocas.task) {
          applyState(SERVO_XOCAS, LED_XOCAS, lcd_xocas, xocas.state, xocas.task);
          prev_xocas = xocas;
          maybeApplyDebouncedLCD("XOCAS", xocas.state, xocas.task);
        }
        if (momo.state != prev_momo.state || momo.task != prev_momo.task) {
          applyState(SERVO_MOMO, LED_MOMO, lcd_momo, momo.state, momo.task);
          prev_momo = momo;
          maybeApplyDebouncedLCD("MOMO", momo.state, momo.task);
        }
        if (llados.state != prev_llados.state || llados.task != prev_llados.task) {
          applyState(SERVO_LLADOS, LED_LLADOS, lcd_llados, llados.state, llados.task);
          prev_llados = llados;
          maybeApplyDebouncedLCD("LLADOS", llados.state, llados.task);
        }
      }
    }

    if (!ok) {
      // Exponential-ish backoff
      pollIntervalMs = min(pollIntervalMs * 2, POLL_INTERVAL_MAX);
      if (now - lastOkPollAt >= OFFLINE_AFTER_MS) {
        if (!offlineMode) {
          offlineMode = true;
          setOfflineUI();
        }
      }
    }
  }

  // LCD scroll updates (non-blocking)
  if (now >= sc_xocas.nextAt) {
    renderAgentLCD(lcd_xocas, "XOCAS", offlineMode ? "OFFLINE" : cur_xocas.state, sc_xocas, offlineMode ? "No API / WiFi" : cur_xocas.task);
    scrollStep(sc_xocas);
  }
  if (now >= sc_momo.nextAt) {
    renderAgentLCD(lcd_momo, "MOMO", offlineMode ? "OFFLINE" : cur_momo.state, sc_momo, offlineMode ? "No API / WiFi" : cur_momo.task);
    scrollStep(sc_momo);
  }
  if (now >= sc_llados.nextAt) {
    renderAgentLCD(lcd_llados, "LLADOS", offlineMode ? "OFFLINE" : cur_llados.state, sc_llados, offlineMode ? "No API / WiFi" : cur_llados.task);
    scrollStep(sc_llados);
  }
}

// ─── Fetch states from API ───────────────────────────────────────
bool fetchStates(AgentState& xocas, AgentState& momo, AgentState& llados) {
  HTTPClient http;
  http.begin(API_URL);
  http.setTimeout(400);

  int code = http.GET();
  if (code != 200) {
    Serial.printf("[API] Error: %d\n", code);
    http.end();
    return false;
  }

  String body = http.getString();
  http.end();

  JsonDocument doc;
  if (deserializeJson(doc, body) != DeserializationError::Ok) {
    Serial.println("[JSON] Parse error");
    return false;
  }

  xocas.state  = doc["xocas"]["state"].as<String>();
  xocas.task   = doc["xocas"]["task"].as<String>();
  momo.state   = doc["momo"]["state"].as<String>();
  momo.task    = doc["momo"]["task"].as<String>();
  llados.state = doc["llados"]["state"].as<String>();
  llados.task  = doc["llados"]["task"].as<String>();

  return true;
}

// ─── Apply state to servo + LED + LCD + PWM ──────────────────────
void applyState(int servoChannel, RGBPin led, LiquidCrystal_I2C& lcd,
                String state, String task) {
  Serial.printf("[State] ch=%d  %s  |  %s\n", servoChannel, state.c_str(), task.c_str());

  // Servo position
  int pulse = SERVO_MID;  // default: IDLE
  if      (state == "WORKING")       pulse = SERVO_MIN;
  else if (state == "COMMUNICATING") pulse = SERVO_MAX;
  pca.setPWM(servoChannel, 0, pulse);

  // LED color
  setLED(led, 0, 0, 0);  // reset
  if      (state == "IDLE")          setLED(led, 255, 255, 0);  // Yellow
  else if (state == "WORKING")       setLED(led, 0, 255, 0);    // Green
  else if (state == "COMMUNICATING") setLED(led, 0, 0, 255);    // Blue
  else                               setLED(led, 255, 0, 0);    // Red (error)

  // PWM buzzer feedback (beep on state change)
  beepFeedback(state);

  // LCD
  // Option 3 (fixed header + scrolling task) se refresca en loop()
  // Aquí solo actualizamos el "current" y reseteamos scroll; el debounce está en maybeApplyDebouncedLCD().
  String agentName = (servoChannel == SERVO_XOCAS)  ? "XOCAS"  :
                     (servoChannel == SERVO_MOMO)    ? "MOMO"   : "LLADOS";

  if (agentName == "XOCAS") {
    cur_xocas.state = state;
    cur_xocas.task = task;
    scrollReset(sc_xocas, task);
  } else if (agentName == "MOMO") {
    cur_momo.state = state;
    cur_momo.task = task;
    scrollReset(sc_momo, task);
  } else {
    cur_llados.state = state;
    cur_llados.task = task;
    scrollReset(sc_llados, task);
  }
}

// ─── PWM Buzzer Feedback ─────────────────────────────────────────
void beepFeedback(String state) {
  // Beeps usando tone() (compatible con ESP32 core v3.x)
  // IDLE: silencioso
  if (state == "WORKING") {
    tone(BUZZER_PIN, 2000, 60);
    delay(80);
    noTone(BUZZER_PIN);
  } else if (state == "COMMUNICATING") {
    tone(BUZZER_PIN, 2300, 40);
    delay(60);
    noTone(BUZZER_PIN);
    delay(40);
    tone(BUZZER_PIN, 1800, 40);
    delay(60);
    noTone(BUZZER_PIN);
  }
}

// ─── LCD helper ──────────────────────────────────────────────────
String fit16(const String& s) {
  if (s.length() >= 16) return s.substring(0, 16);
  String out = s;
  while (out.length() < 16) out += " ";
  return out;
}

String window16(const String& s, int start) {
  if (s.length() == 0) return "                ";
  if (s.length() <= 16) return fit16(s);
  if (start < 0) start = 0;
  if (start > (int)s.length() - 16) start = (int)s.length() - 16;
  return s.substring(start, start + 16);
}

void scrollReset(ScrollState& sc, const String& text) {
  sc.text = text;
  sc.pos = 0;
  sc.forward = true;
  sc.nextAt = millis() + SCROLL_PAUSE_MS;
}

void scrollStep(ScrollState& sc) {
  // If text fits, just refresh occasionally (avoid tight loop)
  if (sc.text.length() <= 16) {
    sc.nextAt = millis() + 500;
    return;
  }
  int maxPos = (int)sc.text.length() - 16;
  if (maxPos <= 0) {
    sc.nextAt = millis() + 500;
    return;
  }

  if (sc.forward) {
    sc.pos++;
    if (sc.pos >= maxPos) {
      sc.pos = maxPos;
      sc.forward = false;
      sc.nextAt = millis() + SCROLL_PAUSE_MS;
      return;
    }
  } else {
    sc.pos--;
    if (sc.pos <= 0) {
      sc.pos = 0;
      sc.forward = true;
      sc.nextAt = millis() + SCROLL_PAUSE_MS;
      return;
    }
  }
  sc.nextAt = millis() + SCROLL_STEP_MS;
}

void printLCD(LiquidCrystal_I2C& lcd, const String& line1, const String& line2) {
  // (used for boot/debug screens)
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(fit16(line1));
  lcd.setCursor(0, 1);
  lcd.print(fit16(line2));
}

void renderAgentLCD(LiquidCrystal_I2C& lcd, const String& agent, const String& state,
                    ScrollState& sc, const String& task) {
  // Line 1 fixed
  String line1 = agent + " " + state;
  line1 = fit16(line1);

  // If task changed, reset scroll
  if (sc.text != task) scrollReset(sc, task);

  String line2 = window16(sc.text, sc.pos);

  lcd.setCursor(0, 0);
  lcd.print(line1);
  lcd.setCursor(0, 1);
  lcd.print(line2);
}

// ─── LED helpers ─────────────────────────────────────────────────
void initLEDs(RGBPin led) {
  pinMode(led.r, OUTPUT);
  pinMode(led.g, OUTPUT);
  pinMode(led.b, OUTPUT);
  setLED(led, 0, 0, 0);
}

void setLED(RGBPin led, int r, int g, int b) {
  analogWrite(led.r, r);
  analogWrite(led.g, g);
  analogWrite(led.b, b);
}

// ─── WiFi ────────────────────────────────────────────────────────
void connectWiFi() {
  Serial.printf("[WiFi] Connecting to %s", WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf("\n[WiFi] Connected! IP: %s\n", WiFi.localIP().toString().c_str());
  } else {
    Serial.println("\n[WiFi] Failed to connect.");
  }
}
