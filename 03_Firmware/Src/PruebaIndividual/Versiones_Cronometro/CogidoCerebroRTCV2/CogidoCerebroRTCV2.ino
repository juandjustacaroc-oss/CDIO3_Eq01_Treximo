#include <WiFi.h>
#include <esp_now.h>
#include <Wire.h>
#include "RTClib.h"

#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>
#include <Fonts/FreeSansBold18pt7b.h> 
#include <Fonts/FreeSansBold12pt7b.h> 
#include <Fonts/FreeSans9pt7b.h>     

#define TFT_CS     5
#define TFT_RST    4
#define TFT_DC     2

#define BTN_START 12
#define BTN_STOP  13
#define BTN_RESET 14

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

#define BG_COLOR    tft.color565(10, 10, 15)   
#define TIME_COLOR  ST77XX_WHITE               
#define VEL_COLOR   tft.color565(0, 255, 255) 
#define BPM_COLOR   ST77XX_WHITE               
#define GREY_TEXT   tft.color565(120, 120, 130)

// ================= RTC =================

RTC_DS3231 rtc;
DateTime horaInicio;
DateTime horaFin;

// ================= CRONOMETRO =================

unsigned long tiempoInicio = 0;
unsigned long tiempoPausado = 0;
bool cronometroActivo = false;

// ================= SENSORES =================

int bpm = 72;
float velocidad = 15.3;

// ================= ESPNOW =================

#define MSG_START  1
#define MSG_FINISH 2

typedef struct{
  uint8_t msgType;
}Packet;


// ================= FUNCIONES COMUNES =================

void iniciarCronometro(){

  if(!cronometroActivo){

    Serial.println("\n===== START =====");

    cronometroActivo = true;
    tiempoInicio = millis() - tiempoPausado;

    horaInicio = rtc.now();

    Serial.printf("Hora inicio: %02d:%02d:%02d\n",
      horaInicio.hour(), horaInicio.minute(), horaInicio.second());
  }
}

void detenerCronometro(){

  if(cronometroActivo){

    Serial.println("\n===== FINISH =====");

    cronometroActivo = false;
    tiempoPausado = millis() - tiempoInicio;

    horaFin = rtc.now();

    Serial.printf("Hora llegada: %02d:%02d:%02d\n",
      horaFin.hour(), horaFin.minute(), horaFin.second());

    Serial.print("Tiempo carrera: ");
    Serial.print(tiempoPausado / 1000.0);
    Serial.println(" s");
  }
}


// ================= RECEPCION ESPNOW =================

void OnDataRecv(const esp_now_recv_info_t *info, const uint8_t *data, int len){

  Packet p;
  memcpy(&p, data, sizeof(p));

  if(p.msgType == MSG_START){
    iniciarCronometro();
  }

  if(p.msgType == MSG_FINISH){
    detenerCronometro();
  }
}


// ================= BOTONES =================

void manejarBotones(){

  static bool lastStart = HIGH;
  static bool lastStop  = HIGH;
  static bool lastReset = HIGH;

  bool estadoStart = digitalRead(BTN_START);
  bool estadoStop  = digitalRead(BTN_STOP);
  bool estadoReset = digitalRead(BTN_RESET);

  // START
  if(estadoStart == LOW && lastStart == HIGH){
    iniciarCronometro();
    delay(200);
  }

  // STOP
  if(estadoStop == LOW && lastStop == HIGH){
    detenerCronometro();
    delay(200);
  }

  // RESET
  if(estadoReset == LOW && lastReset == HIGH){

    cronometroActivo = false;
    tiempoInicio = 0;
    tiempoPausado = 0;

    limpiarTiempo();

    Serial.println("\nSistema reiniciado");

    delay(200);
  }

  lastStart = estadoStart;
  lastStop  = estadoStop;
  lastReset = estadoReset;
}


// ================= SETUP =================

