/*
  PEÓN DE LLEGADA - v3.6
  ANCHOR UWB + HC-SR04 POR INTERRUPCIÓN + ESP-NOW

  Objetivo de esta versión:
  - Usar exactamente la misma lógica rápida de ultrasonido y LED del peón de salida.
  - El LED indica presencia en tiempo real: encendido si detecta dentro del rango, apagado si no.
  - No usa pulseIn() ni delay() en el funcionamiento normal.
  - Mantiene el UWB como ANCHOR.
  - Envía MSG_FINISH al cerebro.

  Hardware:
  - ESP32
  - HC-SR04: TRIG GPIO15, ECHO GPIO2
  - LED: GPIO16
  - BU01 / DW1000 como ANCHOR:
      SCK  GPIO18
      MISO GPIO19
      MOSI GPIO23
      CS   GPIO4
      RST  GPIO27
      IRQ  GPIO34
*/

#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <esp_arduino_version.h>
#ifndef ESP_ARDUINO_VERSION_MAJOR
#define ESP_ARDUINO_VERSION_MAJOR 2
#endif

#include <SPI.h>
#include <math.h>
#include <string.h>
#include <driver/gpio.h>
#include "DW1000Ranging.h"

// ================= PINES =================
#define TRIG_PIN 15
#define ECHO_PIN 2
#define LED_PIN  16

#define SPI_SCK  18
#define SPI_MISO 19
#define SPI_MOSI 23
#define DW_CS    4

const uint8_t PIN_RST = 27;
const uint8_t PIN_IRQ = 34;
const uint8_t PIN_SS  = 4;

// ================= CONFIG =================
#define ESPNOW_CHANNEL 1

// Misma lógica de detección usada en salida.
// Se detecta al usuario hasta 1 metro.
static const float UMBRAL_MIN_CM = 2.0f;
static const float UMBRAL_MAX_CM = 100.0f;
static const uint32_t debounce_ms = 1500UL;
static const uint8_t nLecturasConfirmar = 1;

// 10000 us cubre aprox. 1.7 m, suficiente para detectar 1 m sin esperar demasiado.
static const uint32_t US_PERIOD_MS  = 25UL;
static const uint32_t US_TIMEOUT_US = 10000UL;

// Evita que el anchor sature Serial y retrase el loop.
#define DEBUG_UWB_ANCHOR 0

// MAC del CEREBRO o broadcast.
uint8_t macCerebro[] = {0x94, 0xB5, 0x55, 0x5E, 0xCC, 0x10};

// Dirección ANCHOR usada en tu código previo.
#define ANCHOR_ADD "83:17:5B:D5:A9:9A:E2:9C"

// ================= PROTOCOLO =================
enum : uint8_t {
  MSG_START    = 1,
  MSG_FINISH   = 2,
  MSG_DISTANCE = 3,
  MSG_HR       = 10,
  MSG_CMD      = 20
};

enum : uint8_t {
  SRC_PEON_SALIDA  = 1,
  SRC_PEON_LLEGADA = 2,
  SRC_BANDA_HR     = 3,
  SRC_CEREBRO      = 4
};

enum : uint8_t {
  PHASE_IDLE     = 0,
  PHASE_RUN      = 1,
  PHASE_RECOVERY = 2
};

typedef struct __attribute__((packed)) {
  uint8_t  msgType;
  uint8_t  src;
  uint16_t msgId;
  uint32_t ms;
  uint16_t raceId;
  uint8_t  athleteId;
  uint8_t  phase;
  int16_t  value1;
  int16_t  value2;
} Packet;

// ================= ESTADO =================
static uint16_t msgId = 0;
static uint32_t ultimaDet_ms = 0;
static uint8_t bajoUmbralCount = 0;
static bool ultimoEstadoDetectado = false;

// ================= DECLARACIONES =================
void enviarFinish(float distancia_cm);
void enviarEventoPeon(float distancia_cm);

// ================= LED DIRECTO POR PRESENCIA =================
/*
  Misma filosofía que salida:
  - LED encendido = presencia dentro del rango.
  - LED apagado = sin presencia válida.
  - No se usa parpadeo, porque el parpadeo puede ocultar la detección real.
*/
void actualizarLedPresencia(bool detectado) {
  gpio_set_level((gpio_num_t)LED_PIN, detectado ? 1 : 0);
}

