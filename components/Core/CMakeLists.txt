idf_component_register(SRCS "kernel.c"
                        INCLUDE_DIRS "include"
                        REQUIRES Services 
                                 Application
                                 Drivers
                                 esp_common
                       )
