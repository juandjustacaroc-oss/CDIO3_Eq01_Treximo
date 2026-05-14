/*
  PEÓN DE SALIDA - v3.4 TAG UWB + HC-SR04 POR INTERRUPCIÓN + ESP-NOW

  Hardware:
  - ESP32
  - HC-SR04: TRIG GPIO15, ECHO GPIO2
  - LED confirmación: GPIO16
  - BU01 / DW1000 como TAG:
      SCK  GPIO18
      MISO GPIO19
      MOSI GPIO23
      CS   GPIO4
      RST  GPIO27
      IRQ  GPIO34

  Cambios v3.4:
  - Se elimina pulseIn(). El ECHO del HC-SR04 se mide con attachInterrupt().
  - El disparo TRIG es una máquina de estados no bloqueante.
  - El LED ya no usa delay(); parpadea con esp_timer, sin detener UWB ni ESP-NOW.
  - El HC-SR04 y el LED quedan con la misma lógica rápida usada en ambos peones.
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
#include <esp_timer.h>
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

static const float umbral_cm = 50.0f;
static const uint32_t debounce_ms = 1500;
static const uint8_t nLecturasConfirmar = 1;

static const uint32_t US_PERIOD_MS  = 25UL;
static const uint32_t US_TIMEOUT_US = 10000UL;  // suficiente para >1 m, sin bloquear

static const uint8_t UWB_MUESTRAS_OBJETIVO = 10;
static const uint32_t UWB_CAL_TIMEOUT_MS = 25000UL;
static const uint8_t UWB_MUESTRAS_MINIMAS = 7;
static const uint32_t UWB_BUFFER_MAX_AGE_MS = 5000UL;

static const uint64_t LED_HALF_PERIOD_US = 80000ULL;

// MAC del CEREBRO o broadcast.
uint8_t macCerebro[] = {0x94, 0xB5, 0x55, 0x5E, 0xCC, 0x10};

// Dirección TAG usada en tu código previo.
#define TAG_ADD "7D:00:22:EA:82:60:3B:9C"

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

enum : int16_t {
  CMD_SET_PHASE          = 1,
  CMD_CALIBRATE_DISTANCE = 2
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

// ================= ESTADO GENERAL =================
static uint16_t msgId = 0;
static uint32_t ultimaDet_ms = 0;
static uint8_t bajoUmbralCount = 0;

static bool calibrandoUWB = false;
volatile bool solicitudCalUWB = false;
static uint32_t tInicioCalUWB = 0;
static uint8_t nMuestrasUWB = 0;
static float sumaDistUWB = 0.0f;

static float bufferUWB[UWB_MUESTRAS_OBJETIVO];
static uint8_t bufferUWBCount = 0;
static uint8_t bufferUWBIndex = 0;
static uint32_t ultimaMuestraUWB_ms = 0;
static uint32_t seqUWB = 0;
static uint32_t seqInicioCalUWB = 0;

static bool resultadoCalPendiente = false;
static bool resultadoCalOK = false;
static float resultadoCalDist_m = -1.0f;
static uint8_t resultadoCalMuestras = 0;


// ================= DECLARACIONES =================
void enviarStart(float distancia_cm);
void enviarDistanciaUWB(float distancia_m, bool ok);
void enviarEventoPeon(float distancia_cm);

// ================= LED SIN BLOQUEO =================
/*
  LED v3.4:
  - El LED se enciende inmediatamente cuando el HC-SR04 detecta presencia bajo el umbral.
  - El parpadeo de confirmación del evento se hace con esp_timer, sin delay().
  - Al terminar el parpadeo, el LED vuelve automáticamente al estado de presencia.
*/
static esp_timer_handle_t ledTimer = nullptr;
volatile int ledTogglesRestantes = 0;
volatile bool ledEstado = false;
volatile bool ledSolicitaStop = false;
volatile bool ledBlinkActivo = false;
volatile bool ledPresenciaActiva = false;

void aplicarLedPresencia() {
  if (!ledBlinkActivo) {
    gpio_set_level((gpio_num_t)LED_PIN, ledPresenciaActiva ? 1 : 0);
    ledEstado = ledPresenciaActiva;
  }
}

void ledTimerCallback(void *arg) {
  (void)arg;

  if (ledTogglesRestantes <= 0) {
    ledBlinkActivo = false;
    ledSolicitaStop = true;
    return;
  }

  ledEstado = !ledEstado;
  gpio_set_level((gpio_num_t)LED_PIN, ledEstado ? 1 : 0);
  ledTogglesRestantes--;

  if (ledTogglesRestantes <= 0) {
    ledBlinkActivo = false;
    ledSolicitaStop = true;
  }
}

void configurarLedTimer() {
  esp_timer_create_args_t args;
  memset(&args, 0, sizeof(args));
  args.callback = &ledTimerCallback;
  args.name = "ledBlink";

  if (esp_timer_create(&args, &ledTimer) != ESP_OK) {
    Serial.println("ERROR creando timer de LED");
  }
}

