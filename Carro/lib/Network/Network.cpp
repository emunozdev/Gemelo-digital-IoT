#include "Network.h"
#include "Config.h"

#include <errno.h>
#include <string.h>
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "lwip/err.h"
#include "lwip/sockets.h"

static const char* TAG = "NET";

static int s_sock = -1;
static struct sockaddr_in s_dest;
static volatile uint32_t s_descartes = 0;   
static volatile uint32_t s_errores   = 0;

static void wifiEventHandler(void*, esp_event_base_t base, int32_t id, void* data) {
    if (base != WIFI_EVENT) return;

    if (id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t* e = (wifi_event_ap_staconnected_t*)data;
        ESP_LOGI(TAG, "Estacion conectada: MAC=" MACSTR ", AID=%d", MAC2STR(e->mac), e->aid);
    } else if (id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t* e = (wifi_event_ap_stadisconnected_t*)data;
        ESP_LOGI(TAG, "Estacion desconectada: MAC=" MACSTR ", AID=%d", MAC2STR(e->mac), e->aid);
    }
}

bool Network::iniciarAP() {
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
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID,
                                                        &wifiEventHandler, NULL, NULL));

    wifi_config_t wc = {};
    strncpy((char*)wc.ap.ssid,     Red::SSID,  sizeof(wc.ap.ssid) - 1);
    strncpy((char*)wc.ap.password, Red::CLAVE, sizeof(wc.ap.password) - 1);
    wc.ap.ssid_len       = strlen(Red::SSID);
    wc.ap.channel        = Red::CANAL;
    wc.ap.max_connection = Red::MAX_CLIENTES;
    wc.ap.authmode       = (strlen(Red::CLAVE) == 0) ? WIFI_AUTH_OPEN : WIFI_AUTH_WPA2_PSK;

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wc));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "Access Point iniciado: SSID=%s", Red::SSID);
    return true;
}

bool Network::iniciarUDP() {
    memset(&s_dest, 0, sizeof(s_dest));
    s_dest.sin_family      = AF_INET;
    s_dest.sin_port        = htons(Red::PUERTO);
    s_dest.sin_addr.s_addr = inet_addr(Red::IP_DESTINO);

    s_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (s_sock < 0) {
        ESP_LOGE(TAG, "No se pudo crear el socket UDP");
        return false;
    }
    ESP_LOGI(TAG, "Socket UDP listo -> %s:%u", Red::IP_DESTINO, (unsigned)Red::PUERTO);
    return true;
}

bool Network::enviar(const void* datos, size_t len) {
    if (s_sock < 0) return false;

    const int n = sendto(s_sock, datos, len, MSG_DONTWAIT, (struct sockaddr*)&s_dest, sizeof(s_dest));
    if (n < 0) {
        if (errno == ENOMEM || errno == EAGAIN || errno == EWOULDBLOCK) s_descartes++;
        else                                                            s_errores++;
        return false;
    }
    return true;
}

uint32_t Network::descartes() { return s_descartes; }
uint32_t Network::errores()   { return s_errores; }
