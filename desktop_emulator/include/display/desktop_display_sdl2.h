/**
 * @file desktop_display_sdl2.h
 * @brief SDL2-based display backend for PocketMage desktop emulator
 * 
 * This header defines the cross-platform interface. Platform-specific
 * implementations are in:
 *   - desktop_display_sdl2.cpp (common code)
 *   - desktop_display_macos.cpp (macOS-specific)
 *   - desktop_display_linux.cpp (Linux-specific)  
 *   - desktop_display_windows.cpp (Windows-specific)
 */

#ifndef DESKTOP_DISPLAY_SDL2_H
#define DESKTOP_DISPLAY_SDL2_H

// Platform detection
#if defined(_WIN32) || defined(_WIN64)
    #define PLATFORM_WINDOWS
#elif defined(__APPLE__) && defined(__MACH__)
    #define PLATFORM_MACOS
#elif defined(__linux__)
    #define PLATFORM_LINUX
#else
    #error "Unsupported platform"
#endif

#include <SDL.h>
#include <SDL_ttf.h>
#include <string>
#include <vector>
#include <queue>
#include <mutex>
#include <cstdint>

// Display dimensions (matching real hardware)
#define EINK_WIDTH 310
#define EINK_HEIGHT 260
#define OLED_WIDTH 256
#define OLED_HEIGHT 40  // Slightly taller than real 32px to prevent text cutoff

// Scale factor for visibility on desktop
#define DISPLAY_SCALE 3

// Device frame dimensions and layout
#define DEVICE_BEZEL 0
#define OLED_GAP 15
#define DEVICE_WIDTH (EINK_WIDTH * DISPLAY_SCALE)
#define DEVICE_HEIGHT (EINK_HEIGHT * DISPLAY_SCALE + OLED_HEIGHT * DISPLAY_SCALE + OLED_GAP * 2)
#define EINK_OFFSET_X 0
#define EINK_OFFSET_Y 0
#define OLED_OFFSET_X ((DEVICE_WIDTH - OLED_WIDTH * DISPLAY_SCALE) / 2)
#define OLED_OFFSET_Y (EINK_HEIGHT * DISPLAY_SCALE + OLED_GAP)

// Window positioning (legacy, kept for compatibility)
#define WINDOW_PADDING 20

/**
 * @class DesktopDisplay
 * @brief Cross-platform SDL2-based display emulator
 * 
 * Provides emulation of the PocketMage's dual-display system:
 * - E-ink display (310x240, black/white)
 * - OLED display (256x32, for status)
 */
class DesktopDisplay {
public:
    DesktopDisplay();
    ~DesktopDisplay();
    
    // Prevent copying
    DesktopDisplay(const DesktopDisplay&) = delete;
    DesktopDisplay& operator=(const DesktopDisplay&) = delete;
    
    // ========== Lifecycle ==========
    bool init();
    void shutdown();
    bool isInitialized() const { return _initialized; }
    
    // ========== Event Loop ==========
    /** @return false if quit requested */
    bool handleEvents();
    
    /** Present both displays to screen */
    void present();
    
    // ========== E-ink Display ==========
    void einkClear();
    void einkSetPixel(int x, int y, bool black);
    void einkDrawLine(int x0, int y0, int x1, int y1, bool black = true);
    void einkDrawRect(int x, int y, int w, int h, bool filled, bool black = true);
    void einkDrawCircle(int x, int y, int r, bool filled, bool black = true);
    void einkDrawText(const char* text, int x, int y, int fontSize = 12, bool inverted = false);
    void einkDrawBitmap(int x, int y, const unsigned char* bitmap, int w, int h, bool black = true);
    void fillRect(int x, int y, int w, int h, uint32_t color);  // Generic fill rect
    void einkRefresh();
    void einkPartialRefresh();
    void einkForceFullRefresh();
    
    // E-ink flash animation control
    void setEinkFlashEnabled(bool enabled);
    bool isEinkFlashEnabled() const;
    void einkGetTextBounds(const char* text, int x, int y, 
                           int16_t* x1, int16_t* y1, uint16_t* w, uint16_t* h);
    
    // ========== OLED Display ==========
    void oledClear();
    void oledSetPixel(int x, int y, bool on);
    void oledDrawText(const char* text, int x, int y, int fontSize = 8);
    int oledGetTextWidth(const char* text, int fontSize = 12);
    void oledRefresh();
    
    // ========== Input ==========
    char getLastKey();
    bool hasKeyEvent();
    void clearKeyEvent();
    
    // UTF-8 text input support
    bool hasUTF8Input();
    std::string getUTF8Input();
    
    // ========== Framebuffer Access ==========
    uint8_t* getEinkFramebuffer() { return _einkBuffer.data(); }
    uint8_t* getOledFramebuffer() { return _oledBuffer.data(); }
    const uint8_t* getEinkFramebuffer() const { return _einkBuffer.data(); }
    const uint8_t* getOledFramebuffer() const { return _oledBuffer.data(); }
    
protected:
    // Platform-specific initialization (implemented per-platform)
    bool platformInit();
    void platformShutdown();
    std::string platformGetFontPath();
    
private:
    // ========== SDL Resources ==========
    SDL_Window* _einkWindow = nullptr;
    SDL_Window* _oledWindow = nullptr;
    SDL_Renderer* _einkRenderer = nullptr;
    SDL_Renderer* _oledRenderer = nullptr;
    SDL_Texture* _einkTexture = nullptr;
    SDL_Texture* _oledTexture = nullptr;
    
    // ========== Fonts ==========
    TTF_Font* _fontSmall = nullptr;   // 8pt
    TTF_Font* _fontMedium = nullptr;  // 12pt
    TTF_Font* _fontLarge = nullptr;   // 16pt
    
    // ========== Framebuffers ==========
    // 1 byte per pixel: 0=white/off, 1=black/on
    std::vector<uint8_t> _einkBuffer;
    std::vector<uint8_t> _oledBuffer;
    
    // ========== Input State ==========
    std::queue<char> _keyQueue;
    std::string _utf8Buffer;
    std::mutex _inputMutex;
    bool _shiftPressed = false;
    bool _ctrlPressed = false;
    bool _altPressed = false;
    
    // ========== State ==========
    bool _initialized = false;
    bool _needsEinkRefresh = true;
    bool _needsOledRefresh = true;
    bool _einkFlashEnabled = true;  // E-ink refresh flash animation
    
    // ========== Helper Methods ==========
    void doEinkFlashAnimation();
    void updateEinkTexture();
    void updateOledTexture();
    char sdlKeyToChar(SDL_Keycode key, uint16_t mod);
    bool loadFonts();
    void renderTextToBuffer(const char* text, int x, int y, int fontSize, 
                            std::vector<uint8_t>& buffer, int bufWidth, int bufHeight,
                            bool inverted = false);
    void drawLineImpl(int x0, int y0, int x1, int y1, bool black,
                      std::vector<uint8_t>& buffer, int width, int height);
    void drawCircleImpl(int cx, int cy, int r, bool filled, bool black,
                        std::vector<uint8_t>& buffer, int width, int height);
};

// Global display instance
extern DesktopDisplay* g_display;

#endif // DESKTOP_DISPLAY_SDL2_H
