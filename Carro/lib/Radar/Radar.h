#pragma once
// ============================================================================
//  Radar - Driver de la pantalla OLED SSD1306 (vista del radar).
//  Solo dibuja lo que se le da. Si la pantalla no responde, el sistema sigue.
// ============================================================================
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "Tipos.h"

class Radar {
public:
    static bool inicializar();   // false si no hay pantalla (no bloquea el arranque)
    static void dibujar(const Muestra& m, Movimiento mov, bool alerta);
private:
    static Adafruit_SSD1306 s_display;
    static bool             s_listo;
};
