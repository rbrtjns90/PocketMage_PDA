/**
 * @file hardware_shim.cpp
 * @brief Hardware abstraction layer for desktop emulator
 * 
 * Provides mock implementations of Arduino/ESP32 hardware functions.
 */

#include "pocketmage_compat.h"
#include "Adafruit_GFX.h"
#include "desktop_display_sdl2.h"
#include "oled_service.h"
#include "SD_MMC.h"
#include "Wire.h"
#include "SPI.h"
#include "Preferences.h"
#include "Adafruit_TCA8418.h"
#include "Adafruit_MPR121.h"
#include "RTClib.h"
#include "Buzzer.h"
#include "U8g2lib.h"
#include "USB.h"
#include "esp_log.h"

#include <iostream>
#include <chrono>
#include <thread>
#include <cstdarg>

// ============================================================================
// Global Instances
// ============================================================================

SerialClass Serial;
TwoWire Wire;
SPIClass SPI;
SD_MMCClass SD_MMC;
Buzzer buzzer;
USBCDC USBSerial;

// ESP log level
esp_log_level_t esp_log_level = ESP_LOG_INFO;

// U8g2 fonts (stub data)
const uint8_t u8g2_font_6x10_tf[] = {0};
const uint8_t u8g2_font_ncenB08_tr[] = {0};
const uint8_t u8g2_font_ncenB10_tr[] = {0};
const uint8_t u8g2_font_ncenB12_tr[] = {0};
const uint8_t u8g2_font_ncenB14_tr[] = {0};
const uint8_t u8g2_font_helvB08_tr[] = {0};
const uint8_t u8g2_font_helvB10_tr[] = {0};
const uint8_t u8g2_font_helvB12_tr[] = {0};
const uint8_t u8g2_font_helvR08_tr[] = {0};
const uint8_t u8g2_font_profont12_tr[] = {0};
const uint8_t u8g2_font_profont15_tr[] = {0};
const uint8_t u8g2_font_profont17_tr[] = {0};
const uint8_t u8g2_font_profont22_tr[] = {0};
const uint8_t u8g2_font_t0_11_tf[] = {0};
const uint8_t u8g2_font_t0_12_tf[] = {0};
const uint8_t u8g2_font_t0_13_tf[] = {0};
const uint8_t u8g2_font_t0_14_tf[] = {0};
const uint8_t u8g2_font_t0_15_tf[] = {0};
const uint8_t u8g2_font_t0_16_tf[] = {0};
const uint8_t u8g2_font_5x7_tf[] = {0};
const uint8_t u8g2_font_luBIS18_tf[] = {0};
const uint8_t u8g2_font_luBS18_tf[] = {0};
const uint8_t u8g2_font_luIS18_tf[] = {0};
const uint8_t u8g2_font_lubR18_tf[] = {0};
const uint8_t u8g2_font_7x13B_tf[] = {0};
const uint8_t u8g2_font_7x13_tf[] = {0};
const uint8_t u8g2_font_5x8_tf[] = {0};
const uint8_t u8g2_font_6x12_tf[] = {0};
const uint8_t u8g2_font_9x15_tf[] = {0};
const uint8_t u8g2_font_10x20_tf[] = {0};
const uint8_t u8g2_font_ncenB18_tr[] = {0};
const uint8_t u8g2_font_ncenB24_tr[] = {0};

