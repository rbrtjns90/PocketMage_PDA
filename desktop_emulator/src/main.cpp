/**
 * @file main.cpp
 * @brief PocketMage PDA Desktop Emulator - Main Entry Point
 * 
 * This is the main entry point for the desktop emulator. It initializes
 * the SDL2 display system and runs the PocketMage application loop.
 */

#include "pocketmage_compat.h"
#include "desktop_display_sdl2.h"
#include "oled_service.h"
#include "GxEPD2_BW.h"

#include <iostream>
#include <csignal>
#include <cstring>

// ============================================================================
// External Declarations
// ============================================================================

// PocketMage entry points (from PocketMageV3.cpp)
extern void setup();
extern void loop();
extern void applicationEinkHandler();

// Global display instance
extern DesktopDisplay* g_display;

// ============================================================================
// Signal Handler
// ============================================================================

static volatile bool s_running = true;

// ============================================================================
// Screen Test Mode
// ============================================================================

// Set to true to run screen test instead of normal app
static bool s_screenTestMode = false;
static bool s_noFlash = false;  // Disable e-ink flash animation

// Helper function to wait while keeping display responsive
void testDelay(int ms) {
    int elapsed = 0;
    while (elapsed < ms && s_running) {
        g_display->handleEvents();
        g_display->present();
        delay(16); // ~60fps
        elapsed += 16;
    }
}

void runScreenTest() {
    extern GxEPD2_BW<GxEPD2_310_GDEQ031T10, GxEPD2_310_GDEQ031T10::HEIGHT> display;
    
    std::cout << "[ScreenTest] Starting display test..." << std::endl;
    
    // Test 1: Clear screen to white
    std::cout << "[ScreenTest] Test 1: Fill white" << std::endl;
    display.setRotation(0);
    display.fillScreen(GxEPD_WHITE);
    display.setTextColor(GxEPD_BLACK);
    display.setFont();
    display.setCursor(10, 20);
    display.print("Screen Test - White Fill");
    g_display->einkRefresh();
    testDelay(2000);
    
    // Test 2: Draw border
    std::cout << "[ScreenTest] Test 2: Draw border" << std::endl;
    display.fillScreen(GxEPD_WHITE);
    display.drawRect(0, 0, display.width(), display.height(), GxEPD_BLACK);
    display.drawRect(5, 5, display.width()-10, display.height()-10, GxEPD_BLACK);
    display.setCursor(10, 30);
    display.print("Border Test");
    g_display->einkRefresh();
    testDelay(2000);
    
    // Test 3: Draw grid pattern
    std::cout << "[ScreenTest] Test 3: Grid pattern" << std::endl;
    display.fillScreen(GxEPD_WHITE);
    for (int x = 0; x < display.width(); x += 20) {
        display.drawLine(x, 0, x, display.height(), GxEPD_BLACK);
    }
    for (int y = 0; y < display.height(); y += 20) {
        display.drawLine(0, y, display.width(), y, GxEPD_BLACK);
    }
    display.setCursor(10, 30);
    display.print("Grid Test");
    g_display->einkRefresh();
    testDelay(2000);
    
    // Test 4: Draw circles
    std::cout << "[ScreenTest] Test 4: Circles" << std::endl;
    display.fillScreen(GxEPD_WHITE);
    int cx = display.width() / 2;
    int cy = display.height() / 2;
    for (int r = 10; r < 100; r += 15) {
        display.drawCircle(cx, cy, r, GxEPD_BLACK);
    }
    display.setCursor(10, 20);
    display.print("Circle Test");
    g_display->einkRefresh();
    testDelay(2000);
    
    // Test 5: Fill rectangles (checkerboard)
    std::cout << "[ScreenTest] Test 5: Checkerboard" << std::endl;
    display.fillScreen(GxEPD_WHITE);
    int boxSize = 30;
    for (int y = 0; y < display.height(); y += boxSize) {
        for (int x = 0; x < display.width(); x += boxSize) {
            if ((x / boxSize + y / boxSize) % 2 == 0) {
                display.fillRect(x, y, boxSize, boxSize, GxEPD_BLACK);
            }
        }
    }
    g_display->einkRefresh();
    testDelay(2000);
    
    // Test 6: Text at different positions
    std::cout << "[ScreenTest] Test 6: Text positions" << std::endl;
    display.fillScreen(GxEPD_WHITE);
    display.setCursor(10, 20);
    display.print("Top Left");
    display.setCursor(display.width() - 80, 20);
    display.print("Top Right");
    display.setCursor(10, display.height() - 10);
    display.print("Bottom Left");
    display.setCursor(display.width() - 100, display.height() - 10);
    display.print("Bottom Right");
    display.setCursor(display.width()/2 - 30, display.height()/2);
    display.print("CENTER");
    g_display->einkRefresh();
    testDelay(2000);
    
    // Test 7: Rotation test
    std::cout << "[ScreenTest] Test 7: Rotation test" << std::endl;
    for (int rot = 0; rot < 4; rot++) {
        display.setRotation(rot);
        display.fillScreen(GxEPD_WHITE);
        display.setCursor(10, 30);
        display.print("Rotation: ");
        display.print(rot);
        display.setCursor(10, 50);
        display.print("W:");
        display.print(display.width());
        display.print(" H:");
        display.print(display.height());
        display.drawRect(0, 0, display.width(), display.height(), GxEPD_BLACK);
        g_display->einkRefresh();
        testDelay(2000);
    }
    
    std::cout << "[ScreenTest] All tests complete!" << std::endl;
    display.setRotation(3); // Reset to normal rotation
}

