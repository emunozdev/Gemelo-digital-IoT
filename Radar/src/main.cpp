/*
 * ==========================================================
 *  RADAR ESP32 - Arquitectura de drivers + FreeRTOS
 * ==========================================================
 *  Drivers (cada módulo de hardware = una clase con .h/.cpp):
 *    Ultrasonico  -> HC-SR04 por interrupciones
 *    ServoMotor   -> servo con límites 0..128°
 *    OledRadar    -> pantalla SSD1306 en modo radar
 *    WifiAp       -> Access Point propio con contraseña
 *    UdpSender    -> envío de paquetes a Python
 *    RingBuffer   -> cola circular thread-safe (utilidad)
 *
 *  Tareas FreeRTOS (main.cpp es el único que las conoce):
 *
 *   [taskSensor]  core 1, prio 3   servo + ultrasónico
 *        |--- push ---> RingBuffer ---> [taskComm]    core 0, prio 2  (cada 5 s -> UDP)
 *        '--- queue ---------------->  [taskDisplay]  core 1, prio 1  (OLED radar)
 *
 *  Cada driver lo usa UNA sola tarea, por eso no necesitan mutex propio.
 *  Lo único compartido entre tareas es el RingBuffer y la cola del display.
 *
 *  Protocolo UDP (texto, una línea por datagrama):
 *    RADAR|<seq>|<n>|<ang>,<dist_mm>,<t_ms>;<ang>,<dist_mm>,<t_ms>;...
 *    dist_mm = 0 -> sin objeto en ese ángulo
 * ==========================================================
 */
#include <Arduino.h>

#include "Config.h"
#include "Types.h"
#include "RingBuffer.h"
#include "Ultrasonico.h"
#include "ServoMotor.h"
#include "OledRadar.h"
#include "WifiAp.h"
#include "UdpSender.h"


static const RadarParams radarParams = {
  cfg::MAX_MM, cfg::SERVO_MAX_DEG, cfg::SERVO_STEP_DEG,
  cfg::BLIP_TTL_MS, cfg::INVERT_X, cfg::SHOW_SWEEP_LINE, cfg::SHOW_RANGE_ARC
};

static Ultrasonico ultra(cfg::PIN_TRIG, cfg::PIN_ECHO, cfg::MIN_MM, cfg::MAX_MM);
static ServoMotor  servo(cfg::PIN_SERVO, 0, cfg::SERVO_MAX_DEG);
static OledRadar   oled(cfg::PIN_SDA, cfg::PIN_SCL, cfg::OLED_ADDR, radarParams);
static WifiAp      ap(cfg::AP_SSID, cfg::AP_PASS, cfg::AP_CHANNEL, cfg::AP_MAX_CLIENTS, IPAddress(cfg::AP_IP), IPAddress(cfg::AP_MASK));
static UdpSender   udp(IPAddress(cfg::UDP_DEST), cfg::UDP_PORT);


static RingBuffer<Sample, cfg::RING_CAPACITY> ring;   
static QueueHandle_t qDisplay = nullptr;              

// ===================== TAREA: SENSOR + SERVO =====================

static void taskSensor(void *) {
  int angulo = 0;
  int dir    = +1;

  servo.mover(0);
  vTaskDelay(pdMS_TO_TICKS(600));           

  for (;;) {
    vTaskDelay(pdMS_TO_TICKS(cfg::SERVO_SETTLE_MS));   

    Sample s;
    s.angle  = (uint8_t)angulo;
    s.distMM = ultra.medirMM(cfg::SAMPLES_PER_ANGLE);
    s.tMs    = millis();

    ring.push(s);                           
    xQueueSend(qDisplay, &s, 0);            

    angulo += dir * cfg::SERVO_STEP_DEG;
    if (angulo >= cfg::SERVO_MAX_DEG) { angulo = cfg::SERVO_MAX_DEG; dir = -1; }
    if (angulo <= 0)                  { angulo = 0;                  dir = +1; }
    servo.mover((uint8_t)angulo);
  }
}

static void taskDisplay(void *) {
  TickType_t last = xTaskGetTickCount();

  for (;;) {
    Sample s;
    while (xQueueReceive(qDisplay, &s, 0) == pdTRUE) {   
      oled.actualizarObjeto(s.angle, s.distMM);
      oled.setBarrido(s.angle);
    }
    oled.render();
    vTaskDelayUntil(&last, pdMS_TO_TICKS(cfg::OLED_REFRESH_MS));
  }
}

static void enviarBuffer() {
  static char     pkt[1000];
  static Sample   lote[cfg::CHUNK_SAMPLES];
  static uint32_t seq = 0;

  while (ring.size() > 0) {
    size_t n = 0;
    while (n < cfg::CHUNK_SAMPLES && ring.pop(lote[n])) n++;
    if (n == 0) break;

    int len = snprintf(pkt, sizeof(pkt), "RADAR|%lu|%u|",
                       (unsigned long)seq, (unsigned)n);
    for (size_t i = 0; i < n; i++) {
      len += snprintf(pkt + len, sizeof(pkt) - len, "%u,%u,%lu;",
                      lote[i].angle, lote[i].distMM, (unsigned long)lote[i].tMs);
    }

    bool ok = udp.enviar(pkt, len);
    Serial.printf("[COMM] seq=%lu muestras=%u bytes=%d %s\n",
                  (unsigned long)seq, (unsigned)n, len, ok ? "OK" : "FALLO");
    seq++;
  }
}

static void taskComm(void *) {
  TickType_t last = xTaskGetTickCount();

  for (;;) {
    vTaskDelayUntil(&last, pdMS_TO_TICKS(cfg::SEND_INTERVAL_MS));

    if (ap.clientes() == 0) {
      Serial.println("[COMM] sin clientes conectados, acumulando en buffer");
      continue;
    }
    enviarBuffer();

    if (ring.overruns() > 0)
      Serial.printf("[COMM] aviso: %lu muestras sobrescritas en el ring\n",
                    (unsigned long)ring.overruns());
  }
}



void setup(void) {
  Serial.begin(115200);
  delay(200);

  Serial.printf("[INIT] Ultrasonico: %s\n", ultra.begin() ? "OK" : "FALLO");
  Serial.printf("[INIT] Servo:       %s\n", servo.begin() ? "OK" : "FALLO");
  Serial.printf("[INIT] OLED:        %s\n", oled.begin()  ? "OK" : "FALLO (¿0x3C?)");
  Serial.printf("[INIT] WiFi AP:     %s  IP=%s\n", ap.begin() ? "OK" : "FALLO", ap.ip().toString().c_str());
  Serial.printf("[INIT] UDP:         %s\n", udp.begin() ? "OK" : "FALLO");


  qDisplay = xQueueCreate(cfg::DISPLAY_QUEUE_LEN, sizeof(Sample));

  xTaskCreatePinnedToCore(taskSensor,  "sensor",  cfg::STACK_SENSOR,  nullptr, cfg::PRIO_SENSOR,  nullptr, cfg::CORE_SENSOR);
  xTaskCreatePinnedToCore(taskDisplay, "display", cfg::STACK_DISPLAY, nullptr, cfg::PRIO_DISPLAY, nullptr, cfg::CORE_DISPLAY);
  xTaskCreatePinnedToCore(taskComm,    "comm",    cfg::STACK_COMM,    nullptr, cfg::PRIO_COMM,    nullptr, cfg::CORE_COMM);
}

void loop(void) {
  vTaskDelete(nullptr);
}
