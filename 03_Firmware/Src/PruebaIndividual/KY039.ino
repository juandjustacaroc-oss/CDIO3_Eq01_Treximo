/*
  ======================================
  TEST 3: SENSOR KY-039 (Pulso)
  ======================================
  Objetivo: Leer sensor analógico y detectar pulsos básicos
  
  Pines ESP32:
  - A0 (AO) → GPIO 34 (ADC1_CH6)
  - + (VCC) → 5V o 3.3V
  - - (GND) → GND
  
  Librerías: Ninguna (ADC nativa)
  
  Nota: Este sensor es de baja precisión y es SOLO para demostración.
        NO usar para aplicaciones médicas reales.
        Para mediciones médicas precisas, usar MAX30102, MAX30105, etc.
  
  Requisitos de hardware:
  - Sensor KY-039 (sensor óptico de pulso)
  - Dedo humano para presionar el sensor
  
  Características:
  - Lee valor analógico cada 20ms
  - Detecta transiciones (picos)
  - Calcula BPM básico
  - ADVERTENCIA: No es médicamente preciso
*/

// ====== CONFIGURACION DE PINES ======
#define PIN_SENSOR 34  // GPIO 34 (ADC1_CH6)

// ====== CONFIGURACION ======
#define INTERVALO_LECTURA 20   // Leer cada 20ms
#define BUFFER_SIZE 32          // Tamaño del buffer de muestras
#define THRESHOLD_SENSIBILIDAD 50  // Diferencia mínima para detectar pulso

// ====== VARIABLES ======
uint16_t buffer[BUFFER_SIZE];
uint16_t indiceBuffer = 0;
uint16_t lecturaActual = 0;
uint16_t lecturaAnterior = 0;
unsigned long ultimaLectura = 0;
unsigned long ultimoPulso = 0;
uint16_t contadorPulsos = 0;
float bpmAproximado = 0;

void setup() {
  Serial.begin(115200);
  delay(500);
  
  // Configurar pin analógico
  pinMode(PIN_SENSOR, INPUT);
  analogReadResolution(12);  // 12 bits (0-4095)
  
  Serial.println("\n=====================================");
  Serial.println("  TEST 3: SENSOR KY-039");
  Serial.println("=====================================");
  Serial.println("Pin: GPIO 34 (ADC)");
  Serial.println("\n⚠️  ADVERTENCIA:");
  Serial.println("Este sensor tiene BAJA PRECISIÓN.");
  Serial.println("NO usar para aplicaciones médicas.");
  Serial.println("Es solo para demostración.\n");
  Serial.println("Coloca tu dedo en el sensor...\n");
}

void loop() {
  unsigned long ahora = millis();
  
  // Lectura periódica
  if (ahora - ultimaLectura >= INTERVALO_LECTURA) {
    ultimaLectura = ahora;
    
    lecturaActual = analogRead(PIN_SENSOR);
    
    // Guardar en buffer
    buffer[indiceBuffer] = lecturaActual;
    indiceBuffer = (indiceBuffer + 1) % BUFFER_SIZE;
    
    // Detectar transición (pulso)
    detectarPulso();
    
    // Mostrar datos cada 250ms
    static unsigned long ultimaMuestra = 0;
    if (ahora - ultimaMuestra >= 250) {
      ultimaMuestra = ahora;
      mostrarDatos();
    }
    
    lecturaAnterior = lecturaActual;
  }
}

// ====== FUNCIONES ======

void detectarPulso() {
  // Comparar con lectura anterior
  uint16_t diferencia = abs((int)lecturaActual - (int)lecturaAnterior);
  
  // Detectar un pico (subida rápida = pulso)
  if (diferencia > THRESHOLD_SENSIBILIDAD && 
      lecturaActual > lecturaAnterior) {
    
    unsigned long ahora = millis();
    
    // Solo contar si pasó suficiente tiempo desde el último pulso (>300ms)
    if (ahora - ultimoPulso > 300) {
      ultimoPulso = ahora;
      contadorPulsos++;
      
      // Calcular BPM (simple: esperar varios pulsos)
      static unsigned long tiempoInicioPulsos = 0;
      static uint16_t pulsosIniciales = 0;
      
      if (pulsosIniciales == 0) {
        tiempoInicioPulsos = ahora;
      }
      pulsosIniciales++;
      
      // Cada 6 pulsos, calcular BPM
      if (pulsosIniciales >= 6) {
        unsigned long tiempoTranscurrido = ahora - tiempoInicioPulsos;
        bpmAproximado = (60000.0 / tiempoTranscurrido) * pulsosIniciales;
        pulsosIniciales = 0;
      }
      
      Serial.print("♥ PULSO DETECTADO | BPM aprox: ");
      Serial.println(bpmAproximado, 1);
    }
  }
}

void mostrarDatos() {
  uint16_t promedio = 0;
  uint16_t minimo = 4095;
  uint16_t maximo = 0;
  
  // Calcular estadísticas del buffer
  for (uint16_t i = 0; i < BUFFER_SIZE; i++) {
    promedio += buffer[i];
    if (buffer[i] < minimo) minimo = buffer[i];
    if (buffer[i] > maximo) maximo = buffer[i];
  }
  promedio /= BUFFER_SIZE;
  
  Serial.print("Lectura: ");
  Serial.print(lecturaActual);
  Serial.print(" | Rango: [");
  Serial.print(minimo);
  Serial.print("-");
  Serial.print(maximo);
  Serial.print("] | Promedio: ");
  Serial.print(promedio);
  Serial.print(" | BPM aprox: ");
  Serial.println(bpmAproximado, 1);
}

/*
  INFORMACIÓN DEL SENSOR KY-039:
  
  El KY-039 es un sensor óptico que detecta cambios de luz
  reflejada en la piel (flujo sanguíneo).
  
  CARACTERÍSTICAS:
  - Resolución analógica pobre
  - Muy sensible al movimiento
  - Requiere luz ambiente adecuada
  - Necesita ajuste manual del umbral
  
  VENTAJAS:
  - Muy económico
  - Bajo consumo de energía
  - Fácil de usar
  
  DESVENTAJAS:
  - Precisión limitada (<70%)
  - Mucho ruido
  - No es médicamente válido
  - Requiere calibración manual
  
  USO RECOMENDADO:
  - Solo para proyectos educativos/demostraciones
  - Entretenimiento
  - Prototipos iniciales
  
  ALTERNATIVA PROFESIONAL:
  - MAX30102 (precisión médica)
  - MAX30105 (incluye SpO2)
  - MLX90614 (para temperatura)
  
  CALIBRACIÓN:
  Aumenta THRESHOLD_SENSIBILIDAD si hay muchos falsos positivos
  Disminuye si no detecta pulsos reales
*/
