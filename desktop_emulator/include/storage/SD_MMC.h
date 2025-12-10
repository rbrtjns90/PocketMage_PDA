/**
 * @file SD_MMC.h
 * @brief SD/MMC card library mock for desktop emulator
 * 
 * Uses std::filesystem to provide file operations on the local data/ directory.
 */

#ifndef SD_MMC_H
#define SD_MMC_H

#include "pocketmage/pocketmage_compat.h"
#include <fstream>
#include <filesystem>

// Card types
typedef enum {
    CARD_NONE = 0,
    CARD_MMC,
    CARD_SD,
    CARD_SDHC,
    CARD_UNKNOWN
} sdcard_type_t;

// Forward declaration
class File;

// ============================================================================
// File Class
// ============================================================================
class File {
public:
    File();
    File(const std::string& path, const std::string& mode);
    ~File();
    
    // Move semantics (no copy)
    File(File&& other) noexcept;
    File& operator=(File&& other) noexcept;
    File(const File&) = delete;
    File& operator=(const File&) = delete;
    
    // Validity check
    operator bool() const { return isOpen; }
    
    // File operations
    void close();
    size_t write(const uint8_t* data, size_t len);
    size_t write(uint8_t data) { return write(&data, 1); }
    size_t write(const char* str) { return write((const uint8_t*)str, strlen(str)); }
    size_t write(const String& str);
    
    int read();
    size_t read(uint8_t* buf, size_t size);
    String readString();
    String readStringUntil(char terminator);
    
    bool available();
    void seek(size_t pos);
    size_t position();
    size_t size();
    
    bool isDirectory();
    File openNextFile();
    void rewindDirectory();
    String name();
    String path();
    
    bool print(const char* msg);
    bool print(const String& msg);
    bool print(int val) { return print(String(val)); }
    bool println(const char* msg);
    bool println(const String& msg);
    bool println() { return println(""); }
    
private:
    std::unique_ptr<std::ifstream> inFile;
    std::unique_ptr<std::ofstream> outFile;
    bool isOpen;
    bool isDir;
    std::string filePath;
    std::vector<std::string> dirEntries;
    size_t dirIndex;
};

// ============================================================================
// Filesystem Namespace (for compatibility)
// ============================================================================
namespace fs {
    class FS {
    public:
        virtual File open(const char* path, const char* mode = FILE_READ) = 0;
        virtual File open(const String& path, const char* mode = FILE_READ) = 0;
        virtual bool exists(const char* path) = 0;
        virtual bool exists(const String& path) = 0;
        virtual bool remove(const char* path) = 0;
        virtual bool remove(const String& path) = 0;
        virtual bool rename(const char* pathFrom, const char* pathTo) = 0;
        virtual bool rename(const String& pathFrom, const String& pathTo) = 0;
        virtual bool mkdir(const char* path) = 0;
        virtual bool mkdir(const String& path) = 0;
        virtual bool rmdir(const char* path) = 0;
        virtual bool rmdir(const String& path) = 0;
        virtual ~FS() = default;
    };
}

// ============================================================================
// SD_MMC Class
// ============================================================================
class SD_MMCClass : public fs::FS {
public:
    SD_MMCClass() : _mounted(false) {}
    
    bool begin(const char* mountpoint = "/sdcard", bool mode1bit = false, 
               bool format_if_mount_failed = false, int sdmmc_frequency = 0) {
        _mounted = true;
        _mountpoint = mountpoint;
        
        // Create data directory if it doesn't exist
        std::filesystem::create_directories("./data");
        std::filesystem::create_directories("./data/sys");
        std::filesystem::create_directories("./data/journal");
        std::filesystem::create_directories("./data/dict");
        
        return true;
    }
    
    void end() { _mounted = false; }
    
    void setPins(int clk, int cmd, int d0, int d1 = -1, int d2 = -1, int d3 = -1) {}
    
    sdcard_type_t cardType() { return _mounted ? CARD_SDHC : CARD_NONE; }
    uint64_t cardSize() { return 32ULL * 1024 * 1024 * 1024; } // 32GB
    uint64_t totalBytes() { return cardSize(); }
    uint64_t usedBytes() { return 1024 * 1024 * 100; } // 100MB used
    
    // FS interface implementation
    File open(const char* path, const char* mode = FILE_READ) override;
    File open(const String& path, const char* mode = FILE_READ) override;
    bool exists(const char* path) override;
    bool exists(const String& path) override;
    bool remove(const char* path) override;
    bool remove(const String& path) override;
    bool rename(const char* pathFrom, const char* pathTo) override;
    bool rename(const String& pathFrom, const String& pathTo) override;
    bool mkdir(const char* path) override;
    bool mkdir(const String& path) override;
    bool rmdir(const char* path) override;
    bool rmdir(const String& path) override;
    
private:
    bool _mounted;
    std::string _mountpoint;
    
    std::string toLocalPath(const char* path);
};

extern SD_MMCClass SD_MMC;

// Type alias for sdmmc_card_t
typedef struct {
    int dummy;
} sdmmc_card_t;

#endif // SD_MMC_H