void servicioLED() {
  // No bloqueante. El LED se actualiza directamente al procesar cada medición.
}

// ================= ULTRASONIDO POR INTERRUPCIÓN =================
/*
  Igual que el peón de salida:
  - ECHO se mide con interrupción CHANGE.
  - TRIG se dispara periódicamente.
  - No usa pulseIn().
  - No bloquea el UWB ni ESP-NOW.
*/
volatile uint32_t usEchoInicioUs = 0;
volatile uint32_t usDuracionUs = 0;
volatile uint32_t usUltimoDisparoUs = 0;
volatile bool usMedicionLista = false;
volatile bool usEsperandoEco = false;
volatile bool usEchoAlto = false;

static uint32_t usUltimoTriggerMs = 0;

void IRAM_ATTR isrEchoUltrasonido() {
  uint32_t ahora = micros();
  int nivel = gpio_get_level((gpio_num_t)ECHO_PIN);

  if (nivel) {
    usEchoInicioUs = ahora;
    usEchoAlto = true;
  } else if (usEchoAlto) {
    usDuracionUs = ahora - usEchoInicioUs;
    usMedicionLista = true;
    usEchoAlto = false;
    usEsperandoEco = false;
  }
}

void dispararUltrasonido() {
  noInterrupts();
  usMedicionLista = false;
  usEchoAlto = false;
  usEsperandoEco = true;
  interrupts();

  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  noInterrupts();
  usUltimoDisparoUs = micros();
  interrupts();
}

void servicioTriggerUltrasonido() {
  bool esperando;
  noInterrupts();
  esperando = usEsperandoEco;
  interrupts();

  if (!esperando && (millis() - usUltimoTriggerMs >= US_PERIOD_MS)) {
    usUltimoTriggerMs = millis();
    dispararUltrasonido();
  }
}

bool obtenerMedicionUltrasonido(float &distancia_cm) {
  bool lista = false;
  bool timeout = false;
  uint32_t duracion = 0;

  noInterrupts();
  if (usMedicionLista) {
    lista = true;
    duracion = usDuracionUs;
    usMedicionLista = false;
  } else if (usEsperandoEco && (uint32_t)(micros() - usUltimoDisparoUs) > US_TIMEOUT_US) {
    timeout = true;
    usEsperandoEco = false;
    usEchoAlto = false;
  }
  interrupts();

  if (lista) {
    if (duracion == 0 || duracion > US_TIMEOUT_US) {
      distancia_cm = -1.0f;
    } else {
      distancia_cm = (duracion * 0.0343f) / 2.0f;
    }
    return true;
  }

  if (timeout) {
    distancia_cm = -1.0f;
    return true;
  }

  return false;
}

void procesarMedicionUltrasonido(float d) {
  bool detectado = (d >= UMBRAL_MIN_CM && d <= UMBRAL_MAX_CM);
  actualizarLedPresencia(detectado);

  if (detectado != ultimoEstadoDetectado) {
    ultimoEstadoDetectado = detectado;
    Serial.print(detectado ? "HC-SR04 DETECTA: " : "HC-SR04 libre/fuera de rango: ");
    Serial.print(d, 1);
    Serial.println(" cm");
  }

  if (detectado) {
    bajoUmbralCount++;
  } else {
    bajoUmbralCount = 0;
  }

  uint32_t ahora = millis();
  if (bajoUmbralCount >= nLecturasConfirmar) {
    if (ahora - ultimaDet_ms > debounce_ms) {
      ultimaDet_ms = ahora;
      bajoUmbralCount = 0;
      enviarEventoPeon(d);
    }
  }
}

void configurarUltrasonido() {
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  digitalWrite(TRIG_PIN, LOW);
  attachInterrupt(digitalPinToInterrupt(ECHO_PIN), isrEchoUltrasonido, CHANGE);
}

