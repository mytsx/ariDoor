/*
  ariDoor - Ari Kovani Kapi Surgu Sistemi
  ----------------------------------------
  Step motor + DHT22 sicaklik/nem sensoru
  Web arayuzu uzerinden serial ile kontrol edilir.

  Baglanti:
    Step Motor (28BYJ-48 + ULN2003):
      IN1->Pin8, IN2->Pin9, IN3->Pin10, IN4->Pin11, +->5V, -->GND

    DHT22 Sensor (3 pinli modul, pull-up dahili):
      VCC->5V, DATA->Pin2, GND->GND
*/

#include <Stepper.h>
#include <DHT.h>

// --- Pin Tanimlari ---
#define DHT_PIN 2
#define DHT_TIP DHT22

#define MOTOR_IN1 8
#define MOTOR_IN2 9
#define MOTOR_IN3 10
#define MOTOR_IN4 11

// --- Motor Ayarlari ---
const int ADIM_PER_TUR = 2048;
const int MOTOR_HIZ = 10; // RPM

// --- Nesneler ---
Stepper motor(ADIM_PER_TUR, MOTOR_IN1, MOTOR_IN3, MOTOR_IN2, MOTOR_IN4);
DHT dht(DHT_PIN, DHT_TIP);

// --- Durum ---
long mevcutAdim = 0;

// --- Zamanlama ---
unsigned long sonOkuma = 0;
const unsigned long OKUMA_ARASI = 2000; // 2 saniyede bir sensor oku

void setup() {
  Serial.begin(9600);
  motor.setSpeed(MOTOR_HIZ);
  dht.begin();

  Serial.println("{\"durum\":\"hazir\"}");
}

void loop() {
  // Sensor okuma (2 saniyede bir)
  if (millis() - sonOkuma >= OKUMA_ARASI) {
    sonOkuma = millis();
    sensorOku();
  }

  // Serial komut dinle
  if (Serial.available() > 0) {
    String komut = Serial.readStringUntil('\n');
    komut.trim();
    komutIsle(komut);
  }
}

void sensorOku() {
  float sic = dht.readTemperature();
  float nem = dht.readHumidity();

  Serial.print("{\"sic\":");
  if (isnan(sic)) Serial.print("null"); else Serial.print(sic, 1);
  Serial.print(",\"nem\":");
  if (isnan(nem)) Serial.print("null"); else Serial.print(nem, 1);
  Serial.print(",\"adim\":");
  Serial.print(mevcutAdim);
  Serial.println("}");
}

void komutIsle(String komut) {
  if (komut == "SAG") {
    motor.step(64);
    mevcutAdim += 64;
    adimBildir();
  }
  else if (komut == "SOL") {
    motor.step(-64);
    mevcutAdim -= 64;
    adimBildir();
  }
  else if (komut == "SIFIR") {
    if (mevcutAdim != 0) {
      motor.step(-mevcutAdim);
      mevcutAdim = 0;
    }
    adimBildir();
  }
  else if (komut.startsWith("GIT:")) {
    long hedef = komut.substring(4).toInt();
    long fark = hedef - mevcutAdim;
    if (fark != 0) {
      motor.step(fark);
      mevcutAdim = hedef;
    }
    adimBildir();
  }
}

void adimBildir() {
  Serial.print("{\"adim\":");
  Serial.print(mevcutAdim);
  Serial.println("}");
}
