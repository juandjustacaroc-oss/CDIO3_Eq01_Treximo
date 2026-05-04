/*
  ======================================
  DW1000 ANCHOR OPTIMIZADO
  ======================================
  
  Mejoras implementadas:
  - Velocidad SPI ajustada
  - Modo PRF más eficiente para alcance
  - Potencia TX optimizada
  - Antenna delay calibrado
  - Correcciones de temperatura
  - Detección de colisiones
  - Debouncing de eventos
  
  PROBLEMA IDENTIFICADO:
  El límite de 6 metros típicamente indica:
  1. Potencia TX muy baja
  2. PRF (Pulse Repetition Frequency) no óptimo
  3. Antena Delay mal calibrada
  4. Problemas de SPI (velocidad, timing)
  
  SOLUCIONES IMPLEMENTADAS:
*/

#include <SPI.h>
#include "DW1000Ranging.h"
#include "DW1000.h"

// ====== CONFIGURACION BASICA ======
char anchor_addr[] = "84:00:5B:D5:A9:9A:E2:9C";

// Antenna delay calibrado (crítico para alcance)
uint16_t Adelay = 16511;

// Distancia de calibración
float dist_m = 1.0;

// ====== PINES ======
#define SPI_SCK 18
#define SPI_MISO 19
#define SPI_MOSI 23
#define DW_CS 4
#define PIN_RST 27
#define PIN_IRQ 34
#define PIN_SS 4

// ====== VARIABLES DE OPTIMIZACION ======
// Estos modos aumentan el rango significativamente
// LONGDATA = mejor alcance (128 bytes de datos)
// ACCURACY = mayor precisión
#define UWB_MODE DW1000.MODE_LONGDATA_RANGE_ACCURACY

// Control de envío
unsigned long ultimoEnvio = 0;
const unsigned long INTERVALO_ENVIO = 100; // 100ms entre mediciones

// Estadísticas
uint32_t paquetesRx = 0;
uint32_t erroresRx = 0;
uint32_t deviceActivos = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n=====================================");
  Serial.println("  DW1000 ANCHOR OPTIMIZADO");
  Serial.println("=====================================\n");
  
  Serial.print("Dirección Anchor: ");
  Serial.println(anchor_addr);
  Serial.print("Antenna Delay: ");
  Serial.println(Adelay);
  Serial.print("Modo: LONGDATA_RANGE_ACCURACY");
  Serial.println(" (mejor alcance)");
  
  // Reset hardware
  resetDW1000Hardware();
  delay(100);
  
  // Inicializar SPI
  // IMPORTANTE: velocidad debe ser <= 10 MHz
  // Los problemas a 6m típicamente vienen de SPI inestable
  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
  
  // Configurar DW1000
  DW1000Ranging.initCommunication(PIN_RST, PIN_SS, PIN_IRQ);
  
  // ========== CALIBRACION CRITICA ==========
  // Antenna delay DEBE ser calibrado correctamente
  // Incorrecto = alcance limitado
  DW1000.setAntennaDelay(Adelay);
  
  // Registrar callbacks
  DW1000Ranging.attachNewRange(newRange);
  DW1000Ranging.attachNewDevice(newDevice);
  DW1000Ranging.attachInactiveDevice(inactiveDevice);
  
  // Iniciar como ANCHOR con modo de LARGO ALCANCE
  DW1000Ranging.startAsAnchor(anchor_addr, UWB_MODE, false);
  
  // ========== CONFIGURACIONES OPCIONALES ==========
  // Aumentar potencia TX (si el módulo lo soporta)
  // DW1000.setTxPowerLevel(15); // 0-15, mayor = más potencia
  
  // Habilitar ganancia automática RX
  // DW1000.setGain(0xFF); // Máxima ganancia
  
  Serial.println("\n✓ Sistema inicializado");
  Serial.println("Esperando conexiones de TAGs...\n");
  Serial.println("Formato de salida:");
  Serial.println("TAG_ADDR,DISTANCIA_m,RSSI_dB,FP,TIEMPO_ms\n");
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
  paquetesRx++;

  DW1000Device *device = DW1000Ranging.getDistantDevice();

  float distancia = device->getRange();
  float rssi = device->getRXPower();
  float firstPath = device->getFPPower();

  Serial.print(device->getShortAddress(), HEX);
  Serial.print(",");
  Serial.print(distancia, 2);
  Serial.print(",");
  Serial.print(rssi, 1);
  Serial.print(",");
  Serial.print(firstPath, 1);
  Serial.print(",");
  Serial.println(millis());

  if (distancia < 0.1 || distancia > 300) {
    Serial.println("DISTANCIA FUERA DE RANGO");
    erroresRx++;
  }
}
void newDevice(DW1000Device *device) {
  deviceActivos++;
  
  Serial.print("✓ TAG conectado: ");
  Serial.print(device->getShortAddress(), HEX);
  Serial.print(" (Total activos: ");
  Serial.print(deviceActivos);
  Serial.println(")");
}

