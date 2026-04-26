/*
  ======================================
  TEST 5: RTC DS3231
  ======================================
  Objetivo: Inicializar y leer fecha/hora del DS3231
  
  Pines ESP32 (I2C):
  - SDA (Data)  → GPIO 19
  - SCL (Clock) → GPIO 21
  - GND → GND
  - 3.3V → VCC (el DS3231 típicamente es 5V tolerante)
  
  Librerías necesarias (instalar desde Arduino Library Manager):
  - "RTClib" by Adafruit (incluye DS3231, DS1307, PCF8523)
  
  Requisitos de hardware:
  - Módulo RTC DS3231 (reloj en tiempo real)
  - Batería de botón CR2032 (mantiene la hora sin alimentación)
  
  Características:
  - Lee fecha y hora en tiempo real
  - Precisión ±2 ppm (excelente)
  - Opción para ajustar hora inicial
  - Detección de pérdida de potencia
*/

#include <Wire.h>
#include "RTClib.h"

// ====== CONFIGURACION I2C ======
#define SDA_PIN 19
#define SCL_PIN 21

// ====== VARIABLES ======
RTC_DS3231 rtc;
unsigned long ultimaLectura = 0;
const unsigned long INTERVALO_LECTURA = 500;  // Leer cada 500ms

void setup() {
  Serial.begin(115200);
  delay(500);
  
  // Inicializar I2C
  Wire.begin(SDA_PIN, SCL_PIN);
  
  Serial.println("\n=====================================");
  Serial.println("  TEST 5: RTC DS3231");
  Serial.println("=====================================");
  Serial.println("I2C - SDA:GPIO19 / SCL:GPIO21");
  Serial.println("\nVerificando RTC...\n");
  
  // Inicializar RTC
  if (!rtc.begin()) {
    Serial.println("✗ ERROR: RTC DS3231 no detectado!");
    Serial.println("Verifica:");
    Serial.println("  - Conexiones I2C (SDA=19, SCL=21)");
    Serial.println("  - Alimentación (típicamente 5V)");
    Serial.println("  - Dirección I2C (típica: 0x68)");
    Serial.println("  - Batería CR2032 instalada\n");
    while (1) delay(1000);
  }
  
  Serial.println("✓ RTC DS3231 detectado correctamente\n");
  
  // Verificar si el RTC perdió potencia
  if (rtc.lostPower()) {
    Serial.println("⚠️  AVISO: RTC perdió potencia!");
    Serial.println("La fecha/hora no es confiable.");
    Serial.println("Ajustando a la fecha/hora de compilación...\n");
    
    // Descomenta la siguiente línea para ajustar la hora
    // a la fecha/hora actual de compilación
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    
    // O ajusta manualmente con:
    // rtc.adjust(DateTime(2024, 12, 25, 14, 30, 0)); // AAAA, MM, DD, HH, MM, SS
  }
  
  mostrarInfo();
  
  Serial.println("\n--- Leyendo hora cada 500ms ---\n");
}

void loop() {
  unsigned long ahora = millis();
  
  if (ahora - ultimaLectura >= INTERVALO_LECTURA) {
    ultimaLectura = ahora;
    leerYMostrarHora();
  }
}

// ====== FUNCIONES ======

void mostrarInfo() {
  Serial.println("Información del RTC:");
  Serial.println("  Modelo: DS3231");
  Serial.println("  Precisión: ±2 ppm");
  Serial.println("  Rango: 2000-2099");
  Serial.println("  Interfaz: I2C (Dirección 0x68)");
  Serial.println("  Características:");
  Serial.println("    - Oscilador de cristal TCXO");
  Serial.println("    - Batería de respaldo (CR2032)");
  Serial.println("    - Sensor de temperatura integrado");
}

