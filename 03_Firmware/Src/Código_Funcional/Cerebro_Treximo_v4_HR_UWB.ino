/*
  CEREBRO / CRONÓMETRO TREXIMO - v4.2 API LOCAL WEB
  ESP32 + TFT ST7735 + RTC DS3231 + ESP-NOW + WiFi HTTP

  Cambios v4.1:
  - Mantiene peones por ESP-NOW, calibración UWB, BPM, botones manuales e interfaz actual.
  - Al finalizar carrera + 1 minuto de recuperación, genera el JSON esperado por la app Tréximo.
  - Envía el entrenamiento por HTTP POST a http://192.168.20.21:8081/api/ingest-workout.
  - La subida WiFi ocurre solo en IDLE para no interferir con ESP-NOW durante la carrera.
  - El JSON NO envía null en campos NOT NULL del backend: usa 0 cuando no hay dato.
  - Si falla la subida, guarda el JSON pendiente en Preferences y reintenta.
  - Comando serial opcional: escribe T para subir una carrera simulada de prueba.
*/

#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClient.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <esp_arduino_version.h>
#ifndef ESP_ARDUINO_VERSION_MAJOR
#define ESP_ARDUINO_VERSION_MAJOR 2
#endif

#include <Wire.h>
#include "RTClib.h"
#include <Preferences.h>
#include <math.h>
#include <string.h>

#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>
#include <Fonts/FreeSansBold18pt7b.h>
#include <Fonts/FreeSansBold12pt7b.h>
#include <Fonts/FreeSansBold9pt7b.h>
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
#define CAL_TIMEOUT_MS 30000UL
#define BTN_DEBOUNCE_MS 35UL
#define COMBO_CAL_MS 2000UL

// ================= CONFIG APP / WIFI =================
// Cambia estos valores para la prueba de campo.
// El backend local debe estar corriendo en la misma red WiFi que el ESP32.
const char* WIFI_SSID    = "iPhone de migue";
const char* WIFI_PASS    = "3113756278";  // <-- escribe aquí la clave real del WiFi
const char* API_URL      = "http://172.20.10.2:8081/api/ingest-workout";
const char* DEVICE_TOKEN = "treximo-secret-123";
const char* DEVICE_ID    = "treximo-001";

// Código del atleta activo. Si no existe en la app, el backend debe crear Atleta #codigo.
uint8_t athleteCodeActual = 1;

#define WIFI_CONNECT_TIMEOUT_MS 12000UL
#define HTTP_UPLOAD_TIMEOUT_MS  12000UL
#define UPLOAD_RETRY_MS         30000UL

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);
RTC_DS3231 rtc;
Preferences prefs;

// ================= COLORES =================
#define BG_COLOR      tft.color565(10, 10, 15)
#define LINE_COLOR    tft.color565(40, 40, 50)
#define GREY_TEXT     tft.color565(120, 120, 130)
#define AZUL_TREXIMO  tft.color565(10, 20, 80)

#define TIME_COLOR    ST77XX_WHITE
#define BPM_COLOR     ST77XX_WHITE
#define VEL_COLOR     ST77XX_CYAN
#define DIST_COLOR    ST77XX_GREEN

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

// ================= APP / SUBIDA DE ENTRENAMIENTOS =================
String jsonEntrenamientoPendiente = "";
bool hayUploadPendiente = false;
uint32_t tProximoIntentoUpload = 0;
uint32_t uploadIntentos = 0;
int bpmFinishRegistro = -1;

// Eventos recibidos por ESP-NOW. El callback solo marca banderas.
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

char oldBpmTexto[10] = "";
char oldVelTexto[14] = "";
char oldDistTexto[16] = "";
unsigned long oldTiempoMs = 999999999UL;

// ================= BOTONES =================
struct Boton {
  uint8_t pin;
  bool stable;
  bool lastReading;
  uint32_t lastChange;
  bool pressed;
  bool released;
};

Boton botonStart = {BTN_START, HIGH, HIGH, 0, false, false};
Boton botonStop  = {BTN_STOP,  HIGH, HIGH, 0, false, false};
Boton botonReset = {BTN_RESET, HIGH, HIGH, 0, false, false};

