#include "sm.h"

const int SERVO_GPIO = 12;
Servo Servomotor::servo; 

void Servomotor::inicializar() {
    servo.setPeriodHertz(50); 
    servo.attach(SERVO_GPIO, 500, 2400);
}

void Servomotor::moverAngulo(int angulo) {
    servo.write(angulo);
    delay(500);
}