void leerYMostrarHora() {
  // Obtener fecha/hora actual
  DateTime ahora = rtc.now();
  
  // Formato: AAAA-MM-DD HH:MM:SS (ISO 8601)
  Serial.print(ahora.year());
  Serial.print("-");
  if (ahora.month() < 10) Serial.print("0");
  Serial.print(ahora.month());
  Serial.print("-");
  if (ahora.day() < 10) Serial.print("0");
  Serial.print(ahora.day());
  
  Serial.print("  ");
  
  if (ahora.hour() < 10) Serial.print("0");
  Serial.print(ahora.hour());
  Serial.print(":");
  if (ahora.minute() < 10) Serial.print("0");
  Serial.print(ahora.minute());
  Serial.print(":");
  if (ahora.second() < 10) Serial.print("0");
  Serial.print(ahora.second());
  
  // Información adicional
  Serial.print("  | Día semana: ");
  mostrarDiaSemana(ahora.dayOfTheWeek());
  
  // Temperatura del RTC (opcional)
  Serial.print(" | Temp RTC: ");
  Serial.print(rtc.getTemperature(), 1);
  Serial.println("°C");
}

void mostrarDiaSemana(uint8_t dia) {
  // 0=Domingo, 1=Lunes, ..., 6=Sábado
  const char* dias[] = {"Dom", "Lun", "Mar", "Mié", "Jue", "Vie", "Sab"};
  Serial.print(dias[dia]);
}

/*
  INFORMACIÓN DEL DS3231:
  
  El DS3231 es un reloj en tiempo real de precisión con:
  - Oscilador TCXO integrado (±2 ppm anual)
  - Rango de temperatura de -40°C a +85°C
  - Batería de respaldo (CR2032)
  - Interfaz I2C estándar (típicamente 0x68)
  
  CARACTERÍSTICAS PRINCIPALES:
  - Año: 2000-2099
  - Mes: 1-12
  - Día: 1-31
  - Hora: 0-23
  - Minuto: 0-59
  - Segundo: 0-59
  - Día de semana: 1-7 (1=Lunes, 7=Domingo)
  
  PRECISIÓN:
  - ±2 ppm ≈ ±2 segundos/mes
  - Mucho más preciso que DS1307
  - Compensación automática de temperatura
  
  AJUSTAR LA HORA:
  
  Opción 1: Usar fecha/hora de compilación
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  
  Opción 2: Ajustar manualmente
    rtc.adjust(DateTime(2024, 12, 25, 14, 30, 0));
    // Año, Mes, Día, Hora, Minuto, Segundo
  
  Opción 3: Desde un timestamp UNIX
    rtc.adjust(DateTime(1735082400));  // Marca Unix
  
  OPERACIONES ÚTILES:
  
  DateTime ahora = rtc.now();
  
  // Acceso a componentes
  ahora.year()      // Año (2000-2099)
  ahora.month()     // Mes (1-12)
  ahora.day()       // Día (1-31)
  ahora.hour()      // Hora (0-23)
  ahora.minute()    // Minuto (0-59)
  ahora.second()    // Segundo (0-59)
  ahora.dayOfTheWeek()  // Día semana (0=Dom, 1=Lun, etc.)
  ahora.unixtime()  // Timestamp UNIX
  
  // Operaciones
  DateTime futuro = ahora + TimeSpan(7);  // +7 días
  TimeSpan diferencia = ahora - otroTiempo;  // Diferencia
  
  SOLUCIÓN DE PROBLEMAS:
  
  1. RTC no detectado:
     - Verifica conexiones I2C
     - Comprueba dirección 0x68 con scanner I2C
     - Revisa alimentación
  
  2. Hora incorrecta:
     - Comprueba lostPower()
     - Ajusta manualmente con adjust()
     - Verifica batería CR2032 instalada
  
  3. Batería agotada:
     - Reemplaza CR2032
     - Ajusta hora nuevamente
  
  INSTALACIÓN DE LIBRERÍAS:
  1. Arduino IDE → Sketch → Include Library → Manage Libraries
  2. Buscar "RTClib"
  3. Instalar "RTClib by Adafruit"
  
  CONEXIÓN TÍPICA (5V):
  DS3231     ESP32
  ------     -----
  SDA   ---- GPIO 19
  SCL   ---- GPIO 21
  VCC   ---- 5V
  GND   ---- GND
  
  NOTA: El ESP32 tiene pull-ups internos en I2C, pero si usas
        5V en el DS3231, asegúrate de que los pull-ups no causen
        conflicto (típicamente no hay problema, ya que ESP32 es
        3.3V tolerante).
*/
