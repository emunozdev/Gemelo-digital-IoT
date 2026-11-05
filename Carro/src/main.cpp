// ============================================================================
//  main.cpp - Solo arranque: inicializa los drivers y lanza las tareas.
//  Nada de logica aqui. La logica vive en Tareas.cpp; el hardware, en lib/.
// ============================================================================
#include <Arduino.h>
#include "esp_log.h"

#include "Network.h"
#include "Sensor.h"
#include "sm.h"
#include "motor.h"
#include "Buzzer.h"
#include "Radar.h"
#include "Tareas.h"

void setup() {
    Serial.begin(115200);

    Motor::inicializar();          // primero: motores en reposo lo antes posible
    Buzzer::inicializar();
    Servomotor::inicializar();
    Sensor::inicializar();

    if (!Radar::inicializar()) {
        ESP_LOGW("MAIN", "OLED no responde; el carro funciona igual, sin pantalla");
    }

    Network::iniciarAP();
    Network::iniciarUDP();

    if (!Tareas::iniciar()) {
        ESP_LOGE("MAIN", "No se pudieron crear las tareas; reiniciando");
        delay(1000);
        ESP.restart();
    }
}

void loop() {
    vTaskDelete(nullptr);   // el loop de Arduino ya no se usa: el trabajo lo hacen las tareas
}
