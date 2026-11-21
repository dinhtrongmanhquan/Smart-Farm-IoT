
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>
#include <DHT.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>

// ===== KHAI BAO CHAN =====
#define BUZZER_PIN 9
#define UNO1_STATUS_PIN 12

#define UNO2_RX_PIN 10
#define UNO2_TX_PIN 11

#define DHT_PIN 3
#define DHT_TYPE DHT11

#define BUZZER_FREQ 2500

// LCD I2C 16x2, dia chi 0x27
LiquidCrystal_I2C lcd(0x27, 16, 2);

// UNO 3 nhan du lieu tu UNO 2
SoftwareSerial linkSerial(UNO2_RX_PIN, UNO2_TX_PIN);

// DHT11 doc truc tiep tren UNO3
DHT dht(DHT_PIN, DHT_TYPE);

// ===== DU LIEU CAM BIEN =====
float temperature = NAN;
float humidity = NAN;

int waterValue = -1;
int gasAnalog = -1;
int gasDigital = HIGH;

bool dhtOK = false;
bool pumpOn = false;

bool dataValid = false;
unsigned long lastDataTime = 0;

// ===== THOI GIAN TASK =====
unsigned long lastDHTRead = 0;
const unsigned long DHT_INTERVAL = 2000;

unsigned long lastLcdUpdate = 0;
const unsigned long LCD_INTERVAL = 2000;

byte lcdPage = 0;

// ===== DOC MOT DONG CSV TU UNO 2 =====
void readUNO2Data() {
  static char buffer[100];
  static byte index = 0;

  while (linkSerial.available()) {
    char c = linkSerial.read();

    if (c == '\r') {
      continue;
    }

    if (c == '\n') {
      buffer[index] = '\0';
      index = 0;
      parseUNO2Data(buffer);
    } else {
      if (index < sizeof(buffer) - 1) {
        buffer[index++] = c;
      } else {
        // Dong qua dai: bo dong loi
        index = 0;
      }
    }
  }
}

// Du lieu CSV:
// NAN,NAN,Water,GasAO,GasDO,0,Pump
void parseUNO2Data(char *line) {
  char *fields[7];
  byte count = 0;

  char *token = strtok(line, ",");

  while (token != NULL && count < 7) {
    fields[count++] = token;
    token = strtok(NULL, ",");
  }

  if (count != 7) {
    return;
  }

  int newWater = atoi(fields[2]);
  int newGasAnalog = atoi(fields[3]);
  int newGasDigital = atoi(fields[4]);

  bool newPumpOn =
      (strcmp(fields[6], "1") == 0 ||
       strcmp(fields[6], "ON") == 0 ||
       strcmp(fields[6], "on") == 0);

  // Kiem tra du lieu cam bien
  if (newWater < 0 || newWater > 1023 ||
      newGasAnalog < 0 || newGasAnalog > 1023 ||
      (newGasDigital != LOW && newGasDigital != HIGH)) {
    return;
  }

  waterValue = newWater;
  gasAnalog = newGasAnalog;
  gasDigital = newGasDigital;
  pumpOn = newPumpOn;

  // Chi cap nhat ket noi UNO2.
  // DHT11 doc truc tiep tren UNO3, khong lay DHT_OK tu UNO2.
  dataValid = true;
  lastDataTime = millis();
}

// ===== DOC DHT11 TREN UNO 3 =====
void readDHT11() {
  if (millis() - lastDHTRead < DHT_INTERVAL) {
    return;
  }

  lastDHTRead = millis();

  float newTemperature = dht.readTemperature();
  float newHumidity = dht.readHumidity();

  if (!isnan(newTemperature) && !isnan(newHumidity)) {
    temperature = newTemperature;
    humidity = newHumidity;
    dhtOK = true;
  } else {
    dhtOK = false;
    temperature = NAN;
    humidity = NAN;
  }
}

// ===== KIEM TRA KET NOI UNO 2 =====
bool communicationOK() {
  return dataValid &&
         (millis() - lastDataTime <= 3000);
}

// ===== DIEU KHIEN COI CANH BAO =====
void updateBuzzer() {
  bool connected = communicationOK();

  // Module gas kich hoat muc LOW
  bool gasAlarm =
      connected && (gasDigital == LOW);

  bool temperatureAlarm =
      dhtOK &&
      !isnan(temperature) &&
      temperature > 40.0;

  if (gasAlarm || temperatureAlarm) {
    tone(BUZZER_PIN, BUZZER_FREQ);
  } else {
    noTone(BUZZER_PIN);
  }
}

// ===== HIEN THI LCD =====
void showLCD() {
  lcd.clear();

  if (!communicationOK()) {
    lcd.setCursor(0, 0);
    lcd.print("UNO2 MAT KET NOI");

    lcd.setCursor(0, 1);
    lcd.print("Kiem tra D10/GND");
    return;
  }

  switch (lcdPage) {
    case 0:
      lcd.setCursor(0, 0);
      lcd.print("Nhiet:");

      if (dhtOK && !isnan(temperature)) {
        lcd.print(temperature, 1);
        lcd.print((char)223);
        lcd.print("C");
      } else {
        lcd.print("LOI DHT");
      }

      lcd.setCursor(0, 1);
      lcd.print("Do am:");

      if (dhtOK && !isnan(humidity)) {
        lcd.print(humidity, 1);
        lcd.print("%");
      } else {
        lcd.print("--");
      }
      break;

    case 1:
      lcd.setCursor(0, 0);
      lcd.print("Nuoc:");
      lcd.print(waterValue);

      lcd.setCursor(0, 1);
      lcd.print("Bom:");
      lcd.print(pumpOn ? "BAT" : "TAT");
      break;

    case 2:
      lcd.setCursor(0, 0);
      lcd.print("Gas AO:");
      lcd.print(gasAnalog);

      lcd.setCursor(0, 1);
      lcd.print("Gas DO:");
      lcd.print(gasDigital == LOW ? "CANH BAO" : "BINH THUONG");
      break;

    case 3:
      lcd.setCursor(0, 0);
      lcd.print("UNO1:");

      if (digitalRead(UNO1_STATUS_PIN) == HIGH) {
        lcd.print("DANG CHO AN");
      } else {
        lcd.print("BINH THUONG");
      }

      lcd.setCursor(0, 1);

      if (!dhtOK) {
        lcd.print("LOI CAM BIEN DHT");
      } else if (gasDigital == LOW ||
                 temperature > 40.0) {
        lcd.print("CANH BAO!");
      } else {
        lcd.print("He thong binh thuong");
      }
      break;
  }
}

// ===== SETUP =====
void setup() {
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(UNO1_STATUS_PIN, INPUT);

  noTone(BUZZER_PIN);

  Serial.begin(9600);
  linkSerial.begin(9600);

  dht.begin();

  lcd.init();
  lcd.backlight();

  lcd.setCursor(0, 0);
  lcd.print("TRANG TRAI");
  lcd.setCursor(0, 1);
  lcd.print("DANG KHOI DONG");
  delay(1500);

  lcd.clear();

  Serial.println("UNO 3 san sang");
  Serial.println("DHT11 DATA: D3");
  Serial.println("UNO2: NAN,NAN,Water,GasAO,GasDO,0,Pump");
}

// ===== LOOP =====
void loop() {
  readUNO2Data();
  readDHT11();

  updateBuzzer();

  unsigned long now = millis();

  if (now - lastLcdUpdate >= LCD_INTERVAL) {
    lastLcdUpdate = now;

    showLCD();

    lcdPage++;
    if (lcdPage > 3) {
      lcdPage = 0;
    }
  }
}
