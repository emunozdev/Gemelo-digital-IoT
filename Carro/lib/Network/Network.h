#pragma once
// ============================================================================
//  Network - Driver de red: Access Point WiFi + socket UDP.
//  No conoce el formato de los datos: solo envia bytes. Nunca bloquea.
// ============================================================================
#include <Arduino.h>

class Network {
public:
    static bool iniciarAP();                                  // crea el punto de acceso (Config.h -> Red::)
    static bool iniciarUDP();                                 // abre el socket hacia IP_DESTINO:PUERTO
    static bool enviar(const void* datos, size_t len);        // true si el paquete entro al stack WiFi

    static uint32_t descartes();   // paquetes descartados por buffers llenos (saturacion real)
    static uint32_t errores();     // otros errores de sendto()
};
