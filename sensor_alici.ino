/*
  ESP32 #2 - ALICI (ESP-NOW + Hareket Analizi)
  ------------------------------------------------------------
  Bu ESP32'yi duvarin diger tarafina koyun. Verici'den gelen
  her ESP-NOW paketinin RSSI'sini okur, hareketli ortalama ve
  standart sapma hesaplayip hareket algilar.

  Seri porta CSV formatinda satirlar yazar:
    zaman_ms,rssi,ortalama,sapma,hareket
  Bu satirlar, birlikte verdigim HTML grafik paneli tarafindan
  Web Serial API ile okunup canli grafiklenir.

  ILK ADIM: Bu kodu yukleyip seri monitoru acin, yazdirdigi
  MAC adresini kopyalayip Verici kodundaki ALICI_MAC dizisine
  yapistirin.
*/

#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>

const int PENCERE_BOYU = 25;
const float HASSASIYET = 2.0;
const int WIFI_KANAL = 1;   // Verici ile AYNI olmali

volatile unsigned long sonPaketZamani = 0;
volatile unsigned long toplamPaket = 0;

float rssiPenceresi[PENCERE_BOYU];
int pencereIndex = 0;
bool pencereDolu = false;

float ortalamaHesapla() {
  float toplam = 0;
  int sayi = pencereDolu ? PENCERE_BOYU : pencereIndex;
  if (sayi == 0) return 0;
  for (int i = 0; i < sayi; i++) toplam += rssiPenceresi[i];
  return toplam / sayi;
}

float standartSapmaHesapla(float ortalama) {
  float toplam = 0;
  int sayi = pencereDolu ? PENCERE_BOYU : pencereIndex;
  if (sayi < 2) return 0;
  for (int i = 0; i < sayi; i++) {
    float fark = rssiPenceresi[i] - ortalama;
    toplam += fark * fark;
  }
  return sqrt(toplam / sayi);
}

// ESP32 Arduino core 2.0.x ve uzeri icin callback imzasi:
void onVeriAlindi(const esp_now_recv_info_t *bilgi, const uint8_t *veri, int uzunluk) {
  sonPaketZamani = millis();
  toplamPaket++;

  int rssi = bilgi->rx_ctrl->rssi;

  rssiPenceresi[pencereIndex] = rssi;
  pencereIndex++;
  if (pencereIndex >= PENCERE_BOYU) {
    pencereIndex = 0;
    pencereDolu = true;
  }

  float ortalama = ortalamaHesapla();
  float sapma = standartSapmaHesapla(ortalama);
  float fark = abs(rssi - ortalama);
  bool hareketVar = pencereDolu && (fark > HASSASIYET * max(sapma, (float)1.0));

  // CSV satiri: zaman,rssi,ortalama,sapma,hareket(0/1)
  Serial.printf("%lu,%d,%.2f,%.2f,%d\n",
                millis(), rssi, ortalama, sapma, hareketVar ? 1 : 0);
}

void setup() {
  Serial.begin(115200);
  delay(500);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(200);   // WiFi donanimi tam hazir olsun diye kisa bekleme

  // Kanali acikca sabitle - Verici ile birebir ayni olmali
  esp_wifi_set_channel(WIFI_KANAL, WIFI_SECOND_CHAN_NONE);

  // Daha guvenilir MAC okuma yontemi
  uint8_t mac[6];
  esp_wifi_get_mac(WIFI_IF_STA, mac);
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

  Serial.println("--- Bu cihazin MAC adresi (Verici koduna yazin) ---");
  Serial.println(macStr);
  Serial.println("----------------------------------------------------");

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW baslatilamadi!");
    return;
  }

  esp_now_register_recv_cb(onVeriAlindi);

  Serial.println("Alici hazir, veri bekleniyor...");
}

void loop() {
  // Her sey callback icinde isleniyor.
  // 3 saniyede bir hic paket gelmediyse uyar.
  static unsigned long sonKontrol = 0;
  if (millis() - sonKontrol > 3000) {
    sonKontrol = millis();
    if (toplamPaket == 0) {
      Serial.println("[UYARI] Henuz hic paket alinmadi. MAC/kanal kontrol edin.");
    } else if (millis() - sonPaketZamani > 3000) {
      Serial.println("[UYARI] Veri akisi durdu, Verici'yi kontrol edin.");
    }
  }
  delay(10);
}
