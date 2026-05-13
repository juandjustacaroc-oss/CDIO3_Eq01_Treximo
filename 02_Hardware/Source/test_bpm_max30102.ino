// ================================================================
//  MONITOR CARDÍACO ATLETA — ESP32 + MAX30102  v2.0
//  BPM con retención inteligente durante movimiento
//
//  Librería: "SparkFun MAX3010x Pulse and Proximity Sensor Library"
// ================================================================

#include <Wire.h>
#include "MAX30105.h"

MAX30105 sensor;

// ─── PARÁMETROS AJUSTABLES ──────────────────────────────────────

#define BPM_MIN            35      // BPM fisiológico mínimo
#define BPM_MAX            220     // BPM fisiológico máximo
#define REFRACTARIO_MS     300     // Mínimo tiempo entre latidos (ms)
#define UMBRAL_PICO_RATIO  0.60f   // Fracción del rango AC para umbral
#define CONFIANZA_MIN      3       // Latidos necesarios para primer BPM

// Retención de BPM durante movimiento
// Fase 1 (0 → HOLD_MOV ms)  : señal fresca o movimiento reciente → "MOV "
// Fase 2 (MOV → HOLD_OLD ms): BPM retenido pero ya algo viejo    → "HOLD"
// Fase 3 (> HOLD_OLD ms)    : demasiado tiempo sin latido válido  → "---"
#define HOLD_MOV_MS        5000
#define HOLD_OLD_MS        12000

// Varianza adaptativa — qué tan rápido el baseline sigue al atleta
// 0.02 = lento (estable), 0.10 = rápido (se adapta en ~10s)
#define ALFA_BASELINE      0.03f

// Ratio varianza/baseline para considerar "movimiento brusco"
#define RATIO_MOV_UMBRAL   3.5f

// ─── BUFFERS ────────────────────────────────────────────────────

#define BUFFER_IR    64
#define BUFFER_RR     8
#define BUFFER_BPM    8

// ─── ESTRUCTURAS ────────────────────────────────────────────────

struct FiltroHP {
  float alfa   = 0.95f;
  float prev_x = 0.0f;
  float prev_y = 0.0f;

  float procesar(float x) {
    float y = (x - prev_x) + alfa * prev_y;
    prev_x = x;
    prev_y = y;
    return y;
  }

  void reset() { prev_x = prev_y = 0.0f; }
};

struct MediaMovil {
  float   buf[BUFFER_BPM] = {0};
  uint8_t idx   = 0;
  float   suma  = 0.0f;
  uint8_t lleno = 0;

  void agregar(float v) {
    suma -= buf[idx];
    buf[idx] = v;
    suma += v;
    idx = (idx + 1) % BUFFER_BPM;
    if (lleno < BUFFER_BPM) lleno++;
  }

  float promedio() const {
    return lleno > 0 ? suma / lleno : 0.0f;
  }

  void reset() {
    for (uint8_t i = 0; i < BUFFER_BPM; i++) buf[i] = 0;
    suma = 0; idx = 0; lleno = 0;
  }
};

struct CalidadSenal {
  float varianzaActual   = 0.0f;
  float varianzaBaseline = 0.0f;
  float ratio            = 1.0f;
  bool  baselineIniciado = false;
  float confianza        = 0.0f;
  float bpmHold          = 0.0f;
  uint32_t t_hold        = 0;

  void actualizarVarianza(long* bufIR, uint8_t n) {
    if (n < 8) return;
    long suma = 0;
    for (uint8_t i = 0; i < n; i++) suma += bufIR[i];
    float media = (float)suma / n;
    float var = 0;
    for (uint8_t i = 0; i < n; i++) {
      float d = bufIR[i] - media;
      var += d * d;
    }
    varianzaActual = var / n;

    if (!baselineIniciado) {
      varianzaBaseline = varianzaActual;
      baselineIniciado = true;
    } else {
      // Solo actualiza el baseline cuando no hay movimiento brusco
      // Evita que los picos de carrera contaminen el baseline
      if (ratio < RATIO_MOV_UMBRAL) {
        varianzaBaseline = (1.0f - ALFA_BASELINE) * varianzaBaseline
                         + ALFA_BASELINE * varianzaActual;
        if (varianzaBaseline < 100.0f) varianzaBaseline = 100.0f;
      }
    }
    ratio = varianzaActual / max(varianzaBaseline, 1.0f);
  }

