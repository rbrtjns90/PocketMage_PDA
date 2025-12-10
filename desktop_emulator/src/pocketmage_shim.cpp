/**
 * @file pocketmage_shim.cpp
 * @brief PocketMage library singleton shims for desktop emulator
 * 
 * This file provides mock implementations of the PocketMage library classes
 * (EINK(), OLED(), KB(), SD(), CLOCK(), TOUCH(), BZ()) that bridge to the
 * desktop emulator's display and input systems.
 */

#include "pocketmage_compat.h"
#include "Adafruit_GFX.h"
#include "desktop_display_sdl2.h"
#include "oled_service.h"
#include "GxEPD2_BW.h"
#include "SD_MMC.h"
#include <fstream>
#include "Wire.h"
#include "SPI.h"
#include "Preferences.h"
#include "Adafruit_TCA8418.h"
#include "Adafruit_MPR121.h"
#include "RTClib.h"
#include "Buzzer.h"
#include "GxEPD2_BW.h"
#include "U8g2lib.h"

// PocketMage library headers - commented out, using stub implementations instead
// #include "pocketmage_eink.h"
// #include "pocketmage_oled.h"
// #include "pocketmage_kb.h"
// #include "pocketmage_sd.h"
// #include "pocketmage_clock.h"
// #include "pocketmage_touch.h"
// #include "pocketmage_bz.h"
// #include "MP2722.h"

#include <iostream>

// Constants
#ifndef MAX_FILES
#define MAX_FILES 100
#endif

// IRAM_ATTR is ESP32-specific
#ifndef IRAM_ATTR
#define IRAM_ATTR
#endif

// External globals
extern DesktopDisplay* g_display;
extern SD_MMCClass SD_MMC;
extern Buzzer buzzer;

// ============================================================================
// Forward Declarations for PocketMage Library Classes
// ============================================================================

// These match the classes defined in the PocketMage library
class PocketmageEink;
class PocketmageOled;
class PocketmageKB;
class PocketmageSD;
class PocketmageClock;
class PocketmageTouch;
class PocketmageBZ;
class MP2722;

// ============================================================================
// Display Instance (for GxEPD2 compatibility)
// ============================================================================

// Create a mock display instance
static GxEPD2_310_GDEQ031T10 s_panel(0, 0, 0, 0);
GxEPD2_BW<GxEPD2_310_GDEQ031T10, GxEPD2_310_GDEQ031T10::HEIGHT> display(s_panel);

// OLED instance
U8G2_SSD1326_ER_256X32_F_4W_HW_SPI u8g2(U8G2_R0, 0, 0, 0);

// Keypad instance
Adafruit_TCA8418 keypad;

// Touch sensor instance
Adafruit_MPR121 cap;

// RTC instance
RTC_PCF8563 rtc;

// ============================================================================
// GxEPD2_BW Template Implementation
// ============================================================================

template<typename GxEPD2_Type, int page_height>
void GxEPD2_BW<GxEPD2_Type, page_height>::init(uint32_t serial_diag_bitrate, bool initial,
                                                uint16_t reset_duration, bool pulldown_rst_mode) {
    std::cout << "[E-ink] Display initialized" << std::endl;
    if (g_display) {
        g_display->einkClear();
    }
}

template<typename GxEPD2_Type, int page_height>
void GxEPD2_BW<GxEPD2_Type, page_height>::setFullWindow() {
    _using_partial_mode = false;
    // Clear screen to white when entering full window mode
    // This ensures clean transitions between apps
    if (g_display) {
        g_display->einkClear();
    }
}

template<typename GxEPD2_Type, int page_height>
void GxEPD2_BW<GxEPD2_Type, page_height>::setPartialWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
    _using_partial_mode = true;
}

template<typename GxEPD2_Type, int page_height>
void GxEPD2_BW<GxEPD2_Type, page_height>::firstPage() {
    // Prepare for page-based rendering
}

