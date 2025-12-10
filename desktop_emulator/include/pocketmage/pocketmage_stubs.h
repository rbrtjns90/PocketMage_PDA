/**
 * @file pocketmage_stubs.h
 * @brief Stub implementations of PocketMage library classes for desktop emulator
 * 
 * These classes provide the same interface as the PocketMage library but with
 * stub implementations suitable for desktop emulation.
 */

#ifndef POCKETMAGE_STUBS_H
#define POCKETMAGE_STUBS_H

#include "pocketmage_compat.h"
#include "display/Adafruit_GFX.h"
#include "storage/SD_MMC.h"

// Forward declaration
class DesktopDisplay;
extern DesktopDisplay* g_display;

// ============================================================================
// PocketmageEink - E-ink display controller stub
// ============================================================================
class PocketmageEink {
public:
    PocketmageEink() : currentFont_(nullptr), fullRefreshAfter_(0), forceFullUpdate_(false) {}
    
    void drawStatusBar(const String& text);
    void einkTextDynamic(bool fullRefresh, bool showCursor = false);
    void forceSlowFullUpdate(bool force);
    void multiPassRefresh(int passes);
    void refresh();
    void setTXTFont(const GFXfont* font) { currentFont_ = font; }
    const GFXfont* getCurrentFont() const { return currentFont_; }
    void setCurrentFont(const GFXfont* font) { currentFont_ = font; }
    void setFullRefreshAfter(int count) { fullRefreshAfter_ = count; }
    int getFullRefreshAfter() const { return fullRefreshAfter_; }
    
private:
    const GFXfont* currentFont_;
    int fullRefreshAfter_;
    bool forceFullUpdate_;
};

// ============================================================================
// PocketmageOled - OLED display controller stub
// ============================================================================
class PocketmageOled {
public:
    PocketmageOled() {}
    
    void infoBar();
    void oledLine(String text, bool selected = false, String suffix = "");
    void oledScroll();
    void oledWord(String text, bool selected = false, bool highlight = false);
};

// Keyboard states - defined in globals.h as KBState { NORMAL, SHIFT, FUNC }
// We use int here to avoid redefinition conflicts
// ============================================================================
// PocketmageKB - Keyboard controller stub
// ============================================================================
class PocketmageKB {
public:
    PocketmageKB() : kbState_(0) {}  // 0 = NORMAL
    
    void checkUSBKB();
    char updateKeypress();
    
    int getKeyboardState() { return kbState_; }
    void setKeyboardState(int state) { kbState_ = state; }
    
private:
    int kbState_;
};

// ============================================================================
// PocketmageSD - SD card controller stub
// ============================================================================
class PocketmageSD {
public:
    PocketmageSD() : filesListSize_(0), workingFile_(""), editingFile_("") {}
    
    void listDir(fs::FS& fs, const char* dirname);
    
    // File list management
    String getFilesListIndex(int index) { return (index < filesListSize_) ? filesList_[index] : ""; }
    int getFilesListSize() { return filesListSize_; }
    void setWorkingFile(const String& file) { workingFile_ = file; }
    String getWorkingFile() { return workingFile_; }
    void setEditingFile(const String& file) { editingFile_ = file; }
    String getEditingFile() { return editingFile_; }
    bool getNoSD() { return noSD_; }
    void setNoSD(bool val) { noSD_ = val; }
    
private:
    String filesList_[100];
    int filesListSize_;
    String workingFile_;
    String editingFile_;
    bool noSD_ = false;
};

// ============================================================================
// PocketmageClock - RTC controller stub
// ============================================================================
// DateTime and TimeSpan are defined in RTClib.h
#include "hardware/Wire.h"  // For TwoWire, needed by RTClib.h
#include "hardware/RTClib.h"

class PocketmageClock {
public:
    PocketmageClock() : timeoutMillis_(0) {}
    
    void begin() {}
    long getTimeoutMillis() const { return timeoutMillis_; }
    void setTimeoutMillis(long ms) { timeoutMillis_ = ms; }
    
    DateTime nowDT() const { return DateTime(); }
    RTC_PCF8563& getRTC() { return *rtc_; }
    
private:
    volatile long timeoutMillis_;
    RTC_PCF8563* rtc_ = nullptr;
};

