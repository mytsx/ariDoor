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

#include <AccelStepper.h>
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

#define JS_X    A0
#define JS_Y    A1
#define JS_SW   3

// --- Motor Ayarlari ---
// HALF4WIRE: 8-step half-step modu, daha tork ve akıcı hareket
// 28BYJ-48 half-step: 4096 adim/tur. Kalibrasyon: 150 mm = 9318 logik adim
// (Fiziksel ölçüm sonucu, teorik 9984 değil)
const int MAX_HIZ = 800;        // half-step/sn
const int IVME = 500;            // half-step/sn²
const int TIK_ADIM = 128;        // SAG/SOL bir tıkta hareket (eski 64 full = 128 half)

// Mekanik asimetri telafisi
// Konvansiyon: 0 mm = açık, 150 mm = kapalı
// Motor + yönü = sürgü RETRACT = açılma. Motor − yönü = sürgü EXTEND = kapanma.
// Yani mm azalış (açılma) → motor + yönde, mm artış (kapanma) → motor − yönde
const float ACMA_OLCEK = 0.89;   // motor + (açma), fazla itiyor → daha az bas
const float KAPAMA_OLCEK = 1.68; // motor − (kapama), az itiyor → daha çok bas

// --- Nesneler ---
// AccelStepper pin sırası: IN1, IN3, IN2, IN4 (28BYJ-48 için bobin sırası)
AccelStepper motor(AccelStepper::HALF4WIRE, MOTOR_IN1, MOTOR_IN3, MOTOR_IN2, MOTOR_IN4);
DHT dht(DHT_PIN, DHT_TIP);
LiquidCrystal lcd(LCD_RS, LCD_E, LCD_D4, LCD_D5, LCD_D6, LCD_D7);

// --- Ozel Karakterler ---
byte derece[8] = {0x06, 0x09, 0x09, 0x06, 0x00, 0x00, 0x00, 0x00};

// --- Durum ---
// Başlangıçta sürgünün tamamen kapalı olduğu varsayılır (150 mm = kapalı).
// Kullanıcı cihazı çalıştırmadan önce sürgüyü manuel olarak kapalı konuma getirmeli.
// İlk sensör okumasıyla otomatik kontrol tetiklenir ve sürgü o anki koşullara göre konumlanır.
long logikAdim = 9318;  // HTML'in gördüğü adim (0..9318 = 0..150 mm). Başlangıç = kapalı.

enum Mod { SENSOR, AYAR };
Mod mevcutMod = SENSOR;

float hedefSic = 30.0;  // Bu sıcaklık ve üstünde sürgü tamamen açık
float hedefNem = 50.0;

float sonSic = NAN;     // Son okunan değerler (auto kontrol için)
float sonNem = NAN;

// --- Joystick ---
const int JS_ESIK = 200;          // Merkez 512'den ±200 = aktif yön
const unsigned long JS_ARASI = 300;  // Yön kontrol periyodu (ms)
unsigned long sonAyar = 0;

bool sonButonDurum = HIGH;
unsigned long sonButonZaman = 0;
const unsigned long BUTON_DEBOUNCE = 50;

// --- Auto kontrol ---
const unsigned long OTO_ARASI = 10000;  // 10 sn'de bir auto kontrol
const float MIN_HAREKET_MM = 2.0;
unsigned long sonOto = 0;
bool ilkOtoYapildi = false;  // İlk sensör okumasıyla auto kontrolü tetikle

// --- Zamanlama ---
unsigned long sonOkuma = 0;
const unsigned long OKUMA_ARASI = 2000;

