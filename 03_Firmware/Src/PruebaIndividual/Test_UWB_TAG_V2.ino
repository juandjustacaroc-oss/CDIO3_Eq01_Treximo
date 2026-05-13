/*
  ======================================
  DW1000 TAG OPTIMIZADO
  ======================================
  
  Mejoras implementadas:
  - Mismo modo de rango largo que ANCHOR
  - Gestión de múltiples ANChors
  - Filtrado de distancias anómalas
  - Estadísticas de conexión
  - Detección de pérdida de señal
  - Sincronización con Anchor
  
  NOTA IMPORTANTE:
  El TAG debe usar el MISMO MODO que el ANCHOR
  para máximo alcance y compatibilidad.
*/

#include <SPI.h>
#include "DW1000Ranging.h"
#include "DW1000.h"

// ====== CONFIGURACION ======
char tag_addr[] = "7D:00:22:EA:82:60:3B:9C";

// ====== PINES ======
#define SPI_SCK 18
#define SPI_MISO 19
#define SPI_MOSI 23
#define DW_CS 4
#define PIN_RST 27
#define PIN_IRQ 34
#define PIN_SS 4

// ====== VARIABLES DE OPTIMIZACION ======
// El TAG debe usar el MISMO modo que el ANCHOR para máximo rango
#define UWB_MODE DW1000.MODE_LONGDATA_RANGE_ACCURACY

// Control de mediciones
unsigned long ultimaMedicion = 0;
const unsigned long INTERVALO_MEDICION = 100; // ms

// Estadísticas
uint32_t paquetesTx = 0;
uint32_t anchorsDetectados = 0;
uint32_t perdidaSenal = 0;

// Buffer de distancias para filtrado
#define BUFFER_DISTANCIAS 5
float bufferDistancias[BUFFER_DISTANCIAS] = {0};
uint8_t indiceBuffer = 0;

// Filtros
#define DISTANCIA_MIN 0.1    // metros
#define DISTANCIA_MAX 300.0  // metros
#define RSSI_MIN -100.0      // dB (muy débil)

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n=====================================");
  Serial.println("  DW1000 TAG OPTIMIZADO");
  Serial.println("=====================================\n");
  
  Serial.print("Dirección TAG: ");
  Serial.println(tag_addr);
  Serial.print("Modo: LONGDATA_RANGE_ACCURACY");
  Serial.println(" (coincide con ANCHOR)");
  
  // Reset hardware
  resetDW1000Hardware();
  delay(100);
  
  // Inicializar SPI
  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
  
  // Configurar DW1000
  DW1000Ranging.initCommunication(PIN_RST, PIN_SS, PIN_IRQ);
  
  // Registrar callbacks
  DW1000Ranging.attachNewRange(newRange);
  DW1000Ranging.attachNewDevice(newDevice);
  DW1000Ranging.attachInactiveDevice(inactiveDevice);
  
  // Iniciar como TAG con MISMO MODO que ANCHOR
  // ⚠️  CRÍTICO: El modo debe coincidir con el Anchor
  DW1000Ranging.startAsTag(tag_addr, UWB_MODE, false);
  
  Serial.println("\n✓ Sistema inicializado como TAG");
  Serial.println("Buscando Anchors...\n");
  Serial.println("Formato de salida:");
  Serial.println("ANCHOR_ADDR,DISTANCIA_m,RSSI_dB,PROMEDIO_m\n");
}

void loop() {
  // Procesar rangos
  DW1000Ranging.loop();
  
  // Mostrar estadísticas periódicamente
  static unsigned long ultimaEstadistica = 0;
  if (millis() - ultimaEstadistica >= 10000) {
    ultimaEstadistica = millis();
    mostrarEstadisticas();
  }
}

// ====== CALLBACKS ======

void newRange() {
  paquetesTx++;

  DW1000Device *device = DW1000Ranging.getDistantDevice();

  float distancia = device->getRange();
  float rssi = device->getRXPower();
  float firstPath = device->getFPPower();

  if (distancia < DISTANCIA_MIN || distancia > DISTANCIA_MAX) {
    Serial.print("Distancia fuera de rango: ");
    Serial.print(distancia);
    Serial.println(" m");
    return;
  }

  if (rssi < RSSI_MIN) {
    Serial.println("RSSI muy debil");
    perdidaSenal++;
    return;
  }

  bufferDistancias[indiceBuffer] = distancia;
  indiceBuffer = (indiceBuffer + 1) % BUFFER_DISTANCIAS;

  float promedioDistancia = 0.0;
  uint8_t muestrasValidas = 0;

  for (int i = 0; i < BUFFER_DISTANCIAS; i++) {
    if (bufferDistancias[i] > 0) {
      promedioDistancia += bufferDistancias[i];
      muestrasValidas++;
    }
  }

  if (muestrasValidas > 0) {
    promedioDistancia = promedioDistancia / muestrasValidas;
  } else {
    promedioDistancia = distancia;
  }

  Serial.print(device->getShortAddress(), HEX);
  Serial.print(",");
  Serial.print(distancia, 2);
  Serial.print(",");
  Serial.print(rssi, 1);
  Serial.print(",");
  Serial.print(firstPath, 1);
  Serial.print(",");
  Serial.println(promedioDistancia, 2);
}
void newDevice(DW1000Device *device) {
  anchorsDetectados++;
  
  Serial.print("✓ ANCHOR detectado: ");
  Serial.print(device->getShortAddress(), HEX);
  Serial.print(" (Total: ");
  Serial.print(anchorsDetectados);
  Serial.println(")");
}

