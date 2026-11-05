#include "Radar.h"

#define OLED_ANCHO   128
#define OLED_ALTO    64
#define OLED_RESET   -1
#define OLED_ADDR    0x3C

Adafruit_SSD1306 Radar::s_display(OLED_ANCHO, OLED_ALTO, &Wire, OLED_RESET);
bool             Radar::s_listo = false;

static const char* const NOMBRES[] = { "Parado", "Adelante", "Atras", "Derecha", "Izquierda" };

bool Radar::inicializar() {
    s_listo = s_display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
    if (!s_listo) return false;

    s_display.clearDisplay();
    s_display.setTextSize(1);
    s_display.setTextColor(SSD1306_WHITE);
    s_display.setCursor(0, 0);
    s_display.println("Iniciando...");
    s_display.display();
    return true;
}

void Radar::dibujar(const Muestra& m, Movimiento mov, bool alerta) {
    if (!s_listo) return;
    Adafruit_SSD1306& d = s_display;

    d.clearDisplay();
    d.setTextSize(1);
    d.setTextColor(SSD1306_WHITE);

    // Texto (columna izquierda)
    d.setCursor(0, 0);  d.print(alerta ? "ALERTA!" : "Escaneo");
    d.setCursor(0, 14);
    if (m.dist == DIST_SIN_ECO) { d.print("D: --"); }
    else                        { d.print("D:"); d.print((int)m.dist); d.print("cm"); }
    d.setCursor(0, 26); d.print("A:"); d.print((int)m.angulo);
    d.setCursor(0, 38); d.print(NOMBRES[(uint8_t)mov]);
    d.setCursor(0, 54); d.print("#"); d.print((int)m.seq);

    // Radar semicircular (derecha): 0 grados = izquierda, 90 = arriba, 180 = derecha
    const int cx = 96, cy = 58, r = 30;
    for (int a = 0; a <= 180; a += 6) {
        const float th = radians(a);
        d.drawPixel(cx + (int)(r * cosf(th)), cy - (int)(r * sinf(th)), SSD1306_WHITE);
    }
    d.drawLine(cx - r, cy, cx + r, cy, SSD1306_WHITE);

    const float th = radians(180 - (int)m.angulo);
    d.drawLine(cx, cy, cx + (int)(r * cosf(th)), cy - (int)(r * sinf(th)), SSD1306_WHITE);

    if (m.dist != DIST_SIN_ECO) {                       // punto en la distancia detectada (escala 0..100 cm)
        const int dd = (m.dist > 100) ? 100 : (int)m.dist;
        const float rr = r * dd / 100.0f;
        d.fillCircle(cx + (int)(rr * cosf(th)), cy - (int)(rr * sinf(th)), 2, SSD1306_WHITE);
    }

    d.display();
}
