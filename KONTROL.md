# ariDoor - Kontrol Mantığı

## Modlar

Sistem üç modda çalışır. Geçiş **HW-504 joystick butonu (SW)** veya web arayüzünden yapılır:

`SENSOR → AYAR → KONTROL → SENSOR`

**Otomatik pozisyonlama SENSOR ve AYAR modlarında çalışır.** SENSOR modunda sensör sıcaklığına göre, AYAR modunda joystick/web ile ayarlanan sıcaklık değerine göre pozisyon alınır. KONTROL modunda kapı manuel hareket ettirilir.

- SENSOR modu auto kontrol döngüsü: her 5 dakikada bir
- İlk açılışta sürgü 0 mm (tam açık) kabul edilir; ilk geçerli sensör okumasıyla pozisyon alınır
- Min hareket eşiği: 2 mm (titreşim/aşırı motor kullanımı önlemek için)
- AYAR modunda sıcaklık değeri değişince sürgü hemen yeni değere göre konumlanır

### SENSOR Modu (varsayılan)

Joystick yön girişleri yok sayılır. Sürgü sensörden okunan mevcut sıcaklığa göre konumlanır.

### AYAR Modu

Joystick yön hareketleriyle sıcaklık/nem ayarlanır. Bu modda sürgü, joystick/web ile ayarlanan sıcaklık değerine göre konumlanır.

| Joystick Yönü | Etki | Adim |
|---------------|------|------|
| Sağ           | Hedef sıcaklık ↑ | +5 °C / 300 ms |
| Sol           | Hedef sıcaklık ↓ | -5 °C / 300 ms |
| Yukarı        | Hedef nem ↑      | +5 % / 300 ms |
| Aşağı         | Hedef nem ↓      | -5 % / 300 ms |

### KONTROL Modu

Joystick sağ/sol hareketleriyle kapı doğrudan hareket ettirilir.

| Joystick Yönü | Etki |
|---------------|------|
| Sağ           | Kapı kapanma yönünde hareket eder |
| Sol           | Kapı açılma yönünde hareket eder |

LCD ekran SENSOR modunda mevcut değerleri, AYAR modunda hedef değerleri, KONTROL modunda kapı konumunu gösterir.

## Sürgü Konvansiyonu

- **0 mm = Açık** (sürgü retracted, kovan girişi açık → havalandırma)
- **150 mm = Kapalı** (sürgü extended, kovan girişi kapalı → ısı korumalı)
- Başlangıçta sürgü **0 mm (açık)** varsayılır. Cihaz çalıştırılmadan önce sürgü manuel olarak açık pozisyonda olmalı.

## Sürgü Konumlama Algoritması

Mevcut sıcaklığa göre **lineer mapping**:

- **5°C ve altı** → 150 mm (tam kapalı)
- **30°C ve üstü** → 0 mm (tam açık)
- Arasında lineer interpolasyon:

```
sürgü_mm = 150 × (30 − sicaklik) / (30 − 5)
```

### Mantık

- 5°C alt sınır sabittir (kış güvenliği)
- 30°C üst sınırdır; bu sıcaklık ve üstünde sürgü tamamen açık kalır
- Sıcaklık 5°C ile 30°C arasında ilerledikçe sürgü 150'den 0'a doğru lineer hareket eder

### Örnek

| Mevcut Sic | Sürgü mm | Yorum |
|------------|----------|-------|
| ≤ 5°C      | 150      | Tam kapalı (donma güvenliği) |
| 10°C       | 120      | %80 kapalı |
| 15°C       | 90       | %60 kapalı |
| 17.5°C     | 75       | yarı |
| 20°C       | 60       | %40 kapalı |
| 25°C       | 30       | %20 kapalı |
| ≥ 30°C     | 0        | Tam açık |

### AYAR Modu Sıcaklık Ayarlama

AYAR modunda joystick veya HTML +/− ile ayarlanan sıcaklık değeri, sensör sıcaklığı yerine kullanılır. Örneğin AYAR sıcaklığı 20°C ise sürgü 60 mm konuma gider.

> Nem (hedef_nem) şu an algoritmaya etki etmiyor; sadece bilgi olarak gösteriliyor ve joystick'ten ayarlanabiliyor. İleride eklenebilir.

## Hedef Değer Limitleri

- Hedef sıcaklık: **0°C – 50°C**
- Hedef nem: **0% – 100%**

## Otomatik Güvenlik

Mevcut sıcaklık **≤ 5°C** olduğunda formül devre dışı bırakılır ve sürgü zorla **150 mm** (tam kapalı) konumuna getirilir.

Sebebi:
- Arılar 10°C altında uçamaz
- 5°C ve altında hayati tehlike (kümeleşme moduna geçerler)
- Açık kovan = ısı kaybı + fare/predatör girişi
- Hedef sıcaklık ne olursa olsun, mevcut sıcaklık 5°C altındaysa kapalı tutmak güvenlidir

> Bu güvenlik otomatik kontrol döngüsünde uygulanır. Manuel komutlar (`SAG`, `SOL`, `SIFIR`, `GIT:N`) bu güvenliği geçer — bakım/test için.

## Serial Komutlar

| Komut         | Etki                                      |
|---------------|-------------------------------------------|
| `MOD:SENSOR`  | SENSOR moduna geç                         |
| `MOD:AYAR`    | AYAR moduna geç                           |
| `MOD:KONTROL` | KONTROL moduna geç                        |
| `HEDEFSIC:N`  | AYAR modu sıcaklığını N °C yap (clamp 0-50) |
| `HEDEFNEM:N`  | AYAR modu nemini N % yap (clamp 0-100)    |
| `SAG`         | Sürgü 128 yarım-adım ileri (manuel)       |
| `SOL`         | Sürgü 128 yarım-adım geri (manuel)        |
| `SIFIR`       | Sürgü 0'a dön                             |
| `GIT:N`       | Sürgüyü logical N adimine getir           |

## Serial JSON Çıktısı

Sensör okumalarıyla birlikte mevcut durum yayınlanır:
```json
{
  "sic": 34.5,
  "nem": 62.1,
  "adim": 4500,
  "mod": "SENSOR",
  "hedefSic": 34.0,
  "hedefNem": 60.0
}
```
