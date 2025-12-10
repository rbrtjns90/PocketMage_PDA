// ============================================================================
// APP LAUNCHER - Desktop Emulator App Launcher
// ============================================================================
// Displays installed apps with their icons and allows launching them.
// This is a simplified version for the desktop emulator.

#include <globals.h>
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSansBold12pt7b.h>

// App registry - add installed apps here
struct AppEntry {
    const char* name;
    const char* command;
    const char* iconPath;
    void (*initFunc)();
};

// Forward declarations for installed apps
extern void HELLO_INIT();
extern void ASTRALUAAPP_INIT();
extern void FLASHCARDAPP_INIT();
extern void GLUCOSEAPP_INIT();
extern void MUSICAPP_INIT();
extern void TERMINALAPP_INIT();
extern void STARTERAPP_INIT();

// List of installed apps
static AppEntry installedApps[] = {
    {"Hello World", "hello", "/apps/hello_icon.bin", HELLO_INIT},
    {"StarterApp", "starterapp", "/apps/starterapp_icon.bin", STARTERAPP_INIT},
    {"TerminalApp", "terminalapp", "/apps/terminalapp_icon.bin", TERMINALAPP_INIT},
    {"MusicApp", "musicapp", "/apps/musicapp_icon.bin", MUSICAPP_INIT},
    {"GlucoseApp", "glucoseapp", "/apps/glucoseapp_icon.bin", GLUCOSEAPP_INIT},
    {"FlashCardApp", "flashcardapp", "/apps/flashcardapp_icon.bin", FLASHCARDAPP_INIT},
    {"AstraLuaApp", "astraluaapp", "/apps/astraluaapp_icon.bin", ASTRALUAAPP_INIT},
    // Add more apps here as they are installed
};

static const int NUM_APPS = sizeof(installedApps) / sizeof(installedApps[0]);
static int selectedApp = 0;
static bool needsRedraw = true;

// ============================================================================
// INIT
// ============================================================================
void APPLAUNCHER_INIT() {
    CurrentAppState = APPLAUNCHER;
    selectedApp = 0;
    needsRedraw = true;
    EINK().forceSlowFullUpdate(true);
    newState = true;
}

// ============================================================================
// DRAW ICON
// ============================================================================
static void drawAppIcon(int x, int y, const char* iconPath) {
    // Try to load icon from file
    File f = SD_MMC.open(iconPath, FILE_READ);
    if (f) {
        uint8_t buf[200];  // 40x40 1-bit = 200 bytes
        if (f.read(buf, sizeof(buf)) == sizeof(buf)) {
            display.drawBitmap(x, y, buf, 40, 40, GxEPD_BLACK);
            f.close();
            return;
        }
        f.close();
    }
    
    // Fallback: draw placeholder
    display.drawRect(x, y, 40, 40, GxEPD_BLACK);
    display.setCursor(x + 12, y + 25);
    display.print("?");
}

// ============================================================================
// KEYBOARD HANDLER
// ============================================================================
void processKB_APPLAUNCHER() {
    char key = KB().updateKeypress();
    if (key == 0) return;
    
    switch (key) {
        case 20:  // Down
            selectedApp = (selectedApp + 1) % NUM_APPS;
            needsRedraw = true;
            break;
            
        case 28:  // Up
            selectedApp = (selectedApp - 1 + NUM_APPS) % NUM_APPS;
            needsRedraw = true;
            break;
            
        case 21:  // Right - next row
            selectedApp = (selectedApp + 3) % NUM_APPS;
            needsRedraw = true;
            break;
            
        case 19:  // Left - prev row
            selectedApp = (selectedApp - 3 + NUM_APPS) % NUM_APPS;
            needsRedraw = true;
            break;
            
        case 13:  // Enter - launch app
            if (installedApps[selectedApp].initFunc) {
                installedApps[selectedApp].initFunc();
            }
            break;
            
        case 12:  // ESC - back to HOME
        case 8:   // Backspace
            CurrentAppState = HOME;
            HOME_INIT();
            break;
    }
}

// ============================================================================
// EINK HANDLER
// ============================================================================
void einkHandler_APPLAUNCHER() {
    if (!needsRedraw && !newState) return;
    needsRedraw = false;
    newState = false;
    
    display.fillScreen(GxEPD_WHITE);
    
    // Title
    display.setFont(&FreeSansBold12pt7b);
    display.setTextColor(GxEPD_BLACK);
    display.setCursor(80, 25);
    display.print("App Launcher");
    
    // Draw line under title
    display.drawLine(10, 35, 300, 35, GxEPD_BLACK);
    
    // Draw apps in a grid (3 columns)
    display.setFont(&FreeSans9pt7b);
    
    int startX = 25;
    int startY = 55;
    int colWidth = 95;
    int rowHeight = 70;
    
    for (int i = 0; i < NUM_APPS; i++) {
        int col = i % 3;
        int row = i / 3;
        int x = startX + col * colWidth;
        int y = startY + row * rowHeight;
        
        // Draw selection box
        if (i == selectedApp) {
            display.drawRect(x - 5, y - 5, 90, 65, GxEPD_BLACK);
            display.drawRect(x - 4, y - 4, 88, 63, GxEPD_BLACK);
        }
        
        // Draw icon
        drawAppIcon(x + 20, y, installedApps[i].iconPath);
        
        // Draw name (truncate if needed)
        String name = installedApps[i].name;
        if (name.length() > 10) {
            name = name.substring(0, 9) + "..";
        }
        
        int16_t x1, y1;
        uint16_t w, h;
        display.getTextBounds(name, 0, 0, &x1, &y1, &w, &h);
        display.setCursor(x + (80 - w) / 2, y + 55);
        display.print(name);
    }
    
    // Instructions at bottom
    display.setFont(&FreeSans9pt7b);
    display.setCursor(50, 220);
    display.print("Arrows: Select   Enter: Launch   ESC: Back");
    
    EINK().refresh();
}
