/**
 * @file desktop_display_sdl2.cpp
 * @brief Common SDL2 display implementation (cross-platform)
 * 
 * Platform-specific code is in:
 *   - desktop_display_macos.cpp
 *   - desktop_display_linux.cpp
 *   - desktop_display_windows.cpp
 */

#include "desktop_display_sdl2.h"
#include <iostream>
#include <cmath>
#include <algorithm>
#include <cstring>

// Global display instance
DesktopDisplay* g_display = nullptr;

// ============================================================================
// Constructor / Destructor
// ============================================================================

DesktopDisplay::DesktopDisplay() 
    : _einkBuffer(EINK_WIDTH * EINK_HEIGHT, 0)  // 0 = white
    , _oledBuffer(OLED_WIDTH * OLED_HEIGHT, 0)  // 0 = off
{
}

DesktopDisplay::~DesktopDisplay() {
    shutdown();
}

// ============================================================================
// Initialization
// ============================================================================

bool DesktopDisplay::init() {
    if (_initialized) return true;
    
    std::cout << "[Display] Initializing SDL2..." << std::endl;
    
    // Platform-specific pre-init
    if (!platformInit()) {
        std::cerr << "[Display] Platform init failed" << std::endl;
        return false;
    }
    
    // Initialize SDL (video, events, and audio for buzzer)
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_AUDIO) < 0) {
        std::cerr << "[Display] SDL_Init failed: " << SDL_GetError() << std::endl;
        return false;
    }
    
    // Initialize SDL_ttf
    if (TTF_Init() < 0) {
        std::cerr << "[Display] TTF_Init failed: " << TTF_GetError() << std::endl;
        SDL_Quit();
        return false;
    }
    
    // Create single combined window for the device
    std::cout << "[Display] Creating window: " << DEVICE_WIDTH << "x" << DEVICE_HEIGHT << std::endl;
    std::cout << "[Display] E-ink at: " << EINK_OFFSET_X << "," << EINK_OFFSET_Y 
              << " size: " << (EINK_WIDTH * DISPLAY_SCALE) << "x" << (EINK_HEIGHT * DISPLAY_SCALE) << std::endl;
    std::cout << "[Display] OLED at: " << OLED_OFFSET_X << "," << OLED_OFFSET_Y 
              << " size: " << (OLED_WIDTH * DISPLAY_SCALE) << "x" << (OLED_HEIGHT * DISPLAY_SCALE) << std::endl;
    
    _einkWindow = SDL_CreateWindow(
        "PocketMage Emulator",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        DEVICE_WIDTH,
        DEVICE_HEIGHT,
        SDL_WINDOW_SHOWN
    );
    
    if (!_einkWindow) {
        std::cerr << "[Display] Failed to create window: " << SDL_GetError() << std::endl;
        TTF_Quit();
        SDL_Quit();
        return false;
    }
    
    // Create renderer for the combined window
    _einkRenderer = SDL_CreateRenderer(_einkWindow, -1, 
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!_einkRenderer) {
        std::cerr << "[Display] Failed to create renderer: " << SDL_GetError() << std::endl;
        shutdown();
        return false;
    }
    
    // Set logical size to match window size (prevents HiDPI scaling issues)
    SDL_RenderSetLogicalSize(_einkRenderer, DEVICE_WIDTH, DEVICE_HEIGHT);
    
    // Create E-ink texture
    _einkTexture = SDL_CreateTexture(_einkRenderer, 
        SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING,
        EINK_WIDTH, EINK_HEIGHT);
    if (!_einkTexture) {
        std::cerr << "[Display] Failed to create E-ink texture: " << SDL_GetError() << std::endl;
        shutdown();
        return false;
    }
    
    // Create OLED texture (using same renderer)
    _oledTexture = SDL_CreateTexture(_einkRenderer,
        SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING,
        OLED_WIDTH, OLED_HEIGHT);
    if (!_oledTexture) {
        std::cerr << "[Display] Failed to create OLED texture: " << SDL_GetError() << std::endl;
        shutdown();
        return false;
    }
    
    // No separate OLED window/renderer needed
    _oledWindow = nullptr;
    _oledRenderer = nullptr;
    
    // Load fonts
    if (!loadFonts()) {
        std::cerr << "[Display] Warning: Failed to load fonts, text rendering may not work" << std::endl;
    }
    
    // Clear displays
    einkClear();
    oledClear();
    
    _initialized = true;
    std::cout << "[Display] Initialization complete" << std::endl;
    
    return true;
}

