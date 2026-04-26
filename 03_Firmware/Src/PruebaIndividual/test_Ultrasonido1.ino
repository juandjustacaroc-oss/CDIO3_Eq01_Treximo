/*
  ======================================
  TEST 1: SENSOR HC-SR04 CON LED
  ======================================
  Objetivo: Medir distancia y controlar LED según umbral
  
  Pines ESP32:
  - TRIG (Trigger) → GPIO 25
  - ECHO (Echo)    → GPIO 26
  - LED           → GPIO 27
  
  Librerías: Ninguna (solo Arduino built-in)
  
  Requisitos de hardware:
  - Sensor HC-SR04 (ultrasónico)
  - LED 3mm + resistencia 220Ω
  
  Características:
  - Mide distancia cada 100ms
  - LED se enciende si distancia < umbral
  - Sin delays bloqueantes (usa millis)
*/

// ====== CONFIGURACION DE PINES ======
#define PIN_TRIG 25
#define PIN_ECHO 26
#define PIN_LED  27

// ====== CONFIGURACION ======
#define UMBRAL_DISTANCIA_CM 20  // LED se enciende si distancia < 20cm
#define INTERVALO_MEDIDA 100    // Medir cada 100ms

// ====== VARIABLES ======
unsigned long ultimaMedida = 0;
float distancia = 0;
bool ledEncendido = false;

void setup() {
  Serial.begin(115200);
  delay(500);
  
  // Configurar pines
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
  pinMode(PIN_LED, OUTPUT);
  
  // Estado inicial
  digitalWrite(PIN_TRIG, LOW);
  digitalWrite(PIN_LED, LOW);
  
  Serial.println("\n=====================================");
  Serial.println("  TEST 1: SENSOR HC-SR04 + LED");
  Serial.println("=====================================");
  Serial.print("Umbral LED: ");
  Serial.print(UMBRAL_DISTANCIA_CM);
  Serial.println(" cm");
  Serial.println("Esperando sensor...\n");
}

void loop() {
  unsigned long ahora = millis();
  
  // Medir cada INTERVALO_MEDIDA ms
  if (ahora - ultimaMedida >= INTERVALO_MEDIDA) {
    ultimaMedida = ahora;
    medirDistancia();
    controlarLED();
    mostrarDatos();
  }
}

// ====== FUNCIONES ======

void medirDistancia() {
  // Generar pulso de 10µs en TRIG
  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);
  
  // Leer duración del pulso ECHO (timeout: 30ms)
  unsigned long duracion = pulseIn(PIN_ECHO, HIGH, 30000);
  
  // Convertir duración a distancia
  // Velocidad del sonido: 343 m/s ≈ 0.0343 cm/µs
  // Distancia = (duración * 0.0343) / 2
  // Factor simplificado: 0.01715
  if (duracion > 0) {
    distancia = duracion * 0.01715;
  } else {
    distancia = -1;  // Error o fuera de rango
  }
}

void controlarLED() {
  if (distancia > 0 && distancia < UMBRAL_DISTANCIA_CM) {
    digitalWrite(PIN_LED, HIGH);
    ledEncendido = true;
  } else {
    digitalWrite(PIN_LED, LOW);
    ledEncendido = false;
  }
}

void mostrarDatos() {
  Serial.print("Distancia: ");
  
  if (distancia < 0) {
    Serial.print("--- (sin detección)");
  } else {
    Serial.print(distancia, 1);
    Serial.print(" cm");
  }
  
  Serial.print(" | LED: ");
  Serial.println(ledEncendido ? "ENCENDIDO" : "apagado");
}
