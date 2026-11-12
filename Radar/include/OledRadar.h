#pragma once
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

struct RadarParams {
  uint16_t maxMM;          
  uint8_t  anguloMax;      
  uint8_t  pasoGrados;     
  uint32_t ttlMs;         
  bool     invertirX;      
  bool     lineaBarrido;   
  bool     arcoReferencia; 
};

class OledRadar {
 public:
  OledRadar(uint8_t sda, uint8_t scl, uint8_t addr, const RadarParams &params, uint8_t ancho = 128, uint8_t alto = 64);

  bool begin();
  void actualizarObjeto(uint8_t angulo, uint16_t distMM); 
  void setBarrido(uint8_t angulo) { _anguloActual = angulo; }
  void render();

 private:
  void polarAXY(uint8_t angulo, float distMM, int &x, int &y) const;

  static constexpr uint8_t MAX_SLOTS = 64;
  struct Blip { uint16_t distMM; uint32_t tMs; };

  Adafruit_SSD1306 _disp;
  uint8_t     _sda, _scl, _addr;
  RadarParams _p;
  uint8_t     _slots;
  int         _cx, _cy, _radio;
  uint8_t     _anguloActual = 0;
  Blip        _blips[MAX_SLOTS];
};
