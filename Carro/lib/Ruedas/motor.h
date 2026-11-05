#pragma once
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

    static Movimiento estado();   
private:
    static void aplicar(bool i1, bool i2, bool d1, bool d2, Movimiento m);
    static volatile Movimiento s_estado;
};
