#pragma once
#include <stdint.h>

namespace Pines {
    constexpr uint8_t TRIG        = 18;
    constexpr uint8_t ECHO        = 5;    
    constexpr uint8_t BUZZER      = 19;
    constexpr uint8_t SERVO       = 12;   
    constexpr uint8_t MOTOR_IZQ_1 = 25;
    constexpr uint8_t MOTOR_IZQ_2 = 26;
    constexpr uint8_t MOTOR_DER_1 = 27;
    constexpr uint8_t MOTOR_DER_2 = 14;   
    // OLED I2C: SDA = 21, SCL = 22 
}

namespace Red {
    constexpr const char* SSID         = "ESP32_Radar_AP";
    constexpr const char* CLAVE        = "12345678";
    constexpr uint8_t     CANAL        = 1;
    constexpr uint8_t     MAX_CLIENTES = 4;
    constexpr const char* IP_DESTINO   = "192.168.4.2";   
    constexpr uint16_t    PUERTO       = 4242;
}

namespace CfgSensor {
    constexpr uint16_t DIST_MIN_CM    = 2;     
    constexpr uint16_t DIST_MAX_CM    = 400;
    constexpr uint32_t TIMEOUT_ECO_MS = 25;    // 400 cm = ~23 ms de eco
}

namespace CfgServo {
    constexpr uint8_t  ANGULO_CENTRO  = 90;
    constexpr uint8_t  ANGULO_LATERAL = 30;
    constexpr uint16_t MS_POR_GRADO   = 4;      // SG90 ~2-3 ms/grado + holgura. Calibrable.
    constexpr uint16_t MS_MARGEN      = 40;
}

namespace CfgControl {
    constexpr uint16_t UMBRAL_OBSTACULO_CM   = 20;   // Umbral: distancia.
    constexpr uint16_t T_PARADA_MS           = 200;  
    constexpr uint16_t T_RETRO_MS            = 400;
    constexpr uint16_t T_GIRO_MS             = 400;
    constexpr uint16_t T_ADELANTE_REMATE_MS  = 300;
    constexpr uint16_t T_GIRO_IZQ_MS         = 400;
    constexpr uint16_t EDAD_MAX_MUESTRA_MS   = 300;  //Limite de velocidad 
    constexpr uint16_t BUZZER_MS             = 300;
}

namespace CfgPeriodo {
    constexpr uint32_t SENSOR_MS   = 60;    // ciclo minimo recomendado del HC-SR04; ~16 tramas/s
    constexpr uint32_t CONTROL_MS  = 20;
    constexpr uint32_t PANTALLA_MS = 200;   // 5 fps
}

namespace CfgRtos {
    constexpr int      NUCLEO_APP    = 1;    
    constexpr uint32_t STACK         = 4096; // tamaño en bytes. Ajusta con uxTaskGetStackHighWaterMark()
    constexpr uint8_t  PRIO_SENSOR   = 3;
    constexpr uint8_t  PRIO_CONTROL  = 2;
    constexpr uint8_t  PRIO_RED      = 1;
    constexpr uint8_t  PRIO_PANTALLA = 1;
    constexpr uint8_t  COLA_RED      = 8;    // muestras pendientes de enviar
}