// ================= DECLARACIONES =================
void dibujarLogoTreximo();
void pantallaCarga();
void dibujarInterfazBase();
void dibujarTiempoMs(unsigned long actual);
void actualizarCronometro();
void actualizarSensores(bool forzar = false);
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
bool iniciarEspNow();
void detenerEspNow();
bool bpmValidoActual();
String isoLocal(DateTime dt);
String jsonEscape(const String &s);
String construirJsonEntrenamiento(int bpmRecuperacion);
void guardarEntrenamientoPendiente(int bpmRecuperacion);
void cargarPendienteDesdeFlash();
bool subirEntrenamientoHttp(const String &json);
void gestionarSubidaPendiente();
String construirJsonSimulado();
void manejarComandosSerial();

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

    // Ignora duplicados tardíos si ya no estamos esperando calibración.
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
  peer.ifidx = WIFI_IF_STA;
  peer.encrypt = false;

  if (!esp_now_is_peer_exist(macBroadcast)) {
    esp_err_t err = esp_now_add_peer(&peer);
    if (err != ESP_OK) {
      Serial.print("ERROR agregando broadcast peer: ");
      Serial.println(err);
    }
  }
}

bool iniciarEspNow() {
  WiFi.mode(WIFI_STA);
  WiFi.disconnect(true, true);
  delay(120);
  esp_wifi_set_channel(ESPNOW_CHANNEL, WIFI_SECOND_CHAN_NONE);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error ESP-NOW");
    return false;
  }

  agregarPeerBroadcast();
  esp_now_register_recv_cb(OnDataRecv);

  Serial.print("MAC Cerebro: ");
  Serial.println(WiFi.macAddress());
  Serial.print("Canal ESPNOW: ");
  Serial.println(ESPNOW_CHANNEL);
  return true;
}

void detenerEspNow() {
  esp_now_deinit();
  delay(80);
}

bool bpmValidoActual() {
  return (faseActual == PHASE_RUN || faseActual == PHASE_RECOVERY) &&
         bpm > 0 &&
         (millis() - tUltimoBpm <= HR_TIMEOUT_MS);
}

String isoLocal(DateTime dt) {
  char buffer[32];
  // Colombia: UTC-05:00. El RTC debe estar ajustado a hora local.
  snprintf(buffer, sizeof(buffer), "%04d-%02d-%02dT%02d:%02d:%02d-05:00",
           dt.year(), dt.month(), dt.day(), dt.hour(), dt.minute(), dt.second());
  return String(buffer);
}

String jsonEscape(const String &s) {
  String out;
  out.reserve(s.length() + 8);
  for (size_t i = 0; i < s.length(); i++) {
    char c = s[i];
    if (c == '\\' || c == '"') {
      out += '\\';
      out += c;
    } else if (c == '\n') {
      out += "\\n";
    } else if (c == '\r') {
      out += "\\r";
    } else {
      out += c;
    }
  }
  return out;
}

String construirJsonEntrenamiento(int bpmRecuperacion) {
  // IMPORTANTE:
  // El backend actual usa columnas NOT NULL para distance_m, speed_mps,
  // bpm_finish y bpm_recovery_60s. Por eso aquí NO se envían null.
  // Si no hay dato válido, se envía 0 para evitar que SQLite rechace el POST.
  float distanciaEnvio = distanciaValida ? distancia_m : 0.0f;
  float velocidadEnvio = velocidadValida ? velocidad_ms : 0.0f;
  int bpmFinishEnvio = (bpmFinishRegistro > 0) ? bpmFinishRegistro : 0;
  int bpmRecoveryEnvio = (bpmRecuperacion > 0) ? bpmRecuperacion : 0;

  String json = "{";
  json += "\"device_id\":\"" + jsonEscape(String(DEVICE_ID)) + "\",";
  json += "\"device_type\":\"treximo_esp32\",";
  json += "\"race_id\":" + String(raceIdActual) + ",";
  json += "\"athlete_code\":" + String(athleteCodeActual) + ",";
  json += "\"start_time\":\"" + isoLocal(horaInicio) + "\",";
  json += "\"finish_time\":\"" + isoLocal(horaFin) + "\",";
  json += "\"duration_ms\":" + String(tiempoPausado) + ",";
  json += "\"duration_s\":" + String(tiempoPausado / 1000.0f, 3) + ",";
  json += "\"distance_m\":" + String(distanciaEnvio, 2) + ",";
  json += "\"speed_mps\":" + String(velocidadEnvio, 3) + ",";
  json += "\"bpm_finish\":" + String(bpmFinishEnvio) + ",";
  json += "\"bpm_recovery_60s\":" + String(bpmRecoveryEnvio) + ",";
  json += "\"distance_calibrated\":";
  json += distanciaValida ? "true" : "false";
  json += ",";
  json += "\"firmware\":\"Cerebro_Treximo_v4_2_API_WEB_8081\"";
  json += "}";
  return json;
}

