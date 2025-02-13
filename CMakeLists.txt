# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.0)
set(toolchainVersion 13_3_Rel1)
set(picotoolVersion 2.1.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico_w CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(projeto_embarcatech C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(projeto_embarcatech
        main.c
        ssd1306/ssd1306.c
        ssd1306/ssd1306_fonts.c
        ap_mode/dhcpserver/dhcpserver.c
        ap_mode/dnsserver/dnsserver.c
        )

pico_set_program_name(projeto_embarcatech "projeto_embarcatech")
pico_set_program_version(projeto_embarcatech "0.1")
        

# pull in common dependencies and additional i2c hardware support
target_link_libraries(projeto_embarcatech 
        pico_stdlib
        tinyusb_board 
        hardware_i2c 
        hardware_adc 
        hardware_pwm
        hardware_timer
        pico_cyw43_arch_lwip_threadsafe_background
        pico_lwip_mqtt
        )

pico_enable_stdio_usb(projeto_embarcatech 1)
pico_enable_stdio_uart(projeto_embarcatech 0)

target_include_directories(projeto_embarcatech PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/ap_mode
    ${CMAKE_CURRENT_LIST_DIR}/ap_mode/dhcpserver
    ${CMAKE_CURRENT_LIST_DIR}/ap_mode/dnsserver
    ${PICO_SDK_PATH}/lib/lwip/src/include  # Diretório base do lwIP
    ${CMAKE_CURRENT_LIST_DIR}/path/to/lwipopts  # Se você tiver uma cópia local de lwipopts.h
)

# create map/bin/hex file etc.
pico_add_extra_outputs(projeto_embarcatech)

# add url via pico_set_program_url
