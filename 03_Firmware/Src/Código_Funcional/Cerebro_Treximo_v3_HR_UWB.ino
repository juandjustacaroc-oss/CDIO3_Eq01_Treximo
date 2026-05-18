/*
  CEREBRO / CRONÓMETRO TREXIMO - v3 HR + UWB
  ESP32 + TFT ST7735 + RTC DS3231 + ESP-NOW

  Implementa:
  - Protocolo ESP-NOW unificado.
  - Recepción START / FINISH desde peones.
  - Recepción BPM desde módulo MAX30102 por ESP-NOW.
  - Comando broadcast de fase: IDLE / RUN / RECOVERY.
  - Calibración de distancia UWB con STOP + RESET durante 2 s.
  - Comando broadcast de calibración de distancia.
  - Recepción de distancia promedio desde peón de salida/tag UWB.
  - Distancia guardada en memoria flash con Preferences.
  - Velocidad mostrada en m/s.

  Botones:
  - START: inicia carrera manual.
  - STOP: detiene carrera manual.
  - RESET: reinicia cronómetro.
  - STOP + RESET por 2 s: calibra distancia UWB, solo si el cronómetro está detenido.
*/

#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <esp_arduino_version.h>
#ifndef ESP_ARDUINO_VERSION_MAJOR
#define ESP_ARDUINO_VERSION_MAJOR 2
#endif

#include <Wire.h>
#include "RTClib.h"
#include <Preferences.h>

#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>
#include <Fonts/FreeSansBold18pt7b.h>
#include <Fonts/FreeSansBold12pt7b.h>
#include <Fonts/FreeSans9pt7b.h>

// ================= PINES =================
#define TFT_CS     5
#define TFT_RST    4
#define TFT_DC     2

#define BTN_START 12
#define BTN_STOP  13
#define BTN_RESET 14

// ================= CONFIG =================
#define ESPNOW_CHANNEL 1
#define RECOVERY_MS    60000UL
#define HR_TIMEOUT_MS  3500UL
#define CAL_TIMEOUT_MS 12000UL

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);
RTC_DS3231 rtc;
Preferences prefs;

#define BG_COLOR    tft.color565(10, 10, 15)
#define TIME_COLOR  ST77XX_WHITE
#define VEL_COLOR   tft.color565(0, 255, 255)
#define BPM_COLOR   ST77XX_WHITE
#define GREY_TEXT   tft.color565(120, 120, 130)

// ================= PROTOCOLO ESPNOW =================
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

