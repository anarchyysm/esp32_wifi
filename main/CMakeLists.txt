idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "." # Ou "include" se aplicável
    REQUIRES Drivers
             Services 
             Core
             Application
             nvs_flash
             esp_event
             esp_common
             # wsl_bypasser (se for um componente válido e necessário aqui)
)
