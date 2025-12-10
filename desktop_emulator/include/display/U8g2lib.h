/**
 * @file U8g2lib.h
 * @brief U8g2 OLED library mock for desktop emulator
 */

#ifndef U8G2LIB_H
#define U8G2LIB_H

#include "pocketmage/pocketmage_compat.h"
#include "display/desktop_display_sdl2.h"

// U8g2 rotation constants
#define U8G2_R0 0
#define U8G2_R1 1
#define U8G2_R2 2
#define U8G2_R3 3

// U8g2 draw modes
#define U8G2_DRAW_UPPER_RIGHT 0x01
#define U8G2_DRAW_UPPER_LEFT  0x02
#define U8G2_DRAW_LOWER_LEFT  0x04
#define U8G2_DRAW_LOWER_RIGHT 0x08
#define U8G2_DRAW_ALL         0x0F

// Forward declaration for OLED service
void oled_set_lines(const char* line1, const char* line2, const char* line3);
void oled_clear();

// Base U8g2 class
class U8G2 {
public:
    U8G2() : _width(256), _height(40), _powerSave(false) {}  // Match OLED_HEIGHT
    virtual ~U8G2() {}
    
    // Initialization
    bool begin() { return true; }
    void initDisplay() {}
    
    // Power management
    void setPowerSave(uint8_t is_enable) { _powerSave = (is_enable != 0); }
    bool getPowerSave() const { return _powerSave; }
    
    // Display control
    void clearBuffer() { 
        // Clear the OLED pixel buffer and stored lines
        oled_clear();
    }
    void sendBuffer() { 
        if (g_display) g_display->oledRefresh(); 
    }
    void clearDisplay() { clearBuffer(); sendBuffer(); }
    
    // Bus clock
    void setBusClock(uint32_t clock_speed) {}
    
    // Drawing functions
    void drawPixel(uint16_t x, uint16_t y) {
        if (g_display) g_display->oledSetPixel(x, y, true);
    }
    void drawHLine(uint16_t x, uint16_t y, uint16_t w) {
        if (g_display && w > 0) {
            for (uint16_t i = 0; i < w; i++) {
                g_display->oledSetPixel(x + i, y, true);
            }
        }
    }
    void drawVLine(uint16_t x, uint16_t y, uint16_t h) {
        if (g_display) {
            for (uint16_t i = 0; i < h; i++) {
                g_display->oledSetPixel(x, y + i, true);
            }
        }
    }
    void drawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
        // Simple line drawing using Bresenham's algorithm
        if (g_display) {
            int dx = abs((int)x1 - (int)x0);
            int dy = abs((int)y1 - (int)y0);
            int sx = x0 < x1 ? 1 : -1;
            int sy = y0 < y1 ? 1 : -1;
            int err = dx - dy;
            
            while (true) {
                g_display->oledSetPixel(x0, y0, true);
                if (x0 == x1 && y0 == y1) break;
                int e2 = 2 * err;
                if (e2 > -dy) { err -= dy; x0 += sx; }
                if (e2 < dx) { err += dx; y0 += sy; }
            }
        }
    }
    void drawFrame(uint16_t x, uint16_t y, uint16_t w, uint16_t h) {}
    void drawBox(uint16_t x, uint16_t y, uint16_t w, uint16_t h) {}
    void drawRFrame(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t r) {}
    void drawRBox(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t r) {}
    void drawCircle(uint16_t x0, uint16_t y0, uint16_t rad, uint8_t opt = U8G2_DRAW_ALL) {}
    void drawDisc(uint16_t x0, uint16_t y0, uint16_t rad, uint8_t opt = U8G2_DRAW_ALL) {}
    void drawEllipse(uint16_t x0, uint16_t y0, uint16_t rx, uint16_t ry, uint8_t opt = U8G2_DRAW_ALL) {}
    void drawFilledEllipse(uint16_t x0, uint16_t y0, uint16_t rx, uint16_t ry, uint8_t opt = U8G2_DRAW_ALL) {}
    void drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2) {}
    
    // Bitmap drawing
    void drawXBM(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint8_t* bitmap) {}
    void drawXBMP(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint8_t* bitmap) {}
    
    // Text functions
    void setFont(const uint8_t* font) { _font = font; }
    void setFontMode(uint8_t is_transparent) {}
    void setFontDirection(uint8_t dir) {}
    void setFontPosTop() {}
    void setFontPosCenter() {}
    void setFontPosBottom() {}
    void setFontPosBaseline() {}
    void setFontRefHeightText() {}
    void setFontRefHeightExtendedText() {}
    void setFontRefHeightAll() {}
    void setBitmapMode(uint8_t mode) {}
    
    void setDrawColor(uint8_t color) { _drawColor = color; }
    uint8_t getDrawColor() const { return _drawColor; }
    
    uint16_t drawStr(uint16_t x, uint16_t y, const char* str) {
        // Draw to OLED display
        extern DesktopDisplay* g_display;
        int fontSize = 12;  // Default to 18pt equivalent
        
        if (str && g_display && _drawColor) {
            if (!_font) fontSize = 8;
            g_display->oledDrawText(str, x, y, fontSize);
        }
        // Return actual rendered width (must match getStrWidth)
        if (str && g_display) {
            return g_display->oledGetTextWidth(str, fontSize);
        }
        return str ? strlen(str) * 10 : 0;
    }
    
    uint16_t drawUTF8(uint16_t x, uint16_t y, const char* str) {
        return drawStr(x, y, str);
    }
    
    void drawGlyph(uint16_t x, uint16_t y, uint16_t encoding) {}
    
    // Text measurement - use actual SDL font width
    uint16_t getStrWidth(const char* str) {
        extern DesktopDisplay* g_display;
        if (str && g_display) {
            return g_display->oledGetTextWidth(str, 12);
        }
        return str ? strlen(str) * 10 : 0;
    }
    
    uint16_t getUTF8Width(const char* str) {
        return getStrWidth(str);
    }
    
    uint8_t getMaxCharHeight() { return 8; }
    uint8_t getMaxCharWidth() { return 6; }
    uint8_t getAscent() { return 7; }
    uint8_t getDescent() { return 1; }
    
    // Display dimensions
    uint16_t getDisplayWidth() const { return _width; }
    uint16_t getDisplayHeight() const { return _height; }
    
    // Contrast
    void setContrast(uint8_t value) { _contrast = value; }
    