// GFX fonts - commented out, defined in PocketMage library headers
// const GFXfont Font3x7FixedNum = {nullptr, nullptr, 0, 0, 7};
// const GFXfont Font4x5Fixed = {nullptr, nullptr, 0, 0, 5};
// const GFXfont Font5x7Fixed = {nullptr, nullptr, 0, 0, 7};
// const GFXfont FreeMonoBold9pt7b = {nullptr, nullptr, 0, 0, 9};
// const GFXfont FreeSans9pt7b = {nullptr, nullptr, 0, 0, 9};
// const GFXfont FreeSerif9pt7b = {nullptr, nullptr, 0, 0, 9};
// const GFXfont FreeSerifBold9pt7b = {nullptr, nullptr, 0, 0, 9};
// const GFXfont FreeMono12pt7b = {nullptr, nullptr, 0, 0, 12};
// const GFXfont FreeSans12pt7b = {nullptr, nullptr, 0, 0, 12};
// const GFXfont FreeSerif12pt7b = {nullptr, nullptr, 0, 0, 12};
// const GFXfont FreeMono9pt7b = {nullptr, nullptr, 0, 0, 9};
// const GFXfont FreeSansBold9pt7b = {nullptr, nullptr, 0, 0, 9};
// const GFXfont FreeMonoBold12pt7b = {nullptr, nullptr, 0, 0, 12};
// const GFXfont FreeSansBold12pt7b = {nullptr, nullptr, 0, 0, 12};
// const GFXfont FreeSerifBold12pt7b = {nullptr, nullptr, 0, 0, 12};
// const GFXfont FreeMonoBold18pt7b = {nullptr, nullptr, 0, 0, 18};
// const GFXfont FreeMonoBold24pt7b = {nullptr, nullptr, 0, 0, 24};
// const GFXfont FreeMonoBoldOblique12pt7b = {nullptr, nullptr, 0, 0, 12};
// const GFXfont FreeMonoBoldOblique18pt7b = {nullptr, nullptr, 0, 0, 18};
// const GFXfont FreeMonoBoldOblique24pt7b = {nullptr, nullptr, 0, 0, 24};
// const GFXfont FreeMonoBoldOblique9pt7b = {nullptr, nullptr, 0, 0, 9};
// const GFXfont FreeMonoOblique9pt7b = {nullptr, nullptr, 0, 0, 9};
// const GFXfont FreeSansBold18pt7b = {nullptr, nullptr, 0, 0, 18};
// const GFXfont FreeSansBold24pt7b = {nullptr, nullptr, 0, 0, 24};
// const GFXfont FreeSansBoldOblique12pt7b = {nullptr, nullptr, 0, 0, 12};
// const GFXfont FreeSansBoldOblique18pt7b = {nullptr, nullptr, 0, 0, 18};
// const GFXfont FreeSansBoldOblique24pt7b = {nullptr, nullptr, 0, 0, 24};
// const GFXfont FreeSansBoldOblique9pt7b = {nullptr, nullptr, 0, 0, 9};
// const GFXfont FreeSansOblique9pt7b = {nullptr, nullptr, 0, 0, 9};
// const GFXfont FreeSerifBold18pt7b = {nullptr, nullptr, 0, 0, 18};
// const GFXfont FreeSerifBold24pt7b = {nullptr, nullptr, 0, 0, 24};
// const GFXfont FreeSerifBoldItalic12pt7b = {nullptr, nullptr, 0, 0, 12};
// const GFXfont FreeSerifBoldItalic18pt7b = {nullptr, nullptr, 0, 0, 18};
// const GFXfont FreeSerifBoldItalic24pt7b = {nullptr, nullptr, 0, 0, 24};
// const GFXfont FreeSerifBoldItalic9pt7b = {nullptr, nullptr, 0, 0, 9};
// const GFXfont FreeSerifItalic9pt7b = {nullptr, nullptr, 0, 0, 9};

// ============================================================================
// Timing Functions
// ============================================================================

static auto s_startTime = std::chrono::steady_clock::now();

unsigned long millis() {
    auto now = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(now - s_startTime).count();
}

unsigned long micros() {
    auto now = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::microseconds>(now - s_startTime).count();
}

