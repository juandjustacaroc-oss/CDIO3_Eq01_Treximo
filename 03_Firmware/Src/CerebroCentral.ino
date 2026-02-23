/* 
 * CEREBRO CENTRAL - Sistema Atletismo
 * ESP32 + MAX3010x + OLED + ESP-NOW
 * Recibe señales de salida y llegada de los peones
 */

#include <WiFi.h>
#include <esp_now.h>
#include <Wire.h>
#include "MAX30105.h"
#include "heartRate.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ========== CONFIGURACIÓN MAC ==========
// Direcciones MAC de los 3 ESP32
uint8_t macCerebro[]    = {0x08, 0xB6, 0x1F, 0xBE, 0x2D, 0x34};      // CEREBRO (COM7)
uint8_t macPeonSalida[] = {0x00, 0x4B, 0x12, 0x21, 0x73, 0xC0};      // PEÓN SALIDA (COM8)
uint8_t macPeonLlegada[]= {0xEC, 0x64, 0xC9, 0x86, 0xD0, 0x98};      // PEÓN LLEGADA (COM12)

// ========== OLED ==========
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ========== MAX3010x ==========
MAX30105 particleSensor;
const byte RATE_SIZE = 4;
byte rates[RATE_SIZE];
byte rateSpot = 0;
long lastBeat = 0;
float beatsPerMinute;
int beatAvg;

// ========== ESTADOS ==========
enum EstadoCarrera {
  ESPERANDO_SALIDA,
  EN_CARRERA,
  RECUPERACION,
  FINALIZADO
};

EstadoCarrera estado = ESPERANDO_SALIDA;
unsigned long tInicio_ms = 0;
unsigned long tLlegada_ms = 0;
unsigned long tFinRecuperacion_ms = 0;

const unsigned long VENTANA_RECUP_MS = 60000;  // 60 segundos
const float distanciaPista_m = 100.0;

// ========== BOTÓN REINICIO ==========
// Botón entre PIN 23 y GND, usando INPUT_PULLUP (reposo = HIGH, presionado = LOW)
#define BOTON_REINICIO 23
#define LED_REINICIO   4

unsigned long ultimaPulsacion   = 0;
const unsigned long DEBOUNCE_BOTON = 300;  // 300 ms
bool botonPresionado            = false;

// ========== ESTRUCTURA DE MENSAJE ==========
typedef struct __attribute__((packed)) {
  uint8_t tipo;           // 1 = SALIDA, 2 = LLEGADA, 3 = REINICIO
  uint32_t tEvento_ms;    // Timestamp del evento
  uint8_t peonID;         // Identificador del peón
} MensajeCarrera;

// ========== CALLBACKS ==========
void onDataRecv(const esp_now_recv_info_t *info, const uint8_t *incomingData, int len) {
  if (len != sizeof(MensajeCarrera)) return;

  MensajeCarrera msg;
  memcpy(&msg, incomingData, sizeof(msg));

  // Ignorar mensajes de reinicio de vuelta
  if (msg.tipo == 3) {
    Serial.println("DEBUG: Mensaje tipo 3 (REINICIO) ignorado - no procesar reinicios de vuelta");
    return;
  }

  unsigned long tAhora = millis();

  if (msg.tipo == 1 && estado == ESPERANDO_SALIDA) {
    // SALIDA DETECTADA
    tInicio_ms = tAhora;
    estado = EN_CARRERA;

    Serial.println("\n>>> SALIDA RECIBIDA (ESP-NOW)");
    Serial.print("Tiempo inicio: ");
    Serial.println(tInicio_ms);
  }
  else if (msg.tipo == 2 && estado == EN_CARRERA) {
    // LLEGADA DETECTADA
    tLlegada_ms = tAhora;
    estado = RECUPERACION;
    tFinRecuperacion_ms = tAhora + VENTANA_RECUP_MS;

    Serial.println("\n>>> LLEGADA RECIBIDA (ESP-NOW)");
    Serial.print("Tiempo llegada: ");
    Serial.println(tLlegada_ms);
    Serial.println("Iniciando fase RECUPERACION...");
  }
}

void onDataSent(const wifi_tx_info_t *info, esp_now_send_status_t status) {
  // opcional
}

// ========== PROTOTIPOS ==========
void mostrarPantallaEstado();
void mostrarResultadosFinales();
void leerSensorCardiaco();
void inicializarOLED();
void inicializarMAX3010x();
void inicializarESPNOW();
void verificarBotonReinicio();
void enviarReinicio();
void reiniciarSistema();

