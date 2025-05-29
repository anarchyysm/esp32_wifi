#ifndef WIFI_DEAUTHER_H
#define WIFI_DEAUTHER_H

#include "esp_wifi.h"

typedef enum {
    DEAUTH_INVALID_AUTH,
    DEAUTH_INACTIVITY,
    DEAUTH_CLASS3,
    DEAUTH_TYPE_COUNT
} deauth_frame_type_t;

void wifi_deauther_send_raw_frame(const uint8_t *frame_buffer, int size);
void wifi_deauther_send_deauth_frame(const wifi_ap_record_t *ap_record, deauth_frame_type_t type);
void wifi_deauther_task(void *pvParameters);

#endif
