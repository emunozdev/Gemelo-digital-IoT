// ============================================================================
//  Tareas.cpp
//
//  REGLA DE ORO: cada recurso de hardware tiene UN SOLO dueno (una tarea).
//    - Servo + ultrasonico ... tareaSensor
//    - Motores + buzzer ...... tareaControl
//    - Socket UDP ............ tareaRed
//    - OLED .................. tareaPantalla
//  Las tareas se comunican SOLO por buzones/colas; nadie toca el hardware ajeno.
//
//      tareaSensor --publica--> g_muestra (ultimo valor) --leen--> tareaControl, tareaPantalla
//                  \--envia---> g_colaRed (cola de 8)    --lee---> tareaRed
//      tareaControl --ordena--> g_ordenServo (angulo)    --lee---> tareaSensor
// ============================================================================
#include "Tareas.h"

#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"

#include "Config.h"
#include "Tipos.h"
#include "Buzon.h"
#include "Network.h"
#include "Sensor.h"
#include "sm.h"
#include "motor.h"
#include "Buzzer.h"
#include "Radar.h"

static const char* TAG = "TAREAS";

static Buzon<Muestra> g_muestra;                // ultima medicion (para control y pantalla)
static Buzon<uint8_t> g_ordenServo;             // angulo que pide el control
static QueueHandle_t  g_colaRed = nullptr;      // cada medicion, en orden, para la red

// ----------------------------------------------------------------------------
// Mediana de 3: elimina picos aislados del HC-SR04 sin suavizar de mas.
// ----------------------------------------------------------------------------
static uint16_t mediana3(uint16_t a, uint16_t b, uint16_t c) {
    uint16_t t;
    if (a > b) { t = a; a = b; b = t; }
    if (b > c) { t = b; b = c; c = t; }
    if (a > b) { t = a; a = b; b = t; }
    return b;
}

// ============================================================================
//  tareaSensor - dueno de servo + ultrasonico. Produce UNA muestra cada 60 ms.
// ============================================================================
static void tareaSensor(void*) {
    uint16_t seq = 0;
    uint8_t  angulo = CfgServo::ANGULO_CENTRO;
    uint16_t h[3] = {0, 0, 0};
    bool     reiniciarFiltro = true;
    TickType_t t = xTaskGetTickCount();

    for (;;) {
        // 1) Si el control pidio otro angulo: mover y ESPERAR (la tarea duerme, no se queda en delay())
        uint8_t objetivo;
        if (g_ordenServo.leer(objetivo) && objetivo != angulo) {
            vTaskDelay(pdMS_TO_TICKS(Servomotor::moverA(objetivo)));
            angulo = objetivo;
            reiniciarFiltro = true;       // el historial era de otro angulo
            t = xTaskGetTickCount();      // re-sincroniza el periodo (si no, vTaskDelayUntil dispararia lecturas en rafaga)
        }

        // 2) Medir
        const float cm = Sensor::medirDistancia();
        const uint16_t d = (cm > 0.0f) ? (uint16_t)(cm + 0.5f) : DIST_SIN_ECO;

        if (reiniciarFiltro) { h[0] = h[1] = h[2] = d; reiniciarFiltro = false; }
        else                 { h[0] = h[1]; h[1] = h[2]; h[2] = d; }

        // 3) Publicar
        Muestra m;
        m.seq    = seq++;
        m.ms     = millis();
        m.angulo = angulo;
        m.dist   = mediana3(h[0], h[1], h[2]);

        g_muestra.publicar(m);                // control y pantalla leen el ultimo valor
        xQueueSend(g_colaRed, &m, 0);         // red: si la cola esta llena se descarta; el hueco en seq lo delata

        vTaskDelayUntil(&t, pdMS_TO_TICKS(CfgPeriodo::SENSOR_MS));
    }
}

// ============================================================================
//  tareaControl - dueno de motores + buzzer. Maquina de estados SIN delay():
//  cada 20 ms revisa en que fase esta y si ya paso el tiempo de esa fase.
// ============================================================================
enum class Fase : uint8_t { AVANZAR, PARAR, RETROCEDER, GIRAR, MIRAR, REMATE };

