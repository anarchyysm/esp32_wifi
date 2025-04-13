// interface/wsl_bypasser.h
#ifndef WSL_BYPASSER_H
#define WSL_BYPASSER_H

#include "esp_wifi.h"

typedef enum {
    DEAUTH_INVALID_AUTH,
    DEAUTH_INACTIVITY,
    DEAUTH_CLASS3,
    DEAUTH_TYPE_COUNT
} deauth_frame_type_t;

void wsl_bypasser_send_raw_frame(const uint8_t *frame_buffer, int size);
void wsl_bypasser_send_deauth_frame(const wifi_ap_record_t *ap_record, deauth_frame_type_t type);

#endif