void delay(unsigned long ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

void delayMicroseconds(unsigned int us) {
    std::this_thread::sleep_for(std::chrono::microseconds(us));
}

// ============================================================================
// Random Functions
// ============================================================================

void randomSeed(unsigned long seed) {
    srand(static_cast<unsigned int>(seed));
}

long random(long max) {
    if (max <= 0) return 0;
    return rand() % max;
}

long random(long min, long max) {
    if (max <= min) return min;
    return min + (rand() % (max - min));
}

uint32_t esp_random() {
    return static_cast<uint32_t>(rand());
}

// ============================================================================
// GPIO Functions
// ============================================================================

void pinMode(uint8_t pin, uint8_t mode) {
    // Mock - no actual GPIO on desktop
}

int digitalRead(uint8_t pin) {
    return LOW;
}

void digitalWrite(uint8_t pin, uint8_t value) {
    // Mock - no actual GPIO on desktop
}

int analogRead(uint8_t pin) {
    // Return a reasonable battery voltage simulation
    return 2048;  // ~50% of 12-bit ADC range
}

void analogWrite(uint8_t pin, int value) {
    // Mock - no actual PWM on desktop
}

// ============================================================================
// Interrupt Functions
// ============================================================================

void attachInterrupt(uint8_t pin, void (*ISR)(void), int mode) {
    // Mock - interrupts handled differently in emulator
}

void detachInterrupt(uint8_t pin) {
    // Mock
}

int digitalPinToInterrupt(uint8_t pin) {
    return pin;
}

// ============================================================================
// CPU Functions
// ============================================================================

void setCpuFrequencyMhz(uint32_t freq) {
    // Mock - no CPU frequency control on desktop
}

// ============================================================================
// FreeRTOS Functions
// ============================================================================

BaseType_t xTaskCreatePinnedToCore(
    TaskFunction_t pvTaskCode,
    const char* const pcName,
    const uint32_t usStackDepth,
    void* const pvParameters,
    UBaseType_t uxPriority,
    TaskHandle_t* const pvCreatedTask,
    const BaseType_t xCoreID
) {
    std::cout << "[FreeRTOS] Task created: " << pcName << std::endl;
    // In emulator, we don't actually create threads for tasks
    // The main loop handles everything
    return pdPASS;
}

void vTaskDelay(TickType_t ticks) {
    delay(ticks);
}

void vTaskDelete(TaskHandle_t handle) {
    // Mock
}

void yield() {
    // Allow other threads to run
    std::this_thread::yield();
}

// ============================================================================
// ESP32 Sleep Functions
// ============================================================================

void esp_sleep_enable_ext0_wakeup(gpio_num_t gpio_num, int level) {
    // Mock
}

esp_sleep_wakeup_cause_t esp_sleep_get_wakeup_cause() {
    return ESP_SLEEP_WAKEUP_UNDEFINED;
}

void esp_deep_sleep_start() {
    std::cout << "[ESP32] Deep sleep requested - exiting emulator" << std::endl;
    exit(0);
}

void esp_restart() {
    std::cout << "[ESP32] Restart requested - exiting emulator" << std::endl;
    exit(0);
}

// ============================================================================
// Buzzer Functions
// ============================================================================

void playJingle(const String& name) {
    playJingle(name.c_str());
}

void playJingle(const char* name) {
    std::cout << "[Buzzer] Playing jingle: " << (name ? name : "unknown") << std::endl;
    
    // Play actual tones using the buzzer
    extern Buzzer buzzer;
    
    if (!name) return;
    
    std::string jingleName(name);
    
    if (jingleName == "startup" || jingleName == "Startup") {
        // Startup jingle: ascending tones
        buzzer.sound(523, 100);  // C5
        buzzer.sound(659, 100);  // E5
        buzzer.sound(784, 100);  // G5
        buzzer.sound(1047, 200); // C6
    } 
    else if (jingleName == "shutdown" || jingleName == "Shutdown") {
        // Shutdown jingle: descending tones
        buzzer.sound(784, 100);  // G5
        buzzer.sound(659, 100);  // E5
        buzzer.sound(523, 100);  // C5
        buzzer.sound(392, 200);  // G4
    }
    else if (jingleName == "error" || jingleName == "Error") {
        // Error beep
        buzzer.sound(200, 100);
        buzzer.sound(0, 50);
        buzzer.sound(200, 100);
    }
    else if (jingleName == "success" || jingleName == "Success") {
        // Success beep
        buzzer.sound(880, 100);  // A5
        buzzer.sound(1109, 150); // C#6
    }
    else if (jingleName == "click" || jingleName == "Click") {
        // Click sound
        buzzer.sound(1000, 20);
    }
    else {
        // Default beep
        buzzer.sound(440, 100);  // A4
    }
}

// ============================================================================
// SD_MMC File Implementation
// ============================================================================

File::File() : inFile(nullptr), outFile(nullptr), isOpen(false), isDir(false), dirIndex(0) {}

File::File(const std::string& path, const std::string& mode) 
    : inFile(nullptr), outFile(nullptr), isOpen(false), isDir(false), filePath(path), dirIndex(0) {
    
    // Normalize path - remove leading slash and prepend data directory
    std::string localPath = path;
    if (!localPath.empty() && localPath[0] == '/') {
        localPath = localPath.substr(1);
    }
    std::string fullPath = "./data/" + localPath;
    
    try {
        std::filesystem::path p(fullPath);
        
        if (std::filesystem::exists(p) && std::filesystem::is_directory(p)) {
            // Directory handle
            isDir = true;
            isOpen = true;
            dirIndex = 0;
            dirEntries.clear();
            
            for (const auto& entry : std::filesystem::directory_iterator(p)) {
                dirEntries.push_back(entry.path().string());
            }
        } else {
            isDir = false;
            
            // Ensure parent directory exists for write modes
            if (mode == "w" || mode == FILE_WRITE || mode == "a" || mode == FILE_APPEND) {
                std::filesystem::create_directories(p.parent_path());
            }
            
            if (mode == "r" || mode == FILE_READ) {
                inFile = std::make_unique<std::ifstream>(fullPath, std::ios::binary);
                isOpen = inFile && inFile->is_open();
            } else if (mode == "w" || mode == FILE_WRITE) {
                outFile = std::make_unique<std::ofstream>(fullPath, std::ios::binary | std::ios::trunc);
                isOpen = outFile && outFile->is_open();
            } else if (mode == "a" || mode == FILE_APPEND) {
                outFile = std::make_unique<std::ofstream>(fullPath, std::ios::binary | std::ios::app);
                isOpen = outFile && outFile->is_open();
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "[File] Error opening " << fullPath << ": " << e.what() << std::endl;
        isOpen = false;
    }
}

File::~File() {
    close();
}

File::File(File&& other) noexcept
    : inFile(std::move(other.inFile))
    , outFile(std::move(other.outFile))
    , isOpen(other.isOpen)
    , isDir(other.isDir)
    , filePath(std::move(other.filePath))
    , dirEntries(std::move(other.dirEntries))
    , dirIndex(other.dirIndex)
{
    other.isOpen = false;
    other.isDir = false;
    other.dirIndex = 0;
}

File& File::operator=(File&& other) noexcept {
    if (this != &other) {
        close();
        inFile = std::move(other.inFile);
        outFile = std::move(other.outFile);
        isOpen = other.isOpen;
        isDir = other.isDir;
        filePath = std::move(other.filePath);
        dirEntries = std::move(other.dirEntries);
        dirIndex = other.dirIndex;
        
        other.isOpen = false;
        other.isDir = false;
        other.dirIndex = 0;
    }
    return *this;
}

void File::close() {
    if (inFile) { inFile->close(); inFile.reset(); }
    if (outFile) { outFile->close(); outFile.reset(); }
    isOpen = false;
}

size_t File::write(const uint8_t* data, size_t len) {
    if (!outFile || !outFile->is_open() || !data) return 0;
    outFile->write(reinterpret_cast<const char*>(data), len);
    return outFile->good() ? len : 0;
}

size_t File::write(const String& str) {
    return write(reinterpret_cast<const uint8_t*>(str.c_str()), str.length());
}

int File::read() {
    if (!inFile || !inFile->is_open()) return -1;
    return inFile->get();
}

size_t File::read(uint8_t* buf, size_t size) {
    if (!inFile || !inFile->is_open() || !buf) return 0;
    inFile->read(reinterpret_cast<char*>(buf), size);
    return inFile->gcount();
}

String File::readString() {
    if (!inFile || !inFile->is_open()) return String("");
    std::string result((std::istreambuf_iterator<char>(*inFile)), std::istreambuf_iterator<char>());
    return String(result.c_str());
}

String File::readStringUntil(char terminator) {
    if (!inFile || !inFile->is_open()) {
        return String("");
    }
    std::string result;
    std::getline(*inFile, result, terminator);
    return String(result.c_str());
}

bool File::available() {
    if (!inFile || !inFile->is_open()) return false;
    return inFile->peek() != EOF;
}

void File::seek(size_t pos) {
    if (inFile && inFile->is_open()) inFile->seekg(pos);
    if (outFile && outFile->is_open()) outFile->seekp(pos);
}

size_t File::position() {
    if (inFile && inFile->is_open()) return inFile->tellg();
    if (outFile && outFile->is_open()) return outFile->tellp();
    return 0;
}

size_t File::size() {
    if (!inFile || !inFile->is_open()) return 0;
    auto current = inFile->tellg();
    inFile->seekg(0, std::ios::end);
    auto fileSize = inFile->tellg();
    inFile->seekg(current);
    return fileSize;
}

bool File::isDirectory() {
    return isDir;
}

File File::openNextFile() {
    if (!isDir || dirIndex >= dirEntries.size()) return File();
    
    std::string childPath = dirEntries[dirIndex++];
    
    // Convert to relative path
    const std::string prefix = "./data/";
    if (childPath.find(prefix) == 0) {
        childPath = "/" + childPath.substr(prefix.length());
    }
    
    return File(childPath, "r");
}

void File::rewindDirectory() {
    dirIndex = 0;
}

String File::name() {
    try {
        std::filesystem::path p(filePath);
        return String(p.filename().string().c_str());
    } catch (...) {
        return String("");
    }
}

String File::path() {
    return String(filePath.c_str());
}

bool File::print(const char* msg) {
    if (!outFile || !outFile->is_open()) return false;
    if (msg) *outFile << msg;
    return outFile->good();
}

bool File::print(const String& msg) {
    return print(msg.c_str());
}

bool File::println(const char* msg) {
    if (!outFile || !outFile->is_open()) return false;
    if (msg) *outFile << msg;
    *outFile << '\n';
    return outFile->good();
}

bool File::println(const String& msg) {
    return println(msg.c_str());
}

// ============================================================================
// SD_MMC Class Implementation
// ============================================================================

std::string SD_MMCClass::toLocalPath(const char* path) {
    if (!path) return "./data";
    std::string p = path;
    if (!p.empty() && p[0] == '/') p = p.substr(1);
    return "./data/" + p;
}

File SD_MMCClass::open(const char* path, const char* mode) {
    if (!path) return File();
    return File(std::string(path), std::string(mode ? mode : "r"));
}

File SD_MMCClass::open(const String& path, const char* mode) {
    return open(path.c_str(), mode);
}

bool SD_MMCClass::exists(const char* path) {
    if (!path) return false;
    return std::filesystem::exists(toLocalPath(path));
}

bool SD_MMCClass::exists(const String& path) {
    return exists(path.c_str());
}

bool SD_MMCClass::remove(const char* path) {
    if (!path) return false;
    std::error_code ec;
    return std::filesystem::remove(toLocalPath(path), ec);
}

bool SD_MMCClass::remove(const String& path) {
    return remove(path.c_str());
}

bool SD_MMCClass::rename(const char* pathFrom, const char* pathTo) {
    if (!pathFrom || !pathTo) return false;
    std::error_code ec;
    std::filesystem::rename(toLocalPath(pathFrom), toLocalPath(pathTo), ec);
    return !ec;
}

bool SD_MMCClass::rename(const String& pathFrom, const String& pathTo) {
    return rename(pathFrom.c_str(), pathTo.c_str());
}

bool SD_MMCClass::mkdir(const char* path) {
    if (!path) return false;
    std::error_code ec;
    return std::filesystem::create_directories(toLocalPath(path), ec) || 
           std::filesystem::exists(toLocalPath(path));
}

bool SD_MMCClass::mkdir(const String& path) {
    return mkdir(path.c_str());
}

bool SD_MMCClass::rmdir(const char* path) {
    if (!path) return false;
    std::error_code ec;
    return std::filesystem::remove_all(toLocalPath(path), ec) > 0;
}

bool SD_MMCClass::rmdir(const String& path) {
    return rmdir(path.c_str());
}

// ============================================================================
// Adafruit_GFX Implementation
// ============================================================================

void Adafruit_GFX::fillScreen(uint16_t color) {
    fillRect(0, 0, _width, _height, color);
}

void Adafruit_GFX::drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color) {
    for (int16_t i = 0; i < h; i++) {
        drawPixel(x, y + i, color);
    }
}

void Adafruit_GFX::drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color) {
    for (int16_t i = 0; i < w; i++) {
        drawPixel(x + i, y, color);
    }
}

void Adafruit_GFX::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    for (int16_t j = 0; j < h; j++) {
        for (int16_t i = 0; i < w; i++) {
            drawPixel(x + i, y + j, color);
        }
    }
}

