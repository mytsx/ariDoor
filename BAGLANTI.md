# ariDoor - Pin Bağlantıları

## Arduino Nano

### Step Motor (28BYJ-48 + ULN2003)
| ULN2003 | Arduino Nano |
|---------|-------------|
| IN1     | Pin 8       |
| IN2     | Pin 9       |
| IN3     | Pin 10      |
| IN4     | Pin 11      |
| +       | 5V          |
| -       | GND         |

### DHT22 Sıcaklık ve Nem Sensörü (Kovan İçi)
| DHT22 Modül | Arduino Nano |
|-------------|-------------|
| VCC         | 5V          |
| DATA        | Pin 2       |
| GND         | GND         |

> Not: 3 pinli modül versiyonu kullanılıyor, pull-up direnç modül üzerinde mevcut.

## Kullanılan Pinler
- Pin 2  → DHT22 Sensör (DATA)
- Pin 8  → Step Motor IN1
- Pin 9  → Step Motor IN2
- Pin 10 → Step Motor IN3
- Pin 11 → Step Motor IN4

## Boş Pinler
- Pin 3, 4, 5, 6, 7, 12, 13, A0-A7