template<typename GxEPD2_Type, int page_height>
bool GxEPD2_BW<GxEPD2_Type, page_height>::nextPage() {
    // In emulator, we don't use paged rendering
    return false;
}

template<typename GxEPD2_Type, int page_height>
void GxEPD2_BW<GxEPD2_Type, page_height>::display(bool partial_update_mode) {
    if (g_display) {
        if (partial_update_mode) {
            g_display->einkPartialRefresh();
        } else {
            g_display->einkForceFullRefresh();
        }
    }
}

template<typename GxEPD2_Type, int page_height>
void GxEPD2_BW<GxEPD2_Type, page_height>::displayWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
    if (g_display) {
        g_display->einkPartialRefresh();
    }
}

template<typename GxEPD2_Type, int page_height>
void GxEPD2_BW<GxEPD2_Type, page_height>::powerOff() {
    // No-op in emulator
}

template<typename GxEPD2_Type, int page_height>
void GxEPD2_BW<GxEPD2_Type, page_height>::hibernate() {
    // No-op in emulator
}

template<typename GxEPD2_Type, int page_height>
void GxEPD2_BW<GxEPD2_Type, page_height>::clearScreen(uint8_t value) {
    if (g_display) {
        g_display->einkClear();
    }
}

template<typename GxEPD2_Type, int page_height>
void GxEPD2_BW<GxEPD2_Type, page_height>::writeScreenBuffer(uint8_t value) {
    if (g_display) {
        g_display->einkClear();
    }
}

template<typename GxEPD2_Type, int page_height>
void GxEPD2_BW<GxEPD2_Type, page_height>::drawPixel(int16_t x, int16_t y, uint16_t color) {
    static int pixelCount = 0;
    if (pixelCount++ < 5) {
        std::cout << "[GxEPD2] drawPixel called: " << x << "," << y << " color=" << color << std::endl;
    }
    if (g_display) {
        g_display->einkSetPixel(x, y, color == GxEPD_BLACK);
    }
}

template<typename GxEPD2_Type, int page_height>
void GxEPD2_BW<GxEPD2_Type, page_height>::fillScreen(uint16_t color) {
    if (g_display) {
        if (color == GxEPD_WHITE) {
            g_display->einkClear();
        } else {
            g_display->einkDrawRect(0, 0, EINK_WIDTH, EINK_HEIGHT, true, true);
        }
    }
}

template<typename GxEPD2_Type, int page_height>
void GxEPD2_BW<GxEPD2_Type, page_height>::drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color) {
    if (g_display) {
        g_display->einkDrawLine(x, y, x, y + h - 1, color == GxEPD_BLACK);
    }
}

template<typename GxEPD2_Type, int page_height>
void GxEPD2_BW<GxEPD2_Type, page_height>::drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color) {
    if (g_display) {
        g_display->einkDrawLine(x, y, x + w - 1, y, color == GxEPD_BLACK);
    }
}

template<typename GxEPD2_Type, int page_height>
void GxEPD2_BW<GxEPD2_Type, page_height>::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    if (g_display) {
        g_display->einkDrawRect(x, y, w, h, true, color == GxEPD_BLACK);
    }
}

template<typename GxEPD2_Type, int page_height>
void GxEPD2_BW<GxEPD2_Type, page_height>::refresh(bool partial_update_mode) {
    display(partial_update_mode);
}

template<typename GxEPD2_Type, int page_height>
void GxEPD2_BW<GxEPD2_Type, page_height>::writeImage(const uint8_t* bitmap, int16_t x, int16_t y, 
                                                      int16_t w, int16_t h, bool invert, 
                                                      bool mirror_y, bool pgm) {
    if (g_display && bitmap) {
        // If drawing a large bitmap at origin, clear screen first
        // This handles apps like Journal that don't call setFullWindow/fillScreen
        if (x == 0 && y == 0 && w >= 300 && h >= 200) {
            g_display->einkClear();
        }
        g_display->einkDrawBitmap(x, y, bitmap, w, h, !invert);
    }
}

