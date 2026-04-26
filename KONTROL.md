# ariDoor - Kontrol Mantığı

## Modlar

Sistem iki modda çalışır. Geçiş **HW-504 joystick butonu (SW)** veya web arayüzünden yapılır.

**Otomatik kontrol her iki modda da çalışır.** Mod sadece joystick yön girişlerinin hedef değerleri değiştirip değiştirmediğini belirler.

- Auto kontrol döngüsü: her 10 saniyede bir
- Min hareket eşiği: 2 mm (titreşim/aşırı motor kullanımı önlemek için)
- Hedef değerler değişince bir sonraki döngüde sürgü yeni hedefe göre konumlanır

### SENSOR Modu (varsayılan)

Joystick yön girişleri yok sayılır. Hedef değerleri sadece web arayüzünden değiştirilebilir.

### AYAR Modu

Joystick yön hareketleriyle hedef sıcaklık/nem ayarlanır.

| Joystick Yönü | Etki | Adim |
|---------------|------|------|
| Sağ           | Hedef sıcaklık ↑ | +5 °C / 300 ms |
| Sol           | Hedef sıcaklık ↓ | -5 °C / 300 ms |
| Yukarı        | Hedef nem ↑      | +5 % / 300 ms |
| Aşağı         | Hedef nem ↓      | -5 % / 300 ms |

LCD ekran AYAR modunda hedef değerleri gösterir, SENSOR modunda mevcut değerleri.

## Sürgü Konvansiyonu

- **0 mm = Açık** (sürgü retracted, kovan girişi açık → havalandırma)
- **150 mm = Kapalı** (sürgü extended, kovan girişi kapalı → ısı korumalı)
- Başlangıçta sürgü **150 mm (kapalı)** varsayılır. Cihaz çalıştırılmadan önce sürgü manuel olarak kapalı pozisyonda olmalı.

## Sürgü Konumlama Algoritması

Mevcut sıcaklığa göre **lineer mapping**:

- **5°C ve altı** → 150 mm (tam kapalı)
- **hedef_sic ve üstü** → 0 mm (tam açık)
- Arasında lineer interpolasyon:

```
sürgü_mm = 150 × (hedef_sic − mevcut_sic) / (hedef_sic − 5)
```

### Mantık

- 5°C alt sınır sabittir (kış güvenliği)
- Hedef sıcaklık = sürgünün **tamamen açıldığı** üst sınır
- Mevcut sıcaklık 5°C ile hedef arasında ilerledikçe sürgü 150'den 0'a doğru lineer hareket eder

### Örnek

`hedef_sic = 30°C` (varsayılan):

| Mevcut Sic | Sürgü mm | Yorum |
|------------|----------|-------|
| ≤ 5°C      | 150      | Tam kapalı (donma güvenliği) |
| 10°C       | 120      | %80 kapalı |
| 15°C       | 90       | %60 kapalı |
| 17.5°C     | 75       | yarı |
| 20°C       | 60       | %40 kapalı |
| 25°C       | 30       | %20 kapalı |
| ≥ 30°C     | 0        | Tam açık |

### Hedef Sıcaklık Ayarlama

Hedef = "tam açık olsun istediğim üst sıcaklık". 5°C'lik adımlarla joystick veya HTML +/− ile ayarlanır.

- `hedef_sic = 25` → 25°C ve üstünde tam açık (sıcak iklimler için)
- `hedef_sic = 35` → 35°C ve üstünde tam açık (sıcağa daha toleranslı)
- `hedef_sic = 20` → 20°C bile açtırır (serin iklimler için)

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
| `HEDEFSIC:N`  | Hedef sıcaklığı N °C yap (clamp 20-40)    |
| `HEDEFNEM:N`  | Hedef nemi N % yap (clamp 30-90)          |
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
