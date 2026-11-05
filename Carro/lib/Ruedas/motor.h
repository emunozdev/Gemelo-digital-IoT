#pragma once
// ============================================================================
//  Motor - Driver del L293D (solo direccion, sin PWM). No bloquea nunca.
//  Recuerda su estado para poder reportarlo (gemelo digital).
// ============================================================================
#include <Arduino.h>
#include "Tipos.h"

class Motor {
public:
    static void inicializar();
    static void moverAdelante();
    static void moverAtras();
    static void girarDerecha();
    static void girarIzquierda();
    static void detenerMotores();

    static Movimiento estado();   // un solo byte: lectura atomica desde cualquier tarea
private:
    static void aplicar(bool i1, bool i2, bool d1, bool d2, Movimiento m);
    static volatile Movimiento s_estado;
};