// ========== SETUP ==========
void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n=== CEREBRO - ESP32 (ESP-NOW) ===");

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  
  Serial.print("MAC CEREBRO: ");
  Serial.println(WiFi.macAddress());

  // Pines del botón y LED de reinicio
  pinMode(BOTON_REINICIO, INPUT_PULLUP);   // botón a GND → LOW = presionado
  pinMode(LED_REINICIO, OUTPUT);
  digitalWrite(LED_REINICIO, LOW);

  inicializarESPNOW();
  inicializarOLED();
  inicializarMAX3010x();

  mostrarPantallaEstado();
}

// ========== LOOP ==========
void loop() {
  leerSensorCardiaco();
  verificarBotonReinicio();   // <-- AHORA SÍ SE LEE EL BOTÓN

  static unsigned long ultimoRefresh = 0;
  if (millis() - ultimoRefresh > 500) {
    ultimoRefresh = millis();
    mostrarPantallaEstado();
  }

  if (estado == RECUPERACION && millis() > tFinRecuperacion_ms) {
    estado = FINALIZADO;
    Serial.println("\n=== FIN DE VENTANA DE RECUPERACION ===");
    mostrarResultadosFinales();
  }
}

// ========== FUNCIONES ==========
void inicializarESPNOW() {
  if (esp_now_init() != ESP_OK) {
    Serial.println("✗ Error inicializando ESP-NOW");
    while (true) delay(1000);
  }
  
  esp_now_register_recv_cb(onDataRecv);
  esp_now_register_send_cb(onDataSent);
  
  // Registrar Peón SALIDA como peer
  esp_now_peer_info_t peerSalida;
  memset(&peerSalida, 0, sizeof(peerSalida));
  memcpy(peerSalida.peer_addr, macPeonSalida, 6);
  peerSalida.channel = 0;
  peerSalida.encrypt = false;
  
  if (esp_now_add_peer(&peerSalida) != ESP_OK) {
    Serial.println("✗ Error agregando Peón SALIDA");
    while (true) delay(1000);
  }
  Serial.println("✓ Peón SALIDA registrado como peer");
  
  // Registrar Peón LLEGADA como peer
  esp_now_peer_info_t peerLlegada;
  memset(&peerLlegada, 0, sizeof(peerLlegada));
  memcpy(peerLlegada.peer_addr, macPeonLlegada, 6);
  peerLlegada.channel = 0;
  peerLlegada.encrypt = false;
  
  if (esp_now_add_peer(&peerLlegada) != ESP_OK) {
    Serial.println("✗ Error agregando Peón LLEGADA");
    while (true) delay(1000);
  }
  Serial.println("✓ Peón LLEGADA registrado como peer");
  Serial.println("✓ ESP-NOW inicializado correctamente");
}

void inicializarOLED() {
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println("Error OLED no detectado");
  } else {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("CEREBRO - ATLETISMO");
    display.println("ESP-NOW listo");
    display.display();
    delay(2000);
    Serial.println("OLED inicializado");
  }
}

void inicializarMAX3010x() {
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) {
    Serial.println("MAX3010x no detectado");
  } else {
    particleSensor.setup();
    particleSensor.setPulseAmplitudeRed(0x0A);
    particleSensor.setPulseAmplitudeGreen(0);
    Serial.println("MAX3010x inicializado");
  }
}

void leerSensorCardiaco() {
  long irValue = particleSensor.getIR();
  if (irValue < 50000) return;

  if (checkForBeat(irValue)) {
    long delta = millis() - lastBeat;
    lastBeat = millis();
    
    if (delta > 0) {
      beatsPerMinute = 60.0 / (delta / 1000.0);

      if (beatsPerMinute < 255 && beatsPerMinute > 20) {
        rates[rateSpot++] = (byte)beatsPerMinute;
        rateSpot %= RATE_SIZE;

        int sum = 0;
        for (byte i = 0; i < RATE_SIZE; i++) sum += rates[i];
        beatAvg = sum / RATE_SIZE;
      }
    }
  }
}

void mostrarPantallaEstado() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("CEREBRO ATLETISMO");
  display.drawLine(0, 10, 128, 10, SSD1306_WHITE);

  display.setCursor(0, 15);
  display.print("Estado: ");
  switch (estado) {
    case ESPERANDO_SALIDA: display.println("ESPERA"); break;
    case EN_CARRERA:       display.println("EN CARRERA"); break;
    case RECUPERACION:     display.println("RECUPERACION"); break;
    case FINALIZADO:       display.println("FINALIZADO"); break;
  }

  display.setCursor(0, 30);
  display.print("FC Prom: ");
  if (beatAvg > 0) {
    display.print(beatAvg);
    display.println(" BPM");
  } else {
    display.println("---");
  }

  if (estado == EN_CARRERA || estado == RECUPERACION || estado == FINALIZADO) {
    float tiempoSeg = 0;
    if (tInicio_ms > 0) {
      unsigned long tRef = (estado == EN_CARRERA) ? millis() : tLlegada_ms;
      tiempoSeg = (tRef - tInicio_ms) / 1000.0;
    }
    display.setCursor(0, 45);
    display.print("Tiempo: ");
    display.print(tiempoSeg, 1);
    display.println(" s");
  }

  display.display();
}

