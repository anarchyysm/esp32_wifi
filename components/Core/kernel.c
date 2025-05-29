#include "kernel.h"
#include "wifi_service.h"
#include "led_control.h"
#include "esp_log.h"

static const char *TAG = "kernel";

void kernel_init(void) {
    ESP_LOGI(TAG, "Inicializando o núcleo do sistema");

    // Inicializa o LED RGB
    led_rgb_init();

    // Inicializa o Wi-Fi
    wifi_init();
    wifi_service_init();

    // Cria task para scan e deauth
    // xTaskCreate(wifi_deauther_task, "wifi_deauther", 4096, NULL, 5, NULL);
}
