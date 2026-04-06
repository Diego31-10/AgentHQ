/*
 * AgentCity - ESP32 Controller
 * 
 * Reads agent states from the AgentCity REST API and controls:
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
const char* WIFI_SSID     = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

// ─── AgentCity API ──────────────────────────────────────────────
// Replace with your PC's local IP where AgentCity is running
const char* API_URL = "http://192.168.1.X:5001/states";

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
#define SERVO_MAX  600   // 180° = COMMUNICATING (on phone)

// ─── RGB LED Pins ────────────────────────────────────────────────
// Each agent has 3 pins: R, G, B
struct RGBPin { int r, g, b; };

RGBPin LED_XOCAS  = {25, 26, 27};
RGBPin LED_MOMO   = {14, 12, 13};
RGBPin LED_LLADOS = {32, 33, 23};  // GPIO23 (output-capable pin for PWM)

// ─── PWM Output (Buzzer / Status Indicator) ──────────────────────
#define PWM_OUTPUT_PIN 2    // GPIO2 - supports PWM, safe for ESP32
#define PWM_FREQUENCY 1000  // 1kHz buzzer tone

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

unsigned long lastPoll = 0;

// ─── Setup ───────────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);
  Serial.println("[AgentCity] Booting...");

  // Init LED pins
  initLEDs(LED_XOCAS);
  initLEDs(LED_MOMO);
  initLEDs(LED_LLADOS);

  // Init PWM output
  pinMode(PWM_OUTPUT_PIN, OUTPUT);
  ledcSetup(0, PWM_FREQUENCY, 8);          // Channel 0, 1kHz, 8-bit (0-255)
  ledcAttachPin(PWM_OUTPUT_PIN, 0);
  ledcWrite(0, 0);                         // Start silent

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

  // Connect to WiFi
  connectWiFi();
}

// ─── Loop ────────────────────────────────────────────────────────
void loop() {
  unsigned long now = millis();

  if (now - lastPoll >= POLL_INTERVAL) {
    lastPoll = now;

    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("[WiFi] Disconnected. Reconnecting...");
      connectWiFi();
      return;
    }

    AgentState xocas, momo, llados;
    if (fetchStates(xocas, momo, llados)) {
      if (xocas.state  != prev_xocas.state  || xocas.task  != prev_xocas.task)
        applyState(SERVO_XOCAS,  LED_XOCAS,  lcd_xocas,  xocas.state,  xocas.task);

      if (momo.state   != prev_momo.state   || momo.task   != prev_momo.task)
        applyState(SERVO_MOMO,   LED_MOMO,   lcd_momo,   momo.state,   momo.task);

      if (llados.state != prev_llados.state || llados.task != prev_llados.task)
        applyState(SERVO_LLADOS, LED_LLADOS, lcd_llados, llados.state, llados.task);

      prev_xocas  = xocas;
      prev_momo   = momo;
      prev_llados = llados;
    }
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
  String agentName = (servoChannel == SERVO_XOCAS)  ? "XOCAS"  :
                     (servoChannel == SERVO_MOMO)    ? "MOMO"   : "LLADOS";
  printLCD(lcd, agentName + " " + state, task);
}

// ─── PWM Buzzer Feedback ─────────────────────────────────────────
void beepFeedback(String state) {
  int duration = 50;  // ms
  int intensity = 200; // 0-255

  if (state == "WORKING") {
    ledcWrite(0, intensity);
    delay(duration);
    ledcWrite(0, 0);
  } else if (state == "COMMUNICATING") {
    ledcWrite(0, intensity);
    delay(duration / 2);
    ledcWrite(0, 0);
    delay(duration / 4);
    ledcWrite(0, intensity);
    delay(duration / 2);
    ledcWrite(0, 0);
  }
  // IDLE: silent
}

// ─── LCD helper ──────────────────────────────────────────────────
void printLCD(LiquidCrystal_I2C& lcd, String line1, String line2) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(line1.substring(0, 16));
  lcd.setCursor(0, 1);
  lcd.print(line2.substring(0, 16));
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
