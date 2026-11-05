#pragma once

#include <stdint.h>
#include "Config.h"

constexpr uint16_t DIST_SIN_ECO = 0xFFFF;  

enum class Movimiento : uint8_t { PARADO = 0, ADELANTE = 1, ATRAS = 2, DERECHA = 3, IZQUIERDA = 4 };

struct Muestra {
    uint16_t seq;      // contador; los huecos delatan muestras perdidas
    uint32_t ms;       // millis() del ESP32 al medir
    uint8_t  angulo;   // angulo del servo al medir [0 - 180]
    uint16_t dist;     // cm (mediana de 3); DIST_SIN_ECO si no hay eco
};

constexpr uint8_t TRAMA_MAGIC = 0xA5;
constexpr uint8_t FLAG_ALERTA = 0x01;

#pragma pack(push, 1)
struct Trama {
    uint8_t  magic;    // 0xA5
    uint16_t seq;
    uint32_t ms;
    uint8_t  angulo;
    uint16_t dist;     // 0xFFFF 
    uint8_t  estado;   // Movimiento: 0 parado, 1 adelante, 2 atras, 3 derecha, 4 izquierda
    uint8_t  flags;    // bit0 = alerta (obstaculo <= umbral)
};
#pragma pack(pop)
static_assert(sizeof(Trama) == 12, "La trama debe medir 12 bytes");

inline bool hayObstaculo(uint16_t dist) {
    return dist != DIST_SIN_ECO && dist <= CfgControl::UMBRAL_OBSTACULO_CM;
}
