/**
 * @file pocketmage_compat.h
 * @brief Arduino/ESP32 compatibility layer for desktop emulator
 * 
 * This header provides mock implementations of Arduino and ESP32 APIs
 * to allow PocketMage code to compile and run on desktop platforms.
 */

#ifndef POCKETMAGE_COMPAT_H
#define POCKETMAGE_COMPAT_H

// Ensure DESKTOP_EMULATOR is defined
#ifndef DESKTOP_EMULATOR
#define DESKTOP_EMULATOR
#endif

// Debug output control
#define EMULATOR_DEBUG_OUTPUT false
#if EMULATOR_DEBUG_OUTPUT
#define DEBUG_PRINT(x) std::cout << x << std::endl
#define DEBUG_LOG(tag, msg) std::cout << "[" << tag << "] " << msg << std::endl
#else
#define DEBUG_PRINT(x) do {} while(0)
#define DEBUG_LOG(tag, msg) do {} while(0)
#endif

// Standard library includes
#include <iostream>
#include <fstream>
#include <filesystem>
#include <memory>
#include <cstdint>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <cstdarg>
#include <cmath>
#include <string>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <vector>
#include <functional>

// Include esp_log.h for logging macros
#include "esp32/esp_log.h"

// Forward declarations
class DesktopDisplay;
extern DesktopDisplay* g_display;

// ============================================================================
// Arduino String Class
// ============================================================================
class String {
private:
    std::string data;
public:
    String() {}
    String(const char* str) : data(str ? str : "") {}
    String(const std::string& str) : data(str) {}
    String(char c) : data(1, c) {}
    String(int value) : data(std::to_string(value)) {}
    String(long value) : data(std::to_string(value)) {}
    String(unsigned int value) : data(std::to_string(value)) {}
    String(unsigned long value) : data(std::to_string(value)) {}
    String(float value) : data(std::to_string(value)) {}
    String(double value) : data(std::to_string(value)) {}
    String(float value, int decimalPlaces) {
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(decimalPlaces) << value;
        data = ss.str();
    }
    
    // Accessors
    const char* c_str() const { return data.c_str(); }
    size_t length() const { return data.length(); }
    char operator[](size_t index) const { return data[index]; }
    char& operator[](size_t index) { return data[index]; }
    
    // Implicit conversion to const char* for printf compatibility
    operator const char*() const { return data.c_str(); }
    
    // Operators
    String operator+(const String& other) const { return String(data + other.data); }
    String operator+(const char* str) const { return String(data + (str ? str : "")); }
    String operator+(char c) const { return String(data + c); }
    String& operator+=(const String& other) { data += other.data; return *this; }
    String& operator+=(const char* str) { if(str) data += str; return *this; }
    String& operator+=(char c) { data += c; return *this; }
    
    bool operator==(const String& other) const { return data == other.data; }
    bool operator==(const char* str) const { return data == (str ? str : ""); }
    bool operator!=(const String& other) const { return data != other.data; }
    bool operator!=(const char* str) const { return data != (str ? str : ""); }
    bool operator<(const String& other) const { return data < other.data; }
    bool operator>(const String& other) const { return data > other.data; }
    bool operator<=(const String& other) const { return data <= other.data; }
    bool operator>=(const String& other) const { return data >= other.data; }
    
    // String methods
    String substring(size_t start, size_t end = std::string::npos) const {
        if (start >= data.length()) return String("");
        if (end == std::string::npos) return String(data.substr(start));
        return String(data.substr(start, end - start));
    }
    
    int indexOf(char c, size_t start = 0) const {
        size_t pos = data.find(c, start);
        return (pos == std::string::npos) ? -1 : (int)pos;
    }
    
    int indexOf(const String& str, size_t start = 0) const {
        size_t pos = data.find(str.data, start);
        return (pos == std::string::npos) ? -1 : (int)pos;
    }
    
    int lastIndexOf(char c) const {
        size_t pos = data.find_last_of(c);
        return pos == std::string::npos ? -1 : static_cast<int>(pos);
    }
    
