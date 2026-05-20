#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>
#include <Fonts/FreeSansBold12pt7b.h>
#include <Fonts/FreeSansBold18pt7b.h>
#include <Fonts/FreeSansBold9pt7b.h>

#define TFT_CS   5
#define TFT_RST  4
#define TFT_DC   2

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

// ================= COLORES =================
#define BG_COLOR      tft.color565(10,10,15)
#define LINE_COLOR    tft.color565(40,40,50)
#define GREY_TEXT     tft.color565(120,120,130)

#define TIME_COLOR  ST77XX_WHITE
#define BPM_COLOR   ST77XX_WHITE
#define VEL_COLOR   ST77XX_CYAN
#define DIST_COLOR  ST77XX_GREEN

// ================= VARIABLES =================
unsigned long tiempoInicio;

int bpm = 158;
float velocidad = 22.7;
float distancia = 2.0;

// =====================================================
//         DIBUJAR LOGO TRÉXIMO
// =====================================================
void dibujarLogoTreximo() {

  uint16_t azul   = tft.color565(10, 20, 80);
  uint16_t blanco = ST77XX_WHITE;

  int cx = 38, cy = 62;
  int re = 28;
  int ri = 20;

  float angBase = -30 * 0.01745;

  int xo[6], yo[6], xi[6], yi[6];
  for (int i = 0; i < 6; i++) {
    float ang = angBase + i * 60 * 0.01745;
    xo[i] = cx + re * cos(ang);
    yo[i] = cy + re * sin(ang);
    xi[i] = cx + ri * cos(ang);
    yi[i] = cy + ri * sin(ang);
  }

  // ── Hexágono exterior azul ──
  for (int i = 0; i < 6; i++) {
    int j = (i + 1) % 6;
    tft.fillTriangle(xo[i], yo[i], xo[j], yo[j], cx, cy, azul);
  }

  // ── Interior blanco ──
  for (int i = 0; i < 6; i++) {
    int j = (i + 1) % 6;
    tft.fillTriangle(cx, cy, xi[i], yi[i], xi[j], yi[j], blanco);
  }

  // ── Barra horizontal superior del 7 ──
  // Bajada 6px del borde interior para que sea visible sobre el blanco
  int barraY  = cy - ri + 6;
  int barraX1 = cx - ri + 4;
  int barraX2 = cx + ri - 4;
  int barraH  = 5;                  // 5px de alto = bien visible

  tft.fillRect(barraX1, barraY, barraX2 - barraX1, barraH, azul);

  // ── Diagonal del 7 — 5px de grosor ──
  int dx1 = barraX2 - 2;
  int dy1 = barraY + barraH;
  int dx2 = cx - 5;
  int dy2 = cy + ri - 4;            // toca el borde inferior

  for (int offset = -2; offset <= 2; offset++) {
    tft.drawLine(dx1 + offset, dy1, dx2 + offset, dy2, azul);
  }

  // ── Texto "Treximo" ──
  tft.setFont(&FreeSansBold9pt7b);
  tft.setTextColor(azul);
  tft.setCursor(74, 69);
  tft.print("Treximo");
}

// =====================================================
//                 PANTALLA CARGA
// =====================================================
void pantallaCarga() {

  tft.fillScreen(ST77XX_WHITE);

  dibujarLogoTreximo();

  uint16_t azul = tft.color565(10, 20, 80);
  uint16_t gris = tft.color565(210, 210, 220);

  int barX = 15, barY = 108, barW = 130, barH = 6, barR = 3;

  tft.fillRoundRect(barX, barY, barW, barH, barR, gris);

  int pasos = barW - 2;
  int delayPorPaso = 2000 / pasos;

  for (int i = 1; i <= pasos; i++) {
    tft.fillRoundRect(barX + 1, barY + 1, i, barH - 2, barR, azul);
    delay(delayPorPaso);
  }

  delay(2000);
}

