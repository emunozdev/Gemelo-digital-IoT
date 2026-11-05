#pragma once
// ============================================================================
//  Buzzer - Driver del buzzer. pitar() NO bloquea: un timer de software de
//  FreeRTOS lo apaga cuando vence el tiempo (sin delay, sin polling).
// ============================================================================
#include <Arduino.h>

class Buzzer {
public:
    static void inicializar();
    static void pitar(uint16_t ms);
};