void Adafruit_GFX::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) {
    // Bresenham's line algorithm
    int16_t dx = abs(x1 - x0);
    int16_t dy = abs(y1 - y0);
    int16_t sx = x0 < x1 ? 1 : -1;
    int16_t sy = y0 < y1 ? 1 : -1;
    int16_t err = dx - dy;
    
    while (true) {
        drawPixel(x0, y0, color);
        if (x0 == x1 && y0 == y1) break;
        int16_t e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x0 += sx; }
        if (e2 < dx) { err += dx; y0 += sy; }
    }
}

void Adafruit_GFX::drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    drawFastHLine(x, y, w, color);
    drawFastHLine(x, y + h - 1, w, color);
    drawFastVLine(x, y, h, color);
    drawFastVLine(x + w - 1, y, h, color);
}

void Adafruit_GFX::drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) {
    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x = 0;
    int16_t y = r;
    
    drawPixel(x0, y0 + r, color);
    drawPixel(x0, y0 - r, color);
    drawPixel(x0 + r, y0, color);
    drawPixel(x0 - r, y0, color);
    
    while (x < y) {
        if (f >= 0) { y--; ddF_y += 2; f += ddF_y; }
        x++; ddF_x += 2; f += ddF_x;
        
        drawPixel(x0 + x, y0 + y, color);
        drawPixel(x0 - x, y0 + y, color);
        drawPixel(x0 + x, y0 - y, color);
        drawPixel(x0 - x, y0 - y, color);
        drawPixel(x0 + y, y0 + x, color);
        drawPixel(x0 - y, y0 + x, color);
        drawPixel(x0 + y, y0 - x, color);
        drawPixel(x0 - y, y0 - x, color);
    }
}

