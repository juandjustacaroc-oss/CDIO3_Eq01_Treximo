/*
  ESP32 Tréximo - Simulador de carreras para probar la app/backend

  Objetivo:
  - No usa sensores.
  - Genera entrenamientos aleatorios para un único atleta.
  - Envía los datos por WiFi al backend local de Tréximo:
      POST /api/ingest-workout
  - Compatible con el formato documentado por la app Tréximo.

  Librerías necesarias:
  - WiFi.h
  - HTTPClient.h
  - Preferences.h
  - time.h

  Uso:
  1. Cambia WIFI_SSID, WIFI_PASS, API_URL y DEVICE_TOKEN.
  2. Ejecuta el backend local en tu PC.
  3. Carga este código en un ESP32.
  4. Abre el Monitor Serial a 115200 baudios.
  5. El ESP32 enviará una carrera simulada automáticamente cada SIM_INTERVAL_MS.
     También puedes escribir 's' en el monitor serial para enviar una carrera inmediata.
*/

#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClient.h>
#include <Preferences.h>
#include <time.h>

// ========================== CONFIGURACIÓN DEL USUARIO ==========================

const char* WIFI_SSID = "FLIA-TORRES-BARONA";
const char* WIFI_PASS = "3187887777";

// Usa la IP del computador donde corre el backend local Dart Shelf.
// Ejemplo: http://192.168.20.21:8080/api/ingest-workout
const char* API_URL = "http://192.168.20.21:8081/api/ingest-workout";

// Debe coincidir con el token configurado en el backend.
const char* DEVICE_TOKEN = "treximo-secret-123";

const char* DEVICE_ID = "treximo-sim-001";
const char* DEVICE_TYPE = "treximo_esp32";
const char* FIRMWARE = "ESP32_Treximo_Simulador_API_v1";

// Único atleta para la prueba. Si no existe, el backend debería crear Atleta #1.
const int ATHLETE_CODE = 2;

// Distancia simulada de la prueba.
const float DISTANCE_M = 50.0f;

// Envío automático cada 30 segundos.
const bool AUTO_SEND = true;
const unsigned long SIM_INTERVAL_MS = 30000UL;

// Zona horaria Colombia: UTC-5.
// Cambia estos valores si estás en otra zona horaria.
const long GMT_OFFSET_SEC = -5L * 3600L;
const int DAYLIGHT_OFFSET_SEC = 0;
const char* TZ_SUFFIX = "-05:00";

// ==============================================================================

Preferences prefs;
uint32_t raceId = 1;
unsigned long lastSendMs = 0;

// ------------------------------------------------------------------------------
// Utilidades de tiempo
// ------------------------------------------------------------------------------

bool waitForNtpTime(uint32_t timeoutMs = 10000) {
  Serial.print("Sincronizando hora NTP");
  uint32_t start = millis();

  while (millis() - start < timeoutMs) {
    time_t now = time(nullptr);
    if (now > 1700000000) {  // Fecha razonable posterior a 2023.
      Serial.println(" OK");
      return true;
    }
    Serial.print(".");
    delay(500);
  }

  Serial.println(" NO DISPONIBLE");
  return false;
}

String isoLocalTime(time_t t) {
  struct tm timeinfo;
  localtime_r(&t, &timeinfo);

  char buffer[40];
  snprintf(buffer, sizeof(buffer),
           "%04d-%02d-%02dT%02d:%02d:%02d%s",
           timeinfo.tm_year + 1900,
           timeinfo.tm_mon + 1,
           timeinfo.tm_mday,
           timeinfo.tm_hour,
           timeinfo.tm_min,
           timeinfo.tm_sec,
           TZ_SUFFIX);

  return String(buffer);
}

String fallbackIsoTime(uint32_t id, uint8_t extraSeconds) {
  // Fallback para pruebas si no hay NTP. Mantiene un formato válido.
  uint8_t minute = (id / 2) % 60;
  uint8_t second = (id * 7 + extraSeconds) % 60;

  char buffer[40];
  snprintf(buffer, sizeof(buffer),
           "2026-05-13T15:%02u:%02u%s",
           minute,
           second,
           TZ_SUFFIX);
  return String(buffer);
}

// ------------------------------------------------------------------------------
// WiFi
// ------------------------------------------------------------------------------

bool connectWiFi(uint32_t timeoutMs = 15000) {
  if (WiFi.status() == WL_CONNECTED) return true;

  Serial.println();
  Serial.print("Conectando a WiFi: ");
  Serial.println(WIFI_SSID);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  uint32_t start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < timeoutMs) {
    Serial.print(".");
    delay(500);
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println();
    Serial.print("WiFi conectado. IP ESP32: ");
    Serial.println(WiFi.localIP());
    return true;
  }

  Serial.println();
  Serial.println("ERROR: No se pudo conectar a WiFi.");
  return false;
}

// ------------------------------------------------------------------------------
// Generación de carrera simulada
// ------------------------------------------------------------------------------