  void guardarBPM(float bpm, uint32_t ahora) {
    bpmHold = bpm;
    t_hold  = ahora;
    confianza = min(confianza + 25.0f, 100.0f);
  }

  // Decaimiento de confianza según antigüedad del BPM
  void decaerConfianza(uint32_t ahora) {
    uint32_t edad = ahora - t_hold;
    if (edad < HOLD_MOV_MS) {
      confianza = max(confianza - 2.0f, 40.0f);
    } else if (edad < HOLD_OLD_MS) {
      confianza = max(confianza - 8.0f, 10.0f);
    } else {
      confianza = 0.0f;
    }
  }

  enum class EstadoHold { FRESCO, MOV, HOLD, EXPIRADO };

  EstadoHold estadoHold(uint32_t ahora) const {
    if (bpmHold == 0.0f) return EstadoHold::EXPIRADO;
    uint32_t edad = ahora - t_hold;
    if (edad < 2000)        return EstadoHold::FRESCO;
    if (edad < HOLD_MOV_MS) return EstadoHold::MOV;
    if (edad < HOLD_OLD_MS) return EstadoHold::HOLD;
    return EstadoHold::EXPIRADO;
  }

  void reset() {
    varianzaActual = varianzaBaseline = 0.0f;
    ratio = 1.0f;
    baselineIniciado = false;
    confianza = 0.0f;
    bpmHold = 0.0f;
    t_hold  = 0;
  }
};

// ─── VARIABLES GLOBALES ─────────────────────────────────────────

FiltroHP     filtroHP;
MediaMovil   filtroBPM;
CalidadSenal calidad;

long     bufIR[BUFFER_IR];
uint8_t  idxBuf = 0;
uint8_t  nBufIR = 0;

float    histRR[BUFFER_RR];
uint8_t  idxRR  = 0;
uint8_t  contRR = 0;
float    sumaRR = 0.0f;

float    bpmPromedio  = 0.0f;
uint8_t  latidosConsec = 0;

bool     sensorPuesto    = false;
uint32_t t_ultimo_print  = 0;
uint32_t t_ultimo_latido = 0;

float    umbralPico  = 0.0f;
float    umbralCruce = 0.0f;
bool     enPico      = false;
float    picoCima    = 0.0f;
uint32_t t_ultimoPico = 0;

// ─── SETUP ──────────────────────────────────────────────────────

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);

  Serial.println(F("\n══════════════════════════════════════════"));
  Serial.println(F("  MONITOR CARDIACO ATLETA v2.0"));
  Serial.println(F("  MAX30102 + BPM hold anti-movimiento"));
  Serial.println(F("══════════════════════════════════════════\n"));

  Wire.begin(21, 22);

  if (!sensor.begin(Wire, I2C_SPEED_FAST)) {
    Serial.println(F("ERROR: MAX30102 no encontrado"));
    Serial.println(F("  VCC->3.3V  GND->GND  SDA->GPIO21  SCL->GPIO22"));
    while (1) delay(200);
  }
  Serial.println(F("OK: MAX30102 listo"));

  sensor.setup(60, 4, 2, 400, 411, 4096);
  sensor.setPulseAmplitudeRed(0x40);
  sensor.setPulseAmplitudeIR(0x40);

  Serial.println(F("\nColoca el sensor en el LOBULO de la oreja o dedo indice."));
  Serial.println(F("Mantén firme. Iniciara automaticamente.\n"));
  Serial.println(F("----------------------------------------------------------------"));
  Serial.println(F("Tiempo(s) |  BPM  | Conf | Calidad | Zona"));
  Serial.println(F("----------------------------------------------------------------"));
}

// ─── UTILIDADES ─────────────────────────────────────────────────