void setup() {
  Serial.begin(9600);
  motor.setMaxSpeed(MAX_HIZ);
  motor.setAcceleration(IVME);
  dht.begin();

  pinMode(JS_SW, INPUT_PULLUP);

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

  joystickKontrol();

  bool otoZamani = millis() - sonOto >= OTO_ARASI;
  bool ilkVeriHazir = !ilkOtoYapildi && !isnan(sonSic) && !isnan(sonNem);
  if (otoZamani || ilkVeriHazir) {
    sonOto = millis();
    ilkOtoYapildi = true;
    otoKontrol();
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
  sonSic = sic;
  sonNem = nem;

  lcdGuncelle(sic, nem);

  // Serial JSON
  Serial.print("{\"sic\":");
  if (isnan(sic)) Serial.print("null"); else Serial.print(sic, 1);
  Serial.print(",\"nem\":");
  if (isnan(nem)) Serial.print("null"); else Serial.print(nem, 1);
  Serial.print(",\"adim\":");
  Serial.print(logikAdim);
  Serial.print(",\"mod\":\"");
  Serial.print(mevcutMod == SENSOR ? "SENSOR" : "AYAR");
  Serial.print("\",\"hedefSic\":");
  Serial.print(hedefSic, 1);
  Serial.print(",\"hedefNem\":");
  Serial.print(hedefNem, 1);
  Serial.println("}");
}

void lcdGuncelle(float sic, float nem) {
  lcd.setCursor(0, 0);
  if (mevcutMod == AYAR) {
    lcd.print("Hed: ");
    lcd.print(hedefSic, 1);
    lcd.write(byte(0));
    lcd.print("C ");
  } else if (isnan(sic)) {
    lcd.print("Sic: --.-     ");
  } else {
    lcd.print("Sic: ");
    lcd.print(sic, 1);
    lcd.write(byte(0));
    lcd.print("C    ");
  }

  lcd.setCursor(0, 1);
  if (mevcutMod == AYAR) {
    lcd.print("Hed: ");
    lcd.print(hedefNem, 1);
    lcd.print("%      ");
  } else if (isnan(nem)) {
    lcd.print("Nem: --.-     ");
  } else {
    lcd.print("Nem: ");
    lcd.print(nem, 1);
    lcd.print("%      ");
  }
}

void hareket(long hedef) {
  long fark = hedef - logikAdim;
  if (fark == 0) return;

  long fizikselFark;
  if (fark > 0) {
    // mm artış = kapanma → motor − yönde
    fizikselFark = -(long)(fark * KAPAMA_OLCEK);
  } else {
    // mm azalış = açılma → motor + yönde (fark negatif, − ile pozitif olur)
    fizikselFark = -(long)(fark * ACMA_OLCEK);
  }
  motor.move(fizikselFark);
  motor.runToPosition();
  logikAdim = hedef;
}

void joystickKontrol() {
  // Buton kontrol (debounced)
  bool butonDurum = digitalRead(JS_SW);
  if (butonDurum == LOW && sonButonDurum == HIGH &&
      millis() - sonButonZaman > BUTON_DEBOUNCE) {
    modDegistir();
    sonButonZaman = millis();
  }
  sonButonDurum = butonDurum;

  // Yön kontrol — sadece AYAR modunda hedef değerleri ayarlar
  if (mevcutMod != AYAR) return;
  if (millis() - sonAyar < JS_ARASI) return;

  int x = analogRead(JS_X);
  int y = analogRead(JS_Y);
  bool degisti = false;

  if (x > 512 + JS_ESIK) {
    hedefSic += 5.0;
    degisti = true;
  } else if (x < 512 - JS_ESIK) {
    hedefSic -= 5.0;
    degisti = true;
  }

  if (y < 512 - JS_ESIK) {
    hedefNem += 5.0;
    degisti = true;
  } else if (y > 512 + JS_ESIK) {
    hedefNem -= 5.0;
    degisti = true;
  }

  if (degisti) {
    hedefSic = constrain(hedefSic, 0.0, 50.0);
    hedefNem = constrain(hedefNem, 0.0, 100.0);
    sonAyar = millis();
    ayarBildir();
  }
}

void modDegistir() {
  mevcutMod = (mevcutMod == SENSOR) ? AYAR : SENSOR;
  modBildir();
  if (!isnan(sonSic) || !isnan(sonNem) || mevcutMod == AYAR) {
    lcdGuncelle(sonSic, sonNem);
  }
}

void otoKontrol() {
  if (isnan(sonSic)) return;

  // Lineer mapping: 5°C → 150 mm (kapalı), hedefSic → 0 mm (açık)
  int hedefMm;
  if (sonSic <= 5.0) {
    hedefMm = 150;
  } else if (sonSic >= hedefSic) {
    hedefMm = 0;
  } else {
    float oran = (hedefSic - sonSic) / (hedefSic - 5.0);
    hedefMm = (int)(150.0 * oran);
  }
  hedefMm = constrain(hedefMm, 0, 150);

  long hedefAdimYeni = (long)(hedefMm * 9318L / 150);
  long minFark = (long)(MIN_HAREKET_MM * 9318L / 150);

  if (abs(hedefAdimYeni - logikAdim) >= minFark) {
    hareket(hedefAdimYeni);
    adimBildir();
  }
}

void komutIsle(String komut) {
  if (komut == "SAG") {
    hareket(logikAdim + TIK_ADIM);
    adimBildir();
  }
  else if (komut == "SOL") {
    hareket(logikAdim - TIK_ADIM);
    adimBildir();
  }
  else if (komut == "SIFIR") {
    hareket(0);
    adimBildir();
  }
  else if (komut.startsWith("GIT:")) {
    long hedef = komut.substring(4).toInt();
    hareket(hedef);
    adimBildir();
  }
  else if (komut == "MOD:SENSOR") {
    if (mevcutMod != SENSOR) modDegistir();
    else modBildir();
  }
  else if (komut == "MOD:AYAR") {
    if (mevcutMod != AYAR) modDegistir();
    else modBildir();
  }
  else if (komut.startsWith("HEDEFSIC:")) {
    hedefSic = constrain(komut.substring(9).toFloat(), 0.0, 50.0);
    ayarBildir();
  }
  else if (komut.startsWith("HEDEFNEM:")) {
    hedefNem = constrain(komut.substring(9).toFloat(), 0.0, 100.0);
    ayarBildir();
  }
}

void modBildir() {
  Serial.print("{\"mod\":\"");
  Serial.print(mevcutMod == SENSOR ? "SENSOR" : "AYAR");
  Serial.println("\"}");
}

void ayarBildir() {
  Serial.print("{\"hedefSic\":");
  Serial.print(hedefSic, 1);
  Serial.print(",\"hedefNem\":");
  Serial.print(hedefNem, 1);
  Serial.println("}");
}

void adimBildir() {
  Serial.print("{\"adim\":");
  Serial.print(logikAdim);
  Serial.println("}");
}
