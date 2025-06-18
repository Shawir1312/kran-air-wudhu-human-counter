#define BLYNK_TEMPLATE_ID "TMPL6256nIkxs"
#define BLYNK_TEMPLATE_NAME "MUSHAWIR ODEGOA"
#define BLYNK_AUTH_TOKEN "J5qcZsBevQbw0Xbiu8x0keQsy2lnvuZf"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

char ssid[] = "S.NET(HOME)";
char pass[] = "admin112233";

#define trigPin 19
#define echoPin 18
#define relayPin 23
#define ledstatus 26

#define VP_COUNT V0
#define VP_RESET V1
#define VP_MANUAL V2

int wudhuCount = 0;
bool kranAktif = false;
bool manualMode = false;
bool sedangDeteksi = false;
unsigned long lastDetectTime = 0;

LiquidCrystal_I2C lcd(0x27, 16, 2);
BlynkTimer timer;

long readDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long duration = pulseIn(echoPin, HIGH, 30000);
  return duration * 0.034 / 2;
}

void setRelay(bool state) {
  digitalWrite(relayPin, state ? LOW : HIGH);  // Aktif LOW
  Blynk.virtualWrite(VP_MANUAL, state ? 1 : 0);
}

BLYNK_WRITE(VP_RESET) {
  if (param.asInt() == 1) {
    wudhuCount = 0;
    Blynk.virtualWrite(VP_COUNT, wudhuCount);
    Serial.println("Jumlah orang direset");
  }
}

BLYNK_WRITE(VP_MANUAL) {
  int val = param.asInt();
  manualMode = val == 1;
  setRelay(manualMode);
  if (manualMode) {
    Serial.println("Mode manual AKTIF - Relay ON");
  } else {
    Serial.println("Mode manual NONAKTIF - Relay OFF");
  }
}

enum State { IDLE,
             MENDEKAT,
             MENJAUH };
State sensorState = IDLE;

void checkSensor() {
  if (manualMode) return;

  long distance = readDistance();
  Serial.print("Jarak: ");
  Serial.println(distance);

  const int JARAK_DETEKSI = 23;

  switch (sensorState) {
    case IDLE:
      if (distance < JARAK_DETEKSI) {
        sensorState = MENDEKAT;
        lastDetectTime = millis();
      }
      break;

    case MENDEKAT:
      if (distance > JARAK_DETEKSI) {
        sensorState = MENJAUH;

        if (!kranAktif) {
          // Orang baru masuk
          wudhuCount++;
          Blynk.virtualWrite(VP_COUNT, wudhuCount);
          setRelay(true);
          kranAktif = true;
          digitalWrite(ledstatus,HIGH);
          Serial.println("Orang MASUK - kran ON");
        } else {
          // Orang keluar
          setRelay(false);
          kranAktif = false;
          digitalWrite(ledstatus,LOW);
          Serial.println("Orang KELUAR - kran OFF");
        }
      }
      break;

    case MENJAUH:
      if (distance > JARAK_DETEKSI + 5) {
        sensorState = IDLE;  // Reset ke IDLE setelah keluar
      }
      break;
  }
}

void tampilkan_orang() {
  lcd.setCursor(2, 0);
  lcd.print("JUMLAH ORANG");
  lcd.setCursor(8, 1);
  lcd.print(wudhuCount);
}

void setup() {
  Serial.begin(115200);
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("NAMA");
  lcd.setCursor(0, 1);
  lcd.print("NPM");
  delay(3000);
  lcd.clear();
  pinMode(ledstatus, OUTPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(relayPin, OUTPUT);
  setRelay(false);

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  timer.setInterval(200L, checkSensor);
}

void loop() {
  Blynk.run();
  timer.run();
  tampilkan_orang();
}