// ================= ESPNOW =================
#if ESP_ARDUINO_VERSION_MAJOR >= 3
static void onDataSent(const wifi_tx_info_t *info, esp_now_send_status_t status) {
  (void)info;
#else
static void onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  (void)mac_addr;
#endif
  Serial.print("ESP-NOW FINISH send: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "OK" : "FAIL");
}

void enviarFinish(float distancia_cm) {
  Packet p{};
  p.msgType = MSG_FINISH;
  p.src = SRC_PEON_LLEGADA;
  p.msgId = ++msgId;
  p.ms = millis();
  p.phase = PHASE_IDLE;
  p.value1 = (int16_t)lroundf(distancia_cm * 10.0f); // cm*10 del HC-SR04
  p.value2 = 0;

  for (int i = 0; i < 3; i++) {
    esp_now_send(macCerebro, (uint8_t*)&p, sizeof(p));
  }

  Serial.print(">>> FINISH enviado. Dist HC-SR04=");
  Serial.print(distancia_cm, 1);
  Serial.println(" cm");
}

void enviarEventoPeon(float distancia_cm) {
  enviarFinish(distancia_cm);
}

void configurarEspNow() {
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  esp_wifi_set_channel(ESPNOW_CHANNEL, WIFI_SECOND_CHAN_NONE);

  if (esp_now_init() != ESP_OK) {
    Serial.println("ERROR: esp_now_init()");
    while (true) delay(1000);
  }

  esp_now_register_send_cb(onDataSent);

  esp_now_peer_info_t peer{};
  memcpy(peer.peer_addr, macCerebro, 6);
  peer.channel = ESPNOW_CHANNEL;
  peer.ifidx = WIFI_IF_STA;
  peer.encrypt = false;

  if (!esp_now_is_peer_exist(macCerebro)) {
    if (esp_now_add_peer(&peer) != ESP_OK) {
      Serial.println("ERROR: esp_now_add_peer(macCerebro)");
      while (true) delay(1000);
    }
  }
}

// ================= UWB ANCHOR =================
void newRange() {
#if DEBUG_UWB_ANCHOR
  Serial.print("UWB from: ");
  Serial.print(DW1000Ranging.getDistantDevice()->getShortAddress(), HEX);
  Serial.print(" Range: ");
  Serial.print(DW1000Ranging.getDistantDevice()->getRange());
  Serial.print(" m RX: ");
  Serial.print(DW1000Ranging.getDistantDevice()->getRXPower());
  Serial.println(" dBm");
#endif
}

void newBlink(DW1000Device *device) {
#if DEBUG_UWB_ANCHOR
  Serial.print("UWB blink, device added. short: ");
  Serial.println(device->getShortAddress(), HEX);
#else
  (void)device;
#endif
}

void inactiveDevice(DW1000Device *device) {
#if DEBUG_UWB_ANCHOR
  Serial.print("UWB dispositivo inactivo: ");
  Serial.println(device->getShortAddress(), HEX);
#else
  (void)device;
#endif
}

void configurarUWB() {
  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
  DW1000Ranging.initCommunication(PIN_RST, PIN_SS, PIN_IRQ);
  DW1000Ranging.attachNewRange(newRange);
  DW1000Ranging.attachBlinkDevice(newBlink);
  DW1000Ranging.attachInactiveDevice(inactiveDevice);
  DW1000Ranging.startAsAnchor(ANCHOR_ADD, DW1000.MODE_SHORTDATA_FAST_LOWPOWER, false);
}

// ================= SETUP / LOOP =================
void setup() {
  Serial.begin(115200);
  delay(500);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  configurarUltrasonido();
  configurarEspNow();
  configurarUWB();

  Serial.println();
  Serial.println("PEÓN DE LLEGADA - v3.6 IGUAL A SALIDA: HC-SR04 ISR FAST + LED DIRECTO + ANCHOR UWB");
  Serial.print("MAC local: ");
  Serial.println(WiFi.macAddress());
  Serial.print("Canal ESPNOW: ");
  Serial.println(ESPNOW_CHANNEL);
  Serial.println("Listo.");
}

void loop() {
  DW1000Ranging.loop();
  servicioLED();
  servicioTriggerUltrasonido();

  float d = -1.0f;
  if (obtenerMedicionUltrasonido(d)) {
    procesarMedicionUltrasonido(d);
  }
}
