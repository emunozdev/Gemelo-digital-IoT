#pragma once

#include <Arduino.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "lwip/err.h"
#include "lwip/sockets.h"

#define EXAMPLE_ESP_WIFI_CHANNEL   1
#define EXAMPLE_MAX_STA_CONN       4

typedef struct {
    char ssid[32];
    char password[64];
    char dest_ip_addr[16];
    uint16_t udp_port;
} connect_embebed_wifi_config_t;

class Network {
private:
    static int sock_fd;
    static struct sockaddr_in dest_addr;
    static const char *TAG;
public:
    static void inicializar_wifi(connect_embebed_wifi_config_t* config);
    static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
    static void wifi_init_softap(void);
    static void init_udp_socket(void);
    static void enviar_distancia_udp(uint16_t distancia);
};