static void tareaControl(void*) {
    Fase     fase = Fase::AVANZAR;
    uint32_t t0 = 0;                // inicio de la fase actual
    uint16_t durRemate = 0;
    TickType_t t = xTaskGetTickCount();

    for (;;) {
        Muestra m = {};
        const bool     hay   = g_muestra.leer(m);
        const uint32_t ahora = millis();
        const uint32_t dt    = ahora - t0;
        const bool fresca = hay && (ahora - m.ms) <= CfgControl::EDAD_MAX_MUESTRA_MS;
        const bool libre  = fresca && !hayObstaculo(m.dist);

        switch (fase) {
        case Fase::AVANZAR:
            if (!hay || m.angulo != CfgServo::ANGULO_CENTRO) break;   // espera una muestra tomada de frente
            if (!fresca) { Motor::detenerMotores(); break; }          // FAIL-SAFE: sin datos recientes, no avanza
            if (libre) {
                Motor::moverAdelante();
            } else {
                Motor::detenerMotores();
                Buzzer::pitar(CfgControl::BUZZER_MS);
                t0 = ahora; fase = Fase::PARAR;
            }
            break;

        case Fase::PARAR:
            if (dt >= CfgControl::T_PARADA_MS) { Motor::moverAtras(); t0 = ahora; fase = Fase::RETROCEDER; }
            break;

        case Fase::RETROCEDER:
            if (dt >= CfgControl::T_RETRO_MS)  { Motor::girarDerecha(); t0 = ahora; fase = Fase::GIRAR; }
            break;

        case Fase::GIRAR:
            if (dt >= CfgControl::T_GIRO_MS) {
                Motor::detenerMotores();
                g_ordenServo.publicar(CfgServo::ANGULO_LATERAL);      // pide mirar a 30 grados
                fase = Fase::MIRAR;
            }
            break;

        case Fase::MIRAR:
            if (!hay || m.angulo != CfgServo::ANGULO_LATERAL) break;  // espera una muestra YA tomada a 30 grados
            if (libre) { Motor::moverAdelante();  durRemate = CfgControl::T_ADELANTE_REMATE_MS; }
            else       { Motor::girarIzquierda(); durRemate = CfgControl::T_GIRO_IZQ_MS; }
            t0 = ahora; fase = Fase::REMATE;
            break;

        case Fase::REMATE:
            if (dt >= durRemate) {
                Motor::detenerMotores();
                g_ordenServo.publicar(CfgServo::ANGULO_CENTRO);       // vuelve a mirar de frente
                fase = Fase::AVANZAR;
            }
            break;
        }

        vTaskDelayUntil(&t, pdMS_TO_TICKS(CfgPeriodo::CONTROL_MS));
    }
}

// ============================================================================
//  tareaRed - dueno del socket UDP. DUERME hasta que llega una muestra:
//  una trama por muestra, sin polling. Su tope es la frecuencia del sensor.
// ============================================================================
static void tareaRed(void*) {
    uint32_t ultimoLog = millis();
    uint32_t descartesPrev = 0;

    for (;;) {
        Muestra m;
        if (xQueueReceive(g_colaRed, &m, pdMS_TO_TICKS(1000)) == pdTRUE) {
            Trama f;
            f.magic  = TRAMA_MAGIC;
            f.seq    = m.seq;
            f.ms     = m.ms;
            f.angulo = m.angulo;
            f.dist   = m.dist;
            f.estado = (uint8_t)Motor::estado();
            f.flags  = hayObstaculo(m.dist) ? FLAG_ALERTA : 0;
            Network::enviar(&f, sizeof(f));
        }

        // Un resumen cada 2 s (y solo si cambio), en vez de un log por cada fallo
        if (millis() - ultimoLog >= 2000) {
            ultimoLog = millis();
            const uint32_t d = Network::descartes();
            if (d != descartesPrev) {
                descartesPrev = d;
                ESP_LOGW(TAG, "UDP descartados: %lu (errores: %lu)",
                         (unsigned long)d, (unsigned long)Network::errores());
            }
        }
    }
}

// ============================================================================
//  tareaPantalla - dueno de la OLED. Baja prioridad, 5 fps: si tarda, no
//  afecta a nadie.
// ============================================================================
static void tareaPantalla(void*) {
    TickType_t t = xTaskGetTickCount();
    for (;;) {
        Muestra m;
        if (g_muestra.leer(m)) {
            Radar::dibujar(m, Motor::estado(), hayObstaculo(m.dist));
        }
        vTaskDelayUntil(&t, pdMS_TO_TICKS(CfgPeriodo::PANTALLA_MS));
    }
}

// ============================================================================
bool Tareas::iniciar() {
    if (!g_muestra.crear() || !g_ordenServo.crear()) return false;
    g_colaRed = xQueueCreate(CfgRtos::COLA_RED, sizeof(Muestra));
    if (!g_colaRed) return false;

    g_ordenServo.publicar(CfgServo::ANGULO_CENTRO);

    xTaskCreatePinnedToCore(tareaSensor,   "sensor",   CfgRtos::STACK, nullptr, CfgRtos::PRIO_SENSOR,   nullptr, CfgRtos::NUCLEO_APP);
    xTaskCreatePinnedToCore(tareaControl,  "control",  CfgRtos::STACK, nullptr, CfgRtos::PRIO_CONTROL,  nullptr, CfgRtos::NUCLEO_APP);
    xTaskCreatePinnedToCore(tareaRed,      "red",      CfgRtos::STACK, nullptr, CfgRtos::PRIO_RED,      nullptr, CfgRtos::NUCLEO_APP);
    xTaskCreatePinnedToCore(tareaPantalla, "pantalla", CfgRtos::STACK, nullptr, CfgRtos::PRIO_PANTALLA, nullptr, CfgRtos::NUCLEO_APP);
    return true;
}