void setup() {

  Serial.begin(115200);

  pinMode(BTN_START, INPUT_PULLUP);
  pinMode(BTN_STOP,  INPUT_PULLUP);
  pinMode(BTN_RESET, INPUT_PULLUP);

  // RTC
  Wire.begin(21,22);

  if(!rtc.begin()){
    Serial.println("No se detecto RTC");
  }

  if(rtc.lostPower()){
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  // TFT
  tft.initR(INITR_BLACKTAB);
  tft.setRotation(1);

  pantallaCarga();
  dibujarInterfazBase();

  // WIFI + ESPNOW
  WiFi.mode(WIFI_STA);

  if(esp_now_init()!=ESP_OK){
    Serial.println("Error ESP NOW");
    return;
  }

  esp_now_register_recv_cb(OnDataRecv);

  Serial.print("MAC Cerebro: ");
  Serial.println(WiFi.macAddress());
}


// ================= LOOP =================

void loop() {

  manejarBotones();

  if (cronometroActivo) {
    actualizarCronometro();
  }

  actualizarSensores();

  if (millis() % 2000 < 20) {
    bpm = random(60, 175);
    velocidad = random(50, 400) / 10.0;
  }
}


// ================= UI =================

void pantallaCarga() {

  tft.fillScreen(BG_COLOR);

  tft.setTextColor(ST77XX_CYAN);
  tft.setFont(&FreeSansBold12pt7b);

  tft.setCursor(20, 45); tft.print("Cronometro");
  tft.setCursor(20, 75); tft.print("Inteligente");

  for (int i = 0; i < 360; i += 40) {

    int x = 80 + 15 * cos(i * 0.0174);
    int y = 105 + 15 * sin(i * 0.0174);

    tft.fillCircle(x, y, 3, ST77XX_CYAN);

    delay(100);
  }
}


void dibujarInterfazBase() {

  tft.fillScreen(BG_COLOR);

  tft.drawFastHLine(0, 68, 160, tft.color565(40, 40, 50));
  tft.drawFastVLine(80, 68, 60, tft.color565(40, 40, 50));

  tft.setFont();
  tft.setTextColor(GREY_TEXT);

  tft.setCursor(62, 5);  tft.print("TIEMPO");
  tft.setCursor(10, 75); tft.print("FC");
  tft.setCursor(90, 75); tft.print("VELOCIDAD");

  limpiarTiempo();
}


void actualizarCronometro() {

  static unsigned long lastMillis = 0;
  unsigned long actual = millis() - tiempoInicio;

  if (actual - lastMillis >= 100) {

    lastMillis = actual;

    unsigned int min = actual / 60000;
    unsigned int seg = (actual % 60000) / 1000;
    unsigned int mil = (actual % 1000) / 100;

    char buffer[10];
    sprintf(buffer, "%02u:%02u.%u", min, seg, mil);

    tft.fillRect(5, 18, 150, 35, BG_COLOR);

    tft.setFont(&FreeSansBold18pt7b);
    tft.setTextColor(TIME_COLOR);

    int16_t x1, y1; uint16_t w, h;
    tft.getTextBounds(buffer, 0, 0, &x1, &y1, &w, &h);

    tft.setCursor((160 - w) / 2, 52);
    tft.print(buffer);
  }
}


void limpiarTiempo(){

  tft.fillRect(5, 18, 150, 35, BG_COLOR);

  tft.setFont(&FreeSansBold18pt7b);
  tft.setTextColor(TIME_COLOR);

  tft.setCursor(20, 52);
  tft.print("00:00.0");
}


void actualizarSensores(){

  static int oldBpm = -1;
  static float oldVel = -1;

  if (bpm != oldBpm){

    tft.fillRect(5, 88, 70, 38, BG_COLOR);
    tft.setTextColor(BPM_COLOR);
    tft.setFont(&FreeSansBold12pt7b);
    tft.setCursor(10, 110);
    tft.print(bpm);

    tft.setFont();
    tft.setTextColor(GREY_TEXT);
    tft.setCursor(10, 118);
    tft.print("BPM");

    oldBpm = bpm;
  }

  if (velocidad != oldVel){

    tft.fillRect(85, 88, 70, 38, BG_COLOR);
    tft.setTextColor(VEL_COLOR);
    tft.setFont(&FreeSansBold12pt7b);
    tft.setCursor(90, 110);
    tft.print(velocidad, 1);

    tft.setFont();
    tft.setTextColor(GREY_TEXT);
    tft.setCursor(90, 118);
    tft.print("KM/H");

    oldVel = velocidad;
  }
}