void actualizarLedPresencia(bool detectado) {
  ledPresenciaActiva = detectado;
  aplicarLedPresencia();
}

void iniciarParpadeoLED(uint8_t pulsos) {
  if (ledTimer == nullptr) return;

  esp_timer_stop(ledTimer);
  ledBlinkActivo = true;
  ledSolicitaStop = false;
  ledEstado = false;
  gpio_set_level((gpio_num_t)LED_PIN, 0);
  ledTogglesRestantes = pulsos * 2;
  esp_timer_start_periodic(ledTimer, LED_HALF_PERIOD_US);
}

void servicioLED() {
  if (ledSolicitaStop && ledTimer != nullptr) {
    ledSolicitaStop = false;
    esp_timer_stop(ledTimer);
    aplicarLedPresencia();
  }
}

// ================= ULTRASONIDO POR INTERRUPCIÓN =================
/*
  HC-SR04 v3.4:
  - ECHO se mide por interrupción.
  - TRIG usa solo un pulso bloqueante de 10 us, despreciable frente al resto del sistema.
  - No usa pulseIn(), delay() ni pausas por calibración UWB.
  - Misma lógica en salida y llegada.
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

void reiniciarEstadoUltrasonido() {
  noInterrupts();
  usMedicionLista = false;
  usEsperandoEco = false;
  usEchoAlto = false;
  interrupts();
  digitalWrite(TRIG_PIN, LOW);
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
  bool detectado = (d > 0 && d <= umbral_cm);
  actualizarLedPresencia(detectado);

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
  Serial.print("ESP-NOW send: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "OK" : "FAIL");
}

static void copiarPacket(Packet &p, const uint8_t *data, int len) {
  memset(&p, 0, sizeof(p));
  if (len <= 0) return;
  memcpy(&p, data, min((int)sizeof(Packet), len));
}

void prepararResultadoCalibracion(float distancia_m, bool ok, uint8_t muestras) {
  resultadoCalDist_m = distancia_m;
  resultadoCalOK = ok;
  resultadoCalMuestras = muestras;
  resultadoCalPendiente = true;
}

void iniciarCalibracionUWB() {
  if (calibrandoUWB) {
    Serial.println("Comando de calibración recibido, pero ya estaba calibrando. Se ignora duplicado.");
    return;
  }

  calibrandoUWB = true;
  tInicioCalUWB = millis();
  nMuestrasUWB = 0;
  sumaDistUWB = 0.0f;
  seqInicioCalUWB = seqUWB;
  bajoUmbralCount = 0;
  Serial.println("\n>>> Calibración UWB iniciada: tomando 10 muestras nuevas");
  iniciarParpadeoLED(1);
}

bool rangoUWBValido(float d) {
  return d > 0.05f && d < 300.0f;
}

void guardarMuestraContinuaUWB(float d) {
  if (!rangoUWBValido(d)) return;

  bufferUWB[bufferUWBIndex] = d;
  bufferUWBIndex = (bufferUWBIndex + 1) % UWB_MUESTRAS_OBJETIVO;
  if (bufferUWBCount < UWB_MUESTRAS_OBJETIVO) bufferUWBCount++;

  ultimaMuestraUWB_ms = millis();
  seqUWB++;
}

bool promedioBufferReciente(float &promedio, uint8_t &muestras) {
  muestras = bufferUWBCount;
  if (muestras < UWB_MUESTRAS_MINIMAS) return false;
  if (millis() - ultimaMuestraUWB_ms > UWB_BUFFER_MAX_AGE_MS) return false;

  float suma = 0.0f;
  for (uint8_t i = 0; i < muestras; i++) suma += bufferUWB[i];
  promedio = suma / muestras;
  return true;
}

void enviarResultadoCalibracionSiPendiente() {
  if (!resultadoCalPendiente) return;

  resultadoCalPendiente = false;
  nMuestrasUWB = resultadoCalMuestras;
  enviarDistanciaUWB(resultadoCalDist_m, resultadoCalOK);
}

#if ESP_ARDUINO_VERSION_MAJOR >= 3
static void onDataRecv(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
  (void)info;
#else
static void onDataRecv(const uint8_t *mac, const uint8_t *data, int len) {
  (void)mac;
#endif
  Packet p;
  copiarPacket(p, data, len);

  if (p.msgType == MSG_CMD && p.value1 == CMD_CALIBRATE_DISTANCE) {
    solicitudCalUWB = true;
  }
}

void enviarStart(float distancia_cm) {
  Packet p{};
  p.msgType = MSG_START;
  p.src = SRC_PEON_SALIDA;
  p.msgId = ++msgId;
  p.ms = millis();
  p.phase = PHASE_IDLE;
  p.value1 = (int16_t)lroundf(distancia_cm * 10.0f); // cm*10 del HC-SR04
  p.value2 = 0;

  for (int i = 0; i < 3; i++) {
    esp_now_send(macCerebro, (uint8_t*)&p, sizeof(p));
  }

  iniciarParpadeoLED(3);

  Serial.print(">>> START enviado. Dist HC-SR04=");
  Serial.print(distancia_cm, 1);
  Serial.println(" cm");
}

void enviarEventoPeon(float distancia_cm) {
  enviarStart(distancia_cm);
}

void enviarDistanciaUWB(float distancia_m, bool ok) {
  Packet p{};
  p.msgType = MSG_DISTANCE;
  p.src = SRC_PEON_SALIDA;
  p.msgId = ++msgId;
  p.ms = millis();
  p.phase = PHASE_IDLE;
  p.value1 = ok ? (int16_t)lroundf(distancia_m * 100.0f) : -1; // centímetros
  p.value2 = nMuestrasUWB;

  for (int i = 0; i < 3; i++) {
    esp_now_send(macCerebro, (uint8_t*)&p, sizeof(p));
  }

  iniciarParpadeoLED(ok ? 2 : 5);

  Serial.print(">>> MSG_DISTANCE enviado: ");
  if (ok) {
    Serial.print(distancia_m, 2);
    Serial.print(" m, muestras=");
    Serial.println(nMuestrasUWB);
  } else {
    Serial.print("ERROR, muestras=");
    Serial.println(nMuestrasUWB);
  }
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
  esp_now_register_recv_cb(onDataRecv);

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

// ================= UWB =================
void newRange() {
  float d = DW1000Ranging.getDistantDevice()->getRange();
  guardarMuestraContinuaUWB(d);

  if (calibrandoUWB && rangoUWBValido(d) && seqUWB > seqInicioCalUWB) {
    sumaDistUWB += d;
    nMuestrasUWB++;

    Serial.print("Muestra UWB ");
    Serial.print(nMuestrasUWB);
    Serial.print("/");
    Serial.print(UWB_MUESTRAS_OBJETIVO);
    Serial.print(": ");
    Serial.print(d, 2);
    Serial.println(" m");

    if (nMuestrasUWB >= UWB_MUESTRAS_OBJETIVO) {
      float promedio = sumaDistUWB / nMuestrasUWB;
      calibrandoUWB = false;
      prepararResultadoCalibracion(promedio, true, nMuestrasUWB);
    }
  }
}

void newDevice(DW1000Device *device) {
  Serial.print("UWB tag conectado con anchor. short: ");
  Serial.println(device->getShortAddress(), HEX);
}

void inactiveDevice(DW1000Device *device) {
  Serial.print("UWB dispositivo inactivo: ");
  Serial.println(device->getShortAddress(), HEX);
}

void configurarUWB() {
  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
  DW1000Ranging.initCommunication(PIN_RST, PIN_SS, PIN_IRQ);
  DW1000Ranging.attachNewRange(newRange);
  DW1000Ranging.attachNewDevice(newDevice);
  DW1000Ranging.attachInactiveDevice(inactiveDevice);
  DW1000Ranging.startAsTag(TAG_ADD, DW1000.MODE_SHORTDATA_FAST_LOWPOWER);
}

void manejarTimeoutCalUWB() {
  if (!calibrandoUWB) return;

  if (millis() - tInicioCalUWB > UWB_CAL_TIMEOUT_MS) {
    bool ok = nMuestrasUWB >= UWB_MUESTRAS_MINIMAS;
    float promedio = ok ? (sumaDistUWB / nMuestrasUWB) : -1.0f;

    if (!ok) {
      uint8_t muestrasBuffer = 0;
      float promedioBuffer = -1.0f;
      if (promedioBufferReciente(promedioBuffer, muestrasBuffer)) {
        ok = true;
        promedio = promedioBuffer;
        nMuestrasUWB = muestrasBuffer;
        Serial.println("Usando buffer UWB reciente para calibración.");
      }
    }

    calibrandoUWB = false;
    prepararResultadoCalibracion(promedio, ok, nMuestrasUWB);
  }
}

// ================= SETUP / LOOP =================
void setup() {
  Serial.begin(115200);
  delay(500);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  configurarLedTimer();
  configurarUltrasonido();
  configurarEspNow();
  configurarUWB();

  Serial.println();
  Serial.println("PEÓN DE SALIDA - v3.4 TAG UWB + HC-SR04 ISR FAST");
  Serial.print("MAC local: ");
  Serial.println(WiFi.macAddress());
  Serial.print("Canal ESPNOW: ");
  Serial.println(ESPNOW_CHANNEL);
  Serial.println("Listo.");
}

void loop() {
  DW1000Ranging.loop();
  servicioLED();

  if (solicitudCalUWB) {
    solicitudCalUWB = false;
    iniciarCalibracionUWB();
  }

  manejarTimeoutCalUWB();
  enviarResultadoCalibracionSiPendiente();

  servicioTriggerUltrasonido();

  float d = -1.0f;
  if (obtenerMedicionUltrasonido(d)) {
    procesarMedicionUltrasonido(d);
  }
}
