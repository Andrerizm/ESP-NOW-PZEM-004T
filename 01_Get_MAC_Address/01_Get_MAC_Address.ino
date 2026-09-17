/*
  ========================================================================
  01_Get_MAC_Address.ino
  Program untuk membaca dan menampilkan MAC Address Wi-Fi ESP32.
  
  Gunakan program ini pada ESP32 MASTER untuk mengetahui MAC Address-nya.
  MAC Address ini nantinya akan dimasukkan ke kode ESP32 SLAVE (Pengirim).
  ========================================================================
*/

#include <WiFi.h>

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n================================================");
  Serial.println("       ESP32 MAC ADDRESS SCANNER (STATION)      ");
  Serial.println("================================================");

  // Set Wi-Fi ke mode Station untuk membaca MAC Station (digunakan ESP-NOW)
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  String macStr = WiFi.macAddress();
  Serial.print("Format Standar   : ");
  Serial.println(macStr);

  // Dapatkan MAC dalam format array byte uint8_t
  uint8_t mac[6];
  WiFi.macAddress(mac);

  Serial.print("Format C++ Array : {");
  for (int i = 0; i < 6; i++) {
    Serial.print("0x");
    if (mac[i] < 0x10) Serial.print("0");
    Serial.print(mac[i], HEX);
    if (i < 5) Serial.print(", ");
  }
  Serial.println("};");

  Serial.println("================================================");
  Serial.println("Salin baris 'Format C++ Array' di atas dan ");
  Serial.println("tempelkan (paste) pada variabel 'masterMacAddress'");
  Serial.println("di dalam file '02_Slave_PZEM_Sender.ino'.");
  Serial.println("================================================\n");
}

void loop() {
  // Tidak ada proses berulang yang diperlukan
  delay(5000);
}
