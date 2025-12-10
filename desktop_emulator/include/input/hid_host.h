/**
 * @file hid_host.h
 * @brief ESP32 HID Host library mock for desktop emulator
 */

#ifndef HID_HOST_H
#define HID_HOST_H

#include "pocketmage/pocketmage_compat.h"

// HID protocol types
typedef enum {
    HID_PROTOCOL_NONE = 0,
    HID_PROTOCOL_KEYBOARD,
    HID_PROTOCOL_MOUSE
} hid_protocol_t;

// HID report types
typedef enum {
    HID_REPORT_TYPE_INPUT = 1,
    HID_REPORT_TYPE_OUTPUT,
    HID_REPORT_TYPE_FEATURE
} hid_report_type_t;

// HID host device handle
typedef void* hid_host_device_handle_t;

// HID host configuration
typedef struct {
    void (*callback)(void* arg);
    void* callback_arg;
} hid_host_driver_config_t;

// HID host device configuration
typedef struct {
    void (*callback)(void* arg, void* event);
    void* callback_arg;
} hid_host_device_config_t;

// HID keyboard input report
typedef struct {
    uint8_t modifier;
    uint8_t reserved;
    uint8_t key[6];
} hid_keyboard_input_report_boot_t;

// HID host functions (stubs)
inline esp_err_t hid_host_install(const hid_host_driver_config_t* config) { return ESP_OK; }
inline esp_err_t hid_host_uninstall() { return ESP_OK; }
inline esp_err_t hid_host_device_open(hid_host_device_handle_t dev, const hid_host_device_config_t* config) { return ESP_OK; }
inline esp_err_t hid_host_device_close(hid_host_device_handle_t dev) { return ESP_OK; }
inline esp_err_t hid_host_device_start(hid_host_device_handle_t dev) { return ESP_OK; }
inline esp_err_t hid_host_claim_interface(hid_host_device_handle_t dev, const hid_host_device_config_t* config, uint8_t iface) { return ESP_OK; }
inline esp_err_t hid_host_release_interface(hid_host_device_handle_t dev, uint8_t iface) { return ESP_OK; }
inline hid_protocol_t hid_host_device_get_protocol(hid_host_device_handle_t dev, uint8_t iface) { return HID_PROTOCOL_NONE; }

// Keyboard modifier keys
#define HID_LEFT_CONTROL   (1 << 0)
#define HID_LEFT_SHIFT     (1 << 1)
#define HID_LEFT_ALT       (1 << 2)
#define HID_LEFT_GUI       (1 << 3)
#define HID_RIGHT_CONTROL  (1 << 4)
#define HID_RIGHT_SHIFT    (1 << 5)
#define HID_RIGHT_ALT      (1 << 6)
#define HID_RIGHT_GUI      (1 << 7)

#endif // HID_HOST_H
