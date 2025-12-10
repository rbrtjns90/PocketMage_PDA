/**
 * @file Wire.h
 * @brief I2C/Wire library mock for desktop emulator
 */

#ifndef WIRE_H
#define WIRE_H

#include "pocketmage/pocketmage_compat.h"

class TwoWire {
public:
    TwoWire() {}
    
    void begin() {}
    void begin(int sda, int scl) {}
    void begin(uint8_t address) {}
    void end() {}
    
    void setClock(uint32_t clock) {}
    
    void beginTransmission(uint8_t address) { _address = address; }
    uint8_t endTransmission(bool sendStop = true) { return 0; }
    
    size_t write(uint8_t data) { return 1; }
    size_t write(const uint8_t* data, size_t length) { return length; }
    
    uint8_t requestFrom(uint8_t address, uint8_t quantity, bool sendStop = true) { 
        return quantity; 
    }
    
    int available() { return 0; }
    int read() { return 0; }
    int peek() { return -1; }
    void flush() {}
    
    void onReceive(void (*callback)(int)) {}
    void onRequest(void (*callback)(void)) {}
    
private:
    uint8_t _address = 0;
};

extern TwoWire Wire;

#endif // WIRE_H
