#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_mac.h"

#define TAG "WiFi_Scan" // Tag para logs

// Função para inicializar o NVS (necessário para Wi-Fi)
static void initialize_nvs(void) {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
}

// Função para inicializar o Wi-Fi
static void initialize_wifi(void) {
    // Inicializa o loop de eventos padrão
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // Configuração inicial do Wi-Fi
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    // Define o modo como STA (Station)
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
}

// Função para escanear redes Wi-Fi
static void wifi_scan(void) {
    // Configuração do escaneamento
    wifi_scan_config_t scan_config = {
        .ssid = NULL,           // Escanear todos os SSIDs
        .bssid = NULL,          // Escanear todos os BSSIDs
        .channel = 0,           // Escanear todos os canais
        .show_hidden = true     // Mostrar redes ocultas
    };

    // Inicia o escaneamento
    ESP_ERROR_CHECK(esp_wifi_scan_start(&scan_config, true));
    ESP_LOGI(TAG, "Escaneamento de redes Wi-Fi iniciado...");

    // Obtém os resultados do escaneamento
    uint16_t ap_count = 0;
    esp_wifi_scan_get_ap_num(&ap_count);
    ESP_LOGI(TAG, "%d redes Wi-Fi encontradas", ap_count);

    wifi_ap_record_t *ap_list = (wifi_ap_record_t *)malloc(ap_count * sizeof(wifi_ap_record_t));
    if (ap_list == NULL) {
        ESP_LOGE(TAG, "Erro ao alocar memória para a lista de APs");
        return;
    }

    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&ap_count, ap_list));

    // Exibe as informações de cada rede
    for (int i = 0; i < ap_count; i++) {
        printf("\n");
        printf("SSID: %s\n", (char *)ap_list[i].ssid);
        printf("MAC Address (BSSID): %02x:%02x:%02x:%02x:%02x:%02x\n",
               ap_list[i].bssid[0], ap_list[i].bssid[1], ap_list[i].bssid[2],
               ap_list[i].bssid[3], ap_list[i].bssid[4], ap_list[i].bssid[5]);
        printf("RSSI: %d dBm\n", ap_list[i].rssi);
        printf("Canal: %d\n", ap_list[i].primary);

        // Tipo de criptografia
        const char *auth_mode;
        switch (ap_list[i].authmode) {
            case WIFI_AUTH_OPEN:            auth_mode = "Aberta"; break;
            case WIFI_AUTH_WEP:             auth_mode = "WEP"; break;
            case WIFI_AUTH_WPA_PSK:         auth_mode = "WPA"; break;
            case WIFI_AUTH_WPA2_PSK:        auth_mode = "WPA2"; break;
            case WIFI_AUTH_WPA_WPA2_PSK:    auth_mode = "WPA/WPA2"; break;
            case WIFI_AUTH_WPA2_ENTERPRISE: auth_mode = "WPA2 Enterprise"; break;
            default:                        auth_mode = "Desconhecida"; break;
        }
        printf("Criptografia: %s\n", auth_mode);
        printf("-------------------\n");
    }

    // Libera a memória alocada
    free(ap_list);
}

// Função principal
void app_main(void) {
    // Inicializa o NVS
    initialize_nvs();

    // Inicializa o Wi-Fi
    initialize_wifi();

    // Loop infinito para escanear redes a cada 5 segundos
    while (1) {
        wifi_scan();
        vTaskDelay(5000 / portTICK_PERIOD_MS); // Aguarda 5 segundos
    }
}

