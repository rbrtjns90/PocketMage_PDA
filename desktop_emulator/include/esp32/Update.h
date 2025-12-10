/**
 * @file Update.h
 * @brief ESP32 OTA Update library mock for desktop emulator
 */

#ifndef UPDATE_H
#define UPDATE_H

#include "pocketmage/pocketmage_compat.h"
#include "storage/FS.h"

// Forward declarations
class Stream;
class Print {
public:
    virtual size_t write(uint8_t) = 0;
    virtual size_t write(const uint8_t* buffer, size_t size) { 
        size_t n = 0;
        while (size--) n += write(*buffer++);
        return n;
    }
    size_t println(const char* s) { return write((const uint8_t*)s, strlen(s)) + write('\n'); }
    size_t println() { return write('\n'); }
};

class Stream : public Print {
public:
    virtual int available() = 0;
    virtual int read() = 0;
    virtual int peek() = 0;
};

// Update modes
#define U_FLASH 0
#define U_SPIFFS 1

class UpdateClass {
public:
    UpdateClass() {}
    
    bool begin(size_t size = 0, int command = U_FLASH) {
        _size = size;
        _written = 0;
        _error = 0;
        return true;
    }
    
    size_t write(const uint8_t* data, size_t len) {
        _written += len;
        return len;
    }
    
    size_t writeStream(Stream& data) {
        // Stub
        return 0;
    }
    
    bool end(bool evenIfRemaining = false) {
        return _error == 0;
    }
    
    void abort() {
        _error = 1;
    }
    
    bool isRunning() { return _written > 0 && _written < _size; }
    bool isFinished() { return _written >= _size; }
    size_t size() { return _size; }
    size_t progress() { return _written; }
    size_t remaining() { return _size - _written; }
    
    bool hasError() { return _error != 0; }
    int getError() { return _error; }
    String errorString() { return _error ? "Update error" : ""; }
    
    void printError(Print& out) {
        if (_error) out.println("Update error");
    }
    
    bool canRollBack() { return false; }
    bool rollBack() { return false; }
    
    void onProgress(void (*callback)(size_t, size_t)) {
        _progressCb = callback;
    }
    
private:
    size_t _size = 0;
    size_t _written = 0;
    int _error = 0;
    void (*_progressCb)(size_t, size_t) = nullptr;
};

extern UpdateClass Update;

#endif // UPDATE_H
