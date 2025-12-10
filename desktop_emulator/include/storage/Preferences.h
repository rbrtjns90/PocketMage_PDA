/**
 * @file Preferences.h
 * @brief ESP32 Preferences (NVS) mock for desktop emulator
 * 
 * Stores preferences in a JSON file in the data directory.
 */

#ifndef PREFERENCES_H
#define PREFERENCES_H

#include "pocketmage/pocketmage_compat.h"
#include <map>
#include <fstream>

class Preferences {
public:
    Preferences() : _started(false) {}
    
    bool begin(const char* name, bool readOnly = false, const char* partition_label = nullptr) {
        _namespace = name;
        _readOnly = readOnly;
        _started = true;
        loadFromFile();
        return true;
    }
    
    void end() {
        if (_started && !_readOnly) {
            saveToFile();
        }
        _started = false;
    }
    
    bool clear() {
        _data.clear();
        return true;
    }
    
    bool remove(const char* key) {
        _data.erase(key);
        return true;
    }
    
    // Put methods
    size_t putChar(const char* key, int8_t value) {
        _data[key] = std::to_string(value);
        return 1;
    }
    
    size_t putUChar(const char* key, uint8_t value) {
        _data[key] = std::to_string(value);
        return 1;
    }
    
    size_t putShort(const char* key, int16_t value) {
        _data[key] = std::to_string(value);
        return 2;
    }
    
    size_t putUShort(const char* key, uint16_t value) {
        _data[key] = std::to_string(value);
        return 2;
    }
    
    size_t putInt(const char* key, int32_t value) {
        _data[key] = std::to_string(value);
        return 4;
    }
    
    size_t putUInt(const char* key, uint32_t value) {
        _data[key] = std::to_string(value);
        return 4;
    }
    
    size_t putLong(const char* key, int32_t value) {
        return putInt(key, value);
    }
    
    size_t putULong(const char* key, uint32_t value) {
        return putUInt(key, value);
    }
    
    size_t putLong64(const char* key, int64_t value) {
        _data[key] = std::to_string(value);
        return 8;
    }
    
    size_t putULong64(const char* key, uint64_t value) {
        _data[key] = std::to_string(value);
        return 8;
    }
    
    size_t putFloat(const char* key, float value) {
        _data[key] = std::to_string(value);
        return 4;
    }
    
    size_t putDouble(const char* key, double value) {
        _data[key] = std::to_string(value);
        return 8;
    }
    
    size_t putBool(const char* key, bool value) {
        _data[key] = value ? "1" : "0";
        return 1;
    }
    
    size_t putString(const char* key, const char* value) {
        _data[key] = value ? value : "";
        return strlen(value ? value : "");
    }
    
    size_t putString(const char* key, const String& value) {
        return putString(key, value.c_str());
    }
    
    size_t putBytes(const char* key, const void* value, size_t len) {
        // Store as hex string
        const uint8_t* bytes = static_cast<const uint8_t*>(value);
        std::string hex;
        for (size_t i = 0; i < len; i++) {
            char buf[3];
            snprintf(buf, sizeof(buf), "%02X", bytes[i]);
            hex += buf;
        }
        _data[key] = hex;
        return len;
    }
    
    // Get methods
    int8_t getChar(const char* key, int8_t defaultValue = 0) {
        auto it = _data.find(key);
        if (it != _data.end()) {
            try { return static_cast<int8_t>(std::stoi(it->second)); }
            catch (...) {}
        }
        return defaultValue;
    }
    
    uint8_t getUChar(const char* key, uint8_t defaultValue = 0) {
        auto it = _data.find(key);
        if (it != _data.end()) {
            try { return static_cast<uint8_t>(std::stoul(it->second)); }
            catch (...) {}
        }
        return defaultValue;
    }
    
    int16_t getShort(const char* key, int16_t defaultValue = 0) {
        auto it = _data.find(key);
        if (it != _data.end()) {
            try { return static_cast<int16_t>(std::stoi(it->second)); }
            catch (...) {}
        }
        return defaultValue;
    }
    
