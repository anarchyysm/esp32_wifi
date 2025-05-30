idf_component_register(
    SRCS             "wifi/wifi_service.c" 
                     "cc1101/cc1101.c"
    INCLUDE_DIRS     "wifi/include" 
                     "cc1101/include"
    REQUIRES         Drivers
                     esp_common
                     esp_wifi
                     esp_netif
                     esp_event
    PRIV_REQUIRES    nvs_flash
)
