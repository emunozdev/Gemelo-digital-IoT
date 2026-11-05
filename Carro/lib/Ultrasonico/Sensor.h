#pragma once
// ============================================================================
//  Sensor - Driver del HC-SR04.
//  El eco se cronometra con una interrupcion y la tarea que mide DUERME
//  mientras espera (no hace busy-wait como pulseIn). Es de un solo dueno:
//  solo UNA tarea debe llamar a medirDistancia().
// ============================================================================
#include <Arduino.h>

class Sensor {
public:
    static void  inicializar();
    // Devuelve cm, o -1 si no hubo eco en rango. Distancias < DIST_MIN_CM se devuelven como DIST_MIN_CM.
    static float medirDistancia();
};
