/*
  ======================================
  TEST 8A: ESP-NOW TRANSMISOR
  ======================================
  Objetivo: Enviar datos periódicamente vía ESP-NOW
  
  Pines: GPIO 2 (LED opcional para indicar envío)
  
  Librerías:
  - WiFi.h (incluida en Arduino Core)
  - esp_now.h (incluida en Arduino Core)
  - esp_wifi.h (incluida en Arduino Core)
  
  INSTRUCCIONES:
  1. Carga este código EN UN ESP32 (TRANSMISOR)
  2. Carga TEST 8B en OTRO ESP32 (RECEPTOR)
  3. Obtén la MAC del RECEPTOR con Test 7
  4. Reemplaza macReceptor[] con la MAC del receptor
  5. Abre Serial Monitor en ambos a 115200 bps
  6. Deberías ver mensajes en el receptor
  
  Requisitos:
  - 2 ESP32
  - USB para depuración
  - Proximidad (radio ESP-NOW: ~100 metros en espacio abierto)
  
  Características:
  - Envía estructura de datos cada 1 segundo
  - Incluye tipo de mensaje, ID, contador, timestamp
  - Callback de envío para verificar estado
  - Sin WiFi, solo ESP-NOW
*/

#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <esp_arduino_version.h>

#ifndef ESP_ARDUINO_VERSION_MAJOR
#define ESP_ARDUINO_VERSION_MAJOR 2
#endif

// ====== CONFIGURACIÓN ======
#define CANAL_ESPNOW 1
#define INTERVALO_ENVIO 1000  // ms
#define PIN_LED 2

// ⚠️  REEMPLAZAR CON LA MAC DEL RECEPTOR
// Usa Test 7 para obtener la MAC real del ESP32 receptor
uint8_t macReceptor[] = {0x48, 0xE7, 0x29, 0xXX, 0xXX, 0xXX};

// ====== ESTRUCTURA DE DATOS ======
typedef struct __attribute__((packed)) {
  uint8_t  tipoMensaje;     // Tipo de mensaje
  uint8_t  idDispositivo;   // ID del transmisor
  uint16_t contador;        // Contador incremental
  uint32_t timestamp;       // Marca de tiempo (ms)
  int16_t  valor1;          // Valor adicional 1
  int16_t  valor2;          // Valor adicional 2
} MensajeDatos;

// ====== VARIABLES ======
MensajeDatos datosEnvio;
uint16_t contadorMensajes = 0;
unsigned long ultimoEnvio = 0;
uint32_t mensajesEnviados = 0;
uint32_t mensajesFallidos = 0;

// ====== CALLBACKS ESP-NOW ======

#if ESP_ARDUINO_VERSION_MAJOR >= 3
void callbackEnvio(const wifi_tx_info_t *info, esp_now_send_status_t status) {
  (void)info;
  if (status == ESP_NOW_SEND_SUCCESS) {
    mensajesEnviados++;
  } else {
    mensajesFallidos++;
  }
}
#else
void callbackEnvio(const uint8_t *mac, esp_now_send_status_t status) {
  (void)mac;
  if (status == ESP_NOW_SEND_SUCCESS) {
    mensajesEnviados++;
  } else {
    mensajesFallidos++;
  }
}
#endif

// ====== SETUP ======
void setup() {
  Serial.begin(115200);
  delay(1000);
  
  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, LOW);
  
  Serial.println("\n=====================================");
  Serial.println("  TEST 8A: ESP-NOW TRANSMISOR");
  Serial.println("=====================================\n");
  
  // Mostrar MAC del transmisor
  Serial.print("📡 MAC Transmisor: ");
  Serial.println(WiFi.macAddress());
  
  Serial.print("📤 MAC Receptor:   ");
  for (int i = 0; i < 6; i++) {
    if (macReceptor[i] < 0x10) Serial.print("0");
    Serial.print(macReceptor[i], HEX);
    if (i < 5) Serial.print(":");
  }
  Serial.println("\n");
  
  // Inicializar WiFi
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  
  // Configurar canal
  esp_wifi_set_channel(CANAL_ESPNOW, WIFI_SECOND_CHAN_NONE);
  
  // Inicializar ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("ERROR: No se pudo inicializar ESP-NOW");
    while (1) {
      digitalWrite(PIN_LED, !digitalRead(PIN_LED));
      delay(200);
    }
  }
  
  // Registrar callback de envío
  esp_now_register_send_cb(callbackEnvio);
  
  // Agregar receptor como peer
  esp_now_peer_info_t peer{};
  memcpy(peer.peer_addr, macReceptor, 6);
  peer.channel = CANAL_ESPNOW;
  peer.encrypt = false;
  
  if (esp_now_add_peer(&peer) != ESP_OK) {
    Serial.println("ERROR: No se pudo agregar peer (receptor)");
    while (1) {
      digitalWrite(PIN_LED, !digitalRead(PIN_LED));
      delay(500);
    }
  }
  
  Serial.println("✓ ESP-NOW inicializado");
  Serial.println("✓ Receptor agregado como peer");
  Serial.println("\nComenzando transmisión...\n");
}