void DesktopDisplay::shutdown() {
    if (_fontSmall) { TTF_CloseFont(_fontSmall); _fontSmall = nullptr; }
    if (_fontMedium) { TTF_CloseFont(_fontMedium); _fontMedium = nullptr; }
    if (_fontLarge) { TTF_CloseFont(_fontLarge); _fontLarge = nullptr; }
    
    if (_einkTexture) { SDL_DestroyTexture(_einkTexture); _einkTexture = nullptr; }
    if (_oledTexture) { SDL_DestroyTexture(_oledTexture); _oledTexture = nullptr; }
    if (_einkRenderer) { SDL_DestroyRenderer(_einkRenderer); _einkRenderer = nullptr; }
    if (_oledRenderer) { SDL_DestroyRenderer(_oledRenderer); _oledRenderer = nullptr; }
    if (_einkWindow) { SDL_DestroyWindow(_einkWindow); _einkWindow = nullptr; }
    if (_oledWindow) { SDL_DestroyWindow(_oledWindow); _oledWindow = nullptr; }
    
    platformShutdown();
    
    TTF_Quit();
    SDL_Quit();
    
    _initialized = false;
}

bool DesktopDisplay::loadFonts() {
    std::string fontPath = platformGetFontPath();
    
    if (fontPath.empty()) {
        std::cerr << "[Display] No font path available" << std::endl;
        return false;
    }
    
    std::cout << "[Display] Loading fonts from: " << fontPath << std::endl;
    
    // Font sizes for SDL rendering
    _fontSmall = TTF_OpenFont(fontPath.c_str(), 10);
    _fontMedium = TTF_OpenFont(fontPath.c_str(), 12);
    _fontLarge = TTF_OpenFont(fontPath.c_str(), 16);
    
    if (!_fontSmall || !_fontMedium || !_fontLarge) {
        std::cerr << "[Display] Failed to load font: " << TTF_GetError() << std::endl;
        return false;
    }
    
    return true;
}

// ============================================================================
// Event Handling
// ============================================================================

bool DesktopDisplay::handleEvents() {
    SDL_Event event;
    
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                return false;
                
            case SDL_WINDOWEVENT:
                if (event.window.event == SDL_WINDOWEVENT_CLOSE) {
                    return false;
                }
                break;
                
            case SDL_KEYDOWN: {
                // Track modifier state
                uint16_t mod = event.key.keysym.mod;
                _shiftPressed = (mod & KMOD_SHIFT) != 0;
                _ctrlPressed = (mod & KMOD_CTRL) != 0;
                _altPressed = (mod & KMOD_ALT) != 0;
                
                char key = sdlKeyToChar(event.key.keysym.sym, mod);
                if (key != 0) {
                    std::lock_guard<std::mutex> lock(_inputMutex);
                    _keyQueue.push(key);
                }
                break;
            }
            
            case SDL_KEYUP:
                _shiftPressed = (event.key.keysym.mod & KMOD_SHIFT) != 0;
                _ctrlPressed = (event.key.keysym.mod & KMOD_CTRL) != 0;
                _altPressed = (event.key.keysym.mod & KMOD_ALT) != 0;
                break;
                
            case SDL_TEXTINPUT: {
                // UTF-8 text input
                std::lock_guard<std::mutex> lock(_inputMutex);
                _utf8Buffer += event.text.text;
                break;
            }
        }
    }
    
    return true;
}

