/*
 * SISTEMA AUTÓNOMO DE LLEGADA - ATLETISMO
 * ESP32 + HC-SR04
 * Detecta atleta y se rearma automáticamente
 */

#define TRIG_PIN 15
#define ECHO_PIN 2
#define LED_PIN 16

// ========== CONFIGURACIÓN ==========
float distanciaUmbral_cm = 100;
unsigned long tiempoMinimoEntreDetecciones_ms = 2000;

// ========== ESTADOS ==========
enum EstadoSistema {
  ESPERANDO_ATLETA,
  ATLETA_DETECTADO
};

EstadoSistema estadoActual = ESPERANDO_ATLETA;

unsigned long tiempoDeteccion_ms = 0;

// ========== PROTOTIPOS ==========
float medirDistancia();

// ========== SETUP ==========
void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n=================================");
  Serial.println(" SISTEMA AUTÓNOMO DE LLEGADA ");
  Serial.println("=================================");

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);

  digitalWrite(TRIG_PIN, LOW);
  digitalWrite(LED_PIN, LOW);

  Serial.println("Sistema listo...");
  Serial.println("Esperando atleta...\n");
}

// ========== LOOP ==========
void loop() {

  float d = medirDistancia();

  switch (estadoActual) {

    case ESPERANDO_ATLETA:

      if (d > 0 && d < distanciaUmbral_cm) {

        tiempoDeteccion_ms = millis();

        Serial.println("=================================");
        Serial.println("🏃 ATLETA DETECTADO");
        Serial.print("Distancia: ");
        Serial.print(d);
        Serial.println(" cm");
        Serial.print("Tiempo (ms): ");
        Serial.println(tiempoDeteccion_ms);
        Serial.println("=================================\n");

        // Parpadeo LED
        for (int i = 0; i < 3; i++) {
          digitalWrite(LED_PIN, HIGH);
          delay(150);
          digitalWrite(LED_PIN, LOW);
          delay(150);
        }

        Serial.println("⏳ Esperando que el atleta se retire...\n");

        estadoActual = ATLETA_DETECTADO;
      }

      break;


    case ATLETA_DETECTADO:

      if (d > distanciaUmbral_cm &&
          millis() - tiempoDeteccion_ms > tiempoMinimoEntreDetecciones_ms) {

        Serial.println("✅ Atleta retirado");
        Serial.println("🔁 Sistema listo para nueva detección\n");

        estadoActual = ESPERANDO_ATLETA;
      }

      break;
  }

  delay(50);
}

// ========== FUNCIÓN MEDIR DISTANCIA ==========
float medirDistancia() {

  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);

  digitalWrite(TRIG_PIN, LOW);

  long duracion = pulseIn(ECHO_PIN, HIGH, 30000);

  if (duracion == 0) {
    return -1;
  }

  float distancia = duracion * 0.034 / 2.0;

  return distancia;
}
