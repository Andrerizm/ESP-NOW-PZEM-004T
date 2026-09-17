/*
  ========================================================================
  02_Slave_PZEM_Sender.ino
  Node Sensor: ESP32 + PZEM-004T v3.0 (Pengirim ESP-NOW)
  
  Fungsi:
  1. Membaca data listrik AC dari PZEM-004T v3.0 via HardwareSerial2:
     - ESP32 RX2 (GPIO 16) -> PZEM TX
     - ESP32 TX2 (GPIO 17) -> PZEM RX
  2. Mengemas data ke dalam struct.
  3. Mengirimkan data secara nirkabel melalui ESP-NOW ke ESP32 Master.
  
  Pustaka yang Dibutuhkan:
  - PZEM004Tv30 by Jakub Mandula (Install via Arduino Library Manager)
  ========================================================================
*/

#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <PZEM004Tv30.h>

// ========================================================================
// 1. KONFIGURASI PIN & KONEKSI PZEM-004T
// ========================================================================
#define PZEM_RX_PIN 16  // Terhubung ke pin TX pada modul PZEM-004T
#define PZEM_TX_PIN 17  // Terhubung ke pin RX pada modul PZEM-004T

// Inisialisasi PZEM menggunakan HardwareSerial 2 (Serial2)
PZEM004Tv30 pzem(Serial2, PZEM_RX_PIN, PZEM_TX_PIN);

// ========================================================================
// 2. KONFIGURASI ESP-NOW
// ========================================================================
// Saluran Wi-Fi (Wajib sama persis dengan saluran pada ESP32 Master!)
#define WIFI_CHANNEL 1

// MAC Address dari ESP32 MASTER
uint8_t masterMacAddress[] = {0xB0, 0xCB, 0xD8, 0xCF, 0xB5, 0x9C};

// Struktur data yang dikirim (harus identik dengan Master)
typedef struct struct_message {
  float voltage;      // Tegangan (V)
  float current;      // Arus Listrik (A)
  float power;        // Daya Aktif (W)
  float energy;       // Total Energi (kWh)
  float frequency;    // Frekuensi (Hz)
  float pf;           // Power Factor (0.00 - 1.00)
  bool valid;         // Status validitas data sensor
} struct_message;

struct_message sensorData;
esp_now_peer_info_t peerInfo;

// Variabel waktu pengiriman berkala
unsigned long lastSendTime = 0;
const unsigned long sendInterval = 1500; // Kirim data setiap 1.5 detik
bool lastSendSuccess = false;

// ========================================================================
// 3. CALLBACK STATUS PENGIRIMAN ESP-NOW
// ========================================================================
#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
#else
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
#endif
  lastSendSuccess = (status == ESP_NOW_SEND_SUCCESS);
  Serial.print("[ESP-NOW] Status Kirim: ");
  Serial.println(lastSendSuccess ? "BERHASIL (ACK diterima)" : "GAGAL (Master tidak merespons)");
}

// ========================================================================
// 4. SETUP
// ========================================================================
void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n================================================");
  Serial.println("     ESP32 SLAVE - PZEM-004T ESP-NOW SENDER     ");
  Serial.println("================================================");

  // Set Wi-Fi ke Station Mode
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  // Kunci saluran Wi-Fi agar sama persis dengan Master (Channel 1)
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_channel(WIFI_CHANNEL, WIFI_SECOND_CHAN_NONE);
  esp_wifi_set_promiscuous(false);

  Serial.print("Wi-Fi Channel diatur ke : ");
  Serial.println(WIFI_CHANNEL);
  Serial.print("Slave MAC Address       : ");
  Serial.println(WiFi.macAddress());

  // Inisialisasi ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("[ERROR] Gagal menginisialisasi ESP-NOW!");
    return;
  }
  Serial.println("[OK] ESP-NOW berhasil diinisialisasi.");

  // Daftarkan fungsi callback pengiriman
  esp_now_register_send_cb(OnDataSent);

  // Daftarkan ESP32 Master sebagai Peer
  memcpy(peerInfo.peer_addr, masterMacAddress, 6);
  peerInfo.channel = WIFI_CHANNEL;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("[ERROR] Gagal menambahkan Master sebagai Peer!");
    Serial.println("Pastikan MAC Address Master sudah diisi dengan benar.");
  } else {
    Serial.println("[OK] Master berhasil didaftarkan sebagai Peer.");
  }

  Serial.println("================================================\n");
}

// ========================================================================
// 5. LOOP UTAMA
// ========================================================================
void loop() {
  unsigned long currentMillis = millis();

  // Jalankan pembacaan dan pengiriman setiap interval waktu tertentu
  if (currentMillis - lastSendTime >= sendInterval) {
    lastSendTime = currentMillis;

    // 1. Membaca parameter dari PZEM-004T v3.0
    float v  = pzem.voltage();
    float i  = pzem.current();
    float p  = pzem.power();
    float e  = pzem.energy();
    float f  = pzem.frequency();
    float pf = pzem.pf();

    // 2. Validasi pembacaan (apakah bernilai NaN karena AC belum tersambung)
    if (isnan(v)) {
      sensorData.voltage   = 0.0;
      sensorData.current   = 0.0;
      sensorData.power     = 0.0;
      sensorData.energy    = 0.0;
      sensorData.frequency = 0.0;
      sensorData.pf        = 0.0;
      sensorData.valid     = false;

      Serial.println("[WARNING] PZEM-004T tidak merespons!");
      Serial.println(" -> Pastikan terminal AC 220V sudah tersambung.");
      Serial.println(" -> Periksa kabel RX/TX ke pin GPIO 16 & 17.");
    } else {
      sensorData.voltage   = v;
      sensorData.current   = isnan(i)  ? 0.0 : i;
      sensorData.power     = isnan(p)  ? 0.0 : p;
      sensorData.energy    = isnan(e)  ? 0.0 : e;
      sensorData.frequency = isnan(f)  ? 0.0 : f;
      sensorData.pf        = isnan(pf) ? 0.0 : pf;
      sensorData.valid     = true;

      // Tampilkan hasil pembacaan ke Serial Monitor lokal Slave
      Serial.println("------------------------------------------------");
      Serial.printf("Tegangan   : %.1f V\n", sensorData.voltage);
      Serial.printf("Arus       : %.3f A\n", sensorData.current);
      Serial.printf("Daya       : %.1f W\n", sensorData.power);
      Serial.printf("Energi     : %.3f kWh\n", sensorData.energy);
      Serial.printf("Frekuensi  : %.1f Hz\n", sensorData.frequency);
      Serial.printf("Power Fact : %.2f\n", sensorData.pf);
      Serial.println("------------------------------------------------");
    }

    // 3. Kirim paket data via ESP-NOW ke Master
    esp_err_t result = esp_now_send(masterMacAddress, (uint8_t *)&sensorData, sizeof(sensorData));
    if (result != ESP_OK) {
      Serial.println("[ERROR] Gagal memicu esp_now_send()!");
    }
  }
}
