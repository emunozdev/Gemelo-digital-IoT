#pragma once
// ============================================================================
//  Buzon<T> - "Ultimo valor" compartido entre tareas (cola de largo 1).
//   - publicar(): sobreescribe, nunca bloquea (el productor no espera a nadie).
//   - leer():     lo copia SIN quitarlo, asi varias tareas leen el mismo dato.
//  Es thread-safe: FreeRTOS protege la copia. No hacen falta mutex ni "volatile".
// ============================================================================
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

template <typename T>
class Buzon {
public:
    bool crear() {
        q_ = xQueueCreate(1, sizeof(T));
        return q_ != nullptr;
    }
    void publicar(const T& v) { xQueueOverwrite(q_, &v); }
    bool leer(T& v) const     { return xQueuePeek(q_, &v, 0) == pdTRUE; }   // false si aun no hay dato
private:
    QueueHandle_t q_ = nullptr;
};
