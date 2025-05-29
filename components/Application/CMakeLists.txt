idf_component_register(SRCS "wifi_deauther/wifi_deauther.c" # Assumindo que deauther.c está na pasta src/
                        INCLUDE_DIRS "wifi_deauther/include" # Assumindo que wifi_deauther.h está em include/
                        REQUIRES Drivers
                                 Services
                                 esp_common
                                 esp_wifi
                                 freertos
                       )
target_link_libraries(${COMPONENT_LIB} -Wl,-zmuldefs)

