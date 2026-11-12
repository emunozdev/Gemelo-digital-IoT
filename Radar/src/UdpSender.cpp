#include "UdpSender.h"

UdpSender::UdpSender(const IPAddress &destino, uint16_t puerto)
    : _destino(destino), _puerto(puerto) {}

bool UdpSender::begin(void) { return _udp.begin(_puerto); }

bool UdpSender::enviar(const char *datos, size_t len) {
  if (!_udp.beginPacket(_destino, _puerto)) return false;
  _udp.write(reinterpret_cast<const uint8_t *>(datos), len);
  return _udp.endPacket() == 1;
}
