#pragma once
#include <Arduino.h>

class Buzzer {
public:
    static void inicializar();
    static void pitar(uint16_t ms);
};
