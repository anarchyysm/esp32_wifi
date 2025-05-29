cmake_minimum_required(VERSION 3.16)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(scan)

set(EXTRA_COMPONENT_DIRS components )

# Configurações do compilador
add_definitions(-DESP_LOG_DEFAULT_LEVEL=ESP_LOG_DEBUG)
