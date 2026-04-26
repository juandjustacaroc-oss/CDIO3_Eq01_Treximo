/*
  ======================================
  TEST 4: SENSOR MAX30102
  ======================================
  Objetivo: Inicializar y medir frecuencia cardíaca con MAX30102
  
  Pines ESP32 (I2C):
  - SDA (Data)  → GPIO 19
  - SCL (Clock) → GPIO 21
  - GND → GND
  - 3.3V → VCC
  
  Librerías necesarias (instalar desde Arduino Library Manager):
  - "MAX30105 Library" by SparkFun Electronics (by pfeerick)
  - "SparkFun Bio Sensor Hub Library" by SparkFun Electronics
  
  Requisitos de hardware:
  - Sensor MAX30102 (infrarrojo + rojo, óptico)
  - Dedo humano para colocar en el sensor
  
  Características:
  - Inicializa sensor por I2C
  - Lee valores IR (infrarrojo)
  - Detecta presencia de dedo
  - Calcula BPM usando detección de picos
  - Muestra datos por serial cada 1 segundo
*/

#include <Wire.h>
#include "MAX30105.h"
#include "heartRate.h"

// ====== CONFIGURACION I2C ======
#define SDA_PIN 19
#define SCL_PIN 21

// ====== CONFIGURACION DE SENSADO ======
#define TAMANIO_BUFFER_BPM 4
#define INTERVALO_REPORTE 1000  // ms

// ====== VARIABLES - MAX30102 ======
MAX30105 particleSensor;
uint32_t irValue = 0;
uint32_t redValue = 0;

// ====== VARIABLES - BPM ======
byte rates[TAMANIO_BUFFER_BPM];
byte rateSpot = 0;
long lastBeat = 0;
float bpmInstant = 0;
uint16_t bpmPromedio = 0;

// ====== VARIABLES - CONTROL ======
unsigned long ultimoReporte = 0;
bool dedoDetectado = false;

void setup() {
  Serial.begin(115200);
  delay(500);
  
  // Inicializar I2C con pines específicos
  Wire.begin(SDA_PIN, SCL_PIN);
  
  Serial.println("\n=====================================");
  Serial.println("  TEST 4: SENSOR MAX30102");
  Serial.println("=====================================");
  Serial.println("I2C - SDA:GPIO19 / SCL:GPIO21");
  Serial.println("\nInicializando sensor...");
  
  // Inicializar MAX30102
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) {
    Serial.println("\n✗ ERROR: MAX30102 no detectado!");
    Serial.println("Verifica:");
    Serial.println("  - Conexiones I2C (SDA=19, SCL=21)");
    Serial.println("  - Alimentación (3.3V)");
    Serial.println("  - Dirección I2C (típica: 0x57)");
    while (1) delay(1000);
  }
  
  Serial.println("✓ MAX30102 detectado correctamente\n");
  
  // Configurar sensor
  particleSensor.setup();                    // Usar configuración por defecto
  particleSensor.setPulseAmplitudeRed(0x0A);   // Amplitud LED rojo
  particleSensor.setPulseAmplitudeGreen(0);    // Verde apagado
  
  Serial.println("Configuración:");
  Serial.println("  - Modo: Dual (IR + Rojo)");
  Serial.println("  - Velocidad I2C: 400kHz (Fast)");
  Serial.println("  - Amplitud LED: 0x0A");
  Serial.println("\n⚠️  Coloca tu dedo en el sensor para comenzar.\n");
}

void loop() {
  unsigned long ahora = millis();
  
  // Leer sensor continuamente
  irValue = particleSensor.getIR();
  redValue = particleSensor.getRed();
  
  // Detectar presencia de dedo
  detectarDedo();
  
  // Procesar y calcular BPM si hay dedo
  if (dedoDetectado) {
    procesarPulso();
  } else {
    bpmInstant = 0;
    bpmPromedio = 0;
  }
  
  // Reporte periódico
  if (ahora - ultimoReporte >= INTERVALO_REPORTE) {
    ultimoReporte = ahora;
    mostrarDatos();
  }
  
  delay(5);
}

// ====== FUNCIONES ======

void detectarDedo() {
  // Umbral típico: IR > 50000 indica dedo presente
  dedoDetectado = (irValue > 50000);
}