// =====================================================
//                       SETUP
// =====================================================
void setup() {
  tft.initR(INITR_BLACKTAB);
  tft.setRotation(1);

  pantallaCarga();
  interfazBase();

  tiempoInicio = millis();
}

// =====================================================
//                        LOOP
// =====================================================
void loop() {
  actualizarTiempo();
  actualizarDatos();

  if (millis() % 2000 < 20) {
    bpm = random(60, 180);
    velocidad = random(50, 350) / 10.0;
    distancia += 0.2;
  }
}

// =====================================================
//                INTERFAZ BASE
// =====================================================
void interfazBase() {
  tft.fillScreen(BG_COLOR);

  tft.drawFastHLine(0, 55, 160, LINE_COLOR);
  tft.drawFastVLine(80, 55, 45, LINE_COLOR);
  tft.drawFastHLine(0, 100, 160, LINE_COLOR);

  tft.setTextColor(GREY_TEXT);
  tft.setFont();

  tft.setCursor(60, 6);
  tft.print("TIEMPO");

  tft.setCursor(10, 64);
  tft.print("FC");

  tft.setCursor(92, 64);
  tft.print("VEL");

  tft.setCursor(10, 103);
  tft.print("DIST");
}

// =====================================================
//                  CRONOMETRO
// =====================================================
void actualizarTiempo() {
  static unsigned long previo = 0;
  unsigned long actual = millis() - tiempoInicio;

  if (actual - previo >= 100) {
    previo = actual;

    int min = actual / 60000;
    int seg = (actual % 60000) / 1000;
    int cen = (actual % 1000) / 10;

    char buffer[12];
    sprintf(buffer, "%02d:%02d:%02d", min, seg, cen);

    tft.fillRect(5, 15, 150, 30, BG_COLOR);

    tft.setFont(&FreeSansBold12pt7b);
    tft.setTextColor(TIME_COLOR);

    int16_t x1, y1;
    uint16_t w, h;
    tft.getTextBounds(buffer, 0, 0, &x1, &y1, &w, &h);

    tft.setCursor((160 - w) / 2, 42);
    tft.print(buffer);
  }
}

// =====================================================
//                  VARIABLES
// =====================================================
void actualizarDatos() {
  static int bpmOld = -1;
  static float velOld = -1;
  static float distOld = -1;

  // ================= FC =================
  if (bpm != bpmOld) {
    tft.fillRect(5, 72, 70, 22, BG_COLOR);

    tft.setFont(&FreeSansBold9pt7b);
    tft.setTextColor(BPM_COLOR);
    tft.setCursor(8, 90);
    tft.print(bpm);

    tft.setFont();
    tft.setTextColor(GREY_TEXT);
    tft.setCursor(10, 64);
    tft.print("FC");
    tft.setCursor(48, 84);
    tft.print("BPM");

    bpmOld = bpm;
  }

  // ================= VELOCIDAD =================
  if (velocidad != velOld) {
    tft.fillRect(85, 72, 70, 22, BG_COLOR);

    tft.setFont(&FreeSansBold9pt7b);
    tft.setTextColor(VEL_COLOR);
    tft.setCursor(88, 90);
    tft.print(velocidad, 1);

    tft.setFont();
    tft.setTextColor(GREY_TEXT);
    tft.setCursor(92, 64);
    tft.print("VEL");
    tft.setCursor(125, 84);
    tft.print("KM/H");

    velOld = velocidad;
  }

  // ================= DISTANCIA =================
  if (distancia != distOld) {

    tft.fillRect(40, 110, 120, 18, BG_COLOR);

    tft.setFont();
    tft.setTextColor(GREY_TEXT);
    tft.setCursor(10, 103);
    tft.print("DISTANCIA");

    tft.setFont(&FreeSansBold9pt7b);
    tft.setTextColor(DIST_COLOR);
    tft.setCursor(65, 124);
    tft.print(distancia, 1);

    tft.setFont();
    tft.setTextColor(GREY_TEXT);
    tft.setCursor(105, 115);
    tft.print("METROS");

    distOld = distancia;
  }
}