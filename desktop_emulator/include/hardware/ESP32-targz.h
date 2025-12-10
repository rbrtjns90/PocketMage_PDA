/**
 * @file ESP32-targz.h
 * @brief ESP32-targz library mock for desktop emulator
 * 
 * This provides stub implementations for tar/gz file handling.
 * Full implementation would require linking against zlib/libtar.
 */

#ifndef ESP32_TARGZ_H
#define ESP32_TARGZ_H

#include "pocketmage/pocketmage_compat.h"
#include "storage/FS.h"

// Callback types
typedef void (*tarProgressCallback)(size_t current, size_t total);
typedef void (*tarMessageCallback)(const char* message);

// TarUnpacker class stub
class TarUnpacker {
public:
    TarUnpacker() {}
    
    void setTarProgressCallback(tarProgressCallback cb) { _progressCb = cb; }
    void setTarMessageCallback(tarMessageCallback cb) { _messageCb = cb; }
    void setTarVerify(bool verify) { _verify = verify; }
    
    bool tarExpander(fs::FS& sourceFS, const char* sourcePath, 
                     fs::FS& destFS, const char* destPath) {
        // Stub - would need actual tar implementation
        if (_messageCb) {
            _messageCb("TAR extraction not implemented in emulator");
        }
        return false;
    }
    
private:
    tarProgressCallback _progressCb = nullptr;
    tarMessageCallback _messageCb = nullptr;
    bool _verify = false;
};

// GzUnpacker class stub
class GzUnpacker {
public:
    GzUnpacker() {}
    
    void setGzProgressCallback(tarProgressCallback cb) { _progressCb = cb; }
    void setGzMessageCallback(tarMessageCallback cb) { _messageCb = cb; }
    
    bool gzExpander(fs::FS& sourceFS, const char* sourcePath,
                    fs::FS& destFS, const char* destPath) {
        // Stub - would need actual gzip implementation
        if (_messageCb) {
            _messageCb("GZIP extraction not implemented in emulator");
        }
        return false;
    }
    
private:
    tarProgressCallback _progressCb = nullptr;
    tarMessageCallback _messageCb = nullptr;
};

// TarGzUnpacker combines both
class TarGzUnpacker : public TarUnpacker, public GzUnpacker {
public:
    TarGzUnpacker() {}
    
    bool tarGzExpander(fs::FS& sourceFS, const char* sourcePath,
                       fs::FS& destFS, const char* destPath) {
        // Stub - would need actual tar.gz implementation
        return false;
    }
};

#endif // ESP32_TARGZ_H
