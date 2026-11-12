#pragma once
#include <Arduino.h>

namespace cfg {

// ---------------- WiFi (Access Point ) ----------------
constexpr const char* AP_SSID        = "RadarESP32";
constexpr const char* AP_PASS        = "radar12345";     
constexpr uint8_t     AP_CHANNEL     = 6;
constexpr uint8_t     AP_MAX_CLIENTS = 4;
constexpr uint8_t     AP_IP[4]       = {192, 168, 4, 1};
constexpr uint8_t     AP_MASK[4]     = {255, 255, 255, 0};

// ---------------- UDP ----------------
constexpr uint8_t     UDP_DEST[4]      = {192, 168, 4, 255};  // broadcast de la subred del AP
constexpr uint16_t    UDP_PORT         = 4210;
constexpr uint32_t    SEND_INTERVAL_MS = 5000;                // envío cada 5 s
constexpr size_t      CHUNK_SAMPLES    = 40;                  // muestras por datagrama
constexpr size_t      RING_CAPACITY    = 128;                 // tamaño del ring buffer

// ---------------- Pines ----------------
constexpr uint8_t PIN_TRIG  = 5;
constexpr uint8_t PIN_ECHO  = 18;   
constexpr uint8_t PIN_SERVO = 15;
constexpr uint8_t PIN_SDA   = 21;
constexpr uint8_t PIN_SCL   = 22;

// ---------------- Servo ----------------
constexpr uint8_t  SERVO_MAX_DEG   = 128;   
constexpr uint8_t  SERVO_STEP_DEG  = 4;    
constexpr uint32_t SERVO_SETTLE_MS = 70;    

// ---------------- Ultrasónico ----------------
constexpr uint16_t MIN_MM            = 20;
constexpr uint16_t MAX_MM            = 2000;
constexpr uint8_t  SAMPLES_PER_ANGLE = 1;  

// ---------------- OLED / Radar ----------------
constexpr uint8_t  OLED_ADDR       = 0x3C;
constexpr uint32_t OLED_REFRESH_MS = 60;
constexpr uint32_t BLIP_TTL_MS     = 6000;
constexpr bool     INVERT_X        = false;
constexpr bool     SHOW_SWEEP_LINE = false;
constexpr bool     SHOW_RANGE_ARC  = false;

// ---------------- FreeRTOS ----------------
constexpr uint32_t STACK_SENSOR  = 4096;
constexpr uint32_t STACK_DISPLAY = 4096;
constexpr uint32_t STACK_COMM    = 6144;
constexpr UBaseType_t PRIO_SENSOR  = 3;
constexpr UBaseType_t PRIO_COMM    = 2;
constexpr UBaseType_t PRIO_DISPLAY = 1;
constexpr BaseType_t  CORE_SENSOR  = 1;
constexpr BaseType_t  CORE_DISPLAY = 1;
constexpr BaseType_t  CORE_COMM    = 0;
constexpr UBaseType_t DISPLAY_QUEUE_LEN = 16;

} 