char DesktopDisplay::sdlKeyToChar(SDL_Keycode key, uint16_t mod) {
    bool shift = (mod & KMOD_SHIFT) != 0;
    (void)mod;  // ctrl not currently used
    
    // PocketMage special key codes (from pocketmage_kb.cpp keymaps):
    //   8  = Backspace
    //   9  = Tab
    //   12 = ESC / Home (return to home screen)
    //   13 = Enter
    //   17 = Shift toggle
    //   18 = Func toggle
    //   19 = Left arrow
    //   20 = Select (middle/down)
    //   21 = Right arrow
    //   28 = Up arrow (in SHFT mode)
    //   29 = Select (in SHFT mode)
    //   30 = Down arrow (in SHFT mode)
    
    switch (key) {
        // Navigation
        case SDLK_RETURN:
        case SDLK_KP_ENTER:
            return 13;  // Enter
        case SDLK_BACKSPACE:
            return 8;   // Backspace
        case SDLK_ESCAPE:
        case SDLK_HOME:
            return 12;  // ESC / Home - return to home screen
        case SDLK_TAB:
            return 9;   // Tab
        case SDLK_SPACE:
            return ' '; // Space
            
        // Arrow keys
        case SDLK_LEFT:
            return 19;  // Left arrow
        case SDLK_RIGHT:
            return 21;  // Right arrow
        case SDLK_UP:
            return 28;  // Up arrow (SHFT mode code)
        case SDLK_DOWN:
            return 20;  // Down / Select
            
        // Modifier toggles (F1/F2 to toggle keyboard modes)
        case SDLK_LSHIFT:
        case SDLK_RSHIFT:
            return 17;  // Shift toggle
        case SDLK_LALT:
        case SDLK_RALT:
        case SDLK_F1:
            return 18;  // Func toggle
            
        // Delete
        case SDLK_DELETE:
            return 8;   // Map delete to backspace
            
        default:
            break;
    }
    
    // Letter keys (handled by SDL_TEXTINPUT for proper UTF-8)
    // Return 0 to let text input handle it
    if (key >= SDLK_a && key <= SDLK_z) {
        return 0;  // Let SDL_TEXTINPUT handle it
    }
    
    // Number keys
    if (key >= SDLK_0 && key <= SDLK_9 && !shift) {
        return 0;  // Let SDL_TEXTINPUT handle it
    }
    
    return 0;
}

char DesktopDisplay::getLastKey() {
    std::lock_guard<std::mutex> lock(_inputMutex);
    if (_keyQueue.empty()) return 0;
    char key = _keyQueue.front();
    _keyQueue.pop();
    return key;
}

bool DesktopDisplay::hasKeyEvent() {
    std::lock_guard<std::mutex> lock(_inputMutex);
    return !_keyQueue.empty();
}

void DesktopDisplay::clearKeyEvent() {
    std::lock_guard<std::mutex> lock(_inputMutex);
    while (!_keyQueue.empty()) _keyQueue.pop();
}

bool DesktopDisplay::hasUTF8Input() {
    std::lock_guard<std::mutex> lock(_inputMutex);
    return !_utf8Buffer.empty();
}

std::string DesktopDisplay::getUTF8Input() {
    std::lock_guard<std::mutex> lock(_inputMutex);
    std::string result = _utf8Buffer;
    _utf8Buffer.clear();
    return result;
}

// ============================================================================
// Rendering
// ============================================================================

void DesktopDisplay::present() {
    if (!_initialized) return;
    
    // Update E-ink texture if needed
    if (_needsEinkRefresh) {
        updateEinkTexture();
        _needsEinkRefresh = false;
    }
    
    // Update OLED texture if needed
    if (_needsOledRefresh) {
        updateOledTexture();
        _needsOledRefresh = false;
    }
    
    // Clear with black background
    SDL_SetRenderDrawColor(_einkRenderer, 0, 0, 0, 255);
    SDL_RenderClear(_einkRenderer);
    
    // Draw E-ink display
    SDL_Rect einkDest = {
        EINK_OFFSET_X,
        EINK_OFFSET_Y,
        EINK_WIDTH * DISPLAY_SCALE,
        EINK_HEIGHT * DISPLAY_SCALE
    };
    SDL_RenderCopy(_einkRenderer, _einkTexture, nullptr, &einkDest);
    
    // Draw OLED display
    SDL_Rect oledDest = {
        OLED_OFFSET_X,
        OLED_OFFSET_Y,
        OLED_WIDTH * DISPLAY_SCALE,
        OLED_HEIGHT * DISPLAY_SCALE
    };
    SDL_RenderCopy(_einkRenderer, _oledTexture, nullptr, &oledDest);
    
    SDL_RenderPresent(_einkRenderer);
}

