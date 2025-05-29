#include "wifi_service.h"
#include "led_control.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "wifi_service";

static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        ESP_LOGI(TAG, "Wi-Fi STA iniciado, pronto para escaneamento");
        led_blink_green();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_START) {
        ESP_LOGI(TAG, "Wi-Fi AP iniciado");
        led_blink_green();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_SCAN_DONE) {
        ESP_LOGI(TAG, "Escaneamento Wi-Fi concluído");
        led_blink_blue();
    }
}

void wifi_init(void) {
    // Inicializa NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
        ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Falha ao inicializar NVS: %s", esp_err_to_name(ret));
        led_blink_red();
        return;
    }
    ESP_ERROR_CHECK(ret);

    // Inicializa a pilha de rede
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_ap();
    esp_netif_create_default_wifi_sta();

    // Configura o Wi-Fi
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    // Registra o manipulador de eventos
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
                        WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL));

    // Define o modo Wi-Fi como APSTA
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));

    // Configura o AP
    wifi_config_t ap_config = {
        .ap = {
            .ssid = "Darth Maul",
            .ssid_len = strlen("Darth Maul"),
            .channel = 1,
            .password = "MyPassword123",
            .max_connection = 4,
            .authmode = WIFI_AUTH_WPA2_PSK,
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &ap_config));

    // Inicia o Wi-Fi
    ESP_ERROR_CHECK(esp_wifi_start());

    // Configura a potência de transmissão
    ESP_ERROR_CHECK(esp_wifi_set_max_tx_power(84)); // 21 dBm

    ESP_LOGI(TAG, "Wi-Fi inicializado no modo APSTA");
    led_blink_green();
}

void wifi_service_init(void) {
    ESP_LOGI(TAG, "Serviço Wi-Fi inicializado (outras inicializações específicas podem ir aqui)");
}
