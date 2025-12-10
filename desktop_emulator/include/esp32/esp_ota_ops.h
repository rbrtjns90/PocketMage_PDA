/**
 * @file esp_ota_ops.h
 * @brief ESP32 OTA operations mock for desktop emulator
 */

#ifndef ESP_OTA_OPS_H
#define ESP_OTA_OPS_H

#include "pocketmage/pocketmage_compat.h"

// OTA partition types
typedef struct {
    uint32_t offset;
    uint32_t size;
    char label[17];
} esp_partition_t;

// OTA handle
typedef uint32_t esp_ota_handle_t;

// Error codes
#define ESP_OK 0
#define ESP_FAIL -1
#define ESP_ERR_OTA_BASE 0x1500
#define ESP_ERR_OTA_PARTITION_CONFLICT (ESP_ERR_OTA_BASE + 1)
#define ESP_ERR_OTA_SELECT_INFO_INVALID (ESP_ERR_OTA_BASE + 2)
#define ESP_ERR_OTA_VALIDATE_FAILED (ESP_ERR_OTA_BASE + 3)

// OTA functions (stubs)
inline esp_err_t esp_ota_begin(const esp_partition_t* partition, size_t image_size, esp_ota_handle_t* out_handle) {
    if (out_handle) *out_handle = 1;
    return ESP_OK;
}

inline esp_err_t esp_ota_write(esp_ota_handle_t handle, const void* data, size_t size) {
    return ESP_OK;
}

inline esp_err_t esp_ota_end(esp_ota_handle_t handle) {
    return ESP_OK;
}

inline esp_err_t esp_ota_set_boot_partition(const esp_partition_t* partition) {
    return ESP_OK;
}

inline const esp_partition_t* esp_ota_get_boot_partition() {
    static esp_partition_t boot_partition = {0, 0x100000, "app0"};
    return &boot_partition;
}

inline const esp_partition_t* esp_ota_get_running_partition() {
    return esp_ota_get_boot_partition();
}

inline const esp_partition_t* esp_ota_get_next_update_partition(const esp_partition_t* start_from) {
    static esp_partition_t next_partition = {0x100000, 0x100000, "app1"};
    return &next_partition;
}

inline esp_err_t esp_ota_mark_app_valid_cancel_rollback() {
    return ESP_OK;
}

inline esp_err_t esp_ota_mark_app_invalid_rollback_and_reboot() {
    return ESP_FAIL; // Can't actually reboot in emulator
}

#endif // ESP_OTA_OPS_H
