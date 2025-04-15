idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "."
    REQUIRES wsl_bypasser esp_wifi nvs_flash esp_event led_control
)
