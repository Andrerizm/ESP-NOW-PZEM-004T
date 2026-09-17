/*
  ========================================================================
  03_Master_Receiver_Web.ino
  Node Gateway: ESP32 Master (Penerima ESP-NOW + Web Server + Serial)
  
  Fungsi:
  1. Menerima paket data listrik AC via protokol ESP-NOW dari Slave.
  2. Mencetak hasil pengukuran terformat ke Serial Monitor (115200 baud).
  3. Menjalankan Web Server lokal (Access Point: http://192.168.4.1)
     dengan tampilan Dashboard Dark Mode responsif real-time.
  
  Pustaka yang Dibutuhkan:
  - Pustaka bawaan board ESP32 (WiFi.h, esp_now.h, WebServer.h).
    Tidak memerlukan instalasi library eksternal tambahan.
  ========================================================================
*/

#include <WiFi.h>
#include <esp_now.h>
#include <WebServer.h>
#include "WebPage.h"

// ========================================================================
// 1. KONFIGURASI WI-FI & WEB SERVER
// ========================================================================
// Saluran Wi-Fi (Harus sama persis dengan Saluran di Slave!)
#define WIFI_CHANNEL 1

// Nama & Password Hotspot (AP Mode) ESP32 Master
const char* AP_SSID = "EnergyMonitor-Master";
const char* AP_PASS = "12345678"; // Minimal 8 karakter, atau ubah jadi NULL untuk tanpa password

/*
  [OPSIONAL] Jika ingin menghubungkan ESP32 Master ke Wi-Fi Rumah:
  Buka komentar (uncomment) baris di bawah ini dan isi SSID & Password Anda:
*/
// #define CONNECT_TO_HOME_WIFI
// const char* STA_SSID = "Nama_WiFi_Rumah_Anda";
// const char* STA_PASS = "Password_WiFi_Rumah";

WebServer server(80);

// ========================================================================
// 2. STRUKTUR DATA ESP-NOW (SAMA PERSIS DENGAN SLAVE)
// ========================================================================
typedef struct struct_message {
  float voltage;      // Tegangan (V)
  float current;      // Arus Listrik (A)
  float power;        // Daya Aktif (W)
  float energy;       // Total Energi (kWh)
  float frequency;    // Frekuensi (Hz)
  float pf;           // Power Factor (0.00 - 1.00)
  bool valid;         // Status validitas data sensor
} struct_message;

struct_message receivedData;
volatile unsigned long packetCount = 0;
volatile unsigned long lastReceivedTime = 0;
volatile bool newDataAvailable = false;

// ========================================================================
// 3. CALLBACK PENERIMAAN DATA ESP-NOW
// ========================================================================
// Kompatibilitas ESP32 Arduino Core 2.x & 3.x
#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
void OnDataRecv(const esp_now_recv_info_t * esp_now_info, const uint8_t *incomingData, int len) {
#else
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
#endif
  if (len == sizeof(struct_message)) {
    memcpy(&receivedData, incomingData, sizeof(receivedData));
    packetCount++;
    lastReceivedTime = millis();
    newDataAvailable = true;
  }
}

// ========================================================================
// 4. ROUTE HANDLER WEB SERVER
// ========================================================================
void handleRoot() {
  server.send(200, "text/html", PAGE_HTML);
}

void handleDataApi() {
  // Format JSON untuk dibaca oleh frontend AJAX
  String json = "{";
  json += "\"voltage\":" + String(receivedData.voltage, 1) + ",";
  json += "\"current\":" + String(receivedData.current, 3) + ",";
  json += "\"power\":" + String(receivedData.power, 1) + ",";
  json += "\"energy\":" + String(receivedData.energy, 3) + ",";
  json += "\"frequency\":" + String(receivedData.frequency, 1) + ",";
  json += "\"pf\":" + String(receivedData.pf, 2) + ",";
  json += "\"valid\":" + String(receivedData.valid ? "true" : "false") + ",";
  json += "\"packets\":" + String(packetCount);
  json += "}";

  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", json);
}

