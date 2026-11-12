#pragma once

#include <Arduino.h>
#include <WiFi.h>

class WifiAp {
 public:
  WifiAp(const char *ssid, const char *pass, uint8_t canal, uint8_t maxClientes,
         const IPAddress &ip, const IPAddress &mascara);

  bool       begin();
  uint8_t    clientes() const;     
  IPAddress  ip() const;

 private:
  const char *_ssid;
  const char *_pass;
  uint8_t     _canal, _maxClientes;
  IPAddress   _ip, _mascara;
};