template<typename GxEPD2_Type, int page_height>
void GxEPD2_BW<GxEPD2_Type, page_height>::writeImagePart(const uint8_t* bitmap, int16_t x_part, 
                                                          int16_t y_part, int16_t w_bitmap, 
                                                          int16_t h_bitmap, int16_t x, int16_t y,
                                                          int16_t w, int16_t h, bool invert,
                                                          bool mirror_y, bool pgm) {
    // Simplified - just draw the full bitmap at the target location
    writeImage(bitmap, x, y, w, h, invert, mirror_y, pgm);
}

// Explicit template instantiation
template class GxEPD2_BW<GxEPD2_310_GDEQ031T10, GxEPD2_310_GDEQ031T10::HEIGHT>;

// Static member
volatile bool GxEPD2_310_GDEQ031T10::useFastFullUpdate = false;

// ============================================================================
// PocketMage Library Class Implementations
// ============================================================================

// These are mock implementations that bridge to the emulator

// ---------- PocketmageEink ----------
class PocketmageEinkImpl {
public:
    void refresh() {
        if (g_display) g_display->einkRefresh();
    }
    
    void multiPassRefresh(int passes) {
        refresh();
    }
    
    void setFastFullRefresh(bool setting) {
        GxEPD2_310_GDEQ031T10::useFastFullUpdate = setting;
    }
    
    void statusBar(const String& input, bool fullWindow = false) {
        if (g_display) {
            // Draw status bar at bottom of screen
            g_display->einkDrawRect(0, EINK_HEIGHT - 16, EINK_WIDTH, 16, true, false);
            g_display->einkDrawText(input.c_str(), 5, EINK_HEIGHT - 14, 10);
        }
    }
    
    void drawStatusBar(const String& input) {
        statusBar(input, false);
    }
    
    void setTXTFont(const GFXfont* font) {
        currentFont_ = font;
    }
    
    void einkTextDynamic(bool doFull, bool noRefresh = false) {
        // Text rendering handled by display
    }
    
    int countLines(const String& input, size_t maxLineLength = 29) {
        if (input.length() == 0) return 0;
        return (input.length() / maxLineLength) + 1;
    }
    
    uint8_t maxCharsPerLine() const { return 29; }
    uint8_t maxLines() const { return 12; }
    const GFXfont* getCurrentFont() { return currentFont_; }
    
    void forceSlowFullUpdate(bool force) {
        forceSlowFullUpdate_ = force;
    }
    
    void computeFontMetrics_() {}
    uint16_t getEinkTextWidth(const String& s) { return s.length() * 6; }
    void setLineSpacing(uint8_t ls) { lineSpacing_ = ls; }
    void setFullRefreshAfter(uint8_t n) { fullRefreshAfter_ = n; }
    void setCurrentFont(const GFXfont* f) { currentFont_ = f; }
    
private:
    const GFXfont* currentFont_ = nullptr;
    bool forceSlowFullUpdate_ = false;
    uint8_t lineSpacing_ = 6;
    uint8_t fullRefreshAfter_ = 5;
};

[[maybe_unused]] static PocketmageEinkImpl s_einkImpl;

// ---------- PocketmageOled ----------
class PocketmageOledImpl {
public:
    void oledWord(String word, bool allowLarge = false, bool showInfo = true) {
        oled_set_lines(word.c_str(), "", "");
    }
    
    void oledLine(String line, bool doProgressBar = true, String bottomMsg = "") {
        oled_set_lines(line.c_str(), bottomMsg.c_str(), "");
    }
    
    void oledScroll() {
        // Scroll animation - no-op in emulator
    }
    
    void infoBar() {
        // Draw info bar on OLED
    }
    
    void setPowerSave(int in) {
        // No-op in emulator
    }
};

[[maybe_unused]] static PocketmageOledImpl s_oledImpl;

// ---------- PocketmageKB ----------
class PocketmageKBImpl {
public:
    volatile bool TCA8418_event_ = false;
    
