#include <Wire.h>
#include <U8g2lib.h>

/*
  HealthPod - Smart Medication Management System
  Author: Hamza Ismail

  Description:
  Arduino-based smart pill dispensing prototype with:
  - 3 medication compartments
  - IR/analog stock level sensing
  - OLED status display
  - Manual serial dispensing commands
  - Servo-based dispensing control without Servo library
*/

// -------- OLED --------
U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

// -------- Pin Mapping --------
// Compartment 1: A3 sensor, D2 servo
// Compartment 2: A2 sensor, D3 servo
// Compartment 3: A1 sensor, D4 servo
const int SENSOR_PINS[3] = {A3, A2, A1};
const int SERVO_PINS[3]  = {2, 3, 4};

// -------- Sensor Thresholds (Tune as needed) --------
int RAW_FULL   = 800;
int RAW_MEDIUM = 550;

enum StockLevel { LOW_LVL, MED_LVL, FULL_LVL };

// If sensor gives lower raw value when object is closer, keep this true
const bool SENSOR_INVERTED = true;

StockLevel getStockLevel(int raw) {
  if (!SENSOR_INVERTED) {
    if (raw >= RAW_FULL)   return FULL_LVL;
    if (raw >= RAW_MEDIUM) return MED_LVL;
    return LOW_LVL;
  } else {
    if (raw <= RAW_FULL)   return FULL_LVL;
    if (raw <= RAW_MEDIUM) return MED_LVL;
    return LOW_LVL;
  }
}

// -------- Servo Control (No Servo Library) --------
const int SERVO_FRAME_US = 20000; // 50 Hz
const int SERVO_MIN_US   = 1000;  // ~0 degrees
const int SERVO_MAX_US   = 2000;  // ~180 degrees

// Home position = closed position
int SERVO_HOME_DEG[3] = {15, 15, 15};

// Movement positions for dispensing
int SERVO_PICK_DEG[3] = {30, 30, 30};
int SERVO_DROP_DEG[3] = {105, 105, 105};

const int DEG_STEP          = 2;
const int STEP_HOLD_MS      = 18;
const int PICK_HOLD_MS      = 250;
const int DROP_HOLD_MS      = 300;
const int BETWEEN_ACTION_MS = 200;

bool busy = false;

int clampInt(int v, int lo, int hi) {
  if (v < lo) return lo;
  if (v > hi) return hi;
  return v;
}

int degToPulseUs(int deg) {
  deg = clampInt(deg, 0, 180);
  long us = SERVO_MIN_US + (long)(SERVO_MAX_US - SERVO_MIN_US) * deg / 180L;
  return (int)us;
}

void servoPulseUs(int pin, int pulseUs) {
  digitalWrite(pin, HIGH);
  delayMicroseconds(pulseUs);
  digitalWrite(pin, LOW);

  int rest = SERVO_FRAME_US - pulseUs;
  if (rest > 0) delayMicroseconds(rest);
}

void holdAngle(int servoIdx, int deg, int durationMs) {
  int pin = SERVO_PINS[servoIdx];
  int pulse = degToPulseUs(deg);
  unsigned long start = millis();

  while (millis() - start < (unsigned long)durationMs) {
    servoPulseUs(pin, pulse);
  }
}

void slowMove(int servoIdx, int fromDeg, int toDeg) {
  int step = (toDeg >= fromDeg) ? DEG_STEP : -DEG_STEP;
  int deg = fromDeg;

  while (true) {
    holdAngle(servoIdx, deg, STEP_HOLD_MS);

    if (deg == toDeg) break;

    deg += step;
    if (step > 0 && deg > toDeg) deg = toDeg;
    if (step < 0 && deg < toDeg) deg = toDeg;
  }
}

