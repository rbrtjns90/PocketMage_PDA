/**
 * @file esp32-hal-log.h
 * @brief ESP32 HAL logging mock for desktop emulator
 */

#ifndef ESP32_HAL_LOG_H
#define ESP32_HAL_LOG_H

#include "esp32/esp_log.h"

// ESP32 HAL log macros - redirect to ESP-IDF style
#define log_e(format, ...) ESP_LOGE("HAL", format, ##__VA_ARGS__)
#define log_w(format, ...) ESP_LOGW("HAL", format, ##__VA_ARGS__)
#define log_i(format, ...) ESP_LOGI("HAL", format, ##__VA_ARGS__)
#define log_d(format, ...) ESP_LOGD("HAL", format, ##__VA_ARGS__)
#define log_v(format, ...) ESP_LOGV("HAL", format, ##__VA_ARGS__)

#endif // ESP32_HAL_LOG_H