void DesktopDisplay::updateEinkTexture() {
    if (!_einkTexture) return;
    
    void* pixels;
    int pitch;
    
    if (SDL_LockTexture(_einkTexture, nullptr, &pixels, &pitch) != 0) {
        std::cerr << "[Display] Failed to lock E-ink texture" << std::endl;
        return;
    }
    
    uint8_t* dst = static_cast<uint8_t*>(pixels);
    
    // Debug: count black pixels
    static int debugCounter = 0;
    int blackPixels = 0;
    
    for (int y = 0; y < EINK_HEIGHT; y++) {
        for (int x = 0; x < EINK_WIDTH; x++) {
            int srcIdx = y * EINK_WIDTH + x;
            int dstIdx = y * pitch + x * 3;
            
            // E-ink: 0 = white, 1 = black
            uint8_t color = _einkBuffer[srcIdx] ? 0x00 : 0xFF;
            if (_einkBuffer[srcIdx]) blackPixels++;
            dst[dstIdx + 0] = color;  // R
            dst[dstIdx + 1] = color;  // G
            dst[dstIdx + 2] = color;  // B
        }
    }
    
    // Print debug info every 100 updates
    if (debugCounter++ % 100 == 0) {
        std::cout << "[Display] E-ink update: " << blackPixels << " black pixels" << std::endl;
    }
    
    SDL_UnlockTexture(_einkTexture);
}

void DesktopDisplay::updateOledTexture() {
    if (!_oledTexture) return;
    
    void* pixels;
    int pitch;
    
    if (SDL_LockTexture(_oledTexture, nullptr, &pixels, &pitch) != 0) {
        std::cerr << "[Display] Failed to lock OLED texture" << std::endl;
        return;
    }
    
    uint8_t* dst = static_cast<uint8_t*>(pixels);
    
    for (int y = 0; y < OLED_HEIGHT; y++) {
        for (int x = 0; x < OLED_WIDTH; x++) {
            int srcIdx = y * OLED_WIDTH + x;
            int dstIdx = y * pitch + x * 3;
            
            // OLED: 0 = off (black), 1 = on (cyan/blue-ish for OLED look)
            if (_oledBuffer[srcIdx]) {
                dst[dstIdx + 0] = 0x00;  // R
                dst[dstIdx + 1] = 0xFF;  // G
                dst[dstIdx + 2] = 0xFF;  // B (cyan)
            } else {
                dst[dstIdx + 0] = 0x00;  // R
                dst[dstIdx + 1] = 0x00;  // G
                dst[dstIdx + 2] = 0x00;  // B (black)
            }
        }
    }
    
    SDL_UnlockTexture(_oledTexture);
}

// ============================================================================
// E-ink Display Operations
// ============================================================================

void DesktopDisplay::einkClear() {
    std::fill(_einkBuffer.begin(), _einkBuffer.end(), 0);  // 0 = white
    _needsEinkRefresh = true;
}

void DesktopDisplay::einkSetPixel(int x, int y, bool black) {
    if (x < 0 || x >= EINK_WIDTH || y < 0 || y >= EINK_HEIGHT) return;
    _einkBuffer[y * EINK_WIDTH + x] = black ? 1 : 0;
    _needsEinkRefresh = true;
}

void DesktopDisplay::einkDrawLine(int x0, int y0, int x1, int y1, bool black) {
    drawLineImpl(x0, y0, x1, y1, black, _einkBuffer, EINK_WIDTH, EINK_HEIGHT);
    _needsEinkRefresh = true;
}

void DesktopDisplay::drawLineImpl(int x0, int y0, int x1, int y1, bool black,
                                   std::vector<uint8_t>& buffer, int width, int height) {
    // Bresenham's line algorithm
    int dx = std::abs(x1 - x0);
    int dy = std::abs(y1 - y0);
    int sx = x0 < x1 ? 1 : -1;
    int sy = y0 < y1 ? 1 : -1;
    int err = dx - dy;
    
    while (true) {
        if (x0 >= 0 && x0 < width && y0 >= 0 && y0 < height) {
            buffer[y0 * width + x0] = black ? 1 : 0;
        }
        
        if (x0 == x1 && y0 == y1) break;
        
        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x0 += sx; }
        if (e2 < dx) { err += dx; y0 += sy; }
    }
}

void DesktopDisplay::fillRect(int x, int y, int w, int h, uint32_t color) {
    // Convert color to black/white (0 = white, non-zero = black)
    bool black = (color != 0xFFFFFF && color != 0);
    einkDrawRect(x, y, w, h, true, black);
}