    uint16_t getUShort(const char* key, uint16_t defaultValue = 0) {
        auto it = _data.find(key);
        if (it != _data.end()) {
            try { return static_cast<uint16_t>(std::stoul(it->second)); }
            catch (...) {}
        }
        return defaultValue;
    }
    
    int32_t getInt(const char* key, int32_t defaultValue = 0) {
        auto it = _data.find(key);
        if (it != _data.end()) {
            try { return std::stoi(it->second); }
            catch (...) {}
        }
        return defaultValue;
    }
    
    uint32_t getUInt(const char* key, uint32_t defaultValue = 0) {
        auto it = _data.find(key);
        if (it != _data.end()) {
            try { return static_cast<uint32_t>(std::stoul(it->second)); }
            catch (...) {}
        }
        return defaultValue;
    }
    
    int32_t getLong(const char* key, int32_t defaultValue = 0) {
        return getInt(key, defaultValue);
    }
    
    uint32_t getULong(const char* key, uint32_t defaultValue = 0) {
        return getUInt(key, defaultValue);
    }
    
    int64_t getLong64(const char* key, int64_t defaultValue = 0) {
        auto it = _data.find(key);
        if (it != _data.end()) {
            try { return std::stoll(it->second); }
            catch (...) {}
        }
        return defaultValue;
    }
    
    uint64_t getULong64(const char* key, uint64_t defaultValue = 0) {
        auto it = _data.find(key);
        if (it != _data.end()) {
            try { return std::stoull(it->second); }
            catch (...) {}
        }
        return defaultValue;
    }
    
    float getFloat(const char* key, float defaultValue = 0.0f) {
        auto it = _data.find(key);
        if (it != _data.end()) {
            try { return std::stof(it->second); }
            catch (...) {}
        }
        return defaultValue;
    }
    
    double getDouble(const char* key, double defaultValue = 0.0) {
        auto it = _data.find(key);
        if (it != _data.end()) {
            try { return std::stod(it->second); }
            catch (...) {}
        }
        return defaultValue;
    }
    
    bool getBool(const char* key, bool defaultValue = false) {
        auto it = _data.find(key);
        if (it != _data.end()) {
            return it->second == "1" || it->second == "true";
        }
        return defaultValue;
    }
    
    String getString(const char* key, const String& defaultValue = String()) {
        auto it = _data.find(key);
        if (it != _data.end()) {
            return String(it->second.c_str());
        }
        return defaultValue;
    }
    
    size_t getString(const char* key, char* value, size_t maxLen) {
        auto it = _data.find(key);
        if (it != _data.end()) {
            strncpy(value, it->second.c_str(), maxLen - 1);
            value[maxLen - 1] = '\0';
            return std::min(it->second.length(), maxLen - 1);
        }
        value[0] = '\0';
        return 0;
    }
    
    size_t getBytes(const char* key, void* buf, size_t maxLen) {
        auto it = _data.find(key);
        if (it != _data.end()) {
            // Decode hex string
            uint8_t* bytes = static_cast<uint8_t*>(buf);
            size_t len = std::min(it->second.length() / 2, maxLen);
            for (size_t i = 0; i < len; i++) {
                std::string hex = it->second.substr(i * 2, 2);
                bytes[i] = static_cast<uint8_t>(std::stoul(hex, nullptr, 16));
            }
            return len;
        }
        return 0;
    }
    
    bool isKey(const char* key) {
        return _data.find(key) != _data.end();
    }
    
private:
    bool _started;
    bool _readOnly;
    std::string _namespace;
    std::map<std::string, std::string> _data;
    
    std::string getFilePath() {
        return "./data/sys/prefs_" + _namespace + ".txt";
    }
    
    void loadFromFile() {
        std::ifstream file(getFilePath());
        if (file.is_open()) {
            std::string line;
            while (std::getline(file, line)) {
                size_t pos = line.find('=');
                if (pos != std::string::npos) {
                    std::string key = line.substr(0, pos);
                    std::string value = line.substr(pos + 1);
                    _data[key] = value;
                }
            }
        }
    }
    
    void saveToFile() {
        std::ofstream file(getFilePath());
        if (file.is_open()) {
            for (const auto& pair : _data) {
                file << pair.first << "=" << pair.second << "\n";
            }
        }
    }
};

#endif // PREFERENCES_H
