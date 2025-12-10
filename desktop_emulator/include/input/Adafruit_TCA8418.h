/**
 * @file Adafruit_TCA8418.h
 * @brief TCA8418 keyboard matrix controller mock for desktop emulator
 */

#ifndef ADAFRUIT_TCA8418_H
#define ADAFRUIT_TCA8418_H

#include "pocketmage/pocketmage_compat.h"
#include "hardware/Wire.h"

#define TCA8418_DEFAULT_ADDR 0x34

// Register addresses
#define TCA8418_REG_CFG         0x01
#define TCA8418_REG_INT_STAT    0x02
#define TCA8418_REG_KEY_LCK_EC  0x03
#define TCA8418_REG_KEY_EVENT_A 0x04
#define TCA8418_REG_KEY_EVENT_B 0x05
#define TCA8418_REG_KEY_EVENT_C 0x06
#define TCA8418_REG_KEY_EVENT_D 0x07
#define TCA8418_REG_KEY_EVENT_E 0x08
#define TCA8418_REG_KEY_EVENT_F 0x09
#define TCA8418_REG_KEY_EVENT_G 0x0A
#define TCA8418_REG_KEY_EVENT_H 0x0B
#define TCA8418_REG_KEY_EVENT_I 0x0C
#define TCA8418_REG_KEY_EVENT_J 0x0D
#define TCA8418_REG_KP_LCK_TIMER 0x0E
#define TCA8418_REG_UNLOCK1     0x0F
#define TCA8418_REG_UNLOCK2     0x10
#define TCA8418_REG_GPIO_INT_STAT1 0x11
#define TCA8418_REG_GPIO_INT_STAT2 0x12
#define TCA8418_REG_GPIO_INT_STAT3 0x13
#define TCA8418_REG_GPIO_DAT_STAT1 0x14
#define TCA8418_REG_GPIO_DAT_STAT2 0x15
#define TCA8418_REG_GPIO_DAT_STAT3 0x16
#define TCA8418_REG_GPIO_DAT_OUT1 0x17
#define TCA8418_REG_GPIO_DAT_OUT2 0x18
#define TCA8418_REG_GPIO_DAT_OUT3 0x19
#define TCA8418_REG_GPIO_INT_EN1 0x1A
#define TCA8418_REG_GPIO_INT_EN2 0x1B
#define TCA8418_REG_GPIO_INT_EN3 0x1C
#define TCA8418_REG_KP_GPIO1    0x1D
#define TCA8418_REG_KP_GPIO2    0x1E
#define TCA8418_REG_KP_GPIO3    0x1F
#define TCA8418_REG_GPI_EM1     0x20
#define TCA8418_REG_GPI_EM2     0x21
#define TCA8418_REG_GPI_EM3     0x22
#define TCA8418_REG_GPIO_DIR1   0x23
#define TCA8418_REG_GPIO_DIR2   0x24
#define TCA8418_REG_GPIO_DIR3   0x25
#define TCA8418_REG_GPIO_INT_LVL1 0x26
#define TCA8418_REG_GPIO_INT_LVL2 0x27
#define TCA8418_REG_GPIO_INT_LVL3 0x28
#define TCA8418_REG_DEBOUNCE_DIS1 0x29
#define TCA8418_REG_DEBOUNCE_DIS2 0x2A
#define TCA8418_REG_DEBOUNCE_DIS3 0x2B
#define TCA8418_REG_GPIO_PULL1  0x2C
#define TCA8418_REG_GPIO_PULL2  0x2D
#define TCA8418_REG_GPIO_PULL3  0x2E

class Adafruit_TCA8418 {
public:
    Adafruit_TCA8418() : _begun(false), _interruptsEnabled(false) {}
    
    bool begin(uint8_t addr = TCA8418_DEFAULT_ADDR, TwoWire* wire = &Wire) {
        _addr = addr;
        _wire = wire;
        _begun = true;
        return true;
    }
    
    // Configure as keyboard matrix
    void matrix(uint8_t rows, uint8_t cols) {
        _rows = rows;
        _cols = cols;
    }
    
    // Interrupt control
    void enableInterrupts() { _interruptsEnabled = true; }
    void disableInterrupts() { _interruptsEnabled = false; }
    bool interruptsEnabled() const { return _interruptsEnabled; }
    
    // Key event handling
    void flush() { _keyQueue.clear(); }
    
    int available() { return _keyQueue.size(); }
    
    uint8_t getEvent() {
        if (_keyQueue.empty()) return 0;
        uint8_t event = _keyQueue.front();
        _keyQueue.erase(_keyQueue.begin());
        return event;
    }
    
    // Register access (for compatibility)
    uint8_t readRegister(uint8_t reg) {
        if (reg == TCA8418_REG_INT_STAT) {
            return _keyQueue.empty() ? 0 : 0x01; // Key event pending
        }
        return 0;
    }
    
    void writeRegister(uint8_t reg, uint8_t value) {}
    
    // For emulator: inject a key event
    void injectKeyEvent(uint8_t keyCode, bool pressed) {
        // Key event format: bit 7 = pressed (1) or released (0), bits 0-6 = key code
        uint8_t event = keyCode & 0x7F;
        if (pressed) event |= 0x80;
        _keyQueue.push_back(event);
    }
    
    // Get key code from row/col
    static uint8_t getKeyCode(uint8_t row, uint8_t col) {
        return row * 10 + col + 1; // 1-indexed key codes
    }
    
    // Decode key event to row/col
    static void decodeKeyEvent(uint8_t event, uint8_t& row, uint8_t& col, bool& pressed) {
        pressed = (event & 0x80) != 0;
        uint8_t keyCode = event & 0x7F;
        if (keyCode > 0) {
            keyCode--;
            row = keyCode / 10;
            col = keyCode % 10;
        } else {
            row = col = 0;
        }
    }
    
private:
    bool _begun;
    bool _interruptsEnabled;
    uint8_t _addr;
    TwoWire* _wire;
    uint8_t _rows = 4;
    uint8_t _cols = 10;
    std::vector<uint8_t> _keyQueue;
};

#endif // ADAFRUIT_TCA8418_H
