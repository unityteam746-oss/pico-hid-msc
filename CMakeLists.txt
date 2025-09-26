cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(pico_hid_msc)

pico_sdk_init()

# Add FatFs library
add_library(fatfs STATIC
    pico-sdk/lib/tinyusb/src/class/msc/msc_device.c
    pico-sdk/lib/tinyusb/src/class/msc/msc_host.c
    pico-sdk/lib/tinyusb/src/common/tusb_fifo.c
    pico-sdk/lib/tinyusb/src/device/usbd.c
    pico-sdk/lib/tinyusb/src/device/usbd_control.c
    pico-sdk/lib/tinyusb/src/portable/raspberrypi/rp2040/rp2040_usb.c
    pico-sdk/lib/tinyusb/src/tusb.c
)

target_include_directories(fatfs PUBLIC
    pico-sdk/lib/tinyusb/src
    pico-sdk/lib/tinyusb/hw
)

add_executable(pico_hid_msc hid_msc.c)

# Link required libraries
target_link_libraries(pico_hid_msc 
    pico_stdlib 
    hardware_spi
    fatfs
)

pico_enable_stdio_usb(pico_hid_msc 1)
pico_enable_stdio_uart(pico_hid_msc 0)

pico_add_extra_outputs(pico_hid_msc)
