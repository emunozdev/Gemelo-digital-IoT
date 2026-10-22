#pragma once
#include <ESP32Servo.h>

class Servomotor {
private:
    const int SERVO_GPIO;
    Servo servo;
public:
    //Servomotor(int gpio) : SERVO_GPIO(gpio) {}
    void inicializar();
    void moverAngulo(int angulo);
};