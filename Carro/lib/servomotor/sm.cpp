#include "sm.h"
#include "Config.h"

Servo   Servomotor::s_servo;
uint8_t Servomotor::s_actual = CfgServo::ANGULO_CENTRO;

void Servomotor::inicializar() {
    s_servo.setPeriodHertz(50);
    s_servo.attach(Pines::SERVO, 500, 2400);
    s_servo.write(s_actual);
}

uint16_t Servomotor::moverA(uint8_t angulo) {
    if (angulo > 180) angulo = 180;

    const uint8_t delta = (angulo > s_actual) ? (angulo - s_actual) : (s_actual - angulo);
    s_servo.write(angulo);
    s_actual = angulo;

    return (uint16_t)(CfgServo::MS_MARGEN + CfgServo::MS_POR_GRADO * delta);
}

uint8_t Servomotor::angulo() { return s_actual; }
