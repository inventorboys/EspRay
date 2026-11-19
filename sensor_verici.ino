/*
  ESP32 #1 - VERICI (ESP-NOW Gonderici)
  ------------------------------------------------------------
  Bu ESP32'yi duvarin bir tarafina koyun. Alici ESP32'ye
  ESP-NOW ile periyodik olarak kucuk paketler gonderir.
  Alici, gelen her paketin RSSI'sini olcerek hareket analizi
  yapacak.

  ONEMLI: Asagidaki ALICI_MAC dizisini, Alici ESP32'nin seri
  monitorde yazdirdigi kendi MAC adresi ile degistirin.
*/

#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>

// --- Alici ESP32'nin MAC adresini buraya yazin ---
// Alici kodunu once yukleyip seri porttan MAC'i okuyun.
uint8_t ALICI_MAC[] = {0x1B, 0xC3, 0xAB, 0xB8, 0x04, 0x28};

const int GONDERIM_ARALIK_MS = 50;   // Ne siklikla paket gonderilecek
const int WIFI_KANAL = 1;            // Alici ile AYNI olmali

typedef struct {
  uint32_t sayac;
} VeriPaketi;

VeriPaketi paket;

int basariliSayisi = 0;
int hataliSayisi = 0;

void onGonderimTamam(const uint8_t *mac_addr, esp_now_send_status_t status) {
  if (status == ESP_NOW_SEND_SUCCESS) basariliSayisi++;
  else hataliSayisi++;
}

void setup() {
  Serial.begin(115200);
  delay(500);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(200);   // WiFi donanimi tam hazir olsun diye kisa bekleme

  // Kanali acikca sabitle - Alici ile birebir ayni olmali
  esp_wifi_set_channel(WIFI_KANAL, WIFI_SECOND_CHAN_NONE);

  uint8_t mac[6];
  esp_wifi_get_mac(WIFI_IF_STA, mac);
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  Serial.print("Bu cihazin (Verici) MAC adresi: ");
  Serial.println(macStr);

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW baslatilamadi!");
    return;
  }

  esp_now_register_send_cb(onGonderimTamam);

  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, ALICI_MAC, 6);
  peerInfo.channel = WIFI_KANAL;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Peer eklenemedi! MAC adresini kontrol edin.");
    return;
  }

  Serial.println("Verici hazir, gonderim basliyor...");
  paket.sayac = 0;
}

void loop() {
  paket.sayac++;
  esp_now_send(ALICI_MAC, (uint8_t*)&paket, sizeof(paket));

  // Her 2 saniyede bir istatistik yazdir
  static unsigned long sonRapor = 0;
  if (millis() - sonRapor > 2000) {
    sonRapor = millis();
    Serial.printf("Basarili: %d | Hatali/cevapsiz: %d\n", basariliSayisi, hataliSayisi);
  }

  delay(GONDERIM_ARALIK_MS);
}