void guardarEntrenamientoPendiente(int bpmRecuperacion) {
  if (tiempoPausado == 0) return;

  jsonEntrenamientoPendiente = construirJsonEntrenamiento(bpmRecuperacion);
  hayUploadPendiente = true;
  uploadIntentos = 0;
  tProximoIntentoUpload = millis() + 1200UL;

  prefs.putString("pending_json", jsonEntrenamientoPendiente);
  prefs.putBool("pending", true);

  Serial.println("Entrenamiento listo para subir a la app:");
  Serial.println(jsonEntrenamientoPendiente);
}

void cargarPendienteDesdeFlash() {
  raceIdActual = prefs.getUShort("race_id", 0);
  hayUploadPendiente = prefs.getBool("pending", false);
  if (hayUploadPendiente) {
    jsonEntrenamientoPendiente = prefs.getString("pending_json", "");
    if (jsonEntrenamientoPendiente.length() == 0) {
      hayUploadPendiente = false;
      prefs.putBool("pending", false);
    } else {
      tProximoIntentoUpload = millis() + 5000UL;
      Serial.println("Hay un entrenamiento pendiente guardado en memoria flash.");
    }
  }
}

bool subirEntrenamientoHttp(const String &json) {
  if (String(WIFI_SSID).length() == 0 || String(WIFI_PASS) == "CAMBIA_AQUI_TU_CLAVE_WIFI") {
    Serial.println("Subida omitida: configura WIFI_SSID y WIFI_PASS.");
    return false;
  }

  if (!String(API_URL).startsWith("http://")) {
    Serial.println("Para la prueba local usa API_URL con http://IP_PC:8081/api/ingest-workout");
    return false;
  }

  Serial.println("Preparando subida WiFi. ESP-NOW se pausará temporalmente.");
  detenerEspNow();

  bool ok = false;
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  uint32_t t0 = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - t0 < WIFI_CONNECT_TIMEOUT_MS) {
    delay(250);
    Serial.print('.');
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("WiFi conectado. IP ESP32: ");
    Serial.println(WiFi.localIP());

    HTTPClient http;
    WiFiClient client;
    if (http.begin(client, API_URL)) {
      http.setTimeout(HTTP_UPLOAD_TIMEOUT_MS);
      http.addHeader("Content-Type", "application/json");
      http.addHeader("X-Device-Token", DEVICE_TOKEN);
      http.addHeader("X-Device-Id", DEVICE_ID);

      int code = http.POST(json);
      String body = http.getString();

      Serial.print("HTTP status: ");
      Serial.println(code);
      if (body.length() > 0) {
        Serial.print("Respuesta app: ");
        Serial.println(body);
      }

      ok = (code >= 200 && code < 300);
      http.end();
    } else {
      Serial.println("No se pudo iniciar HTTPClient. Revisa API_URL.");
    }
  } else {
    Serial.println("No se pudo conectar al WiFi.");
  }

  WiFi.disconnect(true, true);
  delay(150);

  if (!iniciarEspNow()) {
    Serial.println("ALERTA: no se pudo reiniciar ESP-NOW después de WiFi.");
  }

  return ok;
}

