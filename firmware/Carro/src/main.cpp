#include <Arduino.h>
#include <Wire.h>
#include "Radar.h"
#include "Network.h"

void setup() {
   Network::wifi_init_softap();
   Network::init_udp_socket();
   Radar::inicializar();
}

void loop() {
    Radar::update();
    delay(10);
}