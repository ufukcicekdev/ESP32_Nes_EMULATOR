# 🎮 Anemoia-ESP32 S3 NES Console (Custom Build)

Bu proje, **ESP32-S3** mikrodenetleyicisi üzerinde çalışan, yüksek performanslı ve taşınabilir bir NES emülatörüdür. Orijinal [Anemoia-ESP32](https://github.com/Shim06/Anemoia-ESP32) projesi referans alınarak, özel donanım konfigürasyonuna ve **MAX98357A I2S DAC** ses sistemine göre optimize edilmiştir.

---

## 🚀 Donanım Özellikleri

* **İşlemci:** ESP32-S3 (N16R8 - 16MB Flash / 8MB Octal PSRAM desteği).
* **Ses Çözücü:** MAX98357A I2S DAC (44100Hz Örnekleme Hızı).
* **Depolama:** MicroSD Kart (Ayrı **FSPI** hattı ile çakışmasız hızlı erişim).
* **Ekran:** ILI9341 TFT LCD (**TFT_eSPI** kütüphanesi ile optimize).

---

## 🔌 Pin Bağlantı Şeması

Proje, ekran ve SD kartın aynı veri yolunda çakışmaması için **ayrı pin gruplarını** kullanacak şekilde yapılandırılmıştır.

### 🔊 Ses (MAX98357A I2S)
Ses kalitesini en üst düzeye çıkarmak için dijital I2S protokolü kullanılmıştır.

| MAX98357A Pini | ESP32-S3 GPIO |
| :--- | :--- |
| **BCLK** (Bit Clock) | **17** |
| **LRC** (WS / Clock) | **18** |
| **DOUT** (Data Out) | **21** |

### 💾 MicroSD Kart (FSPI Hattı)
SD kartın stabil çalışması için ayrılmış özel pinler tanımlanmıştır.

| SD Kart Pini | ESP32-S3 GPIO |
| :--- | :--- |
| **MOSI** | **14** |
| **MISO** | **3** |
| **SCLK** | **41** |
| **CS** | **42** |

### 🕹️ Kontrolcü & Buton Takımı
| Fonksiyon | GPIO | Fonksiyon | GPIO |
| :--- | :--- | :--- | :--- |
| **UP** | 16 | **START** | 1 |
| **DOWN** | 38 | **SELECT** | 2 |
| **LEFT** | 39 | **A** | 0 |
| **RIGHT** | 40 | **B** | 20 |

---

## ⚙️ Yazılım Kurulumu

### 1. TFT_eSPI Konfigürasyonu (Kritik!)
Ekran pinlerini tek tek ayarlamakla vakit kaybetmeyin:
* Repoda bulunan `User_Setup.h` dosyasını indirin.
* Bu dosyayı bilgisayarınızdaki `Documents/Arduino/libraries/TFT_eSPI/` klasörünün içine kopyalayıp eskisini değiştirin.
* Bu işlem, ekranınızın donanımla tam uyumlu çalışmasını sağlar.

### 2. Arduino IDE Ayarları
Yüksek FPS ve kararlılık için **Tools** menüsünden şu ayarları yapın:
* **Board:** `ESP32S3 Dev Module`
* **Flash Mode:** `QIO 80MHz`
* **PSRAM:** `OPI PSRAM` (Kartınız desteklemiyorsa Disabled yapın)
* **USB CDC On Boot:** `Disabled`
* **Core Debug Level:** `None`

---

## 🎮 Kullanım Notları

* **Oyun Yükleme:** `.nes` uzantılı oyun dosyalarınızı SD kartın ana dizinine (root) atın.
* **Menüye Dönüş:** Oyun sırasında **START + SELECT** (Pin 1 + Pin 2) tuşlarına aynı anda basarak ana menüye dönebilirsiniz.
* **Ses Kalitesi:** Varsayılan örnekleme hızı **44100Hz**'dir, retro oyunlarda en net ses deneyimini sunar.

---

## 📜 Kaynakça ve Teşekkür
Bu proje geliştirilirken [Anemoia-ESP32](https://github.com/Shim06/Anemoia-ESP32) kaynak kodları kullanılmış, donanımsal pin haritası ve I2S sürücüleri S3 mimarisi için özelleştirilmiştir.