String buildWorkoutJson(uint32_t currentRaceId) {
  // Simula tiempos típicos para 50 m: 6.80 s a 11.50 s.
  float durationS = random(680, 1151) / 100.0f;
  uint32_t durationMs = (uint32_t)(durationS * 1000.0f + 0.5f);
  float speedMps = DISTANCE_M / durationS;

  int bpmFinish = random(145, 191);
  int recoveryDrop = random(20, 56);
  int bpmRecovery = bpmFinish - recoveryDrop;
  if (bpmRecovery < 80) bpmRecovery = 80;

  String startTime;
  String finishTime;

  time_t now = time(nullptr);
  if (now > 1700000000) {
    time_t finishT = now;
    time_t startT = finishT - (time_t)(durationS + 0.5f);
    startTime = isoLocalTime(startT);
    finishTime = isoLocalTime(finishT);
  } else {
    startTime = fallbackIsoTime(currentRaceId, 0);
    finishTime = fallbackIsoTime(currentRaceId, (uint8_t)(durationS + 1));
  }

  String json = "{";
  json += "\"device_id\":\"" + String(DEVICE_ID) + "\",";
  json += "\"device_type\":\"" + String(DEVICE_TYPE) + "\",";
  json += "\"race_id\":" + String(currentRaceId) + ",";
  json += "\"athlete_code\":" + String(ATHLETE_CODE) + ",";
  json += "\"start_time\":\"" + startTime + "\",";
  json += "\"finish_time\":\"" + finishTime + "\",";
  json += "\"duration_ms\":" + String(durationMs) + ",";
  json += "\"distance_m\":" + String(DISTANCE_M, 2) + ",";
  json += "\"speed_mps\":" + String(speedMps, 3) + ",";
  json += "\"bpm_finish\":" + String(bpmFinish) + ",";
  json += "\"bpm_recovery_60s\":" + String(bpmRecovery) + ",";
  json += "\"distance_calibrated\":true,";
  json += "\"firmware\":\"" + String(FIRMWARE) + "\"";
  json += "}";

  return json;
}

// ------------------------------------------------------------------------------
// Envío HTTP
// ------------------------------------------------------------------------------

bool postWorkout(const String& json) {
  if (!connectWiFi()) return false;

  WiFiClient client;
  HTTPClient http;

  Serial.println();
  Serial.println("Enviando entrenamiento simulado a Tréximo...");
  Serial.print("URL: ");
  Serial.println(API_URL);
  Serial.println("Payload:");
  Serial.println(json);

  if (!http.begin(client, API_URL)) {
    Serial.println("ERROR: http.begin() falló.");
    return false;
  }

  http.addHeader("Content-Type", "application/json");
  http.addHeader("X-Device-Token", DEVICE_TOKEN);

  int httpCode = http.POST(json);
  String response = http.getString();
  http.end();

  Serial.print("HTTP code: ");
  Serial.println(httpCode);
  Serial.print("Respuesta: ");
  Serial.println(response);

  if (httpCode >= 200 && httpCode < 300) {
    Serial.println("OK: entrenamiento guardado en backend/app.");
    return true;
  }

  Serial.println("ERROR: el backend no aceptó el entrenamiento.");
  return false;
}

void sendSimulatedRace() {
  String json = buildWorkoutJson(raceId);
  bool ok = postWorkout(json);

  if (ok) {
    raceId++;
    prefs.putUInt("race_id", raceId);
  }
}

// ------------------------------------------------------------------------------
// Setup / Loop
// ------------------------------------------------------------------------------

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("===============================================");
  Serial.println("ESP32 Tréximo - Simulador API sin sensores");
  Serial.println("===============================================");

  randomSeed(esp_random());

  prefs.begin("treximo_sim", false);
  raceId = prefs.getUInt("race_id", 1);

  Serial.print("Race ID inicial: ");
  Serial.println(raceId);
  Serial.print("Atleta simulado: ");
  Serial.println(ATHLETE_CODE);

  connectWiFi();

  configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, "pool.ntp.org", "time.nist.gov");
  waitForNtpTime(8000);

  Serial.println();
  Serial.println("Comandos por Monitor Serial:");
  Serial.println("  s = enviar una carrera simulada ahora");
  Serial.println("  r = reiniciar race_id a 1");
  Serial.println();

  if (AUTO_SEND) {
    Serial.print("Envío automático activo cada ");
    Serial.print(SIM_INTERVAL_MS / 1000);
    Serial.println(" s.");
    lastSendMs = millis() - SIM_INTERVAL_MS + 3000UL;  // primer envío a los 3 s aprox.
  }
}

void loop() {
  if (Serial.available()) {
    char c = Serial.read();

    if (c == 's' || c == 'S') {
      sendSimulatedRace();
      lastSendMs = millis();
    }

    if (c == 'r' || c == 'R') {
      raceId = 1;
      prefs.putUInt("race_id", raceId);
      Serial.println("Race ID reiniciado a 1.");
    }
  }

  if (AUTO_SEND && millis() - lastSendMs >= SIM_INTERVAL_MS) {
    lastSendMs = millis();
    sendSimulatedRace();
  }
}
