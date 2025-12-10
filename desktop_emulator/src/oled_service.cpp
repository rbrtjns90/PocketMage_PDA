/**
 * @file oled_service.cpp
 * @brief OLED display service implementation
 */

#include "oled_service.h"
#include "desktop_display_sdl2.h"
#include <cstring>
#include <mutex>

// OLED state
static std::string s_line1;
static std::string s_line2;
static std::string s_line3;
static bool s_dirty = false;
static std::mutex s_mutex;

extern DesktopDisplay* g_display;

void oled_set_lines(const char* line1, const char* line2, const char* line3) {
    std::lock_guard<std::mutex> lock(s_mutex);
    
    if (line1) s_line1 = line1;
    if (line2) s_line2 = line2;
    if (line3) s_line3 = line3;
    
    s_dirty = true;
}

void oled_set_line(int lineNum, const char* text) {
    std::lock_guard<std::mutex> lock(s_mutex);
    
    if (!text) return;
    
    switch (lineNum) {
        case 0: s_line1 = text; break;
        case 1: s_line2 = text; break;
        case 2: s_line3 = text; break;
        default: return;
    }
    
    s_dirty = true;
}

void oled_clear() {
    std::lock_guard<std::mutex> lock(s_mutex);
    
    s_line1.clear();
    s_line2.clear();
    s_line3.clear();
    
    if (g_display) {
        g_display->oledClear();
    }
    
    s_dirty = false;
}

void oled_present_if_dirty() {
    std::lock_guard<std::mutex> lock(s_mutex);
    
    if (!g_display) return;
    
    // Only draw line-based content if dirty AND there are lines to draw
    // This allows direct drawing (like TXT app) to work without being overwritten
    if (s_dirty && (!s_line1.empty() || !s_line2.empty() || !s_line3.empty())) {
        // Clear OLED
        g_display->oledClear();
        
        // Draw lines
        int y = 2;
        int lineHeight = 10;
        
        if (!s_line1.empty()) {
            g_display->oledDrawText(s_line1.c_str(), 2, y, 8);
            y += lineHeight;
        }
        
        if (!s_line2.empty()) {
            g_display->oledDrawText(s_line2.c_str(), 2, y, 8);
            y += lineHeight;
        }
        
        if (!s_line3.empty()) {
            g_display->oledDrawText(s_line3.c_str(), 2, y, 8);
        }
        
        s_dirty = false;
    }
    
    // Always refresh to show any direct drawing
    g_display->oledRefresh();
}

const char* oled_get_line(int lineNum) {
    std::lock_guard<std::mutex> lock(s_mutex);
    
    switch (lineNum) {
        case 0: return s_line1.c_str();
        case 1: return s_line2.c_str();
        case 2: return s_line3.c_str();
        default: return "";
    }
}