float mediana3(float a, float b, float c) {
  if ((a <= b && b <= c) || (c <= b && b <= a)) return b;
  if ((b <= a && a <= c) || (c <= a && a <= b)) return a;
  return c;
}

void actualizarUmbrales(float* v, uint8_t n) {
  float mn = v[0], mx = v[0];
  for (uint8_t i = 1; i < n; i++) {
    if (v[i] < mn) mn = v[i];
    if (v[i] > mx) mx = v[i];
  }
  float rango = mx - mn;
  if (rango > 50.0f) {
    umbralPico  = mn + rango * UMBRAL_PICO_RATIO;
    umbralCruce = mn + rango * 0.30f;
  }
}

const char* zonaFC(float bpm) {
  if (bpm < 95)  return "Zona 1 Recuperacion  ";
  if (bpm < 115) return "Zona 2 Base aerobica ";
  if (bpm < 140) return "Zona 3 Aerobico      ";
  if (bpm < 165) return "Zona 4 Umbral lactico";
  return             "Zona 5 VO2max        ";
}

// ─── DETECTOR DE LATIDOS ────────────────────────────────────────

float procesarMuestra(float muestra) {
  static float ventana[16];
  static uint8_t idxV = 0;
  static float h[3] = {0};

  h[2] = h[1]; h[1] = h[0]; h[0] = muestra;
  float m = mediana3(h[0], h[1], h[2]);

  ventana[idxV % 16] = m;
  idxV++;
  if (idxV % 16 == 0) actualizarUmbrales(ventana, 16);

  uint32_t ahora = millis();

  if (!enPico) {
    if (m > umbralPico) { enPico = true; picoCima = m; }
  } else {
    if (m > picoCima) picoCima = m;
    if (m < umbralCruce) {
      enPico = false;
      uint32_t dt = ahora - t_ultimoPico;
      if (dt >= REFRACTARIO_MS) {
        float bpmInst = 60000.0f / (float)dt;
        if (bpmInst >= BPM_MIN && bpmInst <= BPM_MAX) {
          float promedioRR = (contRR > 0)
                             ? sumaRR / (float)min((int)contRR, BUFFER_RR)
                             : (float)dt;
          float ratioRR = (float)dt / promedioRR;
          if (ratioRR > 0.60f && ratioRR < 1.50f) {
            sumaRR -= histRR[idxRR % BUFFER_RR];
            histRR[idxRR % BUFFER_RR] = (float)dt;
            sumaRR += (float)dt;
            idxRR++;
            contRR = (uint8_t)min((int)contRR + 1, BUFFER_RR);
            float bpmRR = 60000.0f / (sumaRR / (float)min((int)contRR, BUFFER_RR));
            filtroBPM.agregar(bpmRR);
            latidosConsec++;
            t_ultimo_latido = ahora;
            t_ultimoPico    = ahora;
            return filtroBPM.promedio();
          }
        }
        t_ultimoPico = ahora;
      }
    }
  }
  return 0.0f;
}

// ─── RESET COMPLETO ─────────────────────────────────────────────

void resetEstado() {
  filtroHP.reset();
  filtroBPM.reset();
  calidad.reset();
  latidosConsec = 0;
  contRR = 0; sumaRR = 0.0f; idxRR = 0;
  bpmPromedio = 0.0f;
  nBufIR = 0; idxBuf = 0;
  enPico = false;
}

// ─── LOOP ────────────────────────────────────────────────────────

