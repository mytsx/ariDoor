# İKLİME DUYARLI MİNİ AKILLI KOVAN MAKETİ

| PROJE TÜRÜ | PROJE ALANI | TEMATİK ALT PROJE ALANI |
|------------|-------------|--------------------------|
| TASARIM    | TEKNOLOJİ TASARIM | STEAM / Akıllı Tarım |

---

## SOL SÜTUN

### Problem / Soru Cümlesi
Az bir bütçeyle, kovan içindeki sıcaklığa göre kovanın kapısını kendi kendine açıp kapatan, arılara zarar vermeyen küçük bir akıllı kovan tasarlanabilir mi?

### Özet
Bu projede arıların kışın soğuktan, yazın sıcaktan korunmasına yardım edecek küçük bir akıllı kovan maketi tasarlandı. Kovanın kapısı, DHT22 adı verilen sıcaklık-nem sensörünün ölçtüğü değerlere göre 28BYJ-48 step motoru ile kendi kendine açılıp kapanmaktadır. Tüm parçalar Arduino Nano kontrol kartına bağlanmıştır. Böylece teknolojinin doğa ve hayvanlar için nasıl kullanılabileceği öğrenilmiştir.

### Yöntem
Proje kapsamında karton ve geri dönüşüm ahşap parçalarıyla mini bir kovan maketi hazırlanmıştır. Kovanın içine DHT22 adı verilen sıcaklık-nem sensörü yerleştirilmiştir. Kovanın giriş kapısının önüne ise 28BYJ-48 step motoru takılmıştır; bu motor, ULN2003 sürücü kartı yardımıyla kapıyı hareket ettirmektedir. Tüm parçalar Arduino Nano kontrol kartına bağlanmıştır. Kart, sensörden okuduğu sıcaklığa göre kararlar verir: sıcaklık 5 °C altına düşerse kapı tamamen kapanır, 30 °C üstüne çıkarsa tamamen açılır, arada ise yarı yarıya açık tutulur. Sıcaklık, nem ve kapı konumu LCD 1602A ekranda gösterilmektedir. Sistemin üç çalışma modu vardır: otomatik modda sensöre göre çalışır, ayar modunda HW-504 joystick ile sanal sıcaklık değerleri verilip kapının tepkisi test edilir, kontrol modunda ise kapı elle hareket ettirilir. Sistem ayrıca telefondan ya da bilgisayardan web arayüzü üzerinden de kontrol edilebilmektedir. Motorun sensörü etkilememesi için motor ve sensör ayrı güç kaynaklarından beslenmiştir.

---

## ORTA SÜTUN

**PROJEDE GÖREV ALAN ÖĞRENCİLER**
Ad-Soyad

**DANIŞMAN ÖĞRETMEN(LER)**
Ad-Soyad

**BULGULAR**
*(Maket fotoğrafları, devre görselleri ve sıcaklık testlerine ait gözlem fotoğrafları buraya eklenecektir.)*

---

## SAĞ SÜTUN

### Sonuç ve Tartışma
Projemizde DHT22 sıcaklık-nem sensörü ve 28BYJ-48 step motoru sayesinde kovan kapısı sıcaklığa göre kendiliğinden açılıp kapanmıştır. Hava soğuduğunda arıları korumak için kapı kapanmış, hava ısındığında havalandırma için açılmıştır. HW-504 joystick sayesinde gerçek arıları riske atmadan tüm sıcaklık koşulları test edilmiştir. Arduino Nano üzerinde yazılan kod ile elektronik parçaların nasıl birbirine bağlanıp kontrol edilebileceği öğrenilmiştir. İleride güneş paneli ekleyerek pilsiz çalışmayı ve gerçek bir arı kovanına uyarlanmasını planlamaktayız.

### Kaynaklar
DHT22 sıcaklık-nem sensörü teknik dokümanı. 28BYJ-48 step motor ve ULN2003 sürücü kartı teknik dokümanı. Arduino resmî sayfası — `docs.arduino.cc`. Türkiye Arıcılık Federasyonu — Kovan içi ideal sıcaklık rehberi.

### Ölçüler
Kovan maketi yaklaşık 20 cm × 15 cm × 12 cm boyutundadır. Kapı 0–15 cm arasında hareket etmektedir.
