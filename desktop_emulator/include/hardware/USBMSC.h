/**
 * @file USBMSC.h
 * @brief USB Mass Storage Class mock for desktop emulator
 */

#ifndef USBMSC_H
#define USBMSC_H

#include "pocketmage/pocketmage_compat.h"

class USBMSC {
public:
    USBMSC() : _started(false) {}
    
    bool begin(uint32_t block_count, uint16_t block_size) {
        _blockCount = block_count;
        _blockSize = block_size;
        _started = true;
        return true;
    }
    
    void end() {
        _started = false;
    }
    
    void vendorID(const char* vid) { _vendorID = vid; }
    void productID(const char* pid) { _productID = pid; }
    void productRevision(const char* rev) { _productRev = rev; }
    
    void onRead(int32_t (*cb)(uint32_t, uint32_t, void*, uint32_t)) { _readCb = cb; }
    void onWrite(int32_t (*cb)(uint32_t, uint32_t, uint8_t*, uint32_t)) { _writeCb = cb; }
    void onStartStop(bool (*cb)(uint8_t, bool, bool)) { _startStopCb = cb; }
    
    void mediaPresent(bool present) { _mediaPresent = present; }
    bool isStarted() const { return _started; }
    
private:
    bool _started;
    bool _mediaPresent = true;
    uint32_t _blockCount = 0;
    uint16_t _blockSize = 512;
    std::string _vendorID;
    std::string _productID;
    std::string _productRev;
    
    int32_t (*_readCb)(uint32_t, uint32_t, void*, uint32_t) = nullptr;
    int32_t (*_writeCb)(uint32_t, uint32_t, uint8_t*, uint32_t) = nullptr;
    bool (*_startStopCb)(uint8_t, bool, bool) = nullptr;
};

#endif // USBMSC_H