void Adafruit_GFX::fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) {
    drawFastVLine(x0, y0 - r, 2 * r + 1, color);
    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x = 0;
    int16_t y = r;
    
    while (x < y) {
        if (f >= 0) { y--; ddF_y += 2; f += ddF_y; }
        x++; ddF_x += 2; f += ddF_x;
        
        drawFastVLine(x0 + x, y0 - y, 2 * y + 1, color);
        drawFastVLine(x0 - x, y0 - y, 2 * y + 1, color);
        drawFastVLine(x0 + y, y0 - x, 2 * x + 1, color);
        drawFastVLine(x0 - y, y0 - x, 2 * x + 1, color);
    }
}

void Adafruit_GFX::drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color) {
    drawLine(x0, y0, x1, y1, color);
    drawLine(x1, y1, x2, y2, color);
    drawLine(x2, y2, x0, y0, color);
}

void Adafruit_GFX::fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color) {
    // Simple scanline fill
    // Sort vertices by y
    if (y0 > y1) { std::swap(y0, y1); std::swap(x0, x1); }
    if (y1 > y2) { std::swap(y1, y2); std::swap(x1, x2); }
    if (y0 > y1) { std::swap(y0, y1); std::swap(x0, x1); }
    
    if (y0 == y2) {
        int16_t a = std::min({x0, x1, x2});
        int16_t b = std::max({x0, x1, x2});
        drawFastHLine(a, y0, b - a + 1, color);
        return;
    }
    
    // Fill bottom flat triangle and top flat triangle
    for (int16_t y = y0; y <= y2; y++) {
        int16_t xa, xb;
        if (y < y1) {
            xa = x0 + (x1 - x0) * (y - y0) / (y1 - y0);
            xb = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
        } else {
            xa = x1 + (x2 - x1) * (y - y1) / (y2 - y1);
            xb = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
        }
        if (xa > xb) std::swap(xa, xb);
        drawFastHLine(xa, y, xb - xa + 1, color);
    }
}

