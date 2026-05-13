#include <Wire.h>
#include "MAX30105.h"

// ====== CONFIGURACION I2C ======
#define SDA_PIN 19
#define SCL_PIN 21

MAX30105 particleSensor;

// Variables para el filtro de la gráfica
double dcOffset = 0;
const double alpha = 0.95; // Factor para extraer la línea base (filtro paso alto)

void setup() {
  Serial.begin(115200);
  
  // Inicializar I2C en los pines del ESP32
  Wire.begin(SDA_PIN, SCL_PIN);

  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) {
    Serial.println("Sensor MAX30102 no encontrado");
    while (1);
  }

  // Configuración optimizada para visualización de onda
  byte ledBrightness = 0x1F; 
  byte sampleAverage = 4;    
  byte ledMode = 2;          
  int sampleRate = 400;      
  int pulseWidth = 411;      
  int adcRange = 4096;       

  particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange);
}

void loop() {
  uint32_t irRaw = particleSensor.getIR();

  if (irRaw < 50000) {
    // Si no hay dedo, ambas líneas (señal y referencia) van a cero
    Serial.print("Latido:0.00");
    Serial.print(",");
    Serial.println("Referencia:0.00");
    dcOffset = 0; 
  } else {
    // --- FILTRO DIGITAL (Eliminación de DC) ---
    if (dcOffset == 0) dcOffset = irRaw;
    
    // Suavizado de la línea base (Componente DC)
    dcOffset = (alpha * dcOffset) + ((1.0 - alpha) * irRaw);
    
    // Señal AC (El latido real)
    double latido = irRaw - dcOffset;

    // --- SALIDA PARA EL SERIAL PLOTTER ---
    // Al imprimir "Etiqueta:Valor", el plotter crea una leyenda automática
    Serial.print("Latido:");
    Serial.print(latido);
    Serial.print(","); // Separador para múltiples gráficas
    Serial.print("Referencia:"); 
    Serial.println(0.00); // Línea constante en cero
  }

  // Delay de 20ms (aprox 50Hz de refresco) para una gráfica estable
  delay(20); 
}
