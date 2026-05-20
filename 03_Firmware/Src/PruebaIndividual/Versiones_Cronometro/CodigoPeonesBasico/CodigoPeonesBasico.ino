#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>

#define TRIG_PIN 15
#define ECHO_PIN 2
#define LED_PIN  4

#define ESPNOW_CHANNEL 1

static const float umbral_cm = 50.0;
static const uint32_t debounce_ms = 2500;
static const uint8_t nLecturasConfirmar = 3;

// MAC del cerebro (CAMBIAR)
uint8_t macCerebro[] = {0x00, 0x4B, 0x12, 0x21, 0x73, 0xC0};

// -------- PROTOCOLO --------

#define MSG_START  1
#define SRC_PEON_SALIDA  1

typedef struct __attribute__((packed)) {
  uint8_t  msgType;
  uint8_t  src;
  uint16_t msgId;
  uint32_t ms;
  uint16_t raceId;
  uint8_t  athleteId;
  uint8_t  phase;
  int16_t  value1;
  int16_t  value2;
} Packet;

// -------- VARIABLES --------

uint16_t msgId = 0;
uint32_t ultimaDet_ms = 0;
uint8_t bajoUmbralCount = 0;

// -------- SENSOR --------

float medirDistanciaCM() {

  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);

  digitalWrite(TRIG_PIN, LOW);

  long dur = pulseIn(ECHO_PIN, HIGH, 30000);

  if (dur == 0) return -1;

  return (dur * 0.0343) / 2;
}

// -------- ENVIO --------

void enviarStart(float distancia) {

  Packet p{};

  p.msgType = MSG_START;
  p.src     = SRC_PEON_SALIDA;
  p.msgId   = ++msgId;
  p.ms      = millis();
  p.value1  = (int16_t)(distancia * 10);

  for (int i = 0; i < 3; i++) {
    esp_now_send(macCerebro, (uint8_t*)&p, sizeof(p));
    delay(20);
  }

  // LED confirmación
  for (int i = 0; i < 3; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(80);
    digitalWrite(LED_PIN, LOW);
    delay(80);
  }

  Serial.println("START enviado");
}

// -------- SETUP --------

void setup() {

  Serial.begin(115200);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  esp_wifi_set_channel(ESPNOW_CHANNEL, WIFI_SECOND_CHAN_NONE);

  esp_now_init();

  esp_now_peer_info_t peer{};
  memcpy(peer.peer_addr, macCerebro, 6);
  peer.channel = ESPNOW_CHANNEL;
  peer.encrypt = false;

  esp_now_add_peer(&peer);

  Serial.println("Peon SALIDA listo");
}

// -------- LOOP --------

void loop() {

  float d = medirDistanciaCM();

  if (d > 0 && d < umbral_cm) {
    bajoUmbralCount++;
  } else {
    bajoUmbralCount = 0;
  }

  if (bajoUmbralCount >= nLecturasConfirmar) {

    if (millis() - ultimaDet_ms > debounce_ms) {

      ultimaDet_ms = millis();
      bajoUmbralCount = 0;

      enviarStart(d);
    }
  }

  delay(50);
}