    char updateKeypress() {
        if (g_display) {
            // Check for UTF-8 input first
            if (g_display->hasUTF8Input()) {
                std::string utf8 = g_display->getUTF8Input();
                if (!utf8.empty()) {
                    return utf8[0];  // Return first character
                }
            }
            
            // Check for special keys
            return g_display->getLastKey();
        }
        return 0;
    }
    
    void checkUSBKB() {
        // USB keyboard check - handled by SDL in emulator
    }
    
    void setKeyboardState(int state) { kbState_ = state; }
    int getKeyboardState() const { return kbState_; }
    void disableInterrupts() {}
    void enableInterrupts() {}
    void flush() {}
    void setTCA8418Event() { TCA8418_event_ = true; }
    
private:
    int kbState_ = 0;
};

[[maybe_unused]] static PocketmageKBImpl s_kbImpl;

// ---------- PocketmageSD ----------
class PocketmageSDImpl {
public:
    void listDir(fs::FS& fs, const char* dirname) {
        // List directory contents
        std::string path = dirname;
        if (path.empty() || path[0] != '/') path = "/" + path;
        
        File dir = SD_MMC.open(path.c_str(), FILE_READ);
        if (!dir || !dir.isDirectory()) return;
        
        int idx = 0;
        File file = dir.openNextFile();
        while (file && idx < MAX_FILES) {
            filesList_[idx++] = file.name();
            file = dir.openNextFile();
        }
        
        // Clear remaining slots
        while (idx < MAX_FILES) {
            filesList_[idx++] = "";
        }
    }
    
    void readFile(fs::FS& fs, const char* path) {
        File file = SD_MMC.open(path, FILE_READ);
        if (!file) return;
        while (file.available()) {
            Serial.print((char)file.read());
        }
        file.close();
    }
    
    String readFileToString(fs::FS& fs, const char* path) {
        File file = SD_MMC.open(path, FILE_READ);
        if (!file) return String("");
        String content = file.readString();
        file.close();
        return content;
    }
    
    void writeFile(fs::FS& fs, const char* path, const char* message) {
        File file = SD_MMC.open(path, FILE_WRITE);
        if (!file) return;
        file.print(message);
        file.close();
    }
    
    void appendFile(fs::FS& fs, const char* path, const char* message) {
        File file = SD_MMC.open(path, FILE_APPEND);
        if (!file) return;
        file.print(message);
        file.close();
    }
    
    void renameFile(fs::FS& fs, const char* path1, const char* path2) {
        SD_MMC.rename(path1, path2);
    }
    
    void deleteFile(fs::FS& fs, const char* path) {
        SD_MMC.remove(path);
    }
    
    bool getNoSD() { return noSD_; }
    void setNoSD(bool in) { noSD_ = in; }
    
    String getWorkingFile() { return workingFile_; }
    void setWorkingFile(String in) { workingFile_ = in; }
    
    String getEditingFile() { return editingFile_; }
    void setEditingFile(String in) { editingFile_ = in; }
    
    String getFilesListIndex(int index) {
        if (index >= 0 && index < MAX_FILES) return filesList_[index];
        return String("");
    }
    void setFilesListIndex(int index, String content) {
        if (index >= 0 && index < MAX_FILES) filesList_[index] = content;
    }
    
private:
    bool noSD_ = false;
    String editingFile_;
    String workingFile_;
    String filesList_[MAX_FILES];
};

static PocketmageSDImpl s_sdImpl;

// ---------- PocketmageClock ----------
class PocketmageClockImpl {
public:
    DateTime nowDT() {
        return DateTime();  // Returns current system time
    }
};

[[maybe_unused]] static PocketmageClockImpl s_clockImpl;

// ---------- PocketmageTouch ----------
class PocketmageTouchImpl {
public:
    uint16_t touched() { return 0; }
    int getSliderPosition() { return -1; }
};

[[maybe_unused]] static PocketmageTouchImpl s_touchImpl;