void DesktopDisplay::einkDrawRect(int x, int y, int w, int h, bool filled, bool black) {
    if (filled) {
        for (int py = y; py < y + h; py++) {
            for (int px = x; px < x + w; px++) {
                if (px >= 0 && px < EINK_WIDTH && py >= 0 && py < EINK_HEIGHT) {
                    _einkBuffer[py * EINK_WIDTH + px] = black ? 1 : 0;
                }
            }
        }
    } else {
        // Top and bottom edges
        for (int px = x; px < x + w; px++) {
            if (px >= 0 && px < EINK_WIDTH) {
                if (y >= 0 && y < EINK_HEIGHT)
                    _einkBuffer[y * EINK_WIDTH + px] = black ? 1 : 0;
                if (y + h - 1 >= 0 && y + h - 1 < EINK_HEIGHT)
                    _einkBuffer[(y + h - 1) * EINK_WIDTH + px] = black ? 1 : 0;
            }
        }
        // Left and right edges
        for (int py = y; py < y + h; py++) {
            if (py >= 0 && py < EINK_HEIGHT) {
                if (x >= 0 && x < EINK_WIDTH)
                    _einkBuffer[py * EINK_WIDTH + x] = black ? 1 : 0;
                if (x + w - 1 >= 0 && x + w - 1 < EINK_WIDTH)
                    _einkBuffer[py * EINK_WIDTH + (x + w - 1)] = black ? 1 : 0;
            }
        }
    }
    _needsEinkRefresh = true;
}

void DesktopDisplay::einkDrawCircle(int cx, int cy, int r, bool filled, bool black) {
    drawCircleImpl(cx, cy, r, filled, black, _einkBuffer, EINK_WIDTH, EINK_HEIGHT);
    _needsEinkRefresh = true;
}

void DesktopDisplay::drawCircleImpl(int cx, int cy, int r, bool filled, bool black,
                                     std::vector<uint8_t>& buffer, int width, int height) {
    // Midpoint circle algorithm
    int x = r;
    int y = 0;
    int err = 0;
    
    auto setPixel = [&](int px, int py) {
        if (px >= 0 && px < width && py >= 0 && py < height) {
            buffer[py * width + px] = black ? 1 : 0;
        }
    };
    
    auto drawHLine = [&](int x1, int x2, int py) {
        if (py < 0 || py >= height) return;
        if (x1 > x2) std::swap(x1, x2);
        for (int px = std::max(0, x1); px <= std::min(width - 1, x2); px++) {
            buffer[py * width + px] = black ? 1 : 0;
        }
    };
    
    while (x >= y) {
        if (filled) {
            drawHLine(cx - x, cx + x, cy + y);
            drawHLine(cx - x, cx + x, cy - y);
            drawHLine(cx - y, cx + y, cy + x);
            drawHLine(cx - y, cx + y, cy - x);
        } else {
            setPixel(cx + x, cy + y);
            setPixel(cx - x, cy + y);
            setPixel(cx + x, cy - y);
            setPixel(cx - x, cy - y);
            setPixel(cx + y, cy + x);
            setPixel(cx - y, cy + x);
            setPixel(cx + y, cy - x);
            setPixel(cx - y, cy - x);
        }
        
        y++;
        if (err <= 0) {
            err += 2 * y + 1;
        }
        if (err > 0) {
            x--;
            err -= 2 * x + 1;
        }
    }
}

void DesktopDisplay::einkDrawText(const char* text, int x, int y, int fontSize, bool inverted) {
    if (!text || !text[0]) return;
    renderTextToBuffer(text, x, y, fontSize, _einkBuffer, EINK_WIDTH, EINK_HEIGHT, inverted);
    _needsEinkRefresh = true;
}

