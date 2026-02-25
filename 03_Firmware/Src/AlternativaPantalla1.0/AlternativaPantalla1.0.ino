#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>
#include <Fonts/FreeSansBold18pt7b.h>
#include <Fonts/FreeSansBold12pt7b.h>
#include <Fonts/FreeSansBold9pt7b.h>

#define TFT_CS     5
#define TFT_RST    4
#define TFT_DC     2

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

#define BG tft.color565(10,10,15)
#define DARKGREY tft.color565(60,60,60)

unsigned long tiempoInicio;
unsigned long tiempoPrevio = 0;

int bpm = 85;
float velocidad = 7.5;

void pantallaInicio() {

  tft.fillScreen(BG);

  tft.setFont(&FreeSansBold12pt7b);
  tft.setTextColor(ST77XX_WHITE);

  int16_t x1, y1;
  uint16_t w, h;

  String titulo = "Cronometro";
  tft.getTextBounds(titulo, 0, 0, &x1, &y1, &w, &h);
  tft.setCursor((160 - w) / 2, 45);
  tft.print(titulo);

  String subtitulo = "Inteligente";
  tft.getTextBounds(subtitulo, 0, 0, &x1, &y1, &w, &h);
  tft.setCursor((160 - w) / 2, 75);
  tft.print(subtitulo);

  int cx = 80;
  int cy = 105;
  int r = 15;

  for (int ang = 0; ang < 360; ang += 20) {
    tft.drawCircle(cx, cy, r, DARKGREY);
    tft.drawLine(cx, cy,
                 cx + r * cos(ang * 3.1416 / 180),
                 cy + r * sin(ang * 3.1416 / 180),
                 ST77XX_CYAN);
    delay(60);
  }

  delay(400);
}

void dibujarBase() {

  tft.fillScreen(BG);

  tft.drawRoundRect(5, 5, 150, 60, 8, ST77XX_CYAN);
  tft.drawFastHLine(0, 75, 160, DARKGREY);

  tft.setFont(&FreeSansBold9pt7b);
  tft.setTextColor(ST77XX_WHITE);

  tft.setCursor(10, 100);
  tft.print("FC:");

  tft.setCursor(10, 120);
  tft.print("VEL:");
}

void actualizarTiempo() {

  unsigned long tiempoActual = millis() - tiempoInicio;

  if (tiempoActual - tiempoPrevio >= 50) {  // refresco más fluido

    tiempoPrevio = tiempoActual;

    unsigned int minutos = tiempoActual / 60000;
    unsigned int segundos = (tiempoActual % 60000) / 1000;
    unsigned int centesimas = (tiempoActual % 1000) / 10;

    char buffer[12];
    sprintf(buffer, "%02u:%02u:%02u", minutos, segundos, centesimas);

    tft.fillRect(10, 20, 140, 35, BG);

    tft.setFont(&FreeSansBold18pt7b);
    tft.setTextColor(ST77XX_GREEN);

    int16_t x1, y1;
    uint16_t w, h;
    tft.getTextBounds(buffer, 0, 0, &x1, &y1, &w, &h);

    tft.setCursor((160 - w) / 2, 50);
    tft.print(buffer);
  }
}

void actualizarDatos() {

  static int bpmPrev = 0;
  static float velPrev = 0;

  if (bpm != bpmPrev) {
    tft.fillRect(55, 88, 100, 20, BG);
    tft.setFont(&FreeSansBold12pt7b);
    tft.setTextColor(ST77XX_RED);
    tft.setCursor(55, 105);
    tft.print(bpm);
    tft.print(" bpm");
    bpmPrev = bpm;
  }

  if (velocidad != velPrev) {
    tft.fillRect(55, 108, 100, 20, BG);
    tft.setFont(&FreeSansBold12pt7b);
    tft.setTextColor(ST77XX_YELLOW);
    tft.setCursor(55, 125);
    tft.print(velocidad, 1);
    tft.print(" km/h");
    velPrev = velocidad;
  }
}

void setup() {

  tft.initR(INITR_BLACKTAB);
  tft.setRotation(1);

  pantallaInicio();
  dibujarBase();

  tiempoInicio = millis();
}

void loop() {

  actualizarTiempo();
  actualizarDatos();

  if (millis() % 3000 == 0) {
    bpm = random(75, 95);
    velocidad = random(60, 95) / 10.0;
  }
}