#include "Sensor.h"
#include "Config.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "driver/gpio.h"

static volatile uint32_t     s_tSubida = 0;
static volatile uint32_t     s_tBajada = 0;
static TaskHandle_t volatile s_tarea   = nullptr;  

// ISR: se ejecuta en cada flanco del pin ECHO (subida y bajada).
static void IRAM_ATTR isrEcho() {
    const uint32_t ahora = (uint32_t)esp_timer_get_time();   

    if (gpio_get_level((gpio_num_t)Pines::ECHO)) {
        s_tSubida = ahora;                                  
    } else {
        s_tBajada = ahora;                                   
        BaseType_t despertar = pdFALSE;
        TaskHandle_t t = s_tarea;
        if (t) vTaskNotifyGiveFromISR(t, &despertar);
        if (despertar) portYIELD_FROM_ISR();
    }
}

void Sensor::inicializar() {
    pinMode(Pines::TRIG, OUTPUT);
    digitalWrite(Pines::TRIG, LOW);
    pinMode(Pines::ECHO, INPUT);
    attachInterrupt(digitalPinToInterrupt(Pines::ECHO), isrEcho, CHANGE);
}

float Sensor::medirDistancia() {
    s_tarea = xTaskGetCurrentTaskHandle();
    ulTaskNotifyTake(pdTRUE, 0);     
    s_tSubida = 0;

    digitalWrite(Pines::TRIG, LOW);  delayMicroseconds(2);
    digitalWrite(Pines::TRIG, HIGH); delayMicroseconds(10);
    digitalWrite(Pines::TRIG, LOW);

    if (ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(CfgSensor::TIMEOUT_ECO_MS)) == 0) return -1.0f;
    if (s_tSubida == 0) return -1.0f;                       

    const uint32_t us = s_tBajada - s_tSubida;
    float cm = us / 58.0f;

    if (cm > CfgSensor::DIST_MAX_CM) return -1.0f;
    if (cm < CfgSensor::DIST_MIN_CM) cm = CfgSensor::DIST_MIN_CM;   
    return cm;
}
