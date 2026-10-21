#pragma once
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Arduino.h>
#include "Sensor.h"
#include "Network.h"

#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64 
#define OLED_RESET    -1 
#define SCREEN_ADDRESS 0x3C 

class Radar {
private:
    static Adafruit_SSD1306 display;
    static const uint8_t distanciaUmbral;
    static unsigned long lastMotionTime;
    static const unsigned long motionTimeout;
    static bool alertaActiva;

    static Sensor sensor; 
public:
    static void inicializar();
    static void update();
    static void drawScanningRadar(const uint8_t distanciaActual);
};