void handleNotFound() {
  server.send(404, "text/plain", "404: Not Found");
}

// ========================================================================
// 5. CETAK SERIAL MONITOR SECARA TERSTRUKTUR
// ========================================================================
void printDataToSerial() {
  Serial.println("\n+--------------------------------------------------+");
  Serial.printf("| [ESP-NOW PACKET #%04lu]                           |\n", packetCount);
  Serial.println("+--------------------------------------------------+");
  if (!receivedData.valid) {
    Serial.println("| STATUS: SENSOR TIDAK VALID / AC DISCONNECTED     |");
  } else {
    Serial.printf("| Tegangan AC   : %7.1f V                         |\n", receivedData.voltage);
    Serial.printf("| Arus Listrik  : %7.3f A                         |\n", receivedData.current);
    Serial.printf("| Daya Aktif    : %7.1f W                         |\n", receivedData.power);
    Serial.printf("| Total Energi  : %7.3f kWh                       |\n", receivedData.energy);
    Serial.printf("| Frekuensi     : %7.1f Hz                        |\n", receivedData.frequency);
    Serial.printf("| Faktor Daya   : %7.2f                           |\n", receivedData.pf);
  }
  Serial.println("+--------------------------------------------------+");
}

// ========================================================================
// 6. SETUP
// ========================================================================
void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n================================================");
  Serial.println("       ESP32 MASTER - GATEWAY & WEB SERVER      ");
  Serial.println("================================================");

  // Default: Inisialisasi Wi-Fi Access Point (Hotspot Lokal Mandiri)
#ifdef CONNECT_TO_HOME_WIFI
  Serial.println("[WIFI] Menghubungkan ke Wi-Fi Rumah...");
  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(STA_SSID, STA_PASS);
  int retry = 0;
  while (WiFi.status() != WL_CONNECTED && retry < 20) {
    delay(500);
    Serial.print(".");
    retry++;
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n[OK] Terhubung ke Wi-Fi!");
    Serial.print("Alamat IP Web Dashboard: http://");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\n[GAGAL] Tidak dapat terhubung ke Wi-Fi. Beralih ke AP Mode...");
    WiFi.mode(WIFI_AP);
    WiFi.softAP(AP_SSID, AP_PASS, WIFI_CHANNEL);
  }
#else
  // Mode Access Point (Hotspot mandiri)
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(AP_SSID, AP_PASS, WIFI_CHANNEL);
  Serial.println("[OK] Wi-Fi Hotspot (AP) Aktif.");
  Serial.print("SSID Hotspot            : "); Serial.println(AP_SSID);
  Serial.print("Password                : "); Serial.println(AP_PASS);
  Serial.print("Wi-Fi Channel           : "); Serial.println(WIFI_CHANNEL);
  Serial.print("Alamat IP Web Dashboard : http://"); Serial.println(WiFi.softAPIP());
#endif

  Serial.print("Master MAC Address      : ");
  Serial.println(WiFi.macAddress());

  // Inisialisasi ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("[ERROR] Gagal menginisialisasi ESP-NOW!");
    return;
  }
  Serial.println("[OK] ESP-NOW berhasil diinisialisasi.");

  // Daftarkan Callback Penerimaan ESP-NOW
  esp_now_register_recv_cb(OnDataRecv);

  // Setup Rute Web Server
  server.on("/", handleRoot);
  server.on("/api/data", handleDataApi);
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("[OK] Local Web Server siap melayani permintaan HTTP.");
  Serial.println("================================================\n");
}

// ========================================================================
// 7. LOOP UTAMA
// ========================================================================
void loop() {
  // Melayani permintaan web client
  server.handleClient();

  // Jika ada data baru dari ESP-NOW, cetak ke Serial Monitor
  if (newDataAvailable) {
    newDataAvailable = false;
    printDataToSerial();
  }
}