// ---------- PocketmageBZ ----------
class PocketmageBZImpl {
public:
    void playJingle(const String& name) {
        std::cout << "[Buzzer] Playing: " << name.c_str() << std::endl;
    }
    
    void tone(int freq, int duration) {
        buzzer.tone(freq, duration);
    }
    
    void noTone() {
        buzzer.noTone();
    }
};

[[maybe_unused]] static PocketmageBZImpl s_bzImpl;

// ---------- MP2722 Power Management ----------
class MP2722Impl {
public:
    bool init(int sda, int scl) { return true; }
    void printDiagnostics() {}
    int getBatteryPercent() { return 75; }
    bool isCharging() { return false; }
};

[[maybe_unused]] static MP2722Impl s_powerImpl;

// ============================================================================
// Singleton Accessor Functions
// ============================================================================

// These functions are called by the PocketMage code

// Note: The actual PocketMage library defines these classes differently.
// We need to provide compatible interfaces.

// For the emulator, we'll define wrapper classes that match the library API

// ============================================================================
// Setup Functions (called during initialization)
// ============================================================================

void setupEink() {
    std::cout << "[Setup] E-ink display" << std::endl;
    display.init(115200);
    display.setRotation(3);
    display.setTextColor(GxEPD_BLACK);
    display.setFullWindow();
}

void setupOled() {
    std::cout << "[Setup] OLED display" << std::endl;
    u8g2.begin();
    u8g2.setPowerSave(0);
    u8g2.clearBuffer();
    u8g2.sendBuffer();
    // Don't set initial lines - let the app control OLED content
}

void setupKB(int kb_irq_pin) {
    std::cout << "[Setup] Keyboard" << std::endl;
    keypad.begin();
    keypad.matrix(4, 10);
    keypad.enableInterrupts();
}

void setupSD() {
    std::cout << "[Setup] SD card" << std::endl;
    SD_MMC.begin();
}

void setupClock() {
    std::cout << "[Setup] RTC" << std::endl;
    rtc.begin();
}

void setupTouch() {
    std::cout << "[Setup] Touch sensor" << std::endl;
    cap.begin();
}

void setupBZ() {
    std::cout << "[Setup] Buzzer - initializing SDL audio" << std::endl;
    buzzer.begin(0);  // Channel 0
    std::cout << "[Setup] Buzzer - playing startup jingle" << std::endl;
    playJingle("startup");
    std::cout << "[Setup] Buzzer - done" << std::endl;
}

// ============================================================================
// PocketMage_INIT - Main initialization function
// ============================================================================

void PocketMage_INIT() {
    std::cout << "========================================" << std::endl;
    std::cout << "  PocketMage Desktop Emulator v1.0" << std::endl;
    std::cout << "========================================" << std::endl;
    
    // Initialize all subsystems
    setupOled();
    setupBZ();
    setupKB(8);  // KB_IRQ
    setupEink();
    setupSD();
    setupClock();
    setupTouch();
    
    // Create a sample file for testing
    {
        std::ofstream testFile("./data/welcome.txt");
        if (testFile.is_open()) {
            testFile << "# Welcome to PocketMage!\n\n";
            testFile << "This is the desktop emulator.\n\n";
            testFile << "You can type here to test the text editor.\n\n";
            testFile << "Press **Home** (key 12) to return to the main menu.\n";
            testFile.close();
            std::cout << "[Setup] Created welcome.txt" << std::endl;
        }
    }
    
#ifdef _WIN32
    // Windows: Initialize global variables that may not be set by preferences
    // This prevents division by zero in processKB_HOME() when OLED_MAX_FPS is 0
    extern int OLED_MAX_FPS;
    extern int OLED_BRIGHTNESS;
    if (OLED_MAX_FPS == 0) OLED_MAX_FPS = 30;
    if (OLED_BRIGHTNESS == 0) OLED_BRIGHTNESS = 255;
    
    // Windows: Initialize HOME app state (sets lastInput, CurrentAppState, etc.)
    extern void HOME_INIT();
    HOME_INIT();
#else
    // Mac/Linux: Set newState to trigger initial screen draw
    extern volatile bool newState;
    newState = true;
#endif
    
    std::cout << "[PocketMage] Initialization complete" << std::endl;
}