void signalHandler(int signum) {
    std::cout << "\n[Emulator] Received signal " << signum << ", shutting down..." << std::endl;
    s_running = false;
}

// ============================================================================
// Main Function
// ============================================================================

int main(int argc, char* argv[]) {
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--test") == 0 || strcmp(argv[i], "-t") == 0) {
            s_screenTestMode = true;
        }
        if (strcmp(argv[i], "--no-flash") == 0 || strcmp(argv[i], "-f") == 0) {
            s_noFlash = true;
        }
        if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            std::cout << "PocketMage PDA Desktop Emulator" << std::endl;
            std::cout << "Usage: " << argv[0] << " [options]" << std::endl;
            std::cout << "Options:" << std::endl;
            std::cout << "  -t, --test      Run screen test mode" << std::endl;
            std::cout << "  -f, --no-flash  Disable e-ink flash animation" << std::endl;
            std::cout << "  -h, --help      Show this help" << std::endl;
            return 0;
        }
    }
    
    std::cout << "========================================" << std::endl;
    std::cout << "  PocketMage PDA Desktop Emulator" << std::endl;
    if (s_screenTestMode) {
        std::cout << "  ** SCREEN TEST MODE **" << std::endl;
    }
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;
    std::cout << "Controls:" << std::endl;
    std::cout << "  Arrow keys    - Navigation" << std::endl;
    std::cout << "  Enter         - Select/Confirm" << std::endl;
    std::cout << "  Backspace     - Delete/Back" << std::endl;
    std::cout << "  Escape        - Return to home" << std::endl;
    std::cout << "  Letters/Nums  - Text input" << std::endl;
    std::cout << "  Close window  - Quit emulator" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;
    
    // Set up signal handlers
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    
    // Initialize SDL display
    std::cout << "[Main] Initializing display..." << std::endl;
    g_display = new DesktopDisplay();
    
    if (!g_display->init()) {
        std::cerr << "[Main] Failed to initialize display!" << std::endl;
        delete g_display;
        g_display = nullptr;
        return 1;
    }
    
    // Apply e-ink flash setting
    if (s_noFlash) {
        g_display->setEinkFlashEnabled(false);
        std::cout << "[Main] E-ink flash animation disabled" << std::endl;
    }
    
    // Call PocketMage setup
    std::cout << "[Main] Calling PocketMage setup()..." << std::endl;
    setup();
    
    // Debug: Check if newState was set
    extern volatile bool newState;
#ifndef _WIN32
    extern int CurrentAppState;
    std::cout << "[Main] After setup: CurrentAppState=" << CurrentAppState << ", newState=" << newState << std::endl;
#else
    std::cout << "[Main] After setup: newState=" << newState << std::endl;
#endif
    
    // Force HOME_INIT to be called if newState wasn't set
    if (!newState) {
        std::cout << "[Main] Forcing HOME_INIT()..." << std::endl;
        extern void HOME_INIT();
        HOME_INIT();
        std::cout << "[Main] After HOME_INIT: newState=" << newState << std::endl;
    }
    
    // Run screen test if requested
    if (s_screenTestMode) {
        runScreenTest();
    }
    
    std::cout << "[Main] Entering main loop..." << std::endl;
    
    // Main loop
    int frameCount = 0;
    while (s_running) {
        // Handle SDL events
        if (!g_display->handleEvents()) {
            break;
        }
        
        // Call PocketMage loop (processes keyboard, updates state)
        loop();
        
        // Call the E-ink handler to render UI
        applicationEinkHandler();
        
        // Present OLED updates
        oled_present_if_dirty();
        
        // Present display
        g_display->present();
        
        // Frame counter (for debugging)
        frameCount++;
        if (frameCount % 300 == 0) {
            // Log every 10 seconds at 30 FPS
            std::cout << "[Main] Frame " << frameCount << std::endl;
        }
        
        // Limit frame rate to ~60 FPS for responsive typing
        delay(16);
    }
    
    // Cleanup
    std::cout << "[Main] Shutting down..." << std::endl;
    
    if (g_display) {
        g_display->shutdown();
        delete g_display;
        g_display = nullptr;
    }
    
    std::cout << "[Main] Emulator shut down successfully." << std::endl;
    return 0;
}