    void replace(const String& from, const String& to) {
        size_t pos = 0;
        while ((pos = data.find(from.data, pos)) != std::string::npos) {
            data.replace(pos, from.data.length(), to.data);
            pos += to.data.length();
        }
    }
    
    String& trim() {
        data.erase(data.begin(), std::find_if(data.begin(), data.end(), [](unsigned char ch) {
            return !std::isspace(ch);
        }));
        data.erase(std::find_if(data.rbegin(), data.rend(), [](unsigned char ch) {
            return !std::isspace(ch);
        }).base(), data.end());
        return *this;
    }
    
    bool endsWith(const String& suffix) const {
        if (suffix.length() > data.length()) return false;
        return data.compare(data.length() - suffix.length(), suffix.length(), suffix.data) == 0;
    }
    
    bool startsWith(const String& prefix) const {
        if (prefix.length() > data.length()) return false;
        return data.compare(0, prefix.length(), prefix.data) == 0;
    }
    
    bool isEmpty() const { return data.empty(); }
    
    int toInt() const {
        try { return std::stoi(data); }
        catch (...) { return 0; }
    }
    
    float toFloat() const {
        try { return std::stof(data); }
        catch (...) { return 0.0f; }
    }
    
    void toLowerCase() {
        std::transform(data.begin(), data.end(), data.begin(), ::tolower);
    }
    
    void toUpperCase() {
        std::transform(data.begin(), data.end(), data.begin(), ::toupper);
    }
    
    char charAt(size_t index) const {
        return index < data.size() ? data[index] : '\0';
    }
    
    bool equals(const String& other) const { return data == other.data; }
    
    bool equalsIgnoreCase(const String& other) const {
        if (data.length() != other.data.length()) return false;
        for (size_t i = 0; i < data.length(); i++) {
            if (std::tolower(data[i]) != std::tolower(other.data[i])) return false;
        }
        return true;
    }
    
    int compareTo(const String& other) const {
        return data.compare(other.data);
    }
    
    void remove(size_t index, size_t count = 1) { 
        if (index < data.size()) data.erase(index, count); 
    }
    
    void reserve(size_t size) { data.reserve(size); }
    
    // For std::string compatibility
    const std::string& toStdString() const { return data; }
};

// String stream operator
inline std::ostream& operator<<(std::ostream& os, const String& s) {
    os << s.c_str();
    return os;
}

// String concatenation with const char* on left
inline String operator+(const char* lhs, const String& rhs) {
    return String(lhs) + rhs;
}

// ============================================================================
// Arduino Types and Constants
// ============================================================================
typedef bool boolean;
typedef uint8_t byte;

#define HIGH 1
#define LOW 0
#define INPUT 0
#define OUTPUT 1
#define INPUT_PULLUP 0x02
#define INPUT_PULLDOWN 0x03
#define PROGMEM
#define F(x) x
#define PSTR(x) x

// Interrupt modes
#define CHANGE 1
#define FALLING 2
#define RISING 3

// File modes
#define FILE_READ "r"
#define FILE_WRITE "w"
#define FILE_APPEND "a"

// ============================================================================
// Arduino Functions
// ============================================================================
unsigned long millis();
unsigned long micros();
void delay(unsigned long ms);
void delayMicroseconds(unsigned int us);
void randomSeed(unsigned long seed);
long random(long max);
long random(long min, long max);
uint32_t esp_random();

void pinMode(uint8_t pin, uint8_t mode);
int digitalRead(uint8_t pin);
void digitalWrite(uint8_t pin, uint8_t value);
int analogRead(uint8_t pin);
void analogWrite(uint8_t pin, int value);

void attachInterrupt(uint8_t pin, void (*ISR)(void), int mode);
void detachInterrupt(uint8_t pin);
int digitalPinToInterrupt(uint8_t pin);

// ============================================================================
// Arduino Utility Functions
// ============================================================================
inline bool isDigit(char c) { return std::isdigit(c); }
inline bool isAlpha(char c) { return std::isalpha(c); }
inline bool isAlphaNumeric(char c) { return std::isalnum(c); }

