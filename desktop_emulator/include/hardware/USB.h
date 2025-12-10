/**
 * @file USB.h
 * @brief USB library mock for desktop emulator
 */

#ifndef USB_H
#define USB_H

#include "pocketmage/pocketmage_compat.h"

// USB mode
#define ARDUINO_USB_MODE 1

// ESP event types
typedef const char* esp_event_base_t;

// USB event types
typedef enum {
    ARDUINO_USB_STARTED_EVENT = 0,
    ARDUINO_USB_STOPPED_EVENT,
    ARDUINO_USB_SUSPEND_EVENT,
    ARDUINO_USB_RESUME_EVENT
} arduino_usb_event_t;

// USB event data
typedef union {
    struct {
        bool connected;
    } connected;
} arduino_usb_event_data_t;

// USB class
class USBCDC {
public:
    USBCDC() {}
    
    void begin(unsigned long baud = 0) {}
    void end() {}
    
    int available() { return 0; }
    int read() { return -1; }
    size_t write(uint8_t c) { return 1; }
    size_t write(const uint8_t* buffer, size_t size) { return size; }
    void flush() {}
    
    operator bool() { return true; }
    
    void onEvent(void (*callback)(void*, esp_event_base_t, int32_t, void*)) {}
};

extern USBCDC USBSerial;

#endif // USB_H
