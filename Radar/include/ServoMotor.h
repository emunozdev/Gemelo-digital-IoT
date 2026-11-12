#pragma once
#include <Arduino.h>
#include <ESP32Servo.h>

class ServoMotor {
 public:
  ServoMotor(uint8_t pin, uint8_t minDeg = 0, uint8_t maxDeg = 180, uint16_t minUs = 500, uint16_t maxUs = 2400);

  bool    begin();
  void    mover(uint8_t grados);          
  uint8_t angulo() const { return _angulo; }

 private:
  Servo    _servo;
  uint8_t  _pin, _minDeg, _maxDeg;
  uint16_t _minUs, _maxUs;
  uint8_t  _angulo = 0;
};
