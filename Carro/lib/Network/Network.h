#pragma once
#include <Arduino.h>

class Network {
public:
    static bool iniciarAP();                                  // crea el punto de acceso (Config.h -> Red::)
    static bool iniciarUDP();                                 // abre el socket hacia IP_DESTINO:PUERTO
    static bool enviar(const void* datos, size_t len);        // true si el paquete entro al stack WiFi

    static uint32_t descartes();   // descartados por buffers llenos 
    static uint32_t errores();     
};
