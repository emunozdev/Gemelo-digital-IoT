#include "Ultrasonico.h"
#include "esp_timer.h"
#include "driver/gpio.h"


static constexpr uint32_t ECHO_WAIT_MS = 45;

Ultrasonico::Ultrasonico(uint8_t trigPin, uint8_t echoPin, uint16_t minMM, uint16_t maxMM)
    : _trig(trigPin), _echo(echoPin), _minMM(minMM), _maxMM(maxMM) {}

bool Ultrasonico::begin() {
  pinMode(_trig, OUTPUT);
  digitalWrite(_trig, LOW);
  pinMode(_echo, INPUT);

  _done = xSemaphoreCreateBinary();
  if (!_done) return false;

  attachInterruptArg(digitalPinToInterrupt(_echo), isrEcho, this, CHANGE);
  return true;
}

// ISR: debe estar en IRAM y usar solo funciones seguras en ISR
void IRAM_ATTR Ultrasonico::isrEcho(void *arg) {
  static_cast<Ultrasonico *>(arg)->onEcho();
}

void IRAM_ATTR Ultrasonico::onEcho() {
  uint32_t now = (uint32_t)esp_timer_get_time();
  if (gpio_get_level((gpio_num_t)_echo)) {
    _tRiseUs = now;                       // flanco de subida
  } else if (_esperando) {
    _tFallUs = now;                       // flanco de bajada
    _esperando = false;
    BaseType_t hpw = pdFALSE;
    xSemaphoreGiveFromISR(_done, &hpw);
    if (hpw) portYIELD_FROM_ISR();
  }
}

uint16_t Ultrasonico::medirUnaVezMM() {
  xSemaphoreTake(_done, 0);               // limpiar semáforo residual
  _tRiseUs   = 0;
  _esperando = true;

  digitalWrite(_trig, LOW);
  delayMicroseconds(3);
  digitalWrite(_trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(_trig, LOW);

  bool ok = (xSemaphoreTake(_done, pdMS_TO_TICKS(ECHO_WAIT_MS)) == pdTRUE);
  _esperando = false;
  if (!ok || _tRiseUs == 0) return 0;

  uint32_t anchoUs = _tFallUs - _tRiseUs;
  uint32_t mm = (uint32_t)(anchoUs * 0.1715f);   // 343 m/s / 2 -> mm/µs
  if (mm < _minMM || mm > _maxMM) return 0;
  return (uint16_t)mm;
}

uint16_t Ultrasonico::medirMM(uint8_t muestras) {
  if (muestras < 1) muestras = 1;
  if (muestras > 5) muestras = 5;

  uint16_t v[5];
  for (uint8_t i = 0; i < muestras; i++) {
    v[i] = medirUnaVezMM();
    if (i + 1 < muestras) vTaskDelay(pdMS_TO_TICKS(10));
  }
  // insertion sort + mediana
  for (uint8_t i = 1; i < muestras; i++) {
    uint16_t k = v[i];
    int8_t j = i - 1;
    while (j >= 0 && v[j] > k) { v[j + 1] = v[j]; j--; }
    v[j + 1] = k;
  }
  return v[muestras / 2];
}
