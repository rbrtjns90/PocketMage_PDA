/**
 * @file Adafruit_MPR121.h
 * @brief MPR121 capacitive touch sensor mock for desktop emulator
 */

#ifndef ADAFRUIT_MPR121_H
#define ADAFRUIT_MPR121_H

#include "pocketmage/pocketmage_compat.h"
#include "hardware/Wire.h"

#define MPR121_I2CADDR_DEFAULT 0x5A

// MPR121 Register addresses
#define MPR121_TOUCHSTATUS_L 0x00
#define MPR121_TOUCHSTATUS_H 0x01
#define MPR121_FILTDATA_0L   0x04
#define MPR121_FILTDATA_0H   0x05
#define MPR121_BASELINE_0    0x1E
#define MPR121_MHDR          0x2B
#define MPR121_NHDR          0x2C
#define MPR121_NCLR          0x2D
#define MPR121_FDLR          0x2E
#define MPR121_MHDF          0x2F
#define MPR121_NHDF          0x30
#define MPR121_NCLF          0x31
#define MPR121_FDLF          0x32
#define MPR121_NHDT          0x33
#define MPR121_NCLT          0x34
#define MPR121_FDLT          0x35
#define MPR121_TOUCHTH_0     0x41
#define MPR121_RELEASETH_0   0x42
#define MPR121_DEBOUNCE      0x5B
#define MPR121_CONFIG1       0x5C
#define MPR121_CONFIG2       0x5D
#define MPR121_CHARGECURR_0  0x5F
#define MPR121_CHARGETIME_1  0x6C
#define MPR121_ECR           0x5E
#define MPR121_AUTOCONFIG0   0x7B
#define MPR121_AUTOCONFIG1   0x7C
#define MPR121_UPLIMIT       0x7D
#define MPR121_LOWLIMIT      0x7E
#define MPR121_TARGETLIMIT   0x7F
#define MPR121_GPIODIR       0x76
#define MPR121_GPIOEN        0x77
#define MPR121_GPIOSET       0x78
#define MPR121_GPIOCLR       0x79
#define MPR121_GPIOTOGGLE    0x7A
#define MPR121_SOFTRESET     0x80

class Adafruit_MPR121 {
public:
    Adafruit_MPR121() : _begun(false) {}
    
    bool begin(uint8_t addr = MPR121_I2CADDR_DEFAULT, TwoWire* wire = &Wire,
               uint8_t touchThreshold = 12, uint8_t releaseThreshold = 6) {
        _addr = addr;
        _wire = wire;
        _touchThreshold = touchThreshold;
        _releaseThreshold = releaseThreshold;
        _begun = true;
        return true;
    }
    
    // Touch status
    uint16_t touched() { return _touchState; }
    
    // Filtered data for each electrode
    uint16_t filteredData(uint8_t electrode) {
        if (electrode >= 12) return 0;
        return _filteredData[electrode];
    }
    
    // Baseline data for each electrode
    uint16_t baselineData(uint8_t electrode) {
        if (electrode >= 12) return 0;
        return _baselineData[electrode];
    }
    
    // Threshold setting
    void setThresholds(uint8_t touch, uint8_t release) {
        _touchThreshold = touch;
        _releaseThreshold = release;
    }
    
    // Register access
    uint8_t readRegister8(uint8_t reg) { return 0; }
    uint16_t readRegister16(uint8_t reg) { return 0; }
    void writeRegister(uint8_t reg, uint8_t value) {}
    
    // For emulator: simulate touch
    void simulateTouch(uint8_t electrode, bool touched) {
        if (electrode >= 12) return;
        if (touched) {
            _touchState |= (1 << electrode);
        } else {
            _touchState &= ~(1 << electrode);
        }
    }
    
    void simulateSlider(int position) {
        // Position 0-100 maps to electrodes 0-11
        _touchState = 0;
        if (position >= 0 && position <= 100) {
            int electrode = (position * 11) / 100;
            _touchState = (1 << electrode);
        }
    }
    
private:
    bool _begun;
    uint8_t _addr;
    TwoWire* _wire;
    uint8_t _touchThreshold;
    uint8_t _releaseThreshold;
    uint16_t _touchState = 0;
    uint16_t _filteredData[12] = {0};
    uint16_t _baselineData[12] = {200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200};
};

#endif // ADAFRUIT_MPR121_H
