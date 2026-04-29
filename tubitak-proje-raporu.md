# İKLİME DUYARLI MİNİ AKILLI KOVAN MAKETİ

| PROJE TÜRÜ | PROJE ALANI | TEMATİK ALT PROJE ALANI |
|------------|-------------|--------------------------|
| TASARIM    | TEKNOLOJİ TASARIM | STEAM / Akıllı Tarım |

---

## SOL SÜTUN

### Problem / Soru Cümlesi
Küçük bir bütçeyle, kovan içindeki sıcaklığa göre uçuş deliğini otomatik açıp kapatan, arılara zarar vermeden havalandırmayı yöneten bir akıllı kovan sürgüsü tasarlanabilir mi?

### Özet
Bu projede arıların kış soğuğundan ve yaz sıcağından korunmasına yardımcı olacak sıcaklığa duyarlı bir mini akıllı kovan maketi tasarlanmıştır. Proje ile sensör verisini kullanarak fiziksel kontrol yapan bir gömülü sistem tasarlamayı, step motor ve elektronik bileşenlerin birlikte çalışma prensiplerini kavramayı, arı sağlığı ve sürdürülebilir tarım bilinci geliştirmeyi amaçlamaktayız.

### Yöntem
Proje kapsamında geri dönüştürülmüş ahşap ve karton parçalarla kovan maketi hazırlanmış, kovan içine sıcaklık-nem ölçümü için DHT22 sensörü yerleştirilmiştir. Uçuş deliği önüne 28BYJ-48 step motor ile hareket eden 0–150 mm aralığında bir sürgü monte edilmiş; bu sürgü, Arduino Nano üzerinde yazılan lineer algoritma ile 5–30 °C aralığında orantılı olarak konumlanmaktadır. Mevcut sıcaklık-nem ve sürgü konumu LCD 1602A ekrana yansıtılmaktadır. Sistem üç çalışma moduyla tasarlanmıştır: SENSOR modunda DHT22'den okunan gerçek sıcaklığa göre sürgü otomatik konumlanır; AYAR modunda HW-504 joystik üzerinden farklı sıcaklık-nem değerleri simüle edilerek sıcak su veya buz aküsüne gerek kalmadan oda sıcaklığında tüm sıcaklık senaryoları test edilebilir; KONTROL modunda ise sürgü manuel olarak hareket ettirilerek kalibrasyon ve bakım yapılır. Modlar arası geçiş joystik butonu ya da web arayüzü ile sağlanmaktadır. Step motor bobinlerinin oluşturduğu gerilim sıçramalarının DHT22 sensörünü etkilememesi için iki ayrı 5 V güç hattı kurulmuş; hassas elemanlar Arduino 5 V'una, motor ve LCD arka ışığı LM2596 regülatörüyle ayrı bir hatta bağlanmıştır. AYAR modu simülasyonu sayesinde sistemin tüm sıcaklık aralığındaki davranışı tek seferde doğrulanmıştır.

---

## ORTA SÜTUN

**PROJEDE GÖREV ALAN ÖĞRENCİLER**
Ad-Soyad

**DANIŞMAN ÖĞRETMEN(LER)**
Ad-Soyad

**BULGULAR**
*(Maket fotoğrafları, devre görselleri ve sıcaklık-sürgü konumu testlerine ait gözlem fotoğrafları buraya eklenecektir.)*

---

## SAĞ SÜTUN

### Sonuç ve Tartışma
Proje sonunda küçük bütçeli, basit sensörlerle çalışan bir mini akıllı kovan sürgü sisteminin başarıyla üretilebildiği gösterilmiştir. Sıcaklığa duyarlı lineer algoritma sayesinde sürgü ani değil kademeli olarak konumlanmış; 5 °C ve altında tam kapalı kalarak donma güvenliği sağlanmış, 30 °C ve üzerinde tam açık konuma geçerek havalandırma yapılmış, ara değerlerde ise orantılı açıklık korunmuştur. Geliştirilen üç mod sayesinde sistem hem otomatik hem manuel kullanılabilir hale gelmiş; özellikle AYAR modunun simülasyon özelliği sayesinde gerçek bir kış-yaz sıcaklığı oluşturmaya gerek kalmadan tüm aralık doğrulanmıştır. Step motor gerilim sıçramalarının DHT22 sensörünü etkilemesi sorunu yaşandığında çift güç hattı çözümü uygulanarak gerçek mühendislikte de kullanılan güç ayrımı yöntemi öğrenilmiştir. Proje gerçek arılar üzerinde denenmemiş, canlıya zarar verme riski tamamen ortadan kaldırılmıştır. Gelecekte güneş paneli ile bataryasız çalışma ve gerçek kovana takılabilir dayanıklı bir versiyon hedeflenmektedir.

### Kaynaklar
DHT22 (AM2302) datasheet — Aosong Electronics. 28BYJ-48 / ULN2003 step motor teknik belgeleri. Arduino resmî dokümantasyonu — `docs.arduino.cc`. Türkiye Arıcılık Federasyonu — Kovan içi ideal sıcaklık rehberi.

### Ölçüler
Kovan maketi yaklaşık 20 cm × 15 cm × 12 cm boyutundadır. Sürgü hareket aralığı 0–150 mm'dir (0 mm = açık, 150 mm = kapalı). Step motor kalibrasyonu 9318 yarım-adım = 150 mm olarak yapılmıştır.
