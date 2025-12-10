/**
 * @file esp_log.h
 * @brief ESP-IDF logging mock for desktop emulator
 */

#ifndef ESP_LOG_H
#define ESP_LOG_H

#include <cstdio>

// Log levels
typedef enum {
    ESP_LOG_NONE = 0,
    ESP_LOG_ERROR,
    ESP_LOG_WARN,
    ESP_LOG_INFO,
    ESP_LOG_DEBUG,
    ESP_LOG_VERBOSE
} esp_log_level_t;

// Current log level (can be changed at runtime)
extern esp_log_level_t esp_log_level;

// Log macros
#define ESP_LOGE(tag, format, ...) \
    do { if (esp_log_level >= ESP_LOG_ERROR) printf("[E][%s] " format "\n", tag, ##__VA_ARGS__); } while(0)

#define ESP_LOGW(tag, format, ...) \
    do { if (esp_log_level >= ESP_LOG_WARN) printf("[W][%s] " format "\n", tag, ##__VA_ARGS__); } while(0)

#define ESP_LOGI(tag, format, ...) \
    do { if (esp_log_level >= ESP_LOG_INFO) printf("[I][%s] " format "\n", tag, ##__VA_ARGS__); } while(0)

#define ESP_LOGD(tag, format, ...) \
    do { if (esp_log_level >= ESP_LOG_DEBUG) printf("[D][%s] " format "\n", tag, ##__VA_ARGS__); } while(0)

#define ESP_LOGV(tag, format, ...) \
    do { if (esp_log_level >= ESP_LOG_VERBOSE) printf("[V][%s] " format "\n", tag, ##__VA_ARGS__); } while(0)

// Log level control
inline void esp_log_level_set(const char* tag, esp_log_level_t level) {
    esp_log_level = level;
}

#endif // ESP_LOG_H
