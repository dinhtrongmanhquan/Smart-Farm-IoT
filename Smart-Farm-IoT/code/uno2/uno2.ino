
#include <SoftwareSerial.h>

// ===== CHAN CAM BIEN =====
#define WATER_PIN     A1
#define GAS_AO_PIN    A2
#define GAS_DO_PIN    8

// ===== CHAN RELAY =====
#define RELAY_PUMP    7
#define RELAY_2       6

// UNO2 TX D10 -> UNO3 RX D10
SoftwareSerial linkUNO3(11, 10);

// ===== CAU HINH RELAY =====
// Relay active LOW
const int RELAY_ON  = LOW;
const int RELAY_OFF = HIGH;

// ===== NGUONG MUC NUOC =====
const int WATER_ON_THRESHOLD  = 100;
const int WATER_OFF_THRESHOLD = 500;

// ===== CHU KY TASK =====
const unsigned long WATER_TASK_MS  = 200;
const unsigned long SEND_TASK_MS   = 1000;
const unsigned long SERIAL_TASK_MS = 1000;

// ===== DU LIEU CAM BIEN =====
int waterValue = 0;
int gasAO = 0;
int gasDO = HIGH;

bool pumpState = false;

unsigned long lastWaterTask = 0;
unsigned long lastSendTask = 0;
unsigned long lastSerialTask = 0;

// ===== DOC CAM BIEN NUOC VA GAS =====
void taskWater() {
  waterValue = analogRead(WATER_PIN);
  gasAO = analogRead(GAS_AO_PIN);
  gasDO = digitalRead(GAS_DO_PIN);
}

// ===== DIEU KHIEN BOM =====
void setPump(bool turnOn) {
  if (turnOn == pumpState) {
    return;
  }

  pumpState = turnOn;

  digitalWrite(
    RELAY_PUMP,
    pumpState ? RELAY_ON : RELAY_OFF
  );

  Serial.println(pumpState ? "PUMP ON" : "PUMP OFF");
}

void taskPump() {
  if (waterValue < WATER_ON_THRESHOLD) {
    setPump(true);
  } else if (waterValue >= WATER_OFF_THRESHOLD) {
    setPump(false);
  }

  // Trong khoang 100-499: giu trang thai truoc do
}

// ===== GUI DU LIEU SANG UNO 3 =====
void taskSend() {
  // Format giu nguyen 7 truong:
  // T,H,Water,GasAO,GasDO,DHT_OK,Pump
  // DHT da chuyen sang UNO3, nen T/H = NAN, DHT_OK = 0

  linkUNO3.print("NAN,NAN,");

  linkUNO3.print(waterValue);
  linkUNO3.print(",");
  linkUNO3.print(gasAO);
  linkUNO3.print(",");
  linkUNO3.print(gasDO);
  linkUNO3.print(",0,");
  linkUNO3.println(pumpState ? 1 : 0);
}

// ===== HIEN THI SERIAL MONITOR =====
void taskSerial() {
  Serial.print("Water=");
  Serial.print(waterValue);

  Serial.print(" GasAO=");
  Serial.print(gasAO);

  Serial.print(" GasDO=");
  Serial.print(gasDO);

  Serial.print(" Pump=");
  Serial.println(pumpState ? "ON" : "OFF");
}

// ===== SETUP =====
void setup() {
  Serial.begin(9600);
  linkUNO3.begin(9600);

  pinMode(GAS_DO_PIN, INPUT);
  pinMode(RELAY_PUMP, OUTPUT);
  pinMode(RELAY_2, OUTPUT);

  // Tat relay luc khoi dong
  digitalWrite(RELAY_PUMP, RELAY_OFF);
  digitalWrite(RELAY_2, RELAY_OFF);

  waterValue = analogRead(WATER_PIN);
  gasAO = analogRead(GAS_AO_PIN);
  gasDO = digitalRead(GAS_DO_PIN);

  unsigned long now = millis();
  lastWaterTask = now;
  lastSendTask = now;
  lastSerialTask = now;

  Serial.println("UNO2 READY");
  Serial.println("DHT11 da chuyen sang UNO3");
  Serial.println("Water < 100: PUMP ON");
  Serial.println("Water >= 500: PUMP OFF");
}

// ===== LOOP: KHONG DUNG DELAY =====
void loop() {
  unsigned long now = millis();

  if (now - lastWaterTask >= WATER_TASK_MS) {
    lastWaterTask = now;
    taskWater();
  }

  taskPump();

  if (now - lastSendTask >= SEND_TASK_MS) {
    lastSendTask = now;
    taskSend();
  }

  if (now - lastSerialTask >= SERIAL_TASK_MS) {
    lastSerialTask = now;
    taskSerial();
  }
}
