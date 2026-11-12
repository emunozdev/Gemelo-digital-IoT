#include "OledRadar.h"
#include <math.h>

OledRadar::OledRadar(uint8_t sda, uint8_t scl, uint8_t addr, const RadarParams &params, uint8_t ancho, uint8_t alto): _disp(ancho, alto, &Wire, -1), _sda(sda), _scl(scl), _addr(addr), _p(params) {
  _slots = _p.anguloMax / _p.pasoGrados + 1;
  if (_slots > MAX_SLOTS) _slots = MAX_SLOTS;
  _cx    = ancho / 2;
  _cy    = alto - 1;
  _radio = alto - 4;
  memset(_blips, 0, sizeof(_blips));
}

bool OledRadar::begin() {
  Wire.begin(_sda, _scl);
  Wire.setClock(400000);
  if (!_disp.begin(SSD1306_SWITCHCAPVCC, _addr)) return false;
  _disp.clearDisplay();
  _disp.display();
  return true;
}

void OledRadar::actualizarObjeto(uint8_t angulo, uint16_t distMM) {
  uint8_t slot = angulo / _p.pasoGrados;
  if (slot >= _slots) return;
  _blips[slot].distMM = distMM;
  _blips[slot].tMs    = millis();
}

void OledRadar::polarAXY(uint8_t angulo, float distMM, int &x, int &y) const {
  float th = ((float)angulo - _p.anguloMax / 2.0f) * DEG_TO_RAD;  // centro del barrido = vertical
  if (_p.invertirX) th = -th;
  float r = (distMM / (float)_p.maxMM) * _radio;
  x = _cx + (int)lroundf(r * sinf(th));
  y = _cy - (int)lroundf(r * cosf(th));
}

void OledRadar::render() {
  _disp.clearDisplay();

  _disp.fillRect(_cx - 1, _cy - 1, 3, 2, SSD1306_WHITE);

  if (_p.arcoReferencia) {
    for (uint8_t a = 0; a <= _p.anguloMax; a += 4) {
      int x, y;
      polarAXY(a, _p.maxMM, x, y);
      _disp.drawPixel(x, y, SSD1306_WHITE);
    }
  }

  if (_p.lineaBarrido) {
    int x, y;
    polarAXY(_anguloActual, _p.maxMM, x, y);
    _disp.drawLine(_cx, _cy, x, y, SSD1306_WHITE);
  }

  uint32_t ahora = millis();
  for (uint8_t i = 0; i < _slots; i++) {
    if (_blips[i].distMM == 0) continue;
    if (ahora - _blips[i].tMs > _p.ttlMs) continue;
    int x, y;
    polarAXY(i * _p.pasoGrados, _blips[i].distMM, x, y);
    _disp.fillCircle(x, y, 1, SSD1306_WHITE);
  }

  _disp.display();
}