// ====== LOOP ======
void loop() {
  unsigned long ahora = millis();
  
  // Enviar datos periódicamente
  if (ahora - ultimoEnvio >= INTERVALO_ENVIO) {
    ultimoEnvio = ahora;
    
    // Preparar datos
    datosEnvio.tipoMensaje = 1;      // Tipo: datos normales
    datosEnvio.idDispositivo = 10;   // ID transmisor
    datosEnvio.contador = contadorMensajes++;
    datosEnvio.timestamp = ahora;
    datosEnvio.valor1 = random(0, 100);   // Sensor 1 simulado
    datosEnvio.valor2 = random(50, 150);  // Sensor 2 simulado
    
    // Enviar
    esp_err_t resultado = esp_now_send(
      macReceptor,
      (uint8_t *)&datosEnvio,
      sizeof(datosEnvio)
    );
    
    // Feedback visual
    digitalWrite(PIN_LED, HIGH);
    delay(50);
    digitalWrite(PIN_LED, LOW);
    
    // Información en serial
    Serial.print("📤 Enviado #");
    Serial.print(datosEnvio.contador);
    Serial.print(" | Timestamp: ");
    Serial.print(datosEnvio.timestamp);
    Serial.print(" | Valores: ");
    Serial.print(datosEnvio.valor1);
    Serial.print(", ");
    Serial.print(datosEnvio.valor2);
    Serial.print(" | Estado: ");
    
    if (resultado == ESP_OK) {
      Serial.println("OK (pendiente ACK)");
    } else {
      Serial.println("FALLO");
    }
    
    // Mostrar estadísticas cada 10 envíos
    if (contadorMensajes % 10 == 0) {
      Serial.print("Estadísticas: Enviados=");
      Serial.print(mensajesEnviados);
      Serial.print(" Fallidos=");
      Serial.println(mensajesFallidos);
    }
  }
  
  delay(10);
}

/*
  =====================================
  TEST 8B: ESP-NOW RECEPTOR
  =====================================
*/

/*
  CÓDIGO PARA EL RECEPTOR (copiar en otro ESP32):
  
  #include <WiFi.h>
  #include <esp_now.h>
  #include <esp_wifi.h>
  #include <esp_arduino_version.h>
  
  #ifndef ESP_ARDUINO_VERSION_MAJOR
  #define ESP_ARDUINO_VERSION_MAJOR 2
  #endif
  
  #define CANAL_ESPNOW 1
  
  typedef struct __attribute__((packed)) {
    uint8_t  tipoMensaje;
    uint8_t  idDispositivo;
    uint16_t contador;
    uint32_t timestamp;
    int16_t  valor1;
    int16_t  valor2;
  } MensajeDatos;
  
  uint32_t mensajesRecibidos = 0;
  
  #if ESP_ARDUINO_VERSION_MAJOR >= 3
  void callbackRecepcion(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
    (void)info;
    if (len != sizeof(MensajeDatos)) return;
    
    MensajeDatos datos;
    memcpy(&datos, data, sizeof(datos));
    
    mensajesRecibidos++;
    
    Serial.print("📥 Recibido #");
    Serial.print(datos.contador);
    Serial.print(" de ID=");
    Serial.print(datos.idDispositivo);
    Serial.print(" | Tipo=");
    Serial.print(datos.tipoMensaje);
    Serial.print(" | Timestamp=");
    Serial.print(datos.timestamp);
    Serial.print(" | Valores: ");
    Serial.print(datos.valor1);
    Serial.print(", ");
    Serial.println(datos.valor2);
  }
  #else
  void callbackRecepcion(const uint8_t *mac, const uint8_t *data, int len) {
    if (len != sizeof(MensajeDatos)) return;
    
    MensajeDatos datos;
    memcpy(&datos, data, sizeof(datos));
    
    mensajesRecibidos++;
    
    Serial.print("📥 Recibido #");
    Serial.print(datos.contador);
    Serial.print(" de ID=");
    Serial.print(datos.idDispositivo);
    Serial.print(" | Tipo=");
    Serial.print(datos.tipoMensaje);
    Serial.print(" | Timestamp=");
    Serial.print(datos.timestamp);
    Serial.print(" | Valores: ");
    Serial.print(datos.valor1);
    Serial.print(", ");
    Serial.println(datos.valor2);
  }
  #endif
  
  void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n=====================================");
    Serial.println("  TEST 8B: ESP-NOW RECEPTOR");
    Serial.println("=====================================\n");
    
    Serial.print("📡 MAC Receptor: ");
    Serial.println(WiFi.macAddress());
    Serial.println("\nEsperando mensajes...\n");
    
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    
    esp_wifi_set_channel(CANAL_ESPNOW, WIFI_SECOND_CHAN_NONE);
    
    if (esp_now_init() != ESP_OK) {
      Serial.println("ERROR: No se pudo inicializar ESP-NOW");
      while (1) delay(1000);
    }
    
    esp_now_register_recv_cb(callbackRecepcion);
    
    Serial.println("✓ ESP-NOW inicializado (modo receptor)");
  }
  
  void loop() {
    delay(1000);
    
    if (mensajesRecibidos % 10 == 0 && mensajesRecibidos > 0) {
      Serial.print("Total recibidos: ");
      Serial.println(mensajesRecibidos);
    }
  }
*/

/*
  GUÍA DE DEPURACIÓN:
  
  1. TRANSMISOR NO ENVÍA:
     - Verifica que macReceptor esté correcta (usa Test 7)
     - Verifica que ambos ESP32 estén alimentados
     - Revisa Serial Monitor del transmisor
  
  2. RECEPTOR NO RECIBE:
     - Verifica canal ESPNOW (debe ser el mismo en ambos)
     - Verifica que ambos estén en WIFI_STA mode
     - Comprueba que el callback esté registrado
     - Verifica la MAC del transmisor en el receptor
  
  3. CONEXIÓN INESTABLE:
     - Acerca los dispositivos (ESP-NOW: ~100m máximo)
     - Evita obstáculos (muros, metal)
     - Reduce interferencia WiFi (cambia canal)
  
  4. MENSAJES INCOMPLETOS:
     - Verifica que sizeof(MensajeDatos) sea igual en ambos
     - Usa __attribute__((packed)) para compatibilidad
     - Comprueba endianness si envías datos complejos
*/
