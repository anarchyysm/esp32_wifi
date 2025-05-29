#include "led_control.h"
#include "led_strip.h"
#include "esp_log.h"

#define LED_RGB_GPIO 48 // GPIO para o LED RGB (v1.0 do ESP32-S3-DevKitC-1)
static const char *TAG = "led_control";

static led_strip_handle_t led_strip;

// Função para inicializar o LED RGB
void led_rgb_init(void) {
    led_strip_config_t strip_config = {
        .strip_gpio_num = LED_RGB_GPIO,
        .max_leds = 1,
    };
    led_strip_rmt_config_t rmt_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = 10 * 1000 * 1000, // 10MHz
        .flags.with_dma = false,
    };
    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));
    ESP_ERROR_CHECK(led_strip_clear(led_strip)); // Desliga o LED inicialmente
    ESP_LOGI(TAG, "LED RGB inicializado no GPIO %d", LED_RGB_GPIO);
}

// Função para piscar o LED em uma cor específica
static void led_blink_color(uint8_t r, uint8_t g, uint8_t b, int duration_ms) {
    ESP_ERROR_CHECK(led_strip_set_pixel(led_strip, 0, r, g, b));
    ESP_ERROR_CHECK(led_strip_refresh(led_strip));
    vTaskDelay(duration_ms / portTICK_PERIOD_MS);
    ESP_ERROR_CHECK(led_strip_clear(led_strip));
    ESP_ERROR_CHECK(led_strip_refresh(led_strip));
}

void led_blink_red(void) {
    ESP_LOGI(TAG, "Piscando LED vermelho (erro)");
    led_blink_color(255, 0, 0, 500); // Vermelho por 500ms
}

void led_blink_green(void) {
    ESP_LOGI(TAG, "Piscando LED verde (sucesso)");
    led_blink_color(0, 255, 0, 200); // Verde por 500ms
}

void led_blink_blue(void) {
    ESP_LOGI(TAG, "Piscando LED azul (info)");
    led_blink_color(0, 0, 255, 500); // Azul por 500ms
}
