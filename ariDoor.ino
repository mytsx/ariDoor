/*
  28BYJ-48 Step Motor Test
  ------------------------
  Baglanti:
    ULN2003   Arduino Nano
    IN1  -->  Pin 8
    IN2  -->  Pin 9
    IN3  -->  Pin 10
    IN4  -->  Pin 11
    -    -->  GND
    +    -->  5V
*/

#include <Stepper.h>

// 28BYJ-48: 2048 adim = 1 tam tur
const int ADIM_PER_TUR = 2048;

// Pin sirasi: IN1, IN3, IN2, IN4 (28BYJ-48 icin ozel sira!)
Stepper motor(ADIM_PER_TUR, 8, 10, 9, 11);

void setup() {
  Serial.begin(9600);
  motor.setSpeed(10); // 10 RPM (yavas ve guvenli)

  Serial.println("=================================");
  Serial.println("  28BYJ-48 Step Motor Test");
  Serial.println("=================================");
  Serial.println();

  // Test 1: Yarim tur ileri
  Serial.println("[1/3] Yarim tur ileri donuyor...");
  motor.step(1024);
  Serial.println("  -> Tamamlandi.");
  delay(1000);

  // Test 2: Yarim tur geri
  Serial.println("[2/3] Yarim tur geri donuyor...");
  motor.step(-1024);
  Serial.println("  -> Tamamlandi.");
  delay(1000);

  // Test 3: Tam tur
  Serial.println("[3/3] Tam tur donuyor...");
  motor.step(2048);
  Serial.println("  -> Tamamlandi.");
  delay(500);

  Serial.println();
  Serial.println("=== TEST TAMAMLANDI ===");
  Serial.println("Motor duzgun dondu mu?");
}

void loop() {
}
