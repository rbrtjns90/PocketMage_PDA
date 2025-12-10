/**
 * @file Adafruit_GFX.h
 * @brief Adafruit GFX graphics library mock for desktop emulator
 */

#ifndef ADAFRUIT_GFX_H
#define ADAFRUIT_GFX_H

#include "pocketmage/pocketmage_compat.h"

// GFX Glyph structure (individual character data)
typedef struct {
    uint16_t bitmapOffset;  // Pointer into GFXfont->bitmap
    uint8_t width;          // Bitmap dimensions in pixels
    uint8_t height;         // Bitmap dimensions in pixels
    uint8_t xAdvance;       // Distance to advance cursor (x axis)
    int8_t xOffset;         // X dist from cursor pos to UL corner
    int8_t yOffset;         // Y dist from cursor pos to UL corner
} GFXglyph;

// GFX Font structure (matches Adafruit_GFX)
typedef struct {
    uint8_t* bitmap;      // Glyph bitmaps, concatenated
    GFXglyph* glyph;      // Glyph array
    uint16_t first;       // ASCII extents (first char)
    uint16_t last;        // ASCII extents (last char)
    uint8_t yAdvance;     // Newline distance (y axis)
} GFXfont;

// Base graphics class
class Adafruit_GFX {
public:
    Adafruit_GFX(int16_t w, int16_t h) : _width(w), _height(h), cursor_x(0), cursor_y(0),
        textcolor(0x0000), textbgcolor(0xFFFF), textsize_x(1), textsize_y(1),
        wrap(true), _cp437(false), gfxFont(nullptr) {}
    
    virtual ~Adafruit_GFX() {}
    
    // Pure virtual - must be implemented by subclass
    virtual void drawPixel(int16_t x, int16_t y, uint16_t color) = 0;
    
    // Virtual methods with default implementations
    virtual void fillScreen(uint16_t color);
    virtual void drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color);
    virtual void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color);
    virtual void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
    
    // Drawing primitives
    void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
    void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
    void drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color);
    void fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color);
    void drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);
    void fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);
    void drawRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color);
    void fillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color);
    
    // Bitmap drawing
    void drawBitmap(int16_t x, int16_t y, const uint8_t bitmap[], int16_t w, int16_t h, uint16_t color);
    void drawBitmap(int16_t x, int16_t y, const uint8_t bitmap[], int16_t w, int16_t h, uint16_t color, uint16_t bg);
    void drawBitmap(int16_t x, int16_t y, uint8_t* bitmap, int16_t w, int16_t h, uint16_t color);
    void drawBitmap(int16_t x, int16_t y, uint8_t* bitmap, int16_t w, int16_t h, uint16_t color, uint16_t bg);
    void drawXBitmap(int16_t x, int16_t y, const uint8_t bitmap[], int16_t w, int16_t h, uint16_t color);
    
    // Text functions
    void setCursor(int16_t x, int16_t y) { cursor_x = x; cursor_y = y; }
    int16_t getCursorX() const { return cursor_x; }
    int16_t getCursorY() const { return cursor_y; }
    
    void setTextColor(uint16_t c) { textcolor = c; textbgcolor = c; }
    void setTextColor(uint16_t c, uint16_t bg) { textcolor = c; textbgcolor = bg; }
    
    void setTextSize(uint8_t s) { textsize_x = textsize_y = (s > 0) ? s : 1; }
    void setTextSize(uint8_t sx, uint8_t sy) { textsize_x = (sx > 0) ? sx : 1; textsize_y = (sy > 0) ? sy : 1; }
    
    void setTextWrap(bool w) { wrap = w; }
    void cp437(bool x = true) { _cp437 = x; }
    
    void setFont(const GFXfont* f = nullptr) { gfxFont = f; }
    const GFXfont* getFont() const { return gfxFont; }
    
    // Text output
    size_t write(uint8_t c);
    size_t write(const char* str);
    size_t write(const uint8_t* buffer, size_t size);
    
    void print(const char* str);
    void print(const String& str);
    void print(char c);
    void print(int n, int base = DEC);
    void print(unsigned int n, int base = DEC);
    void print(long n, int base = DEC);
    void print(unsigned long n, int base = DEC);
    void print(double n, int digits = 2);
    
    void println(const char* str);
    void println(const String& str);
    void println(char c);
    void println(int n, int base = DEC);
    void println(unsigned int n, int base = DEC);
    void println(long n, int base = DEC);
    void println(unsigned long n, int base = DEC);
    void println(double n, int digits = 2);
    void println();
    
    // Text bounds
    void getTextBounds(const char* str, int16_t x, int16_t y, 
                       int16_t* x1, int16_t* y1, uint16_t* w, uint16_t* h);
    void getTextBounds(const String& str, int16_t x, int16_t y,
                       int16_t* x1, int16_t* y1, uint16_t* w, uint16_t* h);
    
    // Flush buffered characters to display
    void flushCharBuffer();
    
    // Dimensions
    int16_t width() const { return _width; }
    int16_t height() const { return _height; }
    
    void setRotation(uint8_t r);
    uint8_t getRotation() const { return rotation; }
    
protected:
    int16_t _width, _height;
    int16_t cursor_x, cursor_y;
    uint16_t textcolor, textbgcolor;
    uint8_t textsize_x, textsize_y;
    uint8_t rotation = 0;
    bool wrap;
    bool _cp437;
    const GFXfont* gfxFont;
    
    void charBounds(unsigned char c, int16_t* x, int16_t* y, 
                    int16_t* minx, int16_t* miny, int16_t* maxx, int16_t* maxy);
    void drawChar(int16_t x, int16_t y, unsigned char c, uint16_t color, uint16_t bg, uint8_t size_x, uint8_t size_y);
};

#endif // ADAFRUIT_GFX_H
