#include <Arduino.h>
#include <Wire.h>
#include "Radar.h"
#include "motor.h"
#include "Sensor.h"
#include "Network.h"
#include "sm.h" 

#define TIME 200

const float UMBRAL_OBSTACULO = 20.0f;

unsigned long ultimoEnvio = 0;
const unsigned long INTERVALO_ENVIO_MS = 100; 

void setup() {
   Network::wifi_init_softap();
   Network::init_udp_socket();

   Radar::inicializar();
   Motor::inicializar();
   Servomotor::inicializar();

   Servomotor::moverAngulo(90);
}

void loop() {
    Radar::update();

    unsigned long ahora = millis();
    if (ahora - ultimoEnvio >= INTERVALO_ENVIO_MS) {

        ultimoEnvio = ahora;
        float distanciaParaRed = Sensor::medirDistancia();

        if (distanciaParaRed > 0 && distanciaParaRed < 400) {

            bool alertaActiva = (distanciaParaRed <= UMBRAL_OBSTACULO);
            Network::enviar_telemetria_udp((uint16_t)distanciaParaRed, alertaActiva);
        }
    }


    Servomotor::moverAngulo(90);
    float distanciaFrente = Sensor::medirDistancia();

    if (distanciaFrente > UMBRAL_OBSTACULO || distanciaFrente < 0) {
        Motor::moverAdelante();
    } else {
        
        Motor::detenerMotores();
        delay(TIME);

        Motor::moverAtras();
        delay(TIME * 2);

        Motor::girarDerecha();
        delay(TIME + 200);
        Motor::detenerMotores();


        Servomotor::moverAngulo(30);
        float distanciaValidacion = Sensor::medirDistancia();

        if (distanciaValidacion > UMBRAL_OBSTACULO) {
            Motor::moverAdelante();
            delay(300);
        } else {
            Motor::girarIzquierda();
            delay(400);
        }

        Servomotor::moverAngulo(90);
        Motor::detenerMotores();
    }

    delay(50);
}