uint8_t macBroadcast[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
uint16_t msgIdLocal = 0;
uint16_t raceIdActual = 0;

// ================= ESTADO CRONÓMETRO =================
DateTime horaInicio;
DateTime horaFin;

unsigned long tiempoInicio = 0;
unsigned long tiempoPausado = 0;
bool cronometroActivo = false;

// ================= ESTADO FASE / RECUPERACIÓN =================
uint8_t faseActual = PHASE_IDLE;
uint32_t tInicioRecuperacion = 0;

// ================= BPM =================
int bpm = -1;
uint32_t tUltimoBpm = 0;

// ================= DISTANCIA / VELOCIDAD =================
float distancia_m = 0.0f;
bool distanciaValida = false;
float velocidad_ms = 0.0f;
bool velocidadValida = false;

bool esperandoDistancia = false;
uint32_t tInicioCalibracion = 0;

// Eventos recibidos por ESP-NOW.
// El callback solo marca banderas; la lógica pesada se ejecuta en loop().
volatile bool evtStart = false;
volatile bool evtFinish = false;
volatile bool evtHr = false;
volatile bool evtDistance = false;
volatile int16_t evtHrValue = -1;
volatile int16_t evtDistanceValue = -1;
volatile int16_t evtDistanceSamples = 0;

// ================= UI =================
bool overlayActivo = false;
uint32_t overlayHasta = 0;

String oldBpmTexto = "";
String oldVelTexto = "";
unsigned long oldTiempoMs = 999999999UL;

// ================= DECLARACIONES =================
void dibujarInterfazBase();
void dibujarTiempoMs(unsigned long actual);
void limpiarTiempo();
void actualizarCronometro();
void actualizarSensores(bool forzar = false);
void pantallaCarga();
void mostrarOverlay(const char* linea1, const char* linea2 = nullptr, uint32_t duracionMs = 0);
void cerrarOverlaySiVence();
void enviarComando(int16_t comando, uint8_t phase, uint8_t repeticiones = 2);
void cambiarFase(uint8_t nuevaFase);
void iniciarCronometro();
void detenerCronometro();
void resetSistema();
void iniciarCalibracionDistancia();
void manejarTimeoutCalibracion();
void manejarRecuperacion();
void actualizarVelocidad();
void manejarBotones();
void procesarEventosEspNow();

// ================= ESPNOW =================
static void copiarPacket(Packet &p, const uint8_t *data, int len) {
  memset(&p, 0, sizeof(p));
  if (len <= 0) return;
  memcpy(&p, data, min((int)sizeof(Packet), len));
}

#if ESP_ARDUINO_VERSION_MAJOR >= 3
void OnDataRecv(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
  (void)info;
#else
void OnDataRecv(const uint8_t *mac, const uint8_t *data, int len) {
  (void)mac;
#endif
  Packet p;
  copiarPacket(p, data, len);

  switch (p.msgType) {
    case MSG_START:
      evtStart = true;
      break;

    case MSG_FINISH:
      evtFinish = true;
      break;

    case MSG_HR:
      evtHrValue = p.value1;
      evtHr = true;
      break;

    case MSG_DISTANCE:
      evtDistanceValue = p.value1;
      evtDistanceSamples = p.value2;
      evtDistance = true;
      break;
  }
}

void procesarEventosEspNow() {
  if (evtStart) {
    evtStart = false;
    iniciarCronometro();
  }

  if (evtFinish) {
    evtFinish = false;
    detenerCronometro();
  }

  if (evtHr) {
    int16_t valor = evtHrValue;
    evtHr = false;

    if ((faseActual == PHASE_RUN || faseActual == PHASE_RECOVERY) && valor > 0) {
      bpm = valor;
      tUltimoBpm = millis();
    } else {
      bpm = -1;
      tUltimoBpm = 0;
    }
  }

  if (evtDistance) {
    int16_t valor = evtDistanceValue;
    int16_t muestras = evtDistanceSamples;
    evtDistance = false;

    // Ignorar duplicados tardíos si ya no estamos esperando calibración.
    if (!esperandoDistancia) return;

    esperandoDistancia = false;
    if (valor > 0) {
      distancia_m = valor / 100.0f;   // value1 viene en centímetros
      distanciaValida = true;
      prefs.putFloat("dist_m", distancia_m);

      char linea[24];
      snprintf(linea, sizeof(linea), "Dist: %.2f m", distancia_m);
      mostrarOverlay(linea, nullptr, 2500);

      Serial.print("Distancia calibrada: ");
      Serial.print(distancia_m, 2);
      Serial.print(" m  muestras=");
      Serial.println(muestras);
    } else {
      mostrarOverlay("ERROR UWB", nullptr, 2500);
      Serial.print("ERROR UWB: calibración sin distancia válida. muestras=");
      Serial.println(muestras);
    }
  }
}

void agregarPeerBroadcast() {
  esp_now_peer_info_t peer{};
  memcpy(peer.peer_addr, macBroadcast, 6);
  peer.channel = ESPNOW_CHANNEL;
  peer.encrypt = false;

  if (!esp_now_is_peer_exist(macBroadcast)) {
    esp_err_t err = esp_now_add_peer(&peer);
    if (err != ESP_OK) {
      Serial.print("ERROR agregando broadcast peer: ");
      Serial.println(err);
    }
  }
}

void enviarComando(int16_t comando, uint8_t phase, uint8_t repeticiones) {
  Packet p{};
  p.msgType = MSG_CMD;
  p.src = SRC_CEREBRO;
  p.msgId = ++msgIdLocal;
  p.ms = millis();
  p.raceId = raceIdActual;
  p.athleteId = 0;
  p.phase = phase;
  p.value1 = comando;
  p.value2 = 0;

  for (uint8_t i = 0; i < repeticiones; i++) {
    esp_now_send(macBroadcast, (uint8_t*)&p, sizeof(p));
    delay(15);
  }
}

void cambiarFase(uint8_t nuevaFase) {
  if (faseActual == nuevaFase) return;

  faseActual = nuevaFase;

  if (faseActual == PHASE_IDLE) {
    bpm = -1;
    tUltimoBpm = 0;
  }

  enviarComando(CMD_SET_PHASE, faseActual, 3);

  Serial.print("Fase actual: ");
  if (faseActual == PHASE_IDLE) Serial.println("IDLE");
  else if (faseActual == PHASE_RUN) Serial.println("RUN");
  else if (faseActual == PHASE_RECOVERY) Serial.println("RECOVERY");
}

// ================= CRONÓMETRO =================
void iniciarCronometro() {
  if (cronometroActivo) return;

  Serial.println("\n===== START =====");

  raceIdActual++;
  cronometroActivo = true;
  tiempoPausado = 0;
  tiempoInicio = millis();
  velocidad_ms = 0.0f;
  velocidadValida = false;
  bpm = -1;
  tUltimoBpm = 0;

  horaInicio = rtc.now();

  Serial.printf("Hora inicio: %02d:%02d:%02d\n",
    horaInicio.hour(), horaInicio.minute(), horaInicio.second());

  cambiarFase(PHASE_RUN);

  if (overlayActivo) {
    overlayActivo = false;
    dibujarInterfazBase();
  }
}

void detenerCronometro() {
  if (!cronometroActivo) return;

  Serial.println("\n===== FINISH =====");

  cronometroActivo = false;
  tiempoPausado = millis() - tiempoInicio;

  horaFin = rtc.now();

  Serial.printf("Hora llegada: %02d:%02d:%02d\n",
    horaFin.hour(), horaFin.minute(), horaFin.second());

  Serial.print("Tiempo carrera: ");
  Serial.print(tiempoPausado / 1000.0f);
  Serial.println(" s");

  actualizarVelocidad();
  dibujarTiempoMs(tiempoPausado);

  tInicioRecuperacion = millis();
  cambiarFase(PHASE_RECOVERY);
}

void resetSistema() {
  cronometroActivo = false;
  tiempoInicio = 0;
  tiempoPausado = 0;
  velocidad_ms = 0.0f;
  velocidadValida = false;
  bpm = -1;
  tUltimoBpm = 0;
  esperandoDistancia = false;

  cambiarFase(PHASE_IDLE);

  overlayActivo = false;
  dibujarInterfazBase();

  Serial.println("\nSistema reiniciado");
}

// ================= CALIBRACIÓN UWB =================
void iniciarCalibracionDistancia() {
  if (cronometroActivo) return;

  esperandoDistancia = true;
  tInicioCalibracion = millis();
  mostrarOverlay("CALIBRANDO", nullptr, 0);
  enviarComando(CMD_CALIBRATE_DISTANCE, faseActual, 3);

  Serial.println("Comando de calibración UWB enviado");
}

void manejarTimeoutCalibracion() {
  if (!esperandoDistancia) return;

  if (millis() - tInicioCalibracion > CAL_TIMEOUT_MS) {
    esperandoDistancia = false;
    mostrarOverlay("ERROR UWB", nullptr, 2500);
    Serial.println("ERROR UWB: timeout esperando distancia");
  }
}

void manejarRecuperacion() {
  if (faseActual != PHASE_RECOVERY) return;

  if (millis() - tInicioRecuperacion >= RECOVERY_MS) {
    cambiarFase(PHASE_IDLE);
    actualizarSensores(true);
    Serial.println("Recuperación finalizada");
  }
}

void actualizarVelocidad() {
  unsigned long t = cronometroActivo ? (millis() - tiempoInicio) : tiempoPausado;

  if (distanciaValida && t > 0) {
    velocidad_ms = distancia_m / (t / 1000.0f);
    velocidadValida = true;
  } else {
    velocidad_ms = 0.0f;
    velocidadValida = false;
  }
}

// ================= BOTONES =================
void manejarBotones() {
  static bool lastStart = HIGH;
  static bool lastStop  = HIGH;
  static bool lastReset = HIGH;

  static uint32_t tComboInicio = 0;
  static bool comboObservado = false;
  static bool comboEjecutado = false;

  bool estadoStart = digitalRead(BTN_START);
  bool estadoStop  = digitalRead(BTN_STOP);
  bool estadoReset = digitalRead(BTN_RESET);

  bool stopDown = (estadoStop == LOW);
  bool resetDown = (estadoReset == LOW);
  bool combo = stopDown && resetDown;

  if (combo) {
    comboObservado = true;
    if (tComboInicio == 0) tComboInicio = millis();

    if (!comboEjecutado && millis() - tComboInicio >= 2000UL) {
      comboEjecutado = true;
      if (!cronometroActivo) iniciarCalibracionDistancia();
    }

    lastStart = estadoStart;
    lastStop  = estadoStop;
    lastReset = estadoReset;
    return;
  } else {
    tComboInicio = 0;
  }

  // START en flanco de bajada.
  if (estadoStart == LOW && lastStart == HIGH) {
    iniciarCronometro();
  }

  // STOP y RESET en liberación para no interferir con STOP+RESET.
  if (lastStop == LOW && estadoStop == HIGH && !comboObservado) {
    detenerCronometro();
  }

  if (lastReset == LOW && estadoReset == HIGH && !comboObservado) {
    resetSistema();
  }

  if (!stopDown && !resetDown) {
    comboObservado = false;
    comboEjecutado = false;
  }

  lastStart = estadoStart;
  lastStop  = estadoStop;
  lastReset = estadoReset;
}

// ================= SETUP =================
void setup() {
  Serial.begin(115200);
  delay(300);

  pinMode(BTN_START, INPUT_PULLUP);
  pinMode(BTN_STOP,  INPUT_PULLUP);
  pinMode(BTN_RESET, INPUT_PULLUP);

  // RTC
  Wire.begin(21, 22);
  if (!rtc.begin()) {
    Serial.println("No se detectó RTC");
  }
  if (rtc.lostPower()) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  prefs.begin("treximo", false);
  distancia_m = prefs.getFloat("dist_m", 0.0f);
  distanciaValida = distancia_m > 0.01f;

  // TFT
  tft.initR(INITR_BLACKTAB);
  tft.setRotation(1);
  pantallaCarga();
  dibujarInterfazBase();

  // WIFI + ESPNOW
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  esp_wifi_set_channel(ESPNOW_CHANNEL, WIFI_SECOND_CHAN_NONE);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error ESP-NOW");
    while (true) delay(1000);
  }

  agregarPeerBroadcast();
  esp_now_register_recv_cb(OnDataRecv);

  Serial.print("MAC Cerebro: ");
  Serial.println(WiFi.macAddress());
  Serial.print("Canal ESPNOW: ");
  Serial.println(ESPNOW_CHANNEL);

  if (distanciaValida) {
    Serial.print("Distancia guardada: ");
    Serial.print(distancia_m, 2);
    Serial.println(" m");
  } else {
    Serial.println("Sin distancia calibrada guardada");
  }

  // Anuncia fase IDLE al arrancar, incluso si internamente ya estaba en IDLE.
  enviarComando(CMD_SET_PHASE, PHASE_IDLE, 3);
}

// ================= LOOP =================
void loop() {
  manejarBotones();
  procesarEventosEspNow();
  manejarTimeoutCalibracion();
  manejarRecuperacion();
  cerrarOverlaySiVence();

  actualizarVelocidad();

  if (!overlayActivo) {
    if (cronometroActivo) actualizarCronometro();
    actualizarSensores();
  }
}

// ================= UI =================
void pantallaCarga() {
  tft.fillScreen(BG_COLOR);
  tft.setTextColor(ST77XX_CYAN);
  tft.setFont(&FreeSansBold12pt7b);

  tft.setCursor(20, 45); tft.print("Cronometro");
  tft.setCursor(20, 75); tft.print("Inteligente");

  for (int i = 0; i < 360; i += 40) {
    int x = 80 + 15 * cos(i * 0.0174f);
    int y = 105 + 15 * sin(i * 0.0174f);
    tft.fillCircle(x, y, 3, ST77XX_CYAN);
    delay(80);
  }
}

void dibujarInterfazBase() {
  tft.fillScreen(BG_COLOR);

  tft.drawFastHLine(0, 68, 160, tft.color565(40, 40, 50));
  tft.drawFastVLine(80, 68, 60, tft.color565(40, 40, 50));

  tft.setFont();
  tft.setTextColor(GREY_TEXT);

  tft.setCursor(62, 5);  tft.print("TIEMPO");
  tft.setCursor(10, 75); tft.print("FC");
  tft.setCursor(88, 75); tft.print("VELOCIDAD");

  oldTiempoMs = 999999999UL;
  oldBpmTexto = "";
  oldVelTexto = "";

  dibujarTiempoMs(tiempoPausado);
  actualizarSensores(true);
}

void dibujarTiempoMs(unsigned long actual) {
  if (actual == oldTiempoMs) return;
  oldTiempoMs = actual;

  unsigned int min = actual / 60000UL;
  unsigned int seg = (actual % 60000UL) / 1000UL;
  unsigned int dec = (actual % 1000UL) / 100UL;

  char buffer[10];
  snprintf(buffer, sizeof(buffer), "%02u:%02u.%u", min, seg, dec);

  tft.fillRect(5, 18, 150, 35, BG_COLOR);
  tft.setFont(&FreeSansBold18pt7b);
  tft.setTextColor(TIME_COLOR);

  int16_t x1, y1;
  uint16_t w, h;
  tft.getTextBounds(buffer, 0, 0, &x1, &y1, &w, &h);

  tft.setCursor((160 - w) / 2, 52);
  tft.print(buffer);
}

void limpiarTiempo() {
  dibujarTiempoMs(0);
}

void actualizarCronometro() {
  static unsigned long lastDraw = 0;
  unsigned long actual = millis() - tiempoInicio;

  if (actual - lastDraw >= 100UL) {
    lastDraw = actual;
    dibujarTiempoMs(actual);
  }
}

void actualizarSensores(bool forzar) {
  bool bpmValido = (faseActual == PHASE_RUN || faseActual == PHASE_RECOVERY) &&
                   bpm > 0 &&
                   (millis() - tUltimoBpm <= HR_TIMEOUT_MS);

  String bpmTexto = bpmValido ? String(bpm) : String("---");

  String velTexto;
  if (velocidadValida) {
    char buf[12];
    snprintf(buf, sizeof(buf), "%.1f", velocidad_ms);
    velTexto = String(buf);
  } else {
    velTexto = "---";
  }

  if (forzar || bpmTexto != oldBpmTexto) {
    tft.fillRect(5, 88, 72, 38, BG_COLOR);
    tft.setTextColor(BPM_COLOR);
    tft.setFont(&FreeSansBold12pt7b);
    tft.setCursor(10, 110);
    tft.print(bpmTexto);

    tft.setFont();
    tft.setTextColor(GREY_TEXT);
    tft.setCursor(10, 118);
    tft.print("BPM");

    oldBpmTexto = bpmTexto;
  }

  if (forzar || velTexto != oldVelTexto) {
    tft.fillRect(83, 88, 76, 38, BG_COLOR);
    tft.setTextColor(VEL_COLOR);
    tft.setFont(&FreeSansBold12pt7b);
    tft.setCursor(88, 110);
    tft.print(velTexto);

    tft.setFont();
    tft.setTextColor(GREY_TEXT);
    tft.setCursor(90, 118);
    tft.print("m/s");

    oldVelTexto = velTexto;
  }
}

void mostrarOverlay(const char* linea1, const char* linea2, uint32_t duracionMs) {
  overlayActivo = true;
  overlayHasta = (duracionMs > 0) ? millis() + duracionMs : 0;

  tft.fillScreen(BG_COLOR);
  tft.setTextColor(ST77XX_CYAN);
  tft.setFont(&FreeSansBold12pt7b);

  int16_t x1, y1;
  uint16_t w, h;
  tft.getTextBounds(linea1, 0, 0, &x1, &y1, &w, &h);
  tft.setCursor((160 - w) / 2, linea2 ? 55 : 70);
  tft.print(linea1);

  if (linea2) {
    tft.setFont(&FreeSans9pt7b);
    tft.getTextBounds(linea2, 0, 0, &x1, &y1, &w, &h);
    tft.setCursor((160 - w) / 2, 88);
    tft.print(linea2);
  }
}

void cerrarOverlaySiVence() {
  if (!overlayActivo || overlayHasta == 0) return;

  if ((int32_t)(millis() - overlayHasta) >= 0) {
    overlayActivo = false;
    dibujarInterfazBase();
  }
}
