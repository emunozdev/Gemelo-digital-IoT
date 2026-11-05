#include "Buzzer.h"
#include "Config.h"

#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"

static TimerHandle_t s_timer = nullptr;

static void apagar(TimerHandle_t) {
    digitalWrite(Pines::BUZZER, LOW);
}

void Buzzer::inicializar() {
    pinMode(Pines::BUZZER, OUTPUT);
    digitalWrite(Pines::BUZZER, LOW);
    s_timer = xTimerCreate("buzzer", pdMS_TO_TICKS(100), pdFALSE /* one-shot */, nullptr, apagar);
}

void Buzzer::pitar(uint16_t ms) {
    if (!s_timer) return;
    digitalWrite(Pines::BUZZER, HIGH);
    xTimerChangePeriod(s_timer, pdMS_TO_TICKS(ms), 0);   
}
