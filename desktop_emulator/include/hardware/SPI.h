/**
 * @file SPI.h
 * @brief SPI library mock for desktop emulator
 */

#ifndef SPI_H
#define SPI_H

#include "pocketmage/pocketmage_compat.h"

#define SPI_MODE0 0
#define SPI_MODE1 1
#define SPI_MODE2 2
#define SPI_MODE3 3

#define MSBFIRST 1
#define LSBFIRST 0

class SPISettings {
public:
    SPISettings() : _clock(1000000), _bitOrder(MSBFIRST), _dataMode(SPI_MODE0) {}
    SPISettings(uint32_t clock, uint8_t bitOrder, uint8_t dataMode)
        : _clock(clock), _bitOrder(bitOrder), _dataMode(dataMode) {}
    
    uint32_t _clock;
    uint8_t _bitOrder;
    uint8_t _dataMode;
};

class SPIClass {
public:
    SPIClass() {}
    
    void begin(int8_t sck = -1, int8_t miso = -1, int8_t mosi = -1, int8_t ss = -1) {}
    void end() {}
    
    void beginTransaction(SPISettings settings) {}
    void endTransaction() {}
    
    void setBitOrder(uint8_t bitOrder) {}
    void setDataMode(uint8_t dataMode) {}
    void setClockDivider(uint8_t clockDiv) {}
    void setFrequency(uint32_t freq) {}
    
    uint8_t transfer(uint8_t data) { return data; }
    uint16_t transfer16(uint16_t data) { return data; }
    void transfer(void* buf, size_t count) {}
    
    void write(uint8_t data) {}
    void write16(uint16_t data) {}
    void write32(uint32_t data) {}
    void writeBytes(const uint8_t* data, uint32_t size) {}
    void writePattern(const uint8_t* data, uint8_t size, uint32_t repeat) {}
};

extern SPIClass SPI;

#endif // SPI_H