protected:
    uint16_t _width;
    uint16_t _height;
    bool _powerSave;
    const uint8_t* _font = nullptr;
    uint8_t _drawColor = 1;
    uint8_t _contrast = 255;
    std::string _lastStr;
    uint16_t _lastX = 0;
    uint16_t _lastY = 0;
};

// Specific OLED display class used by PocketMage (256x32 SPI OLED)
class U8G2_SSD1326_ER_256X32_F_4W_HW_SPI : public U8G2 {
public:
    U8G2_SSD1326_ER_256X32_F_4W_HW_SPI(uint8_t rotation, uint8_t cs, uint8_t dc, uint8_t reset = 255)
        : U8G2() {
        _width = 256;
        _height = 32;
    }
};

// U8g2 fonts (extern declarations - actual data not needed for emulator)
extern const uint8_t u8g2_font_5x7_tf[];
extern const uint8_t u8g2_font_6x10_tf[];
extern const uint8_t u8g2_font_ncenB08_tr[];
extern const uint8_t u8g2_font_ncenB10_tr[];
extern const uint8_t u8g2_font_ncenB12_tr[];
extern const uint8_t u8g2_font_ncenB14_tr[];
extern const uint8_t u8g2_font_helvB08_tr[];
extern const uint8_t u8g2_font_helvB10_tr[];
extern const uint8_t u8g2_font_helvB12_tr[];
extern const uint8_t u8g2_font_helvR08_tr[];
extern const uint8_t u8g2_font_profont12_tr[];
extern const uint8_t u8g2_font_profont15_tr[];
extern const uint8_t u8g2_font_profont17_tr[];
extern const uint8_t u8g2_font_profont22_tr[];
extern const uint8_t u8g2_font_t0_11_tf[];
extern const uint8_t u8g2_font_t0_12_tf[];
extern const uint8_t u8g2_font_t0_13_tf[];
extern const uint8_t u8g2_font_t0_14_tf[];
extern const uint8_t u8g2_font_t0_15_tf[];
extern const uint8_t u8g2_font_t0_16_tf[];
extern const uint8_t u8g2_font_luBIS18_tf[];
extern const uint8_t u8g2_font_luBS18_tf[];
extern const uint8_t u8g2_font_luIS18_tf[];
extern const uint8_t u8g2_font_lubR18_tf[];
extern const uint8_t u8g2_font_7x13B_tf[];
extern const uint8_t u8g2_font_7x13_tf[];
extern const uint8_t u8g2_font_5x8_tf[];
extern const uint8_t u8g2_font_6x12_tf[];
extern const uint8_t u8g2_font_9x15_tf[];
extern const uint8_t u8g2_font_10x20_tf[];
extern const uint8_t u8g2_font_ncenB18_tr[];
extern const uint8_t u8g2_font_ncenB24_tr[];

#endif // U8G2LIB_H
