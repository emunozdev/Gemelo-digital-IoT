#pragma once
#include <Arduino.h>
#include <ESP32Servo.h>

class Servomotor {
public:
    static void     inicializar();
    static uint16_t moverA(uint8_t angulo);   
    static uint8_t  angulo();                 
private:
    static Servo   s_servo;
    static uint8_t s_actual;
};
