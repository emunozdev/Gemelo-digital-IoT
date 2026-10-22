#include <Arduino.h>
#include <Wire.h>
#include "Radar.h"
#include "motor.h"
#include "Sensor.h"
#include "Network.h"

#define TIME 200

const float UMBRAL_OBSTACULO = 20.0f;

void setup() {
   Network::wifi_init_softap();
   Network::init_udp_socket();

   Radar::inicializar();
   Motor::inicializar();
}

void loop() {
    Radar::update();

    float distanciaActual = Sensor::medirDistancia();

    if(distanciaActual > 0 && distanciaActual < UMBRAL_OBSTACULO) {
        Motor::detenerMotores();
        delay(TIME);

        Motor::moverAtras();
        delay(TIME * 2);

        Motor::girarDerecha();
        delay(TIME + 200);

        Motor::detenerMotores();
    } else {
        Motor::moverAdelante();
    }
    delay(50);
}