// ============================================================================
// Namespace Functions (pocketmage::)
// ============================================================================

namespace pocketmage {
    namespace file {
        void saveFile() {
            std::cout << "[File] saveFile()" << std::endl;
        }
        
        void writeMetadata(const String& path) {
            std::cout << "[File] writeMetadata: " << path.c_str() << std::endl;
        }
        
        void loadFile(bool showOLED) {
            std::cout << "[File] loadFile()" << std::endl;
        }
        
        void delFile(String fileName) {
            SD_MMC.remove(fileName.c_str());
        }
        
        void deleteMetadata(String path) {
            std::cout << "[File] deleteMetadata: " << path.c_str() << std::endl;
        }
        
        void renFile(String oldFile, String newFile) {
            SD_MMC.rename(oldFile.c_str(), newFile.c_str());
        }
        
        void renMetadata(String oldPath, String newPath) {
            std::cout << "[File] renMetadata" << std::endl;
        }
        
        void copyFile(String oldFile, String newFile) {
            String content = s_sdImpl.readFileToString(SD_MMC, oldFile.c_str());
            s_sdImpl.writeFile(SD_MMC, newFile.c_str(), content.c_str());
        }
        
        void appendToFile(String path, String inText) {
            s_sdImpl.appendFile(SD_MMC, path.c_str(), inText.c_str());
        }
    }
    
    namespace time {
        void setTimeFromString(String timeStr) {
            std::cout << "[Time] setTimeFromString: " << timeStr.c_str() << std::endl;
        }
        
        void checkTimeout() {
            // Timeout check - no-op in emulator
        }
        
        void setCpuSpeed(int newFreq) {
            // No-op in emulator
        }
    }
    
    namespace power {
        void deepSleep(bool alternateScreenSaver) {
            std::cout << "[Power] Deep sleep requested" << std::endl;
        }
        
        void IRAM_ATTR PWR_BTN_irq() {
            std::cout << "[Power] Power button interrupt" << std::endl;
        }
        
        void updateBattState() {
            // Battery state update - mock 75% battery
        }
        
        void loadState(bool changeState) {
            std::cout << "[Power] loadState()" << std::endl;
        }
    }
    
    namespace debug {
        void printDebug() {
            // Debug output
        }
    }
}

// ============================================================================
// Global Helper Functions
// ============================================================================

String vectorToString() {
    // Convert allLines vector to string
    extern std::vector<String> allLines;
    String result;
    for (size_t i = 0; i < allLines.size(); i++) {
        result += allLines[i];
        if (i < allLines.size() - 1) result += "\n";
    }
    return result;
}

void stringToVector(String inputText) {
    extern std::vector<String> allLines;
    allLines.clear();
    
    String current;
    for (size_t i = 0; i < inputText.length(); i++) {
        if (inputText[i] == '\n') {
            allLines.push_back(current);
            current = "";
        } else {
            current += inputText[i];
        }
    }
    if (current.length() > 0) {
        allLines.push_back(current);
    }
}

String removeChar(String str, char character) {
    String result;
    for (size_t i = 0; i < str.length(); i++) {
        if (str[i] != character) {
            result += str[i];
        }
    }
    return result;
}

int stringToInt(String str) {
    return str.toInt();
}

// ============================================================================
// Missing Global Variables
// ============================================================================

std::vector<String> allLines;
bool newLineAdded = false;
bool noTimeout = false;
bool SDActive = false;
const unsigned char* KBStatusallArray[8] = {nullptr};

// ============================================================================
// Missing App Functions (stubs for excluded APPLOADER)
// ============================================================================

void APPLOADER_INIT() {
    std::cout << "[AppLoader] Not available in emulator" << std::endl;
}

