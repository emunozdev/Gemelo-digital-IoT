#pragma once
#include <ESP32Servo.h>

class Servomotor {
private:
    static Servo servo;
public:
    static void inicializar();
    static void moverAngulo(int angulo);
};