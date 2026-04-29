# İKLİME DUYARLI MİNİ AKILLI KOVAN MAKETİ

| PROJE TÜRÜ | ANA ALANI | TEMATİK KONUSU |
|------------|-----------|----------------|
| TASARIM    | TEKNOLOJİ VE TASARIM / STEM | STEAM |

---

## Problem / Soru Cümlesi

Küçük bir bütçe ve sınırlı malzemeyle, kovan içindeki sıcaklığa göre uçuş deliğini otomatik açıp kapatan, arılara zarar vermeden havalandırmayı yöneten bir akıllı kovan sürgüsü tasarlanabilir mi?

---

## Özet

Bu projede arıların kış soğuğundan ve yaz sıcağından korunmasına yardımcı olacak, sıcaklığa duyarlı bir mini akıllı kovan maketi tasarlandı. Maketin uçuş deliği önüne 28BYJ-48 step motorla hareket eden bir sürgü yerleştirildi. Kovan içine konulan DHT22 sıcaklık-nem sensöründen alınan veri Arduino Nano üzerinde işlendi; sıcaklık 5 °C altına düştüğünde sürgü tamamen kapatıldı, 30 °C ve üzerinde tamamen açıldı, arada ise lineer (orantılı) olarak konumlandı. Kullanıcı; LCD ekrandan mevcut sıcaklık-nem ve sürgü konumunu izleyebildi, joystik ya da web arayüzü üzerinden **SENSOR / AYAR / KONTROL** modları arasında geçiş yapabildi. Hassas sensörü step motorun gerilim sıçramalarından korumak için iki ayrı 5 V güç hattı kuruldu.

---

## Yöntem

**1. Tasarım ve Malzeme Seçimi**
Kovan maketi geri dönüştürülmüş ahşap ve karton parçalardan hazırlandı. Kovanın uçuş deliğine, step motorla hareket edecek 0–150 mm aralığında doğrusal bir sürgü tasarlandı.

**2. Donanım Kurulumu**
Aşağıdaki bileşenler Arduino Nano üzerinde birleştirildi:

| Bileşen | Görevi |
|---------|--------|
| Arduino Nano (CH340) | Ana kontrol birimi |
| DHT22 sıcaklık/nem sensörü | Kovan içi ölçüm |
| 28BYJ-48 step motor + ULN2003 sürücü | Sürgüyü hareket ettirme |
| LCD 1602A ekran | Veri ve mod gösterimi |
| HW-504 analog joystik | Mod değiştirme ve hedef ayarı |
| 10 kΩ potansiyometre | LCD kontrast ayarı |
| LM2596 voltaj regülatörü (9 V → 5 V) | Motor ve LCD arka ışığı için ayrı hat |

**Önemli tasarım kararı — Çift güç hattı:**
İlk denemelerde step motor bobinleri kapanıp açılırken oluşan gerilim sıçramaları (inductive kickback) DHT22 sensörünü zarar görme noktasına getirdi. Bu nedenle hassas elemanlar (DHT22, LCD mantık devresi, joystik) Arduino 5 V hattında, motor ve LCD arka ışığı ayrı bir LM2596 regülatör çıkışında kullanıldı. Tüm GND'ler ortak rayda birleştirildi.

**3. Yazılım ve Algoritma**
Arduino C++ ile yazılan yazılım sürgü konumunu mevcut sıcaklığa göre lineer olarak hesaplar:

```
sürgü_mm = 150 × (30 − sıcaklık) / (30 − 5)
```

- ≤ 5 °C → 150 mm (tam kapalı, donma güvenliği)
- ≥ 30 °C → 0 mm (tam açık, havalandırma)
- Arada orantılı konumlandırma

Üç çalışma modu tanımlandı:
- **SENSOR**: Mevcut sıcaklığa göre otomatik konumlanma (5 dakikada bir kontrol).
- **AYAR**: Joystik ya da web arayüzünden hedef sıcaklık girilir; sürgü o değere göre konumlanır.
- **KONTROL**: Sürgü manuel olarak hareket ettirilir (kalibrasyon ve bakım için).

Step motor kalibrasyonu fiziksel ölçümle yapıldı: 150 mm hareket için 9318 yarım-adım gerektiği hesaplandı. Açma ve kapama yönlerindeki mekanik asimetri için ayrı ölçek katsayıları (`ACMA_OLCEK = 0.876`, `KAPAMA_OLCEK = 0.873`) eklendi. Titreşimi önlemek için 2 mm minimum hareket eşiği kullanıldı.