void Adafruit_GFX::drawRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color) {
    drawFastHLine(x + r, y, w - 2 * r, color);
    drawFastHLine(x + r, y + h - 1, w - 2 * r, color);
    drawFastVLine(x, y + r, h - 2 * r, color);
    drawFastVLine(x + w - 1, y + r, h - 2 * r, color);
    // Corners would need arc drawing
}

void Adafruit_GFX::fillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color) {
    fillRect(x + r, y, w - 2 * r, h, color);
    // Corners would need filled arc drawing
}

void Adafruit_GFX::drawBitmap(int16_t x, int16_t y, const uint8_t bitmap[], int16_t w, int16_t h, uint16_t color) {
    // If drawing a large bitmap at origin, clear screen first
    // This handles apps like Journal that don't call fillScreen before drawing
    if (x == 0 && y == 0 && w >= 300 && h >= 200 && g_display) {
        g_display->einkClear();
    }
    
    int16_t byteWidth = (w + 7) / 8;
    for (int16_t j = 0; j < h; j++) {
        for (int16_t i = 0; i < w; i++) {
            if (bitmap[j * byteWidth + i / 8] & (128 >> (i & 7))) {
                drawPixel(x + i, y + j, color);
            }
        }
    }
}

void Adafruit_GFX::drawBitmap(int16_t x, int16_t y, const uint8_t bitmap[], int16_t w, int16_t h, uint16_t color, uint16_t bg) {
    int16_t byteWidth = (w + 7) / 8;
    for (int16_t j = 0; j < h; j++) {
        for (int16_t i = 0; i < w; i++) {
            if (bitmap[j * byteWidth + i / 8] & (128 >> (i & 7))) {
                drawPixel(x + i, y + j, color);
            } else {
                drawPixel(x + i, y + j, bg);
            }
        }
    }
}

void Adafruit_GFX::drawBitmap(int16_t x, int16_t y, uint8_t* bitmap, int16_t w, int16_t h, uint16_t color) {
    drawBitmap(x, y, (const uint8_t*)bitmap, w, h, color);
}

void Adafruit_GFX::drawBitmap(int16_t x, int16_t y, uint8_t* bitmap, int16_t w, int16_t h, uint16_t color, uint16_t bg) {
    drawBitmap(x, y, (const uint8_t*)bitmap, w, h, color, bg);
}

void Adafruit_GFX::drawXBitmap(int16_t x, int16_t y, const uint8_t bitmap[], int16_t w, int16_t h, uint16_t color) {
    int16_t byteWidth = (w + 7) / 8;
    for (int16_t j = 0; j < h; j++) {
        for (int16_t i = 0; i < w; i++) {
            if (bitmap[j * byteWidth + i / 8] & (1 << (i & 7))) {
                drawPixel(x + i, y + j, color);
            }
        }
    }
}