void DesktopDisplay::renderTextToBuffer(const char* text, int x, int y, int fontSize,
                                         std::vector<uint8_t>& buffer, int bufWidth, int bufHeight,
                                         bool inverted) {
    TTF_Font* font = nullptr;
    if (fontSize <= 10) font = _fontSmall;
    else if (fontSize <= 14) font = _fontMedium;
    else font = _fontLarge;
    
    if (!font) {
        // Fallback: simple bitmap font rendering
        int charWidth = 6;
        int charHeight = 8;
        for (int i = 0; text[i]; i++) {
            int cx = x + i * charWidth;
            // Just draw a placeholder rectangle for each character
            for (int py = 0; py < charHeight; py++) {
                for (int px = 0; px < charWidth - 1; px++) {
                    int bx = cx + px;
                    int by = y + py;
                    if (bx >= 0 && bx < bufWidth && by >= 0 && by < bufHeight) {
                        buffer[by * bufWidth + bx] = inverted ? 1 : 0;  // 1 = black, 0 = white
                    }
                }
            }
        }
        return;
    }
    
    // Render with SDL_ttf
    SDL_Color color = inverted ? SDL_Color{255, 255, 255, 255} : SDL_Color{0, 0, 0, 255};
    SDL_Surface* surface = TTF_RenderUTF8_Solid(font, text, color);
    
    if (!surface) return;
    
    // Copy surface to buffer
    SDL_LockSurface(surface);
    
    for (int py = 0; py < surface->h; py++) {
        for (int px = 0; px < surface->w; px++) {
            int bx = x + px;
            int by = y + py;
            
            if (bx < 0 || bx >= bufWidth || by < 0 || by >= bufHeight) continue;
            
            // Get pixel from surface
            uint8_t* pixels = static_cast<uint8_t*>(surface->pixels);
            uint8_t pixel = pixels[py * surface->pitch + px];
            
            if (pixel) {
                buffer[by * bufWidth + bx] = inverted ? 1 : 0;  // 1 = black, 0 = white
            }
        }
    }
    
    SDL_UnlockSurface(surface);
    SDL_FreeSurface(surface);
}

void DesktopDisplay::einkDrawBitmap(int x, int y, const unsigned char* bitmap, int w, int h, bool black) {
    if (!bitmap) return;
    
    // Bitmap is stored as 1 bit per pixel, MSB first
    int byteWidth = (w + 7) / 8;
    
    for (int py = 0; py < h; py++) {
        for (int px = 0; px < w; px++) {
            int byteIdx = py * byteWidth + px / 8;
            int bitIdx = 7 - (px % 8);
            bool pixelSet = (bitmap[byteIdx] >> bitIdx) & 1;
            
            int bx = x + px;
            int by = y + py;
            
            if (bx >= 0 && bx < EINK_WIDTH && by >= 0 && by < EINK_HEIGHT) {
                if (pixelSet) {
                    _einkBuffer[by * EINK_WIDTH + bx] = black ? 1 : 0;
                }
            }
        }
    }
    _needsEinkRefresh = true;
}

void DesktopDisplay::einkRefresh() {
    // Simulate e-ink refresh with black/white flash animation
    if (_einkFlashEnabled) {
        doEinkFlashAnimation();
    }
    _needsEinkRefresh = true;
}

void DesktopDisplay::einkPartialRefresh() {
    // Partial refresh - no flash animation (faster update)
    _needsEinkRefresh = true;
}

void DesktopDisplay::einkForceFullRefresh() {
    // Full refresh with flash animation
    if (_einkFlashEnabled) {
        doEinkFlashAnimation();
    }
    _needsEinkRefresh = true;
}

void DesktopDisplay::doEinkFlashAnimation() {
    if (!_einkTexture || !_einkRenderer) return;
    
    // Save current buffer
    std::vector<uint8_t> savedBuffer = _einkBuffer;
    
    SDL_Rect einkDest = {
        EINK_OFFSET_X,
        EINK_OFFSET_Y,
        EINK_WIDTH * DISPLAY_SCALE,
        EINK_HEIGHT * DISPLAY_SCALE
    };
    
    // Flash sequence: black -> white -> black -> white -> final image
    // This simulates real e-ink refresh behavior
    for (int flash = 0; flash < 2; flash++) {
        // Fill black
        std::fill(_einkBuffer.begin(), _einkBuffer.end(), 1);
        updateEinkTexture();
        SDL_SetRenderDrawColor(_einkRenderer, 0, 0, 0, 255);
        SDL_RenderClear(_einkRenderer);
        SDL_RenderCopy(_einkRenderer, _einkTexture, nullptr, &einkDest);
        
        // Also render OLED so it stays visible
        SDL_Rect oledDest = {
            OLED_OFFSET_X,
            OLED_OFFSET_Y,
            OLED_WIDTH * DISPLAY_SCALE,
            OLED_HEIGHT * DISPLAY_SCALE
        };
        SDL_RenderCopy(_einkRenderer, _oledTexture, nullptr, &oledDest);
        SDL_RenderPresent(_einkRenderer);
        SDL_Delay(50);  // 50ms black
        
        // Fill white
        std::fill(_einkBuffer.begin(), _einkBuffer.end(), 0);
        updateEinkTexture();
        SDL_SetRenderDrawColor(_einkRenderer, 0, 0, 0, 255);
        SDL_RenderClear(_einkRenderer);
        SDL_RenderCopy(_einkRenderer, _einkTexture, nullptr, &einkDest);
        SDL_RenderCopy(_einkRenderer, _oledTexture, nullptr, &oledDest);
        SDL_RenderPresent(_einkRenderer);
        SDL_Delay(50);  // 50ms white
    }
    
    // Restore original buffer
    _einkBuffer = savedBuffer;
}

