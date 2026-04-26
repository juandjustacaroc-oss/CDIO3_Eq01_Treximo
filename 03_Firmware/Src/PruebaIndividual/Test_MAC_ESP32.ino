#include <esp_wifi.h>
#include <esp_mac.h>

void setup() {
  Serial.begin(115200);
  delay(2000);
  
  Serial.println("\n\n");
  Serial.println("================================");
  Serial.println("LEYENDO MAC DEL HARDWARE");
  Serial.println("================================\n");
  
  // Leer MAC directamente del hardware
  uint8_t mac[6];
  esp_read_mac(mac, ESP_MAC_WIFI_STA);
  
  Serial.print("Dirección MAC: ");
  for (int i = 0; i < 6; i++) {
    if (i > 0) Serial.print(":");
    if (mac[i] < 0x10) Serial.print("0");
    Serial.print(mac[i], HEX);
  }
  Serial.println();
  
  Serial.println("\nCopia esto en tu código:");
  Serial.print("uint8_t miMAC[] = {");
  for (int i = 0; i < 6; i++) {
    if (i > 0) Serial.print(", ");
    Serial.print("0x");
    if (mac[i] < 0x10) Serial.print("0");
    Serial.print(mac[i], HEX);
  }
  Serial.println("};");
  
  Serial.println("\n================================");
  Serial.println("INFORMACIÓN ADICIONAL:");
  Serial.println("================================");
  
  // Chip info
  Serial.print("Modelo: ESP32");
  Serial.print(" | Revisión: ");
  Serial.print(ESP.getChipRevision());
  Serial.print(" | Núcleos: ");
  Serial.println(ESP.getChipCores());
  
  Serial.print("MAC Completa (BT): ");
  esp_read_mac(mac, ESP_MAC_BT);
  for (int i = 0; i < 6; i++) {
    if (i > 0) Serial.print(":");
    if (mac[i] < 0x10) Serial.print("0");
    Serial.print(mac[i], HEX);
  }
  Serial.println();
  
  Serial.println("\n================================\n");
}

void loop() {
  delay(1000);
}
