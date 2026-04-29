# ariDoor - Pin Bağlantıları

Görsel bağlantı şeması: [`tinkercad-baglanti-semasi.html`](tinkercad-baglanti-semasi.html)

## Güç Hattı

İki ayrı güç hattı var. Sebebi: step motorun bobin kapanma/açılması Arduino'nun 5V hattında voltaj sıçramaları (inductive kickback) üretip DHT22'yi yakıyordu. Motor ve LCD arka ışığı ayrı bir hatta alındı.

### Arduino 5V hattı (hassas bileşenler)
- DHT22 VCC
- LCD VDD (mantık devresi)
- Potansiyometre (kontrast referansı)

### Harici 5V hattı (9V pil → regülatör → 5V)
- ULN2003 `+` (motor gücü)
- LCD `A` (arka ışık)

### Ortak GND
**Tüm GND'ler ortak rayda birleşir** — Arduino GND, harici regülatör `-`, ULN2003 `-`, LCD VSS, LCD K, DHT22 GND. Ortak GND olmadan ULN2003 sinyal referansını kaybeder.

## Arduino Nano

### Step Motor (28BYJ-48 + ULN2003)
| ULN2003 | Bağlantı            |
|---------|---------------------|
| IN1     | Arduino Pin 8       |
| IN2     | Arduino Pin 9       |
| IN3     | Arduino Pin 10      |
| IN4     | Arduino Pin 11      |
| +       | Harici 5V (regülatör çıkışı) |
| -       | Ortak GND           |

### DHT22 Sıcaklık ve Nem Sensörü (Kovan İçi)
| DHT22 Modül | Bağlantı           |
|-------------|--------------------|
| VCC         | Arduino 5V         |
| DATA        | Arduino Pin 2      |
| GND         | Ortak GND          |

> Not: 3 pinli modül versiyonu, pull-up direnç modül üzerinde mevcut.
> **Önemli:** DHT22 mutlaka **Arduino 5V'undan** beslenmeli (motor hattından DEĞİL). Aksi halde motor spike'larından zarar görür.

### LCD 1602A Ekran (HY-1602F6)
| LCD Pin | Bağlantı                          |
|---------|-----------------------------------|
| VSS (1) | Ortak GND                         |
| VDD (2) | Arduino 5V                        |
| V0  (3) | Potansiyometre orta bacak         |
| RS  (4) | Arduino Pin 12                    |
| RW  (5) | Ortak GND                         |
| E   (6) | Arduino Pin 13                    |
| D4  (11)| Arduino Pin 4                     |
| D5  (12)| Arduino Pin 5                     |
| D6  (13)| Arduino Pin 6                     |
| D7  (14)| Arduino Pin 7                     |
| A   (15)| Harici 5V (arka ışık +)           |
| K   (16)| Ortak GND (arka ışık -)           |

> Not: 4-bit modda kullanılıyor (D0-D3 boş kalıyor). Mantık devresi (VDD) Arduino 5V'unda, arka ışık (A) harici 5V'da.

### Potansiyometre 10kΩ B10K (LCD Kontrast)
| Potansiyometre Bacak | Bağlantı                |
|----------------------|-------------------------|
| Sol bacak            | Arduino 5V              |
| Orta bacak (wiper)   | LCD V0 (Pin 3)          |
| Sağ bacak            | Ortak GND               |

> Not: Sol/sağ uç bağlantıları yer değiştirebilir, sadece dönüş yönü ters olur. Çevirerek kontrastı ayarla.

### HW-504 Joystick (Mod ve Hedef Ayarı)
| HW-504 Pin | Bağlantı       | Görev                                   |
|------------|----------------|-----------------------------------------|
| GND        | Ortak GND      | Toprak                                  |
| +5V        | Arduino 5V     | Güç (motor hattına DEĞİL)               |
| VRx        | Arduino A0     | X ekseni — sıcaklık ayarı (sağ/sol)     |
| VRy        | Arduino A1     | Y ekseni — nem ayarı (yukarı/aşağı)     |
| SW         | Arduino Pin 3  | Buton — mod geçişi (SENSOR ↔ AYAR)      |

> Not: Buton dahili pull-up ile çalışır (`INPUT_PULLUP`), dış direnç gerekmez. Joystick'in 5V'u DHT22 ile aynı hassas hattan beslenmeli (motor hattından değil).

## Kullanılan Pinler
- Pin 2  → DHT22 Sensör (DATA)
- Pin 3  → Joystick SW (buton)
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
- A0     → Joystick VRx (X ekseni — sıcaklık)
- A1     → Joystick VRy (Y ekseni — nem)

## Boş Pinler
- A2-A7
