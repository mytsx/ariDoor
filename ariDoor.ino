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
// 8-step half-step modu, daha tork ve akıcı hareket
// 28BYJ-48 half-step: 4096 adim/tur. Kalibrasyon: 150 mm = 9318 logik adim
// (Fiziksel ölçüm sonucu, teorik 9984 değil)
const int MAX_HIZ = 160;        // half-step/sn - kapalı uçtan açma testi için düşük hız
const int IVME = 50;             // half-step/sn² - ilk kalkışta adım kaçırmayı azaltır
const int TIK_ADIM = 128;        // SAG/SOL bir tıkta hareket (eski 64 full = 128 half)
const long MAX_LOGIK_ADIM = 9318; // 150 mm tam hareket
const float MAX_MM = 150.0;
const float SIC_KAPALI = 5.0;    // 5°C ve altı = tam kapalı
const float SIC_ACIK = 30.0;     // 30°C ve üstü = tam açık

// Mekanik asimetri telafisi
// Konvansiyon: 0 mm = açık, 150 mm = kapalı
// Doğrudan half-step sürücüde motor − yönü = kapanma, motor + yönü = açılma.
// Yani mm artış (kapanma) → motor − yönde, mm azalış (açılma) → motor + yönde.
const float ACMA_OLCEK = 0.876;  // motor + (açma), 150 mm açmada yaklaşık 1 mm fazla hareket düzeltildi
const float KAPAMA_OLCEK = 0.873; // motor − (kapama), 40 mm komut = 77 mm ölçümden kalibre edildi

// --- Motor Half-Step Dizisi ---
// BAGLANTI.md sırası kullanılır: IN1, IN2, IN3, IN4.
// Bu dizi AccelStepper soyutlamasını bypass eder; yön testinde + ve - kesin ters çalışmalı.
const byte MOTOR_DIZI[8][4] = {
  {1, 0, 0, 0},
  {1, 1, 0, 0},
  {0, 1, 0, 0},
  {0, 1, 1, 0},
  {0, 0, 1, 0},
  {0, 0, 1, 1},
  {0, 0, 0, 1},
  {1, 0, 0, 1}
};
int motorDiziIndeks = 0;
const unsigned int MOTOR_ADIM_GECIKME_US = 1250; // yaklaşık 800 half-step/sn

// --- Nesneler ---
DHT dht(DHT_PIN, DHT_TIP);
LiquidCrystal lcd(LCD_RS, LCD_E, LCD_D4, LCD_D5, LCD_D6, LCD_D7);

// --- Ozel Karakterler ---
byte derece[8] = {0x06, 0x09, 0x09, 0x06, 0x00, 0x00, 0x00, 0x00};

// --- Durum ---
// Kalibrasyon/test aşaması: başlangıçta sürgünün tamamen açık olduğu varsayılır.
// 0 mm = açık, 150 mm = kapalı. Fiziksel konum değişirse SETADIM komutuyla senkronla.
long logikAdim = 0;  // HTML'in gördüğü adim (0..9318 = 0..150 mm). Başlangıç = açık.

enum Mod { SENSOR, AYAR, KONTROL };
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
const unsigned long OTO_ARASI = 300000UL;  // SENSOR modunda 5 dk'da bir auto kontrol
const float MIN_HAREKET_MM = 2.0;
unsigned long sonOto = 0;
bool ilkOtoYapildi = false;  // İlk sensör okumasıyla auto kontrolü tetikle
bool otoAktif = true;

// --- Zamanlama ---
unsigned long sonOkuma = 0;
const unsigned long OKUMA_ARASI = 2000;

