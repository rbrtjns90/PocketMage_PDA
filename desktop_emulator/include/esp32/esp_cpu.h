/**
 * @file esp_cpu.h
 * @brief ESP32 CPU functions mock for desktop emulator
 */

#ifndef ESP_CPU_H
#define ESP_CPU_H

#include "pocketmage/pocketmage_compat.h"

// CPU core IDs
#define PRO_CPU_NUM 0
#define APP_CPU_NUM 1

// Get current CPU core
inline int esp_cpu_get_core_id() {
    return 0; // Always return core 0 in emulator
}

// CPU cycle count
inline uint32_t esp_cpu_get_cycle_count() {
    return static_cast<uint32_t>(micros() * 240); // Approximate at 240MHz
}

#endif // ESP_CPU_H