void procesarPulso() {
  // Usar librería heartRate para detectar picos
  if (checkForBeat(irValue)) {
    
    long tiempoAhora = millis();
    long delta = tiempoAhora - lastBeat;
    lastBeat = tiempoAhora;
    
    if (delta > 0) {
      // BPM = 60000 / delta(ms)
      bpmInstant = 60000.0 / delta;
      
      // Filtrar valores razonables (20-255 BPM)
      if (bpmInstant >= 20 && bpmInstant <= 255) {
        rates[rateSpot++] = (byte)bpmInstant;
        rateSpot %= TAMANIO_BUFFER_BPM;
        
        // Calcular promedio del buffer
        bpmPromedio = 0;
        for (byte i = 0; i < TAMANIO_BUFFER_BPM; i++) {
          bpmPromedio += rates[i];
        }
        bpmPromedio /= TAMANIO_BUFFER_BPM;
      }
    }
  }
}

void mostrarDatos() {
  static bool encabezadoMostrado = false;
  
  if (!encabezadoMostrado) {
    Serial.println("TIEMPO\t| DEDO\t| IR\t\t| ROJO\t\t| BPM Inst\t| BPM Prom");
    Serial.println("---------+-------+-----------+-----------+---------+---------");
    encabezadoMostrado = true;
  }
  
  Serial.print(millis() / 1000);
  Serial.print("s\t| ");
  
  // Estado del dedo
  if (dedoDetectado) {
    Serial.print("✓");
  } else {
    Serial.print("✗");
  }
  Serial.print("\t| ");
  
  // Valores IR y Rojo
  Serial.print(irValue);
  Serial.print("\t| ");
  Serial.print(redValue);
  Serial.print("\t| ");
  
  // BPM instantáneo
  if (dedoDetectado && bpmInstant > 0) {
    Serial.print(bpmInstant, 1);
  } else {
    Serial.print("---");
  }
  Serial.print("\t| ");
  
  // BPM promedio
  if (bpmPromedio > 0) {
    Serial.print(bpmPromedio);
  } else {
    Serial.print("---");
  }
  
  Serial.println();
}

/*
  INFORMACIÓN DEL MAX30102:
  
  El MAX30102 es un sensor óptico de frecuencia cardíaca y SpO2
  desarrollado por Maxim Integrated.
  
  CARACTERÍSTICAS TÉCNICAS:
  - 2 LEDs: Infrarrojo (IR) + Rojo
  - Fotodiodo integrado
  - Conversor ADC de 18 bits
  - Interfaz I2C
  - Voltaje: 3.3V típico
  - Consumo: ~20mA
  
  VALORES TÍPICOS:
  - IR con dedo: 50000-200000
  - IR sin dedo: < 20000
  - Frecuencia de muestreo: 100Hz (recomendado)
  
  RANGO DE MEDICIÓN:
  - BPM: 20-255
  - SpO2: 70-100% (si se implementa algoritmo)
  
  CALIBRACIÓN:
  El sensor necesita ajustes de amplitud de LED según:
  - Tipo de piel (melanina)
  - Circulación sanguínea
  - Movimiento del dedo
  
  Si no detecta pulsos:
  1. Ajusta setPulseAmplitudeRed() (0x00 a 0xFF)
  2. Asegúrate de que el dedo esté presionando bien
  3. Evita movimiento exesivo
  4. Verifica iluminación ambiente (no luz directa)
  
  INSTALACIÓN DE LIBRERÍAS:
  1. Arduino IDE → Sketch → Include Library → Manage Libraries
  2. Buscar "MAX30105"
  3. Instalar "MAX30105 Library by SparkFun Electronics"
  4. Buscar "Heart Rate Zone"
  5. Instalar "heartRate by corsair, pfeerick"
  
  ALGORITMO DE DETECCIÓN DE BPM:
  - checkForBeat() detecta picos en el flujo IR
  - Mide tiempo entre picos
  - Calcula BPM = 60000 / tiempo_entre_picos
  
  LIMITACIONES:
  - Sensible al movimiento (mantener dedo quieto)
  - Requiere buena presión sobre el sensor
  - No mide SpO2 sin algoritmo de ratio IR/Red
  - Precisión ±3-5% con calibración adecuada
  
  USOS:
  - Monitoreo de frecuencia cardíaca
  - Detección de arritmias
  - Aplicaciones fitness/deportivas
  - Sistemas de salud wearables
*/
