#include "Network.h"

int Network::sock_fd = -1;
struct sockaddr_in Network::dest_addr;
const char *Network::TAG = "RADAR_WIFI";

void Network::inicializar_wifi(connect_embebed_wifi_config_t* config) {
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(esp_netif_init());

    esp_netif_create_default_wifi_ap();

    wifi_config_t wifi_config = {};
    strncpy((char*)wifi_config.ap.ssid, config->ssid, sizeof(wifi_config.ap.ssid));
    strncpy((char*)wifi_config.ap.password, config->password, sizeof(wifi_config.ap.password));
    wifi_config.ap.ssid_len = strlen(config->ssid);
    wifi_config.ap.max_connection = EXAMPLE_MAX_STA_CONN;
    wifi_config.ap.authmode = WIFI_AUTH_WPA_WPA2_PSK;

    if (strlen(config->password) == 0) {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI("WIFI", "Punto de acceso creado: SSID=%s", config->ssid);
}

void Network::wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*)event_data;

        ESP_LOGI("WIFI", "Estación conectada: MAC=" MACSTR ", AID=%d", MAC2STR(event->mac), event->aid);
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*)event_data;

        ESP_LOGI("WIFI", "Estación desconectada: MAC=" MACSTR ", AID=%d", MAC2STR(event->mac), event->aid);
    }
}

void Network::wifi_init_softap(void) {
    esp_err_t ret = nvs_flash_init();

    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }

    ESP_ERROR_CHECK(ret);

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL));

    wifi_config_t wifi_config = {};

    strcpy((char*)wifi_config.ap.ssid, "ESP32_Radar_AP");
    strcpy((char*)wifi_config.ap.password, "12345678");
    wifi_config.ap.ssid_len = strlen("ESP32_Radar_AP");

    wifi_config.ap.channel = EXAMPLE_ESP_WIFI_CHANNEL;
    wifi_config.ap.max_connection = EXAMPLE_MAX_STA_CONN;
    wifi_config.ap.authmode = WIFI_AUTH_WPA2_PSK;

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "Access Point iniciado.");
}

void Network::init_udp_socket(void) {
    dest_addr.sin_addr.s_addr = inet_addr("192.168.4.2");
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(4242);

    sock_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (sock_fd < 0) {
        ESP_LOGE(TAG, "No se pudo crear el socket UDP");
    } else {
        ESP_LOGI(TAG, "Socket UDP configurado correctamente.");
    }
}

void Network::enviar_distancia_udp(uint16_t distancia) {
    if (sock_fd < 0) return;

    char payload[16];
    int len = snprintf(payload, sizeof(payload), "%u", distancia);

    int err = sendto(sock_fd, payload, len, 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (err < 0) {
        ESP_LOGE(TAG, "Error durante el envio UDP");
    }
}

void Network::enviar_telemetria_udp(uint16_t distancia, bool alerta) {
    if (sock_fd < 0) return;

    char payload[64];
    int len = snprintf(payload, sizeof(payload), "{\"distancia_cm\": %u, \"alerta\": %s}", 
                       distancia, alerta ? "true" : "false");

    int err = sendto(sock_fd, payload, len, 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (err < 0) {
        ESP_LOGE(TAG, "Error durante el envio UDP");
    }
}