size_t Adafruit_GFX::write(uint8_t c) {
    if (!gfxFont) {
        // Classic built-in font (6x8)
        if (c == '\n') {
            flushCharBuffer();
            cursor_x = 0;
            cursor_y += textsize_y * 8;
        } else if (c == ' ') {
            flushCharBuffer();
            cursor_x += textsize_x * 6;
        } else if (c != '\r') {
            drawChar(cursor_x, cursor_y, c, textcolor, textbgcolor, textsize_x, textsize_y);
            cursor_x += textsize_x * 6;
        }
    } else {
        // Custom GFX font - cursor_y is baseline
        if (c == '\n') {
            flushCharBuffer();
            cursor_x = 0;
            cursor_y += textsize_y * gfxFont->yAdvance;
        } else if (c == ' ') {
            // Flush buffer on space, then advance cursor
            flushCharBuffer();
            // Space width should match SDL font rendering (narrower than letter width)
            int yAdv = gfxFont->yAdvance;
            int spaceWidth;
            if (yAdv >= 32) spaceWidth = 8;       // Large fonts
            else if (yAdv >= 24) spaceWidth = 7;  // Medium-large fonts
            else if (yAdv >= 18) spaceWidth = 6;  // FreeMonoBold9pt7b - space is narrower
            else spaceWidth = 5;                  // Small fonts
            cursor_x += spaceWidth * textsize_x;
        } else if (c != '\r') {
            // Map yAdvance to SDL font size and char width
            // Character widths match SDL font rendering
            int yAdv = gfxFont->yAdvance;
            int charWidth;
            if (yAdv >= 32) charWidth = 10;       // Large fonts
            else if (yAdv >= 24) charWidth = 9;   // Medium-large fonts
            else if (yAdv >= 18) charWidth = 8;   // FreeMonoBold9pt7b
            else charWidth = 6;                   // Small fonts
            
            drawChar(cursor_x, cursor_y, c, textcolor, textbgcolor, textsize_x, textsize_y);
            cursor_x += charWidth * textsize_x;
        }
    }
    return 1;
}

size_t Adafruit_GFX::write(const char* str) {
    if (!str) return 0;
    size_t n = 0;
    while (*str) {
        write(*str++);
        n++;
    }
    flushCharBuffer();  // Flush any remaining characters
    return n;
}

size_t Adafruit_GFX::write(const uint8_t* buffer, size_t size) {
    size_t n = 0;
    while (size--) {
        write(*buffer++);
        n++;
    }
    return n;
}

void Adafruit_GFX::print(const char* str) {
    extern DesktopDisplay* g_display;
    if (!str || !str[0] || !g_display) return;
    
    // Determine font size based on current font
    int fontSize = 10;
    int drawY = cursor_y;
    
    if (gfxFont && gfxFont->yAdvance > 0) {
        int yAdv = gfxFont->yAdvance;
        if (yAdv >= 32) fontSize = 14;
        else if (yAdv >= 24) fontSize = 12;
        else if (yAdv >= 18) fontSize = 10;
        else fontSize = 8;
        // Adafruit GFX cursor_y is baseline; SDL draws from top-left
        // Adjust by approximate ascent (font height minus descent)
        drawY = cursor_y - fontSize;  // Simple approximation
    }
    
    bool black = (textcolor == 0x0000);
    
    // Draw the entire string at once
    g_display->einkDrawText(str, cursor_x, drawY, fontSize, black);
    
    // Advance cursor by actual rendered width
    int16_t x1, y1;
    uint16_t w, h;
    g_display->einkGetTextBounds(str, cursor_x, cursor_y, &x1, &y1, &w, &h);
    cursor_x += w;
}
void Adafruit_GFX::print(const String& str) { print(str.c_str()); }
void Adafruit_GFX::print(char c) { write(c); }
void Adafruit_GFX::print(int n, int base) { print(String(n)); }
void Adafruit_GFX::print(unsigned int n, int base) { print(String(n)); }
void Adafruit_GFX::print(long n, int base) { print(String((int)n)); }
void Adafruit_GFX::print(unsigned long n, int base) { print(String((unsigned int)n)); }
void Adafruit_GFX::print(double n, int digits) { print(String((float)n, digits)); }

void Adafruit_GFX::println(const char* str) { print(str); write('\n'); }
void Adafruit_GFX::println(const String& str) { print(str); write('\n'); }
void Adafruit_GFX::println(char c) { print(c); write('\n'); }
void Adafruit_GFX::println(int n, int base) { print(n, base); write('\n'); }
void Adafruit_GFX::println(unsigned int n, int base) { print(n, base); write('\n'); }
void Adafruit_GFX::println(long n, int base) { print(n, base); write('\n'); }
void Adafruit_GFX::println(unsigned long n, int base) { print(n, base); write('\n'); }
void Adafruit_GFX::println(double n, int digits) { print(n, digits); write('\n'); }
void Adafruit_GFX::println() { write('\n'); }

