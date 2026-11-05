#include "motor.h"
#include "Config.h"

volatile Movimiento Motor::s_estado = Movimiento::PARADO;

void Motor::aplicar(bool i1, bool i2, bool d1, bool d2, Movimiento m) {
    digitalWrite(Pines::MOTOR_IZQ_1, i1);
    digitalWrite(Pines::MOTOR_IZQ_2, i2);
    digitalWrite(Pines::MOTOR_DER_1, d1);
    digitalWrite(Pines::MOTOR_DER_2, d2);
    s_estado = m;
}

void Motor::inicializar() {
    pinMode(Pines::MOTOR_IZQ_1, OUTPUT);
    pinMode(Pines::MOTOR_IZQ_2, OUTPUT);
    pinMode(Pines::MOTOR_DER_1, OUTPUT);
    pinMode(Pines::MOTOR_DER_2, OUTPUT);
    detenerMotores();
}

//                             IZQ1   IZQ2   DER1   DER2
void Motor::moverAdelante()   { aplicar(true,  false, true,  false, Movimiento::ADELANTE);  }
void Motor::moverAtras()      { aplicar(false, true,  false, true,  Movimiento::ATRAS);     }
void Motor::girarDerecha()    { aplicar(true,  false, false, true,  Movimiento::DERECHA);   }
void Motor::girarIzquierda()  { aplicar(false, true,  true,  false, Movimiento::IZQUIERDA); }
void Motor::detenerMotores()  { aplicar(false, false, false, false, Movimiento::PARADO);    }

Movimiento Motor::estado() { return s_estado; }
