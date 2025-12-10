/**
 * @file driver/gpio.h
 * @brief ESP32 GPIO driver mock for desktop emulator
 */

#ifndef DRIVER_GPIO_H
#define DRIVER_GPIO_H

#include "pocketmage/pocketmage_compat.h"

// GPIO modes
typedef enum {
    GPIO_MODE_DISABLE = 0,
    GPIO_MODE_INPUT,
    GPIO_MODE_OUTPUT,
    GPIO_MODE_OUTPUT_OD,
    GPIO_MODE_INPUT_OUTPUT_OD,
    GPIO_MODE_INPUT_OUTPUT
} gpio_mode_t;

// GPIO pull modes
typedef enum {
    GPIO_FLOATING = 0,
    GPIO_PULLUP_ONLY,
    GPIO_PULLDOWN_ONLY,
    GPIO_PULLUP_PULLDOWN
} gpio_pull_mode_t;

typedef enum {
    GPIO_PULLUP_DISABLE = 0,
    GPIO_PULLUP_ENABLE
} gpio_pullup_t;

typedef enum {
    GPIO_PULLDOWN_DISABLE = 0,
    GPIO_PULLDOWN_ENABLE
} gpio_pulldown_t;

// GPIO interrupt types
typedef enum {
    GPIO_INTR_DISABLE = 0,
    GPIO_INTR_POSEDGE,
    GPIO_INTR_NEGEDGE,
    GPIO_INTR_ANYEDGE,
    GPIO_INTR_LOW_LEVEL,
    GPIO_INTR_HIGH_LEVEL
} gpio_int_type_t;

// GPIO configuration
typedef struct {
    uint64_t pin_bit_mask;
    gpio_mode_t mode;
    gpio_pullup_t pull_up_en;
    gpio_pulldown_t pull_down_en;
    gpio_int_type_t intr_type;
} gpio_config_t;

// GPIO functions
inline esp_err_t gpio_config(const gpio_config_t* pGPIOConfig) { return ESP_OK; }
inline esp_err_t gpio_set_direction(gpio_num_t gpio_num, gpio_mode_t mode) { return ESP_OK; }
inline esp_err_t gpio_set_level(gpio_num_t gpio_num, uint32_t level) { return ESP_OK; }
inline int gpio_get_level(gpio_num_t gpio_num) { return 0; }
inline esp_err_t gpio_set_pull_mode(gpio_num_t gpio_num, gpio_pull_mode_t pull) { return ESP_OK; }
inline esp_err_t gpio_pullup_en(gpio_num_t gpio_num) { return ESP_OK; }
inline esp_err_t gpio_pullup_dis(gpio_num_t gpio_num) { return ESP_OK; }
inline esp_err_t gpio_pulldown_en(gpio_num_t gpio_num) { return ESP_OK; }
inline esp_err_t gpio_pulldown_dis(gpio_num_t gpio_num) { return ESP_OK; }
inline esp_err_t gpio_set_intr_type(gpio_num_t gpio_num, gpio_int_type_t intr_type) { return ESP_OK; }
inline esp_err_t gpio_intr_enable(gpio_num_t gpio_num) { return ESP_OK; }
inline esp_err_t gpio_intr_disable(gpio_num_t gpio_num) { return ESP_OK; }

#endif // DRIVER_GPIO_H