// -------- Dispense Logic --------
// Rotate slightly to pick pill -> continue to drop -> return home
void dispense(uint8_t idx) {
  busy = true;

  int home = SERVO_HOME_DEG[idx];
  int pick = SERVO_PICK_DEG[idx];
  int drop = SERVO_DROP_DEG[idx];

  // Enforce correct movement order
  if (pick < home) pick = home;
  if (drop < pick) drop = pick;

  slowMove(idx, home, pick);
  holdAngle(idx, pick, PICK_HOLD_MS);

  slowMove(idx, pick, drop);
  holdAngle(idx, drop, DROP_HOLD_MS);

  slowMove(idx, drop, home);
  delay(BETWEEN_ACTION_MS);

  busy = false;
}

// -------- Sensors + OLED --------
int readSensorRaw(int pin) {
  return analogRead(pin);
}

void drawUI(const int raw[3]) {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x12_tr);

  u8g2.setCursor(0, 10);
  u8g2.print("HealthPod");

  u8g2.setCursor(92, 10);
  u8g2.print(busy ? "BUSY" : "OK");

  const int rowY[3] = {16, 24, 31};

  for (int i = 0; i < 3; i++) {
    StockLevel lvl = getStockLevel(raw[i]);

    u8g2.setCursor(0, rowY[i]);
    u8g2.print("C");
    u8g2.print(i + 1);
    u8g2.print(": ");

    if (lvl == FULL_LVL)      u8g2.print("FULL");
    else if (lvl == MED_LVL)  u8g2.print("MED ");
    else                      u8g2.print("LOW ");

    u8g2.setCursor(70, rowY[i]);
    u8g2.print(raw[i]); // raw value for calibration
  }

  u8g2.sendBuffer();
}

// Keep servos holding home position while idle
void keepServosAtHomeIdle() {
  static unsigned long lastRefresh = 0;
  static int servoIdx = 0;

  if (busy) return;
  if (millis() - lastRefresh < 60) return;
  lastRefresh = millis();

  holdAngle(servoIdx, SERVO_HOME_DEG[servoIdx], 18);
  servoIdx = (servoIdx + 1) % 3;
}

void setup() {
  Serial.begin(115200);

  for (int i = 0; i < 3; i++) {
    pinMode(SERVO_PINS[i], OUTPUT);
    digitalWrite(SERVO_PINS[i], LOW);
  }

  // Move servos to home position on startup
  for (int i = 0; i < 3; i++) {
    holdAngle(i, SERVO_HOME_DEG[i], 350);
  }

  Wire.begin();
  u8g2.begin();

  Serial.println("HealthPod Ready");
  Serial.println("Send 1 / 2 / 3 to dispense from compartment 1 / 2 / 3");
  Serial.println("Tune thresholds:");
  Serial.println("f / F => RAW_FULL +/- 10");
  Serial.println("m / M => RAW_MEDIUM +/- 10");
}

void loop() {
  int raw[3];
  for (int i = 0; i < 3; i++) {
    raw[i] = readSensorRaw(SENSOR_PINS[i]);
  }

  drawUI(raw);
  keepServosAtHomeIdle();

  // Print raw values every second
  static unsigned long lastPrint = 0;
  if (millis() - lastPrint > 1000) {
    lastPrint = millis();
    Serial.print("Raw: ");
    Serial.print(raw[0]);
    Serial.print("  ");
    Serial.print(raw[1]);
    Serial.print("  ");
    Serial.println(raw[2]);
  }

  if (Serial.available()) {
    char c = (char)Serial.read();

    // Dispense commands
    if (!busy && c >= '1' && c <= '3') {
      dispense((uint8_t)(c - '1'));
    }

    // Threshold tuning commands
    if (c == 'f') {
      RAW_FULL += 10;
      Serial.print("RAW_FULL = ");
      Serial.println(RAW_FULL);
    }

    if (c == 'F') {
      RAW_FULL -= 10;
      Serial.print("RAW_FULL = ");
      Serial.println(RAW_FULL);
    }

    if (c == 'm') {
      RAW_MEDIUM += 10;
      Serial.print("RAW_MEDIUM = ");
      Serial.println(RAW_MEDIUM);
    }

    if (c == 'M') {
      RAW_MEDIUM -= 10;
      Serial.print("RAW_MEDIUM = ");
      Serial.println(RAW_MEDIUM);
    }
  }
}