**4. Test Yöntemi**
Maket arı kullanılmadan test edildi. Sıcak su torbası ve buz aküsüyle DHT22 çevresinde farklı sıcaklıklar oluşturularak sürgünün doğru oranda açılıp kapandığı gözlemlendi. Web arayüzünden de sıcaklık-nem ve sürgü konumu canlı izlendi.

---

## Bulgular

**Donanım Doğrulaması**
- Çift güç hattı uygulandıktan sonra DHT22'de hatalı okuma ve donanım kaybı yaşanmadı.
- 28BYJ-48 step motor 150 mm tam hareketi ortalama ~57 saniyede tamamladı (yaklaşık 800 yarım-adım/sn).
- LCD ekran 16x2 hücrede sıcaklık, nem, mod ve sürgü konumunu eş zamanlı gösterdi.

**Algoritma Doğrulaması — Sıcaklık → Sürgü Konumu**

| Mevcut Sıcaklık | Hesaplanan Sürgü | Yorum |
|-----------------|------------------|-------|
| ≤ 5 °C | 150 mm | Tam kapalı (donma güvenliği) |
| 10 °C | 120 mm | %80 kapalı |
| 15 °C | 90 mm | %60 kapalı |
| 17,5 °C | 75 mm | Yarı açık |
| 20 °C | 60 mm | %40 kapalı |
| 25 °C | 30 mm | %20 kapalı |
| ≥ 30 °C | 0 mm | Tam açık |

**Mod Geçişleri**
Joystik butonuyla yapılan SENSOR → AYAR → KONTROL → SENSOR geçişleri 50 ms debounce ile titreşimsiz çalıştı. Web arayüzünden seri port üzerinden gönderilen JSON komutları (`MOD:AYAR`, `HEDEFSIC:22`, `GIT:4500` vb.) doğru biçimde uygulandı.

**Güvenlik Kuralı**
Mevcut sıcaklık 5 °C altına düştüğünde, hedef sıcaklık ne olursa olsun, sürgü 150 mm konumuna kilitlenerek tam kapalı kaldı.

---

## Sonuç ve Tartışma

Proje sonunda küçük bütçeli, basit sensörlerle çalışan bir mini akıllı kovan sürgü sisteminin **çalışır halde** üretilebildiği gösterildi. Sıcaklığa duyarlı lineer konumlama algoritması, kovan girişini ani açma-kapama yerine kademeli olarak ayarladığı için arıların yaşam alanına daha yumuşak bir geçiş sağladı. 5 °C eşiği ile getirilen otomatik güvenlik kuralı, sıcaklık düşüşünde donma ve predatör riskini en aza indirdi.

Çift güç hattı çözümü, projenin en önemli mühendislik kararlarından biri oldu: Tek güç hattıyla yapılan ilk denemede sensör verilerinin bozulduğu görülünce, sorun yazılım değil donanım kaynaklı tespit edildi ve gerçek hayatta endüstriyel sistemlerde de kullanılan **güç hattı ayrımı** çözümü uygulandı. Bu süreçte güç elektroniği, motor sürücü mantığı ve sensör koruma konularında uygulamalı bilgi kazanıldı.

Proje, gerçek arı kovanlarında denenmedi; canlıya zarar verme riski tamamen ortadan kaldırıldı. Maket öğretmenin izniyle sınıfta sergilendi ve arkadaşlara çalışma prensibi anlatıldı.

**Geliştirilebilir Alanlar:**
- Güneş paneli ve şarj devresiyle bataryasız çalışma.
- Nem değerinin de sürgü kararına dahil edilmesi (şu an sadece bilgi amaçlı).
- Veri kaydı (SD kart veya WiFi modül) ile mevsimsel sıcaklık-davranış analizi.
- Gerçek kovanlara takılabilecek dayanıklı bir mekanik aparata dönüştürülmesi.

---

## Projede Görev Alan Öğrenciler
Ad-Soyad

## Danışman Öğretmen(ler)
Ad-Soyad

---

## Kaynaklar

- DHT22 (AM2302) datasheet — Aosong Electronics
- 28BYJ-48 step motor & ULN2003 sürücü kartı teknik belgeleri
- Arduino resmî dokümantasyonu — `https://docs.arduino.cc`
- AccelStepper kütüphanesi — Mike McCauley
- Türkiye Arıcılık Federasyonu — Kovan içi ideal sıcaklık ve havalandırma rehberi
- Tinkercad devre simülasyonu — `https://www.tinkercad.com`