void USB_INIT() {
    std::cout << "[USB] Not available in emulator" << std::endl;
}

void einkHandler_APPLOADER() {
    // No-op
}

void einkHandler_USB() {
    // No-op
}

void processKB_APPLOADER() {
    // No-op
}

void processKB_USB() {
    // No-op
}

void rebootToAppSlot(int slot) {
    std::cout << "[System] Reboot to slot " << slot << " not available in emulator" << std::endl;
}

void loadAndDrawAppIcon(int x, int y, int slot, bool selected, int size) {
    // Draw placeholder icon
    if (g_display) {
        g_display->fillRect(x, y, size, size, selected ? 0x000000 : 0xCCCCCC);
    }
}

// ============================================================================
// Commented out - these require including PocketMage library headers which
// have constructor requirements incompatible with desktop emulator
// ============================================================================

// // Power system stub
// class PowerSystemClass {
// public:
//     int getBatteryPercent() { return 75; }
//     bool isCharging() { return false; }
//     void printDiagnostics() {}
// };
// PowerSystemClass PowerSystem;

// // ============================================================================
// // Missing PocketMage Class Method Implementations
// // ============================================================================

// // PocketmageEink methods
// void PocketmageEink::drawStatusBar(const String& text) {
//     if (g_display) {
//         g_display->drawText(5, 10, text.c_str(), 0x000000);
//     }
// }

// void PocketmageEink::einkTextDynamic(bool fullRefresh, bool showCursor) {
//     if (g_display) {
//         g_display->present();
//     }
// }

// void PocketmageEink::forceSlowFullUpdate(bool force) {
//     // No-op in emulator
// }

// void PocketmageEink::multiPassRefresh(int passes) {
//     if (g_display) {
//         g_display->present();
//     }
// }

// void PocketmageEink::refresh() {
//     if (g_display) {
//         g_display->present();
//     }
// }

// // PocketmageOled methods
// void PocketmageOled::infoBar() {
//     oled_set_line(0, "PocketMage");
// }

// void PocketmageOled::oledLine(String text, bool selected, String suffix) {
//     oled_set_line(1, text.c_str());
// }

// void PocketmageOled::oledScroll() {
//     // Scroll animation - no-op in emulator
// }

// void PocketmageOled::oledWord(String text, bool selected, bool highlight) {
//     oled_set_line(1, text.c_str());
// }

// // PocketmageKB methods
// void PocketmageKB::checkUSBKB() {
//     // USB keyboard check - handled by SDL in emulator
// }

// void PocketmageKB::updateKeypress() {
//     // Keypress update - handled by SDL in emulator
// }

// // PocketmageSD methods
// void PocketmageSD::listDir(fs::FS& fs, const char* dirname) {
//     std::cout << "[SD] listDir: " << dirname << std::endl;
// }

// // PocketmageTOUCH methods
// bool PocketmageTOUCH::updateScroll(int maxScroll, unsigned long& lineScroll) {
//     return false;
// }

// void PocketmageTOUCH::updateScrollFromTouch() {
//     // Touch scroll - no-op in emulator
// }

// // MP2722 methods
// void MP2722::printDiagnostics() {
//     std::cout << "[Power] Battery: 75%, Not charging" << std::endl;
// }

// // ============================================================================
// // Missing Singleton Accessor Functions
// // ============================================================================

// static PocketmageEink s_eink;
// static PocketmageOled s_oled;
// static PocketmageKB s_kb;
// static PocketmageSD s_sd;
// static PocketmageClock s_clock;
// static PocketmageTOUCH s_touch;

// PocketmageEink& EINK() { return s_eink; }
// PocketmageOled& OLED() { return s_oled; }
// PocketmageKB& KB() { return s_kb; }
// PocketmageSD& SD() { return s_sd; }
// PocketmageClock& CLOCK() { return s_clock; }
// PocketmageTOUCH& TOUCH() { return s_touch; }