void DesktopDisplay::setEinkFlashEnabled(bool enabled) {
    _einkFlashEnabled = enabled;
}

bool DesktopDisplay::isEinkFlashEnabled() const {
    return _einkFlashEnabled;
}

void DesktopDisplay::einkGetTextBounds(const char* text, int x, int y,
                                        int16_t* x1, int16_t* y1, uint16_t* w, uint16_t* h) {
    if (!text || !text[0]) {
        if (x1) *x1 = x;
        if (y1) *y1 = y;
        if (w) *w = 0;
        if (h) *h = 0;
        return;
    }
    
    TTF_Font* font = _fontMedium ? _fontMedium : _fontSmall;
    
    if (font) {
        int tw, th;
        TTF_SizeUTF8(font, text, &tw, &th);
        if (x1) *x1 = x;
        if (y1) *y1 = y;
        if (w) *w = tw;
        if (h) *h = th;
    } else {
        // Fallback estimate
        if (x1) *x1 = x;
        if (y1) *y1 = y;
        if (w) *w = strlen(text) * 6;
        if (h) *h = 8;
    }
}

// ============================================================================
// OLED Display Operations
// ============================================================================

void DesktopDisplay::oledClear() {
    std::fill(_oledBuffer.begin(), _oledBuffer.end(), 0);  // 0 = off
    _needsOledRefresh = true;
}

void DesktopDisplay::oledSetPixel(int x, int y, bool on) {
    if (x < 0 || x >= OLED_WIDTH || y < 0 || y >= OLED_HEIGHT) return;
    _oledBuffer[y * OLED_WIDTH + x] = on ? 1 : 0;
    _needsOledRefresh = true;
}

void DesktopDisplay::oledDrawText(const char* text, int x, int y, int fontSize) {
    if (!text || !text[0]) return;
    
    // U8g2 uses y as baseline (bottom of text), but renderTextToBuffer uses y as top
    // Get the actual font to calculate proper ascent
    TTF_Font* font = nullptr;
    if (fontSize >= 14) font = _fontLarge;
    else if (fontSize >= 10) font = _fontMedium;
    else font = _fontSmall;
    
    int adjustedY = y;
    if (font) {
        // Use actual font ascent for accurate positioning
        int ascent = TTF_FontAscent(font);
        adjustedY = y - ascent;
    } else {
        adjustedY = y - fontSize;
    }
    
    // Clamp to valid range
    if (adjustedY < 0) adjustedY = 0;
    
    // OLED: inverted=true means pixels ON (white/colored text on black background)
    renderTextToBuffer(text, x, adjustedY, fontSize, _oledBuffer, OLED_WIDTH, OLED_HEIGHT, true);
    _needsOledRefresh = true;
}

int DesktopDisplay::oledGetTextWidth(const char* text, int fontSize) {
    if (!text || !text[0]) return 0;
    
    TTF_Font* font = nullptr;
    if (fontSize >= 14) font = _fontLarge;
    else if (fontSize >= 10) font = _fontMedium;
    else font = _fontSmall;
    
    if (font) {
        int w, h;
        TTF_SizeUTF8(font, text, &w, &h);
        return w;
    }
    // Fallback
    return strlen(text) * 7;
}

void DesktopDisplay::oledRefresh() {
    _needsOledRefresh = true;
}
