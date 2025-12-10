/**
 * @file hid_usage_mouse.h
 * @brief HID mouse usage codes for desktop emulator
 */

#ifndef HID_USAGE_MOUSE_H
#define HID_USAGE_MOUSE_H

// HID Mouse button definitions
#define HID_MOUSE_BUTTON_LEFT   (1 << 0)
#define HID_MOUSE_BUTTON_RIGHT  (1 << 1)
#define HID_MOUSE_BUTTON_MIDDLE (1 << 2)

// Mouse input report structure
typedef struct {
    uint8_t buttons;
    int8_t x;
    int8_t y;
    int8_t wheel;
} hid_mouse_input_report_boot_t;

#endif // HID_USAGE_MOUSE_H
