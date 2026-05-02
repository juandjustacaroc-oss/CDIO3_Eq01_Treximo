#include <SPI.h>
#include "DW1000Ranging.h"

#define TAG_ADD "7D:00:22:EA:82:60:3B:9C"

#define SPI_SCK 18
#define SPI_MISO 19
#define SPI_MOSI 23
#define DW_CS 4

#define UWB_MODE DW1000.MODE_SHORTDATA_FAST_LOWPOWER

const uint8_t PIN_RST = 27;
const uint8_t PIN_IRQ = 34;
const uint8_t PIN_SS  = 4;

unsigned long lastUwbEvent = 0;
unsigned long lastPrint = 0;

void resetDW1000Hardware()
{
    pinMode(PIN_RST, OUTPUT);
    digitalWrite(PIN_RST, LOW);
    delay(50);

    // Liberar reset
    digitalWrite(PIN_RST, HIGH);
    delay(500);
}

void setup()
{
    Serial.begin(115200);
    delay(3000);   // Importante: deja que la alimentacion se estabilice

    Serial.println();
    Serial.println("Iniciando TAG...");

    resetDW1000Hardware();

    SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI, DW_CS);
    delay(100);

    DW1000Ranging.initCommunication(PIN_RST, PIN_SS, PIN_IRQ);

    DW1000Ranging.attachNewRange(newRange);
    DW1000Ranging.attachNewDevice(newDevice);
    DW1000Ranging.attachInactiveDevice(inactiveDevice);

    DW1000Ranging.startAsTag(TAG_ADD, UWB_MODE);

    lastUwbEvent = millis();

    Serial.println("TAG iniciado correctamente");
}

void loop()
{
    DW1000Ranging.loop();

    if (millis() - lastPrint > 1000) {
        lastPrint = millis();
        Serial.println("TAG vivo, buscando anchor...");
    }

    // Si pasan 15 segundos sin ningun evento UWB, reinicia el ESP32
    if (millis() - lastUwbEvent > 15000) {
        Serial.println("No se detecto anchor. Reiniciando TAG...");
        delay(500);
        ESP.restart();
    }
}

void newRange()
{
    lastUwbEvent = millis();

    Serial.print("from: ");
    Serial.print(DW1000Ranging.getDistantDevice()->getShortAddress(), HEX);
    Serial.print("\t Range: ");
    Serial.print(DW1000Ranging.getDistantDevice()->getRange());
    Serial.print(" m");
    Serial.print("\t RX power: ");
    Serial.print(DW1000Ranging.getDistantDevice()->getRXPower());
    Serial.println(" dBm");
}

void newDevice(DW1000Device *device)
{
    lastUwbEvent = millis();

    Serial.print("ranging init; 1 device added ! -> ");
    Serial.print(" short:");
    Serial.println(device->getShortAddress(), HEX);
}

void inactiveDevice(DW1000Device *device)
{
    Serial.print("delete inactive device: ");
    Serial.println(device->getShortAddress(), HEX);
}