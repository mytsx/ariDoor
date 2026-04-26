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

### LCD 1602A Ekran (HY-1602F6)
| LCD Pin | Bağlantı            |
|---------|---------------------|
| VSS (1) | GND                 |
| VDD (2) | 5V                  |
| V0  (3) | Potansiyometre orta bacak (kontrast) |
| RS  (4) | Pin 12              |
| RW  (5) | GND                 |
| E   (6) | Pin 13              |
| D4  (11)| Pin 4               |
| D5  (12)| Pin 5               |
| D6  (13)| Pin 6               |
| D7  (14)| Pin 7               |
| A   (15)| 5V (arka ışık +)    |
| K   (16)| GND (arka ışık -)   |

> Not: 4-bit modda kullanılıyor (D0-D3 boş kalıyor). Kontrast için 10kΩ B10K potansiyometre; uçları 5V ve GND, orta bacak V0'a.

## Kullanılan Pinler
- Pin 2  → DHT22 Sensör (DATA)
- Pin 4  → LCD D4
- Pin 5  → LCD D5
- Pin 6  → LCD D6
- Pin 7  → LCD D7
- Pin 8  → Step Motor IN1
- Pin 9  → Step Motor IN2
- Pin 10 → Step Motor IN3
- Pin 11 → Step Motor IN4
- Pin 12 → LCD RS
- Pin 13 → LCD E

## Boş Pinler
- Pin 3, A0-A7
