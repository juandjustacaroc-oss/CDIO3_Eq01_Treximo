#include <Arduino.h>
#include <SPI.h>

#define SPI_SCK   18
#define SPI_MISO  19
#define SPI_MOSI  23
#define DW_CS     4

const uint8_t PIN_RST = 27;
const uint8_t PIN_IRQ = 34;
const uint8_t PIN_SS  = 4;

SPIClass spi = SPIClass(VSPI);

uint32_t readDevId(uint8_t spiMode) {
  SPISettings settings(1000000, MSBFIRST, spiMode);

  uint8_t b0, b1, b2, b3;

  spi.beginTransaction(settings);
  digitalWrite(PIN_SS, LOW);

  spi.transfer(0x00);

  b0 = spi.transfer(0x00);
  b1 = spi.transfer(0x00);
  b2 = spi.transfer(0x00);
  b3 = spi.transfer(0x00);

  digitalWrite(PIN_SS, HIGH);
  spi.endTransaction();

  return ((uint32_t)b3 << 24) |
         ((uint32_t)b2 << 16) |
         ((uint32_t)b1 << 8)  |
         ((uint32_t)b0);
}

void resetDW() {
  pinMode(PIN_RST, OUTPUT);
  digitalWrite(PIN_RST, LOW);
  delay(10);

  // Para DW1000/DWM1000: liberar reset en alta impedancia
  pinMode(PIN_RST, INPUT);
  delay(100);
}

void testMode(uint8_t mode, const char* name) {
  uint32_t devId = readDevId(mode);

  Serial.print(name);
  Serial.print(" -> DEV_ID: 0x");
  Serial.println(devId, HEX);

  if (devId == 0xDECA0130) {
    Serial.println("OK: ID correcto para DW1000.");
  } else if (devId == 0xBC950360) {
    Serial.println("Lectura desplazada 1 bit: probable modo SPI/fase incorrecta.");
  } else if (devId == 0xFFFFFFFF) {
    Serial.println("Error: bus sin respuesta o MISO en alto.");
  } else if (devId == 0x00000000) {
    Serial.println("Error: bus sin respuesta o MISO en bajo.");
  } else {
    Serial.println("Respuesta no esperada, pero no vacía.");
  }

  Serial.println("----------------------------------");
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("===== Test de modos SPI DW1000 =====");

  pinMode(PIN_SS, OUTPUT);
  digitalWrite(PIN_SS, HIGH);

  pinMode(PIN_IRQ, INPUT);

  spi.begin(SPI_SCK, SPI_MISO, SPI_MOSI, PIN_SS);

  resetDW();

  testMode(SPI_MODE0, "SPI_MODE0");
  testMode(SPI_MODE1, "SPI_MODE1");
  testMode(SPI_MODE2, "SPI_MODE2");
  testMode(SPI_MODE3, "SPI_MODE3");
}

void loop() {
}