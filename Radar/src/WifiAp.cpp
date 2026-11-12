#include "WifiAp.h"

WifiAp::WifiAp(const char *ssid, const char *pass, uint8_t canal, uint8_t maxClientes,
               const IPAddress &ip, const IPAddress &mascara)
    : _ssid(ssid), _pass(pass), _canal(canal), _maxClientes(maxClientes),
      _ip(ip), _mascara(mascara) {}

bool WifiAp::begin() {
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(_ip, _ip, _mascara);   
  return WiFi.softAP(_ssid, _pass, _canal, 0, _maxClientes);
}

uint8_t WifiAp::clientes() const { return WiFi.softAPgetStationNum(); }

IPAddress WifiAp::ip() const { return WiFi.softAPIP(); }
