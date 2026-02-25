#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>

// Pines
#define TFT_CS     5
#define TFT_RST    4
#define TFT_DC     2

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

// Función para dibujar persona corriendo
void drawRunner(int x, int y, uint16_t color) {
  // Cabeza
  tft.fillCircle(x, y, 5, color);

  // Cuerpo
  tft.drawLine(x, y+5, x, y+18, color);

  // Brazos
  tft.drawLine(x, y+8, x-8, y+14, color);
  tft.drawLine(x, y+8, x+8, y+4, color);

  // Piernas
  tft.drawLine(x, y+18, x-8, y+30, color);
  tft.drawLine(x, y+18, x+10, y+24, color);
}

void setup() {
  Serial.begin(115200);

  tft.initR(INITR_BLACKTAB);
  tft.setRotation(1);

  // Test colores
  tft.fillScreen(ST77XX_RED); delay(500);
  tft.fillScreen(ST77XX_GREEN); delay(500);
  tft.fillScreen(ST77XX_BLUE); delay(500);

  tft.fillScreen(ST77XX_BLACK);

  // Marco
  tft.drawRect(2, 2, 156, 124, ST77XX_YELLOW);

  // Texto principal
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(2);
  tft.setCursor(20, 15);
  tft.println("TREXIMO");

  tft.setTextSize(1);
  tft.setCursor(35, 45);
  tft.println("CDIO III");

  // Dibujar corredor
  drawRunner(80, 70, ST77XX_RED);
}

void loop() {
  static int pos = 0;

  // Borrar zona animación
  tft.fillRect(0, 90, 160, 30, ST77XX_BLACK);

  // Dibujar corredor moviéndose
  drawRunner(pos, 100, ST77XX_GREEN);

  pos += 5;
  if (pos > 160) pos = 0;

  delay(120);
}