void setup() {
  Serial.begin(9600);
  pinMode(MOTOR_IN1, OUTPUT);
  pinMode(MOTOR_IN2, OUTPUT);
  pinMode(MOTOR_IN3, OUTPUT);
  pinMode(MOTOR_IN4, OUTPUT);
  motorCikisYaz(0);
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

  if (otoAktif && mevcutMod == SENSOR) {
    bool otoZamani = millis() - sonOto >= OTO_ARASI;
    bool ilkVeriHazir = !ilkOtoYapildi && !isnan(sonSic);
    if (otoZamani || ilkVeriHazir) {
      sonOto = millis();
      ilkOtoYapildi = true;
      pozisyonGuncelle(false);
    }
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
  Serial.print(",\"mm\":");
  Serial.print(adimToMm(logikAdim), 1);
  Serial.print(",\"mod\":\"");
  Serial.print(modAdi());
  Serial.print("\",\"hedefSic\":");
  Serial.print(hedefSic, 1);
  Serial.print(",\"hedefNem\":");
  Serial.print(hedefNem, 1);
  Serial.print(",\"oto\":");
  Serial.print(otoAktif ? "true" : "false");
  Serial.println("}");
}

void lcdGuncelle(float sic, float nem) {
  lcd.setCursor(0, 0);
  if (mevcutMod == KONTROL) {
    lcd.print("Kapi Kontrol   ");
  } else if (mevcutMod == AYAR) {
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
  if (mevcutMod == KONTROL) {
    lcd.print("Konum:");
    lcd.print(adimToMm(logikAdim), 1);
    lcd.print("mm   ");
  } else if (mevcutMod == AYAR) {
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
  hedef = constrain(hedef, 0L, MAX_LOGIK_ADIM);
  long fark = hedef - logikAdim;
  if (fark == 0) return;

  long fizikselFark;
  if (fark > 0) {
    // mm artış = kapanma → motor − yönde
    fizikselFark = -(long)(fark * KAPAMA_OLCEK);
  } else {
    // mm azalış = açılma → motor + yönde
    fizikselFark = -(long)(fark * ACMA_OLCEK);
  }
  motorHamHareket(fizikselFark);
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

  // Yön kontrol — AYAR modunda hedef değerleri, KONTROL modunda kapıyı ayarlar
  if (mevcutMod != AYAR && mevcutMod != KONTROL) return;
  if (millis() - sonAyar < JS_ARASI) return;

  int x = analogRead(JS_X);
  int y = analogRead(JS_Y);

  if (mevcutMod == KONTROL) {
    bool hareketEtti = false;
    if (x > 512 + JS_ESIK) {
      hareket(logikAdim + TIK_ADIM);
      hareketEtti = true;
    } else if (x < 512 - JS_ESIK) {
      hareket(logikAdim - TIK_ADIM);
      hareketEtti = true;
    }

    if (hareketEtti) {
      sonAyar = millis();
      adimBildir();
      lcdGuncelle(sonSic, sonNem);
    }
    return;
  }

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
    if (otoAktif) {
      pozisyonGuncelle(true);
    }
  }
}

void modDegistir() {
  Mod yeniMod;
  if (mevcutMod == SENSOR) {
    yeniMod = AYAR;
  } else if (mevcutMod == AYAR) {
    yeniMod = KONTROL;
  } else {
    yeniMod = SENSOR;
  }
  modAyarla(yeniMod);
}

void modAyarla(Mod yeniMod) {
  mevcutMod = yeniMod;
  modBildir();

  if (mevcutMod == SENSOR) {
    sensorOku();
    if (otoAktif) {
      pozisyonGuncelle(true);
      sonOto = millis();
      ilkOtoYapildi = true;
    }
  } else if (mevcutMod == AYAR) {
    lcdGuncelle(sonSic, sonNem);
    if (otoAktif) {
      pozisyonGuncelle(true);
    }
  } else {
    lcdGuncelle(sonSic, sonNem);
  }
}

void otoKontrol() {
  pozisyonGuncelle(false);
}

void pozisyonGuncelle(bool zorla) {
  if (mevcutMod == KONTROL) return;

  float kontrolSic = (mevcutMod == SENSOR) ? sonSic : hedefSic;
  if (isnan(kontrolSic)) return;

  long hedefAdimYeni = sicaklikToAdim(kontrolSic);
  long minFark = mmToAdim(MIN_HAREKET_MM);

  if (zorla || abs(hedefAdimYeni - logikAdim) >= minFark) {
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
  else if (komut.startsWith("RAW:")) {
    long adim = komut.substring(4).toInt();
    motorHamHareket(adim);
    rawBildir(adim);
  }
  else if (komut == "LEDTEST") {
    ledTest();
  }
  else if (komut.startsWith("GITMM:")) {
    float hedefMm = komut.substring(6).toFloat();
    hareket(mmToAdim(hedefMm));
    adimBildir();
  }
  else if (komut.startsWith("KAPATMM:")) {
    float mm = komut.substring(8).toFloat();
    hareket(logikAdim + mmToAdim(mm));
    adimBildir();
  }
  else if (komut.startsWith("ACMAMM:")) {
    float mm = komut.substring(7).toFloat();
    hareket(logikAdim - mmToAdim(mm));
    adimBildir();
  }
  else if (komut.startsWith("KAPATCM:")) {
    float cm = komut.substring(8).toFloat();
    hareket(logikAdim + mmToAdim(cm * 10.0));
    adimBildir();
  }
  else if (komut.startsWith("ACMACM:")) {
    float cm = komut.substring(7).toFloat();
    hareket(logikAdim - mmToAdim(cm * 10.0));
    adimBildir();
  }
  else if (komut.startsWith("SETADIM:")) {
    logikAdim = constrain(komut.substring(8).toInt(), 0L, MAX_LOGIK_ADIM);
    adimBildir();
  }
  else if (komut.startsWith("SETMM:")) {
    logikAdim = mmToAdim(komut.substring(6).toFloat());
    adimBildir();
  }
  else if (komut == "OTO:1") {
    otoAktif = true;
    otoBildir();
    if (mevcutMod == SENSOR) {
      sensorOku();
      pozisyonGuncelle(true);
      sonOto = millis();
      ilkOtoYapildi = true;
    } else if (mevcutMod == AYAR) {
      pozisyonGuncelle(true);
    }
  }
  else if (komut == "OTO:0") {
    otoAktif = false;
    otoBildir();
  }
  else if (komut == "MOD:SENSOR") {
    if (mevcutMod != SENSOR) modAyarla(SENSOR);
    else modBildir();
  }
  else if (komut == "MOD:AYAR") {
    if (mevcutMod != AYAR) modAyarla(AYAR);
    else modBildir();
  }
  else if (komut == "MOD:KONTROL") {
    if (mevcutMod != KONTROL) modAyarla(KONTROL);
    else modBildir();
  }
  else if (komut.startsWith("HEDEFSIC:")) {
    hedefSic = constrain(komut.substring(9).toFloat(), 0.0, 50.0);
    ayarBildir();
    if (otoAktif && mevcutMod == AYAR) {
      pozisyonGuncelle(true);
    }
  }
  else if (komut.startsWith("HEDEFNEM:")) {
    hedefNem = constrain(komut.substring(9).toFloat(), 0.0, 100.0);
    ayarBildir();
    if (otoAktif && mevcutMod == AYAR) {
      pozisyonGuncelle(true);
    }
  }
}

void modBildir() {
  Serial.print("{\"mod\":\"");
  Serial.print(modAdi());
  Serial.println("\"}");
}

const char* modAdi() {
  if (mevcutMod == SENSOR) return "SENSOR";
  if (mevcutMod == AYAR) return "AYAR";
  return "KONTROL";
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
  Serial.print(",\"mm\":");
  Serial.print(adimToMm(logikAdim), 1);
  Serial.println("}");
}

long mmToAdim(float mm) {
  mm = constrain(mm, 0.0, MAX_MM);
  return (long)(mm * MAX_LOGIK_ADIM / MAX_MM + 0.5);
}

long sicaklikToAdim(float sic) {
  return mmToAdim(sicaklikToMm(sic));
}

float sicaklikToMm(float sic) {
  if (sic <= SIC_KAPALI) return MAX_MM;
  if (sic >= SIC_ACIK) return 0.0;

  float oran = (SIC_ACIK - sic) / (SIC_ACIK - SIC_KAPALI);
  return MAX_MM * oran;
}

float adimToMm(long adim) {
  adim = constrain(adim, 0L, MAX_LOGIK_ADIM);
  return (float)adim * MAX_MM / (float)MAX_LOGIK_ADIM;
}

void otoBildir() {
  Serial.print("{\"oto\":");
  Serial.print(otoAktif ? "true" : "false");
  Serial.println("}");
}

void rawBildir(long adim) {
  Serial.print("{\"raw\":");
  Serial.print(adim);
  Serial.println("}");
}

void motorCikisYaz(int indeks) {
  indeks = (indeks + 8) % 8;
  digitalWrite(MOTOR_IN1, MOTOR_DIZI[indeks][0]);
  digitalWrite(MOTOR_IN2, MOTOR_DIZI[indeks][1]);
  digitalWrite(MOTOR_IN3, MOTOR_DIZI[indeks][2]);
  digitalWrite(MOTOR_IN4, MOTOR_DIZI[indeks][3]);
}

void motorHamHareket(long adim) {
  int yon = (adim >= 0) ? 1 : -1;
  unsigned long kalan = (adim >= 0) ? adim : -adim;

  while (kalan > 0) {
    motorDiziIndeks += yon;
    if (motorDiziIndeks >= 8) motorDiziIndeks = 0;
    if (motorDiziIndeks < 0) motorDiziIndeks = 7;
    motorCikisYaz(motorDiziIndeks);
    delayMicroseconds(MOTOR_ADIM_GECIKME_US);
    kalan--;
  }
}

void ledTest() {
  const int pinler[4] = {MOTOR_IN1, MOTOR_IN2, MOTOR_IN3, MOTOR_IN4};
  for (int tur = 0; tur < 2; tur++) {
    for (int i = 0; i < 4; i++) {
      digitalWrite(MOTOR_IN1, LOW);
      digitalWrite(MOTOR_IN2, LOW);
      digitalWrite(MOTOR_IN3, LOW);
      digitalWrite(MOTOR_IN4, LOW);
      digitalWrite(pinler[i], HIGH);
      Serial.print("{\"led\":\"");
      Serial.print(i == 0 ? "IN1/A" : i == 1 ? "IN2/B" : i == 2 ? "IN3/C" : "IN4/D");
      Serial.println("\"}");
      delay(1000);
    }
  }
  digitalWrite(MOTOR_IN1, LOW);
  digitalWrite(MOTOR_IN2, LOW);
  digitalWrite(MOTOR_IN3, LOW);
  digitalWrite(MOTOR_IN4, LOW);
  Serial.println("{\"ledtest\":\"bitti\"}");
}
