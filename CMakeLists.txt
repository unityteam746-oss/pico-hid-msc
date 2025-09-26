cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(pico_hid_msc)

pico_sdk_init()

add_executable(pico_hid_msc hid_msc.c)

target_link_libraries(pico_hid_msc pico_stdlib)

pico_enable_stdio_usb(pico_hid_msc 1)

pico_add_extra_outputs(pico_hid_msc)
