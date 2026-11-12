#include "ServoMotor.h"

ServoMotor::ServoMotor(uint8_t pin, uint8_t minDeg, uint8_t maxDeg, uint16_t minUs, uint16_t maxUs): _pin(pin), _minDeg(minDeg), _maxDeg(maxDeg), _minUs(minUs), _maxUs(maxUs) {}

bool ServoMotor::begin() {
  ESP32PWM::allocateTimer(0);
  _servo.setPeriodHertz(50);
  _servo.attach(_pin, _minUs, _maxUs);
  return _servo.attached();
}

void ServoMotor::mover(uint8_t grados) {
  if (grados < _minDeg) grados = _minDeg;
  if (grados > _maxDeg) grados = _maxDeg;
  _servo.write(grados);
  _angulo = grados;
}
