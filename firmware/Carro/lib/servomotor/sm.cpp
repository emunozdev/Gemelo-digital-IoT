#include <sm.h>

const int SERVO_GPIO = 12;

void Servomotor::inicializar() {
    ServoMotor::servo.setPeriodHertz(50); 
    ServoMotor::servo.attach(SERVO_GPIO, 500, 2400);
}

void Servomotor::moverAngulo(int angulo) {
    ServoMotor::servo.write(angulo);
    delay(500);
}