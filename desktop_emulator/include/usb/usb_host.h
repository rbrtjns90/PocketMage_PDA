/**
 * @file usb/usb_host.h
 * @brief ESP32 USB Host library mock for desktop emulator
 */

#ifndef USB_HOST_H
#define USB_HOST_H

#include "pocketmage/pocketmage_compat.h"

// USB host configuration
typedef struct {
    bool skip_phy_setup;
    int intr_flags;
} usb_host_config_t;

// USB host client configuration
typedef struct {
    bool is_synchronous;
    int max_num_event_msg;
    void (*async_client_event_callback)(void* event_data, void* arg);
    void* callback_arg;
} usb_host_client_config_t;

// USB host handle types
typedef void* usb_host_client_handle_t;
typedef void* usb_device_handle_t;

// USB host functions (stubs)
inline esp_err_t usb_host_install(const usb_host_config_t* config) { return ESP_OK; }
inline esp_err_t usb_host_uninstall() { return ESP_OK; }
inline esp_err_t usb_host_client_register(const usb_host_client_config_t* config, usb_host_client_handle_t* client_hdl) {
    if (client_hdl) *client_hdl = nullptr;
    return ESP_OK;
}
inline esp_err_t usb_host_client_deregister(usb_host_client_handle_t client_hdl) { return ESP_OK; }
inline esp_err_t usb_host_lib_handle_events(uint32_t timeout_ms, uint32_t* event_flags) { return ESP_OK; }
inline esp_err_t usb_host_client_handle_events(usb_host_client_handle_t client_hdl, uint32_t timeout_ms) { return ESP_OK; }

#endif // USB_HOST_H
