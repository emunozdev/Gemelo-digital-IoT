#pragma once
#include <Arduino.h>

class Sensor {
    private:
        static const int triggerPin;
        static const int echoPin;
        static const int maxDistance;
    public:
        static const int buzzerPin;
        
        static void inicializar();
        static float medirDistancia();
        static bool detectarObstaculo();
        static void activarAlarma();
        static void envioDato(float distancia);
};