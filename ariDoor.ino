/*
  ariDoor - Ari Kovani Kapi Surgu Sistemi
  ----------------------------------------
  Step motor + DHT22 sicaklik/nem + LCD 1602A
  Web arayuzu uzerinden serial ile kontrol edilir.

  Baglanti:
    Step Motor (28BYJ-48 + ULN2003):
      IN1->Pin8, IN2->Pin9, IN3->Pin10, IN4->Pin11, +->5V, -->GND

    DHT22 Sensor (3 pinli modul, pull-up dahili):
      VCC->5V, DATA->Pin2, GND->GND

    LCD 1602A:
      RS->Pin12, E->Pin13, D4->Pin4, D5->Pin5, D6->Pin6, D7->Pin7
      VSS->GND, VDD->5V, RW->GND, V0->Pot orta, A->5V, K->GND
*/

#include <Stepper.h>
#include <DHT.h>
#include <LiquidCrystal.h>

// --- Pin Tanimlari ---
#define DHT_PIN 2
#define DHT_TIP DHT22

#define MOTOR_IN1 8
#define MOTOR_IN2 9
#define MOTOR_IN3 10
#define MOTOR_IN4 11

#define LCD_RS 12
#define LCD_E  13
#define LCD_D4 4
#define LCD_D5 5
#define LCD_D6 6
#define LCD_D7 7

// --- Motor Ayarlari ---
const int ADIM_PER_TUR = 2048;
const int MOTOR_HIZ = 10; // RPM

// --- Nesneler ---
Stepper motor(ADIM_PER_TUR, MOTOR_IN1, MOTOR_IN3, MOTOR_IN2, MOTOR_IN4);
DHT dht(DHT_PIN, DHT_TIP);
LiquidCrystal lcd(LCD_RS, LCD_E, LCD_D4, LCD_D5, LCD_D6, LCD_D7);

// --- Ozel Karakterler ---
byte derece[8] = {0x06, 0x09, 0x09, 0x06, 0x00, 0x00, 0x00, 0x00};

// --- Durum ---
long mevcutAdim = 0;

// --- Zamanlama ---
unsigned long sonOkuma = 0;
const unsigned long OKUMA_ARASI = 2000;

void setup() {
  Serial.begin(9600);
  motor.setSpeed(MOTOR_HIZ);
  dht.begin();

  lcd.begin(16, 2);
  lcd.createChar(0, derece);
  lcd.setCursor(0, 0);
  lcd.print("ariDoor v1.0");
  lcd.setCursor(0, 1);
  lcd.print("Baslatiliyor...");
  delay(1000);
  lcd.clear();

  Serial.println("{\"durum\":\"hazir\"}");
}

void loop() {
  if (millis() - sonOkuma >= OKUMA_ARASI) {
    sonOkuma = millis();
    sensorOku();
  }

  if (Serial.available() > 0) {
    String komut = Serial.readStringUntil('\n');
    komut.trim();
    komutIsle(komut);
  }
}

void sensorOku() {
  float sic = dht.readTemperature();
  float nem = dht.readHumidity();

  // LCD guncelle
  lcd.setCursor(0, 0);
  if (isnan(sic)) {
    lcd.print("Sic: --.- ");
  } else {
    lcd.print("Sic: ");
    lcd.print(sic, 1);
    lcd.write(byte(0));
    lcd.print("C ");
  }

  lcd.setCursor(0, 1);
  if (isnan(nem)) {
    lcd.print("Nem: --.- ");
  } else {
    lcd.print("Nem: ");
    lcd.print(nem, 1);
    lcd.print("%  ");
  }

  // Serial JSON
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
