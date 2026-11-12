#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>

class UdpSender {
 public:
  UdpSender(const IPAddress &destino, uint16_t puerto);

  bool begin(void);
  bool enviar(const char *datos, size_t len);

 private:
  WiFiUDP   _udp;
  IPAddress _destino;
  uint16_t  _puerto;
};