// ========== REINICIO DEL SISTEMA ==========
void verificarBotonReinicio() {
  // Con INPUT_PULLUP: HIGH = suelto, LOW = presionado
  int lectura = digitalRead(BOTON_REINICIO);

  if (lectura == LOW) {   // botón presionado
    if (!botonPresionado) {    // flanco de bajada
      unsigned long ahora = millis();
      if (ahora - ultimaPulsacion > DEBOUNCE_BOTON) {
        ultimaPulsacion = ahora;
        botonPresionado = true;

        Serial.println("\n========================================");
        Serial.println(">>> BOTÓN REINICIO PRESIONADO");
        Serial.println("========================================");
        reiniciarSistema();
      }
    }
  } else {
    // botón suelto
    botonPresionado = false;
  }
}

void reiniciarSistema() {
  Serial.println("Reiniciando estado del CEREBRO...");
  
  // Reiniciar variables
  estado = ESPERANDO_SALIDA;
  tInicio_ms = 0;
  tLlegada_ms = 0;
  tFinRecuperacion_ms = 0;
  beatAvg = 0;
  
  Serial.println("Estado reiniciado - Enviando señal a peones...");
  
  // Parpadeo LED de confirmación
  for (int i = 0; i < 5; i++) {
    digitalWrite(LED_REINICIO, HIGH);
    delay(100);
    digitalWrite(LED_REINICIO, LOW);
    delay(100);
  }
  
  enviarReinicio();
  
  Serial.println("\n✓ Sistema reiniciado - Esperando nueva salida");
  
  // Mostrar en OLED
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("SISTEMA REINICIADO");
  display.println("Esperando salida...");
  display.display();
  delay(2000);
  
  Serial.println("========================================\n");
}

void enviarReinicio() {
  MensajeCarrera msg;
  msg.tipo = 3;          // REINICIO
  msg.tEvento_ms = millis();
  msg.peonID = 0;
  
  Serial.println("\n[REINICIO] Preparando mensaje...");
  Serial.print("  Tipo: ");
  Serial.println(msg.tipo);
  Serial.print("  Timestamp: ");
  Serial.println(msg.tEvento_ms);
  Serial.print("  Tamaño: ");
  Serial.print(sizeof(msg));
  Serial.println(" bytes");
  
  // Enviar a peón SALIDA
  Serial.print("\n[REINICIO] Enviando a Peón SALIDA: ");
  int status1 = esp_now_send(macPeonSalida, (uint8_t*)&msg, sizeof(msg));
  if (status1 == ESP_OK) {
    Serial.println("✓ ENVIADO");
  } else {
    Serial.print("✗ ERROR (código: ");
    Serial.print(status1);
    Serial.println(")");
  }
  delay(300);
  
  // Enviar a peón LLEGADA
  Serial.print("[REINICIO] Enviando a Peón LLEGADA: ");
  int status2 = esp_now_send(macPeonLlegada, (uint8_t*)&msg, sizeof(msg));
  if (status2 == ESP_OK) {
    Serial.println("✓ ENVIADO");
  } else {
    Serial.print("✗ ERROR (código: ");
    Serial.print(status2);
    Serial.println(")");
  }
  
  Serial.println("[REINICIO] Proceso completado\n");
}

void mostrarResultadosFinales() {
  float tiempoCarrera_s = (tLlegada_ms - tInicio_ms) / 1000.0;
  float vel_m_s = distanciaPista_m / tiempoCarrera_s;
  float vel_km_h = vel_m_s * 3.6;

  Serial.println("\n=== REPORTE FINAL ===");
  Serial.print("Tiempo carrera: ");
  Serial.print(tiempoCarrera_s, 2);
  Serial.println(" s");

  Serial.print("Velocidad prom: ");
  Serial.print(vel_m_s, 2);
  Serial.print(" m/s (");
  Serial.print(vel_km_h, 2);
  Serial.println(" km/h)");

  Serial.print("FC promedio: ");
  Serial.print(beatAvg);
  Serial.println(" BPM");

  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("RESULTADO FINAL");
  display.drawLine(0, 10, 128, 10, SSD1306_WHITE);

  display.setCursor(0, 15);
  display.print("Tiempo: ");
  display.print(tiempoCarrera_s, 2);
  display.println(" s");

  display.setCursor(0, 30);
  display.print("Vel: ");
  display.print(vel_km_h, 1);
  display.println(" km/h");

  display.setCursor(0, 45);
  display.print("FC Prom: ");
  display.print(beatAvg);
  display.println(" BPM");

  display.display();
}