inline long map(long x, long in_min, long in_max, long out_min, long out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

inline long constrain(long x, long a, long b) {
    if (x < a) return a;
    if (x > b) return b;
    return x;
}

template<typename T>
inline const T& min(const T& a, const T& b) {
    return (a < b) ? a : b;
}

template<typename T>
inline const T& max(const T& a, const T& b) {
    return (a > b) ? a : b;
}

inline uint32_t getCpuFrequencyMhz() { return 240; }
void setCpuFrequencyMhz(uint32_t freq);

// ============================================================================
// Serial Mock
// ============================================================================
class SerialClass {
public:
    void begin(int baud) {}
    void end() {}
    void println(const String& str) { std::cout << str.c_str() << std::endl; }
    void println(const char* str) { std::cout << (str ? str : "") << std::endl; }
    void println(int val) { std::cout << val << std::endl; }
    void println(float val) { std::cout << val << std::endl; }
    void println() { std::cout << std::endl; }
    void print(const String& str) { std::cout << str.c_str(); }
    void print(const char* str) { std::cout << (str ? str : ""); }
    void print(int val) { std::cout << val; }
    void print(float val, int precision = 2) { 
        std::cout << std::fixed << std::setprecision(precision) << val; 
    }
    void printf(const char* format, ...) {
        va_list args;
        va_start(args, format);
        vprintf(format, args);
        va_end(args);
    }
    void write(int val) { std::cout << (char)val; }
    void flush() { std::cout.flush(); }
    int available() { return 0; }
    int read() { return -1; }
};
extern SerialClass Serial;

// ============================================================================
// FreeRTOS Types and Functions
// ============================================================================
typedef void* TaskHandle_t;
typedef void (*TaskFunction_t)(void*);
typedef int BaseType_t;
typedef unsigned int UBaseType_t;
typedef uint32_t TickType_t;

#define portTICK_PERIOD_MS 1
#define pdMS_TO_TICKS(ms) (ms)
#define pdPASS 1
#define pdFAIL 0
#define pdTRUE 1
#define pdFALSE 0

BaseType_t xTaskCreatePinnedToCore(
    TaskFunction_t pvTaskCode,
    const char* const pcName,
    const uint32_t usStackDepth,
    void* const pvParameters,
    UBaseType_t uxPriority,
    TaskHandle_t* const pvCreatedTask,
    const BaseType_t xCoreID
);

void vTaskDelay(TickType_t ticks);
void vTaskDelete(TaskHandle_t handle);
void yield();

// ============================================================================
// ESP32 Sleep Functions
// ============================================================================
typedef enum {
    ESP_SLEEP_WAKEUP_UNDEFINED = 0,
    ESP_SLEEP_WAKEUP_EXT0,
    ESP_SLEEP_WAKEUP_EXT1,
    ESP_SLEEP_WAKEUP_TIMER,
    ESP_SLEEP_WAKEUP_TOUCHPAD,
    ESP_SLEEP_WAKEUP_ULP
} esp_sleep_wakeup_cause_t;

typedef enum {
    GPIO_NUM_0 = 0, GPIO_NUM_1, GPIO_NUM_2, GPIO_NUM_3, GPIO_NUM_4,
    GPIO_NUM_5, GPIO_NUM_6, GPIO_NUM_7, GPIO_NUM_8, GPIO_NUM_9,
    GPIO_NUM_10, GPIO_NUM_11, GPIO_NUM_12, GPIO_NUM_13, GPIO_NUM_14,
    GPIO_NUM_15, GPIO_NUM_16, GPIO_NUM_17, GPIO_NUM_18, GPIO_NUM_19,
    GPIO_NUM_20, GPIO_NUM_21, GPIO_NUM_22, GPIO_NUM_23, GPIO_NUM_24,
    GPIO_NUM_25, GPIO_NUM_26, GPIO_NUM_27, GPIO_NUM_28, GPIO_NUM_29,
    GPIO_NUM_30, GPIO_NUM_31, GPIO_NUM_32, GPIO_NUM_33, GPIO_NUM_34,
    GPIO_NUM_35, GPIO_NUM_36, GPIO_NUM_37, GPIO_NUM_38, GPIO_NUM_39,
    GPIO_NUM_MAX = 40
} gpio_num_t;

void esp_sleep_enable_ext0_wakeup(gpio_num_t gpio_num, int level);
esp_sleep_wakeup_cause_t esp_sleep_get_wakeup_cause();
void esp_deep_sleep_start();
void esp_restart();

// ============================================================================
// ESP Error Types
// ============================================================================
typedef int esp_err_t;
#define ESP_OK 0
#define ESP_FAIL -1

// ============================================================================
// Display Constants
// ============================================================================
#define GxEPD_WHITE 0xFF
#define GxEPD_BLACK 0x00

// ============================================================================
// Musical Note Constants (for buzzer)
// ============================================================================
#define NOTE_B0  31
#define NOTE_C1  33
#define NOTE_CS1 35
#define NOTE_D1  37
#define NOTE_DS1 39
#define NOTE_E1  41
#define NOTE_F1  44
#define NOTE_FS1 46
#define NOTE_G1  49
#define NOTE_GS1 52
#define NOTE_A1  55
#define NOTE_AS1 58
#define NOTE_B1  62
#define NOTE_C2  65
#define NOTE_CS2 69
#define NOTE_D2  73
#define NOTE_DS2 78
#define NOTE_E2  82
#define NOTE_F2  87
#define NOTE_FS2 93
#define NOTE_G2  98
#define NOTE_GS2 104
#define NOTE_A2  110
#define NOTE_AS2 117
#define NOTE_B2  123
#define NOTE_C3  131
#define NOTE_CS3 139
#define NOTE_D3  147
#define NOTE_DS3 156
#define NOTE_E3  165
#define NOTE_F3  175
#define NOTE_FS3 185
#define NOTE_G3  196
#define NOTE_GS3 208
#define NOTE_A3  220
#define NOTE_AS3 233
#define NOTE_B3  247
#define NOTE_C4  262
#define NOTE_CS4 277
#define NOTE_D4  294
#define NOTE_DS4 311
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_FS4 370
#define NOTE_G4  392
#define NOTE_GS4 415
#define NOTE_A4  440
#define NOTE_AS4 466
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_CS5 554
#define NOTE_D5  587
#define NOTE_DS5 622
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_FS5 740
#define NOTE_G5  784
#define NOTE_GS5 831
#define NOTE_A5  880
#define NOTE_AS5 932
#define NOTE_B5  988
#define NOTE_C6  1047
#define NOTE_CS6 1109
#define NOTE_D6  1175
#define NOTE_DS6 1245
#define NOTE_E6  1319
#define NOTE_F6  1397
#define NOTE_FS6 1480
#define NOTE_G6  1568
#define NOTE_GS6 1661
#define NOTE_A6  1760
#define NOTE_AS6 1865
#define NOTE_B6  1976
#define NOTE_C7  2093
#define NOTE_CS7 2217
#define NOTE_D7  2349
#define NOTE_DS7 2489
#define NOTE_E7  2637
#define NOTE_F7  2794
#define NOTE_FS7 2960
#define NOTE_G7  3136
#define NOTE_GS7 3322
#define NOTE_A7  3520
#define NOTE_AS7 3729
#define NOTE_B7  3951
#define NOTE_C8  4186
#define NOTE_CS8 4435
#define NOTE_D8  4699
#define NOTE_DS8 4978

// Arduino DEC constant
#define DEC 10
#define HEX 16
#define BIN 2

// Additional note definitions (octave 8)
#define NOTE_A8  7040
#define NOTE_AS8 7459
#define NOTE_B8  7902

// Type aliases
typedef unsigned long ulong;
typedef unsigned int uint;

// IRAM_ATTR is ESP32-specific
#ifndef IRAM_ATTR
#define IRAM_ATTR
#endif

#endif // POCKETMAGE_COMPAT_H
