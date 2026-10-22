#pragma once 
#include <Arduino.h>

class Motor {
private:
    static const int MOTOR_IZQ_DIR1;
    static const int MOTOR_IZQ_DIR2; 
    static const int MOTOR_DER_DIR1; 
    static const int MOTOR_DER_DIR2; 
public:
    static void inicializar();
    static void moverAdelante();
    static void moverAtras();
    static void girarDerecha();
    static void detenerMotores();
};