void Adafruit_GFX::getTextBounds(const char* str, int16_t x, int16_t y,
                                  int16_t* x1, int16_t* y1, uint16_t* w, uint16_t* h) {
    extern DesktopDisplay* g_display;
    
    if (!str || !str[0]) {
        if (x1) *x1 = x;
        if (y1) *y1 = y;
        if (w) *w = 0;
        if (h) *h = 0;
        return;
    }
    
    // Use SDL to get actual rendered text dimensions
    if (g_display) {
        g_display->einkGetTextBounds(str, x, y, x1, y1, w, h);
    } else {
        // Fallback
        int strLen = strlen(str);
        if (x1) *x1 = x;
        if (y1) *y1 = y;
        if (w) *w = strLen * 8;
        if (h) *h = 16;
    }
}

void Adafruit_GFX::getTextBounds(const String& str, int16_t x, int16_t y,
                                  int16_t* x1, int16_t* y1, uint16_t* w, uint16_t* h) {
    getTextBounds(str.c_str(), x, y, x1, y1, w, h);
}

void Adafruit_GFX::setRotation(uint8_t r) {
    rotation = r & 3;
    switch (rotation) {
        case 0:
        case 2:
            // Portrait
            break;
        case 1:
        case 3:
            // Landscape - swap dimensions
            std::swap(_width, _height);
            break;
    }
}

void Adafruit_GFX::charBounds(unsigned char c, int16_t* x, int16_t* y,
                               int16_t* minx, int16_t* miny, int16_t* maxx, int16_t* maxy) {
    if (gfxFont) {
        if (c == '\n') {
            *x = 0;
            *y += textsize_y * gfxFont->yAdvance;
        } else if (c != '\r') {
            uint8_t first = gfxFont->first;
            uint8_t last = gfxFont->last;
            if ((c >= first) && (c <= last)) {
                GFXglyph* glyph = &gfxFont->glyph[c - first];
                uint8_t gw = glyph->width;
                uint8_t gh = glyph->height;
                uint8_t xa = glyph->xAdvance;
                int8_t xo = glyph->xOffset;
                int8_t yo = glyph->yOffset;
                
                int16_t tsx = textsize_x;
                int16_t tsy = textsize_y;
                int16_t x1 = *x + xo * tsx;
                int16_t y1 = *y + yo * tsy;
                int16_t x2 = x1 + gw * tsx - 1;
                int16_t y2 = y1 + gh * tsy - 1;
                
                if (x1 < *minx) *minx = x1;
                if (y1 < *miny) *miny = y1;
                if (x2 > *maxx) *maxx = x2;
                if (y2 > *maxy) *maxy = y2;
                *x += xa * tsx;
            }
        }
    } else {
        // Default font (6x8)
        if (c == '\n') {
            *x = 0;
            *y += textsize_y * 8;
        } else if (c != '\r') {
            int16_t x2 = *x + textsize_x * 6 - 1;
            int16_t y2 = *y + textsize_y * 8 - 1;
            if (x2 > *maxx) *maxx = x2;
            if (y2 > *maxy) *maxy = y2;
            if (*x < *minx) *minx = *x;
            if (*y < *miny) *miny = *y;
            *x += textsize_x * 6;
        }
    }
}

// Static buffer to accumulate characters for batch rendering
static std::string s_charBuffer;
static int16_t s_bufferStartX = 0;
static int16_t s_bufferStartY = 0;
static int s_bufferFontSize = 8;
static bool s_bufferBlack = true;

void Adafruit_GFX::flushCharBuffer() {
    extern DesktopDisplay* g_display;
    if (!g_display || s_charBuffer.empty()) return;
    
    g_display->einkDrawText(s_charBuffer.c_str(), s_bufferStartX, s_bufferStartY, s_bufferFontSize, s_bufferBlack);
    s_charBuffer.clear();
}

void Adafruit_GFX::drawChar(int16_t x, int16_t y, unsigned char c, uint16_t color, uint16_t bg, uint8_t size_x, uint8_t size_y) {
    extern DesktopDisplay* g_display;
    if (!g_display || c < 32 || c > 126) return;
    
    bool black = (color == 0x0000);  // GxEPD_BLACK = 0x0000
    
    // Determine font size for SDL rendering
    int fontSize = 8;  // Default for classic font
    int drawY = y;
    
    if (gfxFont && gfxFont->yAdvance > 0) {
        int yAdv = gfxFont->yAdvance;
        if (yAdv >= 32) fontSize = 14;
        else if (yAdv >= 24) fontSize = 12;
        else if (yAdv >= 18) fontSize = 10;
        else fontSize = 8;
        drawY = y - (yAdv * 0.7);
    }
    
    // If this is a new position/style, flush the buffer and start fresh
    if (s_charBuffer.empty()) {
        s_bufferStartX = x;
        s_bufferStartY = drawY;
        s_bufferFontSize = fontSize;
        s_bufferBlack = black;
    }
    
    // Add character to buffer
    s_charBuffer += (char)c;
}
