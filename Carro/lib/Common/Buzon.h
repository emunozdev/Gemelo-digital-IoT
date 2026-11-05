#pragma once

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