void gestionarSubidaPendiente() {
  if (!hayUploadPendiente) return;
  if (cronometroActivo || esperandoDistancia || faseActual != PHASE_IDLE) return;
  if ((int32_t)(millis() - tProximoIntentoUpload) < 0) return;

  uploadIntentos++;
  mostrarOverlay("SUBIENDO", "APP", 0);

  bool ok = subirEntrenamientoHttp(jsonEntrenamientoPendiente);

  if (ok) {
    hayUploadPendiente = false;
    jsonEntrenamientoPendiente = "";
    prefs.putBool("pending", false);
    prefs.remove("pending_json");
    mostrarOverlay("APP OK", nullptr, 1800);
    Serial.println("Entrenamiento subido correctamente a la app.");
  } else {
    tProximoIntentoUpload = millis() + UPLOAD_RETRY_MS;
    mostrarOverlay("ERROR APP", "Guardado local", 2200);
    Serial.println("No se pudo subir. Se conserva entrenamiento local para reintento.");
  }
}

String construirJsonSimulado() {
  // Carrera simulada para probar la interfaz web sin depender de peones/sensores.
  // Se envía al mismo endpoint real y debe aparecer en Dashboard/Historial.
  uint32_t nowMs = millis();
  uint32_t durMs = 7200UL + (esp_random() % 2600UL);  // 7.2 s a 9.8 s
  float dist = distanciaValida ? distancia_m : 50.0f;
  float vel = dist / (durMs / 1000.0f);
  int bpmFin = 145 + (esp_random() % 28);
  int bpmRec = 105 + (esp_random() % 22);

  DateTime fin = rtc.now();
  DateTime ini = fin - TimeSpan((int)(durMs / 1000UL));

  uint16_t simRaceId = ++raceIdActual;
  prefs.putUShort("race_id", raceIdActual);

  String json = "{";
  json += "\"device_id\":\"" + jsonEscape(String(DEVICE_ID)) + "\",";
  json += "\"device_type\":\"treximo_esp32\",";
  json += "\"race_id\":" + String(simRaceId) + ",";
  json += "\"athlete_code\":" + String(athleteCodeActual) + ",";
  json += "\"start_time\":\"" + isoLocal(ini) + "\",";
  json += "\"finish_time\":\"" + isoLocal(fin) + "\",";
  json += "\"duration_ms\":" + String(durMs) + ",";
  json += "\"duration_s\":" + String(durMs / 1000.0f, 3) + ",";
  json += "\"distance_m\":" + String(dist, 2) + ",";
  json += "\"speed_mps\":" + String(vel, 3) + ",";
  json += "\"bpm_finish\":" + String(bpmFin) + ",";
  json += "\"bpm_recovery_60s\":" + String(bpmRec) + ",";
  json += "\"distance_calibrated\":";
  json += distanciaValida ? "true" : "false";
  json += ",";
  json += "\"firmware\":\"Cerebro_Treximo_v4_2_TEST_SERIAL\"";
  json += "}";
  (void)nowMs;
  return json;
}

