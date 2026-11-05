#pragma once
// ============================================================================
//  Servomotor - Driver del servo que orienta el ultrasonico.
//  NO bloquea: mueve el servo y le dice al llamador cuanto debe esperar.
// ============================================================================
#include <Arduino.h>
#include <ESP32Servo.h>

class Servomotor {
public:
    static void     inicializar();
    static uint16_t moverA(uint8_t angulo);   // devuelve ms estimados hasta que el servo se asienta
    static uint8_t  angulo();                 // ultimo angulo comandado
private:
    static Servo   s_servo;
    static uint8_t s_actual;
};