void inactiveDevice(DW1000Device *device) {
  anchorsDetectados--;
  
  Serial.print("✗ ANCHOR desconectado: ");
  Serial.print(device->getShortAddress(), HEX);
  Serial.print(" (Total: ");
  Serial.print(anchorsDetectados);
  Serial.println(")");
}

// ====== FUNCIONES AUXILIARES ======

void resetDW1000Hardware() {
  Serial.println("Reseteando DW1000...");
  
  pinMode(PIN_RST, OUTPUT);
  digitalWrite(PIN_RST, LOW);
  delay(50);
  
  digitalWrite(PIN_RST, HIGH);
  delay(500);
  
  Serial.println("✓ Reset completado");
}

void mostrarEstadisticas() {
  Serial.println("\n======== ESTADÍSTICAS TAG ========");
  Serial.print("Mediciones transmitidas: ");
  Serial.println(paquetesTx);
  Serial.print("Anchors activos: ");
  Serial.println(anchorsDetectados);
  Serial.print("Pérdidas de señal: ");
  Serial.println(perdidaSenal);
  
  if (anchorsDetectados == 0) {
    Serial.println("⚠️  SIN CONEXIÓN A ANCHORS");
  }
  
  Serial.println("==================================\n");
}

/*
  ========== DIFERENCIAS CLAVE TAG vs ANCHOR ==========
  
  1. MODO DE OPERACIÓN:
     ✓ TAG debe usar MISMO MODO que ANCHOR
     ✗ NO usar modos diferentes (incompatibles)
  
  2. ANTENNA DELAY:
     ANCHOR: Valor calibrado (ej: 16511)
     TAG: Usa predeterminado (16384) - normalmente está bien
  
  3. INICIACIÓN:
     ANCHOR: startAsAnchor()
     TAG: startAsTag()
  
  4. COMPORTAMIENTO:
     ANCHOR: Transmite beacons periódicamente
     TAG: Responde a beacons e inicia mediciones
  
  ========== CONFIGURACIÓN PARA MÁXIMO RANGO ==========
  
  Ambos deben usar:
  DW1000.MODE_LONGDATA_RANGE_ACCURACY
  
  Esto configura:
  - PRF: 64 MHz (mejor para alcance largo)
  - Tasa de datos: Baja (128kbps, mejor alcance)
  - Longitud preambulo: Larga (mejora detección)
  - Duración de símbolo: Larga (más robusta)
  
  ========== FILTRADO DE DATOS ==========
  
  Los filtros implementados descartan:
  - Distancias < 0.1m (ruido muy cercano)
  - Distancias > 300m (fuera de rango realista)
  - RSSI < -100dB (señal demasiado débil)
  
  El promediado (buffer circular) reduce:
  - Ruido de medición
  - Variabilidad instantánea
  - Jitter en aplicaciones en tiempo real
  
  ========== POSIBLES PROBLEMAS Y SOLUCIONES ==========
  
  PROBLEMA: TAG no encuentra ANCHOR
  SOLUCIONES:
  - Verifica que ANCHOR esté ejecutándose primero
  - Ambos deben estar en MISMO MODO
  - Acerca los dispositivos
  - Comprueba que dirección MAC es diferente
  
  PROBLEMA: Distancias erráticas
  SOLUCIONES:
  - Aumenta BUFFER_DISTANCIAS (5 → 10)
  - Verifica antenna delay del Anchor
  - Aleja de obstáculos metálicos
  - Reduce interferencia WiFi/Bluetooth
  
  PROBLEMA: Rango máximo bajo
  SOLUCIONES:
  - Ambos en MODE_LONGDATA_RANGE_ACCURACY
  - Calibra antenna delay del Anchor
  - Verifica capacitores bypass
  - Comprueba conexión SPI
  
  ========== MÚLTIPLES ANCHORS ==========
  
  El TAG puede medir desde varios Anchors:
  - Cada Anchor envía su identificador
  - TAG recibe desde múltiples fuentes
  - Callback newRange() se llama para cada medición
  - anchorsDetectados cuenta los activos
  
  Formato CSV facilita:
  - Importar en Excel/Python
  - Graficar resultados
  - Calcular posición (triangulación)
*/
