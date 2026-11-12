#pragma once
#include <Arduino.h>

class Ultrasonico {
 public:
  Ultrasonico(uint8_t trigPin, uint8_t echoPin,
              uint16_t minMM = 20, uint16_t maxMM = 2000);

  bool     begin();
  uint16_t medirMM(uint8_t muestras = 1);   

 private:
  uint16_t medirUnaVezMM();
  static void isrEcho(void *arg);
  void        onEcho();

  uint8_t  _trig, _echo;
  uint16_t _minMM, _maxMM;

  volatile uint32_t _tRiseUs = 0;
  volatile uint32_t _tFallUs = 0;
  volatile bool     _esperando = false;
  SemaphoreHandle_t _done = nullptr;
};