void loop() {
  while (!sensor.available()) sensor.check();
  long irRaw = sensor.getIR();
  sensor.nextSample();

  bool puestoAhora = (irRaw > 50000L);

  if (!puestoAhora) {
    if (sensorPuesto) {
      Serial.println(F("\nSin contacto - reposicionar sensor\n"));
      sensorPuesto = false;
      resetEstado();
    }
    delay(50);
    return;
  }

  if (!sensorPuesto) {
    Serial.println(F("Contacto detectado - calibrando 5 s..."));
    sensorPuesto = true;
    uint32_t t0 = millis();
    while (millis() - t0 < 5000) {
      while (!sensor.available()) sensor.check();
      sensor.getIR();
      sensor.nextSample();
    }
    resetEstado();
    Serial.println(F("Listo.\n"));
  }

  // Buffer circular IR para varianza
  bufIR[idxBuf] = irRaw;
  idxBuf = (idxBuf + 1) % BUFFER_IR;
  if (nBufIR < BUFFER_IR) nBufIR++;

  // Varianza adaptativa cada 32 muestras (~320ms)
  static uint8_t cntVar = 0;
  if (++cntVar >= 32) {
    cntVar = 0;
    calidad.actualizarVarianza(bufIR, nBufIR);
  }

  // Filtrado y detección
  float acSenal  = filtroHP.procesar((float)irRaw);
  float bpmNuevo = procesarMuestra(acSenal);

  uint32_t ahora = millis();

  // Si hay un latido válido y estable, guardarlo como BPM hold
  if (bpmNuevo > 0.0f && latidosConsec >= CONFIANZA_MIN) {
    bpmPromedio = bpmNuevo;
    calidad.guardarBPM(bpmPromedio, ahora);
  }

  // Timeout de detección: si no hay latido en 3s, reducir confianza
  if (latidosConsec > 0 && (ahora - t_ultimo_latido) > 3000) {
    latidosConsec = 0;
    contRR = 0;
  }

  // Imprimir cada segundo
  if (ahora - t_ultimo_print < 1000) return;
  t_ultimo_print = ahora;

  calidad.decaerConfianza(ahora);

  auto estado   = calidad.estadoHold(ahora);
  float bpmDisp = calidad.bpmHold;

  const char* etiqueta;
  if (estado == CalidadSenal::EstadoHold::EXPIRADO || bpmDisp == 0.0f) {
    etiqueta = "-----";
  } else if (estado == CalidadSenal::EstadoHold::FRESCO) {
    etiqueta = (calidad.ratio > RATIO_MOV_UMBRAL) ? "MOV  " : "OPTIM";
  } else if (estado == CalidadSenal::EstadoHold::MOV) {
    etiqueta = "MOV  ";
  } else {
    etiqueta = "HOLD ";
  }

  if (estado == CalidadSenal::EstadoHold::EXPIRADO || bpmDisp == 0.0f) {
    Serial.printf("%9.1f |  ---  | %3.0f%% | %-5s   | (sin senal valida)\n",
      ahora / 1000.0f, calidad.confianza, etiqueta);
  } else {
    uint32_t edad = ahora - calidad.t_hold;
    Serial.printf("%9.1f | %5.1f | %3.0f%% | %-5s   | %s  [+%us]\n",
      ahora / 1000.0f,
      bpmDisp,
      calidad.confianza,
      etiqueta,
      zonaFC(bpmDisp),
      edad / 1000);
  }
}

// ================================================================
//  GUIA DE ETIQUETAS EN SERIAL MONITOR
// ----------------------------------------------------------------
//  OPTIM  -> senal limpia, BPM < 2s, sin movimiento brusco
//  MOV    -> movimiento detectado, BPM retenido confiable (< 5s)
//  HOLD   -> sin latido nuevo en 5-12s, usar con precaucion
//  -----  -> sin BPM valido (> 12s sin latido o sensor suelto)
//
//  Conf   -> confianza 0-100%. Sube con latidos, decae con tiempo
//  [+Xs]  -> segundos desde el ultimo latido detectado
//
//  ZONAS FC (FC max = 220 - edad del atleta):
//    Zona 1 < 95 bpm     -> Recuperacion activa
//    Zona 2 95-114       -> Base aerobica
//    Zona 3 115-139      -> Aerobico
//    Zona 4 140-164      -> Umbral lactico
//    Zona 5 165+         -> VO2max / sprint
//
//  CABLEADO:
//    VCC -> 3.3V  |  GND -> GND  |  SDA -> GPIO21  |  SCL -> GPIO22
//
//  LIBRERIA: "SparkFun MAX3010x Pulse and Proximity Sensor Library"
// ================================================================
