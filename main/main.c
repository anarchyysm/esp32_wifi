#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "kernel.h"
#include "wifi_service.h"
#include "wifi_deauther.h"
#include "led_control.h"

static const char *TAG = "app_main";

void app_main(void) {
    ESP_LOGI(TAG, "Iniciando o aplicativo");

    // Inicializa o núcleo (que inicializa o Wi-Fi)
    kernel_init();

    // Inicializa o LED (se ainda não foi inicializado em outro componente)
    led_rgb_init();

    // Cria a task de deautenticação
    xTaskCreate(wifi_deauther_task, "wifi_deauther", 4096, NULL, 5, NULL);
}
