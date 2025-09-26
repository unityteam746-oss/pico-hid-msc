cmake_minimum_required(VERSION 3.13)

# This pulls in pico_sdk_import.cmake from the Pico SDK
include(pico_sdk_import.cmake)

# Initialize the SDK
pico_sdk_init()

# Set your project name
project(pico_hid_msc)

# Add your executable
add_executable(pico_hid_msc hid_msc.c)

# Link against pico_stdlib which pulls in common libraries
target_link_libraries(pico_hid_msc pico_stdlib)

# Enable USB serial output
pico_enable_stdio_usb(pico_hid_msc 1)

# Create map/bin/hex/uf2 files
pico_add_extra_outputs(pico_hid_msc)
