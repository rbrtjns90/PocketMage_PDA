/**
 * @file pocketmage_stubs.cpp
 * @brief Stub implementations of PocketMage library classes for desktop emulator
 */

#include "pocketmage_stubs.h"
#include "desktop_display_sdl2.h"
#include "oled_service.h"
#include <iostream>
#include <string>
#include <filesystem>

// External display pointer
extern DesktopDisplay* g_display;

// ============================================================================
// Singleton instances
// ============================================================================
static PocketmageEink s_eink;
static PocketmageOled s_oled;
static PocketmageKB s_kb;
static PocketmageSD s_sd;
static PocketmageClock s_clock;
static PocketmageTOUCH s_touch;

// Global power system
MP2722 PowerSystem;

// ============================================================================
// Singleton accessor functions
// ============================================================================
PocketmageEink& EINK() { return s_eink; }
PocketmageOled& OLED() { return s_oled; }
PocketmageKB& KB() { return s_kb; }
PocketmageSD& SD() { return s_sd; }
PocketmageClock& CLOCK() { return s_clock; }
PocketmageTOUCH& TOUCH() { return s_touch; }

// ============================================================================
// PocketmageEink implementations
// ============================================================================
void PocketmageEink::drawStatusBar(const String& text) {
    if (g_display) {
        // Match real implementation: fillRect white, then drawRect black border
        int displayHeight = 240;  // E-ink display height
        int displayWidth = 310;   // E-ink display width
        
        // Fill white background for status bar area
        g_display->einkDrawRect(0, displayHeight - 26, displayWidth, 26, true, false);  // White fill
        // Draw black border rectangle
        g_display->einkDrawRect(0, displayHeight - 20, displayWidth, 20, false, true);  // Black outline
        // Draw text
        g_display->einkDrawText(text.c_str(), 4, displayHeight - 6, 9, true);
    }
}

void PocketmageEink::einkTextDynamic(bool fullRefresh, bool showCursor) {
    if (g_display) {
        g_display->present();
    }
}

void PocketmageEink::forceSlowFullUpdate(bool force) {
    // This is called to indicate the next refresh should be a full update
    // On real hardware this triggers a slower but cleaner refresh
    // In the emulator, we just set a flag - the actual flash happens in refresh()
    forceFullUpdate_ = force;
}

void PocketmageEink::multiPassRefresh(int passes) {
    if (g_display) {
        // multiPassRefresh is used for full screen updates (like app transitions)
        // Do flash animation if enabled
        g_display->einkRefresh();
        g_display->present();
    }
}

void PocketmageEink::refresh() {
    if (g_display) {
        g_display->einkRefresh();
        g_display->present();
    }
}

// ============================================================================
// PocketmageOled implementations
// ============================================================================
void PocketmageOled::infoBar() {
    oled_set_line(0, "PocketMage");
}

void PocketmageOled::oledLine(String text, bool selected, String suffix) {
    oled_set_line(1, text.c_str());
}

void PocketmageOled::oledScroll() {
    // Scroll animation - no-op in emulator
}

void PocketmageOled::oledWord(String text, bool selected, bool highlight) {
    oled_set_line(1, text.c_str());
}

// ============================================================================
// PocketmageKB implementations
// ============================================================================
void PocketmageKB::checkUSBKB() {
    // USB keyboard check - handled by SDL in emulator
}

char PocketmageKB::updateKeypress() {
    if (!g_display) return 0;
    
    // First check for special keys
    char key = g_display->getLastKey();
    if (key != 0) {
        return key;
    }
    
    // Then check for UTF-8 text input (regular typing)
    if (g_display->hasUTF8Input()) {
        std::string utf8 = g_display->getUTF8Input();
        if (!utf8.empty()) {
            // Return first character (for ASCII compatibility)
            return utf8[0];
        }
    }
    
    return 0;
}

// ============================================================================
// PocketmageSD implementations
// ============================================================================
void PocketmageSD::listDir(fs::FS& fs, const char* dirname) {
    std::cout << "[SD] listDir: " << dirname << std::endl;
    
    // Clear existing list
    filesListSize_ = 0;
    
    // List files in the data directory
    std::string basePath = "./data";
    if (dirname && dirname[0] == '/') {
        basePath += dirname;
    }
    
    try {
        for (const auto& entry : std::filesystem::directory_iterator(basePath)) {
            if (filesListSize_ >= 100) break;
            
            std::string filename = entry.path().filename().string();
            // Store with leading slash for compatibility
            filesList_[filesListSize_] = String(("/" + filename).c_str());
            std::cout << "[SD] Found: " << filesList_[filesListSize_].c_str() << std::endl;
            filesListSize_++;
        }
    } catch (const std::exception& e) {
        std::cout << "[SD] Error listing directory: " << e.what() << std::endl;
    }
}

// ============================================================================
// PocketmageTOUCH implementations
// ============================================================================
bool PocketmageTOUCH::updateScroll(int maxScroll, unsigned long& lineScroll) {
    return false;
}

void PocketmageTOUCH::updateScrollFromTouch() {
    // Touch scroll - no-op in emulator
}

// ============================================================================
// MP2722 implementations
// ============================================================================
void MP2722::printDiagnostics() {
    // Only print occasionally to reduce spam
    static int counter = 0;
    if (++counter % 100 == 0) {
        std::cout << "[Power] Battery: 75%, Not charging" << std::endl;
    }
}