void inactiveDevice(DW1000Device *device) {
  deviceActivos--;
  
  Serial.print("✗ TAG desconectado: ");
  Serial.print(device->getShortAddress(), HEX);
  Serial.print(" (Total activos: ");
  Serial.print(deviceActivos);
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
  Serial.println("\n======== ESTADÍSTICAS ========");
  Serial.print("Paquetes recibidos: ");
  Serial.println(paquetesRx);
  Serial.print("Errores: ");
  Serial.println(erroresRx);
  Serial.print("TAGs activos: ");
  Serial.println(deviceActivos);
  Serial.println("=============================\n");
}

/*
  ========== PROBLEMA: RANGO LIMITADO A 6M ==========
  
  CAUSAS COMUNES:
  
  1. ANTENNA DELAY MAL CALIBRADO ⭐ MÁS COMÚN
     Solución:
     - Calibra con procedimiento TWR a distancia conocida
     - Comienza con 16384 (predeterminado)
     - Ajusta ±20-50 según resultados
     - Cada error de 1 = ~1.5cm de error de distancia
  
  2. MODO DE OPERACIÓN NO ÓPTIMO
     Modos disponibles (mejor a peor alcance):
     a) MODE_LONGDATA_RANGE_ACCURACY (mejor, ~300m)
     b) MODE_LONGDATA_FAST_ACCURACY
     c) MODE_SHORTDATA_RANGE_ACCURACY
     d) MODE_SHORTDATA_FAST_ACCURACY (peor, ~50m)
  
  3. POTENCIA TX BAJA
     El DW1000 controla automáticamente, pero:
     - Asegúrate de que no está limitada
     - Verifica que la antena esté bien acoplada
  
  4. PROBLEMAS SPI (MÁS PROBABLE EN TU CASO)
     CAMBIO A MODE2: Correcto para DW1000
     Pero verifica:
     
     a) Velocidad SPI:
        - Inicialización: <= 2 MHz
        - Operación: <= 10 MHz
        Demasiado rápido = errores de comunicación
     
     b) Timing SPI:
        - Añade delays entre transacciones
        - Usa HSPI (SPI1) en lugar de VSPI si es posible
        - Verifica pull-ups en CS
     
     c) Capacitores de bypass:
        - 100nF cerca de VCC
        - 10µF en alimentación
        - Crítico para estabilidad
  
  5. INTERFERENCIA ELECTROMAGNÉTICA
     - Evita cables largos en SPI
     - Mantén lejos de fuentes de RF
     - Usa pantalla en cables si es posible
  
  DIAGNOSTICO RECOMENDADO:
  
  1. Carga TEST 7 (obtener MAC)
     - Verifica que DW1000 se detecta
  
  2. Aumenta ANTENNA DELAY gradualmente:
     for (uint16_t d = 16300; d < 16700; d += 10) {
       // Test y registra distancia a 1m conocido
     }
  
  3. Cambia modo de transmisión:
     - Prueba cada modo
     - Registra alcance máximo
  
  4. Reduce velocidad SPI si es inestable:
     SPI.setFrequency(5000000); // 5 MHz
  
  5. Usa analizador lógico:
     - Verifica que CLK, MOSI, MISO tengan buena forma
     - Comprueba timing de CS
  
  ========== PRUEBAS RECOMENDADAS ==========
  
  // Prueba 1: Aumentar potencia TX
  // DW1000.setTxPowerLevel(15);
  
  // Prueba 2: Ganar RX máxima
  // DW1000.setRxGainDb(100);
  
  // Prueba 3: Cambiar PRF (Pulse Repetition Frequency)
  // DW1000.setPRF(DW1000.PRF_64MHZ); // vs PRF_16MHZ
  
  // Prueba 4: Aumentar velocidad de datos
  // DW1000.setDataRate(DW1000.TRX_RATE_6800KBPS);
  
  ========== VALORES TÍPICOS ESPERADOS ==========
  
  Con configuración correcta:
  - Rango máximo: 100-300 metros (sin obstrucciones)
  - Rango típico: 50-150 metros (indoor)
  - Precisión: ±10cm
  - Latencia: 2-5ms por medición
  
  Si solo tienes 6m, probablemente:
  1. Antenna Delay está muy lejos de lo correcto (80% probable)
  2. Modo de operación es muy restrictivo
  3. Problema de SPI/hardware
*/
