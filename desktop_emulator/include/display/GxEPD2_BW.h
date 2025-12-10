/**
 * @file GxEPD2_BW.h
 * @brief E-ink display library mock for desktop emulator
 */

#ifndef GXEPD2_BW_H
#define GXEPD2_BW_H

#include "pocketmage/pocketmage_compat.h"
#include "display/Adafruit_GFX.h"

// Forward declaration
class DesktopDisplay;
extern DesktopDisplay* g_display;

// Display panel class
class GxEPD2_310_GDEQ031T10 {
public:
    static const int WIDTH = 310;
    static const int HEIGHT = 240;
    static volatile bool useFastFullUpdate;
    
    GxEPD2_310_GDEQ031T10(int cs, int dc, int rst, int busy) {}
};

// Main display template class
template<typename GxEPD2_Type, int page_height>
class GxEPD2_BW : public Adafruit_GFX {
public:
    GxEPD2_BW(GxEPD2_Type panel) 
        : Adafruit_GFX(GxEPD2_Type::WIDTH, GxEPD2_Type::HEIGHT)
        , _panel(panel) {}
    
    void init(uint32_t serial_diag_bitrate = 0, bool initial = true, 
              uint16_t reset_duration = 10, bool pulldown_rst_mode = false);
    
    void setRotation(uint8_t r) { _rotation = r; }
    uint8_t getRotation() const { return _rotation; }
    
    void setFullWindow();
    void setPartialWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h);
    
    void firstPage();
    bool nextPage();
    
    void display(bool partial_update_mode = false);
    void displayWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h);
    
    void powerOff();
    void hibernate();
    
    void clearScreen(uint8_t value = 0xFF);
    void writeScreenBuffer(uint8_t value = 0xFF);
    
    // Drawing functions (inherited from Adafruit_GFX, but we override some)
    void drawPixel(int16_t x, int16_t y, uint16_t color) override;
    void fillScreen(uint16_t color) override;
    void drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color) override;
    void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color) override;
    void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) override;
    
    
    // Additional methods used by PocketMage
    void refresh(bool partial_update_mode = false);
    void writeImage(const uint8_t* bitmap, int16_t x, int16_t y, int16_t w, int16_t h, 
                    bool invert = false, bool mirror_y = false, bool pgm = false);
    void writeImagePart(const uint8_t* bitmap, int16_t x_part, int16_t y_part, 
                        int16_t w_bitmap, int16_t h_bitmap, int16_t x, int16_t y, 
                        int16_t w, int16_t h, bool invert = false, bool mirror_y = false, bool pgm = false);
    
    int16_t width() const { return _width; }
    int16_t height() const { return _height; }
    
private:
    GxEPD2_Type _panel;
    uint8_t _rotation = 0;
    bool _using_partial_mode = false;
    bool _initial_refresh = true;
};

// Type alias used in PocketMage
using DisplayT = GxEPD2_BW<GxEPD2_310_GDEQ031T10, GxEPD2_310_GDEQ031T10::HEIGHT>;

// Template implementations are in the cpp file via explicit instantiation

#endif // GXEPD2_BW_H