// ============================================================================
// PocketmageTOUCH - Touch sensor controller stub
// ============================================================================
class PocketmageTOUCH {
public:
    PocketmageTOUCH() : dynamicScroll_(0), prevDynamicScroll_(0) {}
    
    bool updateScroll(int maxScroll, unsigned long& lineScroll);
    void updateScrollFromTouch();
    
    int getDynamicScroll() { return dynamicScroll_; }
    void setDynamicScroll(int scroll) { dynamicScroll_ = scroll; }
    int getPrevDynamicScroll() { return prevDynamicScroll_; }
    void setPrevDynamicScroll(int scroll) { prevDynamicScroll_ = scroll; }
    unsigned long getLastTouch() { return lastTouch_; }
    void setLastTouch(unsigned long t) { lastTouch_ = t; }
    
private:
    int dynamicScroll_;
    int prevDynamicScroll_;
    unsigned long lastTouch_;
};

// ============================================================================
// PocketmageBZ - Buzzer controller stub
// ============================================================================
class PocketmageBZ {
public:
    PocketmageBZ() {}
    
    void begin(int pin) {}
    void playTone(int frequency, int duration) {}
    void stop() {}
};

// ============================================================================
// MP2722 - Power management IC stub
// ============================================================================
class MP2722 {
public:
    MP2722() {}
    
    bool init(int sda, int scl) { return true; }
    void printDiagnostics();
    int getBatteryPercent() { return 75; }
    bool isCharging() { return false; }
};

// ============================================================================
// Singleton accessor functions
// ============================================================================
PocketmageEink& EINK();
PocketmageOled& OLED();
PocketmageKB& KB();
PocketmageSD& SD();
PocketmageClock& CLOCK();
PocketmageTOUCH& TOUCH();

// Global power system instance
extern MP2722 PowerSystem;

// Global variables used by PocketMage
extern bool noTimeout;
extern bool SDActive;
extern bool newLineAdded;
extern std::vector<String> allLines;
extern const unsigned char* KBStatusallArray[8];

// Keypad instance (from Adafruit_TCA8418)
#include "input/Adafruit_TCA8418.h"
extern Adafruit_TCA8418 keypad;

// Display instance (GxEPD2)
#include "display/GxEPD2_BW.h"
extern GxEPD2_BW<GxEPD2_310_GDEQ031T10, GxEPD2_310_GDEQ031T10::HEIGHT> display;

// U8G2 OLED instance
#include "display/U8g2lib.h"
extern U8G2_SSD1326_ER_256X32_F_4W_HW_SPI u8g2;

// Font externs (defined in include/Fonts/*.h)
extern const GFXfont FreeSerif9pt7b;
extern const GFXfont FreeSerif12pt7b;
extern const GFXfont FreeSerifBold9pt7b;
extern const GFXfont FreeMonoBold9pt7b;
extern const GFXfont FreeMono9pt7b;
extern const GFXfont FreeMono12pt7b;
extern const GFXfont FreeSans9pt7b;
extern const GFXfont FreeSans12pt7b;
extern const GFXfont Font5x7Fixed;
extern const GFXfont Font3x7FixedNum;
extern const GFXfont Font4x5Fixed;

// Helper functions
String removeChar(String str, char character);
int stringToInt(String str);
String vectorToString();
void stringToVector(String inputText);

// ============================================================================
// pocketmage namespace - utility functions
// ============================================================================
namespace pocketmage {
    namespace file {
        void saveFile();
        void writeMetadata(const String& path);
        void loadFile(bool showOLED = true);
        void delFile(String fileName);
        void deleteMetadata(String path);
        void renFile(String oldFile, String newFile);
        void renMetadata(String oldPath, String newPath);
        void copyFile(String oldFile, String newFile);
        void appendToFile(String path, String inText);
    }
    
    namespace time {
        void setTimeFromString(String timeStr);
        void checkTimeout();
        void setCpuSpeed(int newFreq);
    }
    
    namespace power {
        void deepSleep(bool alternateScreenSaver);
        void PWR_BTN_irq();
        void updateBattState();
        void loadState(bool changeState);
    }
    
    namespace debug {
        void printDebug();
    }
}

#endif // POCKETMAGE_STUBS_H