void manejarComandosSerial() {
  if (!Serial.available()) return;
  char c = Serial.read();
  if (c == 't' || c == 'T' || c == 's' || c == 'S') {
    if (cronometroActivo || esperandoDistancia || faseActual != PHASE_IDLE) {
      Serial.println("No se puede subir prueba serial: el sistema no está en IDLE.");
      return;
    }
    jsonEntrenamientoPendiente = construirJsonSimulado();
    hayUploadPendiente = true;
    uploadIntentos = 0;
    tProximoIntentoUpload = millis() + 500UL;
    prefs.putString("pending_json", jsonEntrenamientoPendiente);
    prefs.putBool("pending", true);
    Serial.println("Carrera simulada lista para subir a la app:");
    Serial.println(jsonEntrenamientoPendiente);
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
  prefs.putUShort("race_id", raceIdActual);
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

  Serial.println("\n===== FINISH / STOP =====");

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
  actualizarSensores(true);

  bpmFinishRegistro = bpmValidoActual() ? bpm : -1;

  // Guarda una primera versión del entrenamiento.
  // Se reemplazará al completar 1 minuto de recuperación con bpm_recovery_60s.
  guardarEntrenamientoPendiente(-1);

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
  bpmFinishRegistro = -1;
  esperandoDistancia = false;

  cambiarFase(PHASE_IDLE);

  overlayActivo = false;
  dibujarInterfazBase();

  Serial.println("\nSistema reiniciado manualmente");
}

// ================= CALIBRACIÓN UWB =================
void iniciarCalibracionDistancia() {
  if (cronometroActivo || esperandoDistancia) return;

  esperandoDistancia = true;
  tInicioCalibracion = millis();
  mostrarOverlay("CALIBRANDO", nullptr, 0);
  enviarComando(CMD_CALIBRATE_DISTANCE, faseActual, 3);

  Serial.println("Comando de calibración UWB enviado por broadcast");
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
    int bpmRecuperacion = bpmValidoActual() ? bpm : -1;

    // Actualiza el JSON pendiente con BPM al minuto de recuperación.
    guardarEntrenamientoPendiente(bpmRecuperacion);

    cambiarFase(PHASE_IDLE);
    actualizarSensores(true);
    Serial.println("Recuperación finalizada. Entrenamiento listo para subir a la app.");
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
void actualizarBoton(Boton &b) {
  b.pressed = false;
  b.released = false;

  bool lectura = digitalRead(b.pin);
  if (lectura != b.lastReading) {
    b.lastReading = lectura;
    b.lastChange = millis();
  }

  if ((millis() - b.lastChange) >= BTN_DEBOUNCE_MS && lectura != b.stable) {
    bool anterior = b.stable;
    b.stable = lectura;

    if (anterior == HIGH && b.stable == LOW) b.pressed = true;
    if (anterior == LOW  && b.stable == HIGH) b.released = true;
  }
}

void manejarBotones() {
  static uint32_t tComboInicio = 0;
  static bool comboEjecutado = false;
  static bool bloquearStopResetAlSoltar = false;

  actualizarBoton(botonStart);
  actualizarBoton(botonStop);
  actualizarBoton(botonReset);

  bool combo = (botonStop.stable == LOW && botonReset.stable == LOW);

  if (botonStart.pressed) {
    iniciarCronometro();
  }

  if (combo) {
    if (tComboInicio == 0) tComboInicio = millis();

    if (!comboEjecutado && (millis() - tComboInicio >= COMBO_CAL_MS)) {
      comboEjecutado = true;
      bloquearStopResetAlSoltar = true;
      if (!cronometroActivo) {
        iniciarCalibracionDistancia();
      }
    }
    return;
  }

  if (botonStop.released || botonReset.released) {
    if (!bloquearStopResetAlSoltar) {
      if (botonStop.released) detenerCronometro();
      if (botonReset.released) resetSistema();
    }
  }

  if (botonStop.stable == HIGH && botonReset.stable == HIGH) {
    tComboInicio = 0;
    comboEjecutado = false;
    bloquearStopResetAlSoltar = false;
  }
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
  cargarPendienteDesdeFlash();

  // TFT
  tft.initR(INITR_BLACKTAB);
  tft.setRotation(1);
  pantallaCarga();
  dibujarInterfazBase();

  // WIFI + ESPNOW
  if (!iniciarEspNow()) {
    while (true) delay(1000);
  }

  Serial.println("\n=== CONFIG API WEB TREXIMO ===");
  Serial.print("SSID: "); Serial.println(WIFI_SSID);
  Serial.print("API_URL: "); Serial.println(API_URL);
  Serial.print("DEVICE_ID: "); Serial.println(DEVICE_ID);
  Serial.print("Athlete code: "); Serial.println(athleteCodeActual);
  Serial.println("Escribe T o S en el Monitor Serial para subir una carrera simulada.");

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
  manejarComandosSerial();
  manejarBotones();
  procesarEventosEspNow();
  manejarTimeoutCalibracion();
  manejarRecuperacion();
  cerrarOverlaySiVence();
  gestionarSubidaPendiente();

  actualizarVelocidad();

  if (!overlayActivo) {
    if (cronometroActivo) actualizarCronometro();
    actualizarSensores();
  }
}

// =====================================================
//         DIBUJAR LOGO TRÉXIMO
// =====================================================
void dibujarLogoTreximo() {
  uint16_t azul   = AZUL_TREXIMO;
  uint16_t blanco = ST77XX_WHITE;

  int cx = 38, cy = 62;
  int re = 28;
  int ri = 20;

  float angBase = -30 * 0.01745f;

  int xo[6], yo[6], xi[6], yi[6];
  for (int i = 0; i < 6; i++) {
    float ang = angBase + i * 60 * 0.01745f;
    xo[i] = cx + re * cos(ang);
    yo[i] = cy + re * sin(ang);
    xi[i] = cx + ri * cos(ang);
    yi[i] = cy + ri * sin(ang);
  }

  for (int i = 0; i < 6; i++) {
    int j = (i + 1) % 6;
    tft.fillTriangle(xo[i], yo[i], xo[j], yo[j], cx, cy, azul);
  }

  for (int i = 0; i < 6; i++) {
    int j = (i + 1) % 6;
    tft.fillTriangle(cx, cy, xi[i], yi[i], xi[j], yi[j], blanco);
  }

  int barraY  = cy - ri + 6;
  int barraX1 = cx - ri + 4;
  int barraX2 = cx + ri - 4;
  int barraH  = 5;

  tft.fillRect(barraX1, barraY, barraX2 - barraX1, barraH, azul);

  int dx1 = barraX2 - 2;
  int dy1 = barraY + barraH;
  int dx2 = cx - 5;
  int dy2 = cy + ri - 4;

  for (int offset = -2; offset <= 2; offset++) {
    tft.drawLine(dx1 + offset, dy1, dx2 + offset, dy2, azul);
  }

  tft.setFont(&FreeSansBold9pt7b);
  tft.setTextColor(azul);
  tft.setCursor(74, 69);
  tft.print("Treximo");
}

// =====================================================
//                 PANTALLA CARGA
// =====================================================
void pantallaCarga() {
  tft.fillScreen(ST77XX_WHITE);
  dibujarLogoTreximo();

  uint16_t azul = AZUL_TREXIMO;
  uint16_t gris = tft.color565(210, 210, 220);

  int barX = 15, barY = 108, barW = 130, barH = 6, barR = 3;
  tft.fillRoundRect(barX, barY, barW, barH, barR, gris);

  int pasos = barW - 2;
  int delayPorPaso = 2000 / pasos;

  for (int i = 1; i <= pasos; i++) {
    tft.fillRoundRect(barX + 1, barY + 1, i, barH - 2, barR, azul);
    delay(delayPorPaso);
  }

  delay(400);
}

// =====================================================
//                INTERFAZ BASE
// =====================================================
void dibujarInterfazBase() {
  tft.fillScreen(BG_COLOR);

  tft.drawFastHLine(0, 55, 160, LINE_COLOR);
  tft.drawFastVLine(80, 55, 45, LINE_COLOR);
  tft.drawFastHLine(0, 100, 160, LINE_COLOR);

  tft.setTextColor(GREY_TEXT);
  tft.setFont();

  tft.setCursor(60, 6);
  tft.print("TIEMPO");

  tft.setCursor(10, 64);
  tft.print("FC");

  tft.setCursor(92, 64);
  tft.print("VEL");

  tft.setCursor(10, 103);
  tft.print("DISTANCIA");

  oldTiempoMs = 999999999UL;
  oldBpmTexto[0] = '\0';
  oldVelTexto[0] = '\0';
  oldDistTexto[0] = '\0';

  dibujarTiempoMs(tiempoPausado);
  actualizarSensores(true);
}

// =====================================================
//                  CRONÓMETRO
// =====================================================
void dibujarTiempoMs(unsigned long actual) {
  if (actual == oldTiempoMs) return;
  oldTiempoMs = actual;

  unsigned int min = actual / 60000UL;
  unsigned int seg = (actual % 60000UL) / 1000UL;
  unsigned int cen = (actual % 1000UL) / 10UL;

  char buffer[12];
  snprintf(buffer, sizeof(buffer), "%02u:%02u:%02u", min, seg, cen);

  tft.fillRect(5, 15, 150, 30, BG_COLOR);

  tft.setFont(&FreeSansBold12pt7b);
  tft.setTextColor(TIME_COLOR);

  int16_t x1, y1;
  uint16_t w, h;
  tft.getTextBounds(buffer, 0, 0, &x1, &y1, &w, &h);

  tft.setCursor((160 - w) / 2, 42);
  tft.print(buffer);
}

void actualizarCronometro() {
  static unsigned long lastDraw = 0;
  unsigned long actual = millis() - tiempoInicio;

  if (actual - lastDraw >= 50UL) {
    lastDraw = actual;
    dibujarTiempoMs(actual);
  }
}

// =====================================================
//                  VARIABLES
// =====================================================
void actualizarSensores(bool forzar) {
  bool bpmValido = (faseActual == PHASE_RUN || faseActual == PHASE_RECOVERY) &&
                   bpm > 0 &&
                   (millis() - tUltimoBpm <= HR_TIMEOUT_MS);

  char bpmTexto[10];
  if (bpmValido) snprintf(bpmTexto, sizeof(bpmTexto), "%d", bpm);
  else snprintf(bpmTexto, sizeof(bpmTexto), "---");

  char velTexto[14];
  if (velocidadValida) snprintf(velTexto, sizeof(velTexto), "%.1f", velocidad_ms);
  else snprintf(velTexto, sizeof(velTexto), "---");

  char distTexto[16];
  if (distanciaValida) snprintf(distTexto, sizeof(distTexto), "%.2f", distancia_m);
  else snprintf(distTexto, sizeof(distTexto), "---");

  // ================= FC =================
  if (forzar || strcmp(bpmTexto, oldBpmTexto) != 0) {
    tft.fillRect(5, 72, 70, 22, BG_COLOR);

    tft.setFont(&FreeSansBold9pt7b);
    tft.setTextColor(BPM_COLOR);
    tft.setCursor(8, 90);
    tft.print(bpmTexto);

    tft.setFont();
    tft.setTextColor(GREY_TEXT);
    tft.setCursor(10, 64);
    tft.print("FC");
    tft.setCursor(48, 84);
    tft.print("BPM");

    strncpy(oldBpmTexto, bpmTexto, sizeof(oldBpmTexto));
    oldBpmTexto[sizeof(oldBpmTexto) - 1] = '\0';
  }

  // ================= VELOCIDAD =================
  if (forzar || strcmp(velTexto, oldVelTexto) != 0) {
    tft.fillRect(85, 72, 70, 22, BG_COLOR);

    tft.setFont(&FreeSansBold9pt7b);
    tft.setTextColor(VEL_COLOR);
    tft.setCursor(88, 90);
    tft.print(velTexto);

    tft.setFont();
    tft.setTextColor(GREY_TEXT);
    tft.setCursor(92, 64);
    tft.print("VEL");
    tft.setCursor(127, 84);
    tft.print("m/s");

    strncpy(oldVelTexto, velTexto, sizeof(oldVelTexto));
    oldVelTexto[sizeof(oldVelTexto) - 1] = '\0';
  }

  // ================= DISTANCIA =================
  if (forzar || strcmp(distTexto, oldDistTexto) != 0) {
    tft.fillRect(40, 110, 120, 18, BG_COLOR);

    tft.setFont();
    tft.setTextColor(GREY_TEXT);
    tft.setCursor(10, 103);
    tft.print("DISTANCIA");

    tft.setFont(&FreeSansBold9pt7b);
    tft.setTextColor(DIST_COLOR);
    tft.setCursor(65, 124);
    tft.print(distTexto);

    tft.setFont();
    tft.setTextColor(GREY_TEXT);
    tft.setCursor(112, 115);
    tft.print("m");

    strncpy(oldDistTexto, distTexto, sizeof(oldDistTexto));
    oldDistTexto[sizeof(oldDistTexto) - 1] = '\0';
  }
}

// ================= OVERLAY =================
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
