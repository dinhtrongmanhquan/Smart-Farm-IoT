
#include <AccelStepper.h>
#include <ThreeWire.h>
#include <RtcDS1302.h>

// ===== CHAN DONG CO ULN2003 =====
#define IN1 2
#define IN2 3
#define IN3 4
#define IN4 5

// ===== RTC DS1302 =====
#define RTC_CLK 6
#define RTC_DAT 7
#define RTC_RST 8

// ===== CAC CHAN KHAC =====
#define LIGHT_SENSOR 9
#define LED_PIN 10
#define MOTOR_BUTTON 11
#define LIGHT_BUTTON 12
#define STATUS_TO_UNO3 13

// ===== RTC =====
ThreeWire rtcWire(RTC_DAT, RTC_CLK, RTC_RST);
RtcDS1302<ThreeWire> rtc(rtcWire);

// ===== DONG CO: QUAY NGUOC =====
AccelStepper stepper(
  AccelStepper::FULL4WIRE,
  IN1, IN3, IN2, IN4
);

const float MOTOR_SPEED = -300.0;

// ===== THOI GIAN =====
const unsigned long FEED_DURATION_MS = 180000UL;
const unsigned long FEED_INTERVAL_MS = 60000UL;
const unsigned long RETURN_AUTO_MS = 15000UL;

// HIGH = toi
const int DARK_LEVEL = HIGH;

// ===== TRANG THAI =====
bool feeding = false;
bool ledEnabled = false;
bool manualMode = false;
bool waitingToAuto = false;

bool lastMotorSwitch = HIGH;
bool lastLightButton = HIGH;

unsigned long feedStartTime = 0;
unsigned long lastFeedTime = 0;
unsigned long switchOffTime = 0;
unsigned long lastRtcPrint = 0;

// ===== DIEU KHIEN DEN =====
void handleLight() {
  bool dark = (digitalRead(LIGHT_SENSOR) == DARK_LEVEL);

  if (ledEnabled && dark) {
    digitalWrite(LED_PIN, HIGH);
  } else {
    digitalWrite(LED_PIN, LOW);
  }
}

// ===== DOC CONG TAC VA NUT DEN =====
void handleButtons() {
  // Cong tac D11 noi giua D11 va GND:
  // LOW = bat, HIGH = tat
  bool motorSwitch = digitalRead(MOTOR_BUTTON);
  bool lightButton = digitalRead(LIGHT_BUTTON);

  // Cong tac motor dang ON
  if (motorSwitch == LOW) {
    if (!manualMode) {
      Serial.println("THU CONG: DONG CO CHAY");
    }

    manualMode = true;
    waitingToAuto = false;
    feeding = false;
  } else {
    // Vua chuyen cong tac tu ON sang OFF
    if (lastMotorSwitch == LOW && motorSwitch == HIGH) {
      manualMode = false;
      waitingToAuto = true;
      switchOffTime = millis();
      feeding = false;

      Serial.println("DUNG DONG CO - CHO 15 GIAY");
    }
  }

  // Nut den D12: nhan de bat/tat che do den
  if (lastLightButton == HIGH && lightButton == LOW) {
    ledEnabled = !ledEnabled;

    Serial.print("Che do den: ");
    Serial.println(ledEnabled ? "BAT" : "TAT");
  }

  lastMotorSwitch = motorSwitch;
  lastLightButton = lightButton;
}

// ===== LICH CHO AN TU DONG =====
void handleFeedingSchedule() {
  unsigned long now = millis();

  // Khi cong tac dang bat, khong chay lich tu dong
  if (manualMode) {
    feeding = false;
    return;
  }

  // Sau khi tat cong tac, cho 15 giay roi moi tro ve tu dong
  if (waitingToAuto) {
    feeding = false;

    if (now - switchOffTime >= RETURN_AUTO_MS) {
      waitingToAuto = false;
      lastFeedTime = now;

      Serial.println("TRO VE CHE DO TU DONG");
    }

    return;
  }

  // Bat dau tinh chu ky tu dong
  if (lastFeedTime == 0) {
    lastFeedTime = now;
  }

  // Moi 60 giay bat dau mot chu ky cho an
  if (!feeding &&
      now - lastFeedTime >= FEED_INTERVAL_MS) {
    feeding = true;
    feedStartTime = now;
    lastFeedTime = now;

    Serial.println("BAT DAU CHO AN");
  }

  // Dung cho an sau 20 giay
  if (feeding &&
      now - feedStartTime >= FEED_DURATION_MS) {
    feeding = false;

    Serial.println("KET THUC CHO AN");
  }
}

// ===== HIEN THI THOI GIAN RTC =====
void printRtcTime() {
  if (millis() - lastRtcPrint < 1000UL) {
    return;
  }

  lastRtcPrint = millis();

  if (!rtc.IsDateTimeValid()) {
    Serial.println("RTC KHONG HOP LE");
    return;
  }

  RtcDateTime now = rtc.GetDateTime();

  char buf[30];

  snprintf(
    buf,
    sizeof(buf),
    "%02u/%02u/%04u %02u:%02u:%02u",
    now.Day(),
    now.Month(),
    now.Year(),
    now.Hour(),
    now.Minute(),
    now.Second()
  );

  Serial.println(buf);
}

// ===== KHOI DONG =====
void setup() {
  Serial.begin(9600);

  pinMode(LIGHT_SENSOR, INPUT);
  pinMode(LED_PIN, OUTPUT);

  pinMode(MOTOR_BUTTON, INPUT_PULLUP);
  pinMode(LIGHT_BUTTON, INPUT_PULLUP);

  pinMode(STATUS_TO_UNO3, OUTPUT);

  digitalWrite(LED_PIN, LOW);
  digitalWrite(STATUS_TO_UNO3, LOW);

  rtc.Begin();

  stepper.setMaxSpeed(500);
  stepper.setSpeed(MOTOR_SPEED);

  // Neu UNO khoi dong khi cong tac da ON,
  // nhan dien ngay trang thai cong tac.
  lastMotorSwitch = digitalRead(MOTOR_BUTTON);

  if (lastMotorSwitch == LOW) {
    manualMode = true;
  }

  Serial.println("UNO1 KHOI DONG");
  Serial.println("Motor cong tac ON: chay thu cong");
  Serial.println("Motor cong tac OFF: dung 15 giay");
  Serial.println("Sau do tro ve tu dong");
}

// ===== VONG LAP CHINH =====
void loop() {
  handleButtons();
  handleLight();
  handleFeedingSchedule();
  printRtcTime();

  // Uu tien cong tac thu cong
  if (manualMode) {
    stepper.setSpeed(MOTOR_SPEED);
    stepper.runSpeed();
  }
  // Tu dong chi chay trong chu ky cho an
  else if (feeding && !waitingToAuto) {
    stepper.setSpeed(MOTOR_SPEED);
    stepper.runSpeed();
  }
  // Cac truong hop con lai: dung motor
  else {
    stepper.setSpeed(0);
  }

  // Bao trang thai cho an sang UNO3
  digitalWrite(
    STATUS_TO_UNO3,
    (feeding && !manualMode && !waitingToAuto)
      ? HIGH : LOW
  );
}
