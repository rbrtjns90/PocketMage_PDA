// oooo   oooo oooooooooooo ooooo        ooooo          .oooooo.   //
// `888   `888 `888'     `8 `888'        `888'         d8P'  `Y8b  //
//  888    888  888          888          888         888      888 //
//  888oooo888  888oooo8     888          888         888      888 //
//  888    888  888    "     888          888         888      888 //
//  888    888  888       o  888       o  888       o `88b    d88' //
// o888o  o888o o888ooooood8 o888ooooood8 o888ooooood8  `Y8bood8P'  //
//
// HELLO WORLD - A simple example app for PocketMage developers
//
// This file demonstrates the basic structure of a PocketMage app.
// Use this as a starting point for your own apps!

#include <globals.h>
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSansBold18pt7b.h>
#include <Fonts/FreeSansBold24pt7b.h>

// ============================================================================
// APP STATE
// ============================================================================
// Define your app's internal states here. Most apps have multiple screens
// or modes, so use an enum to track which one is active.

enum HelloState { 
    HELLO_MAIN,      // Main screen showing "Hello World"
    HELLO_COUNTER    // Counter screen (demonstrates state changes)
};

static HelloState CurrentHelloState = HELLO_MAIN;
static int counter = 0;

// ============================================================================
// INIT FUNCTION
// ============================================================================
// This function is called when your app is launched from the home screen.
// Add it to HOME.cpp's command parser to make it accessible.

void HELLO_INIT() {
    // Set the global app state to HELLO
    CurrentAppState = HELLO;
    
    // Reset app-specific state
    CurrentHelloState = HELLO_MAIN;
    counter = 0;
    
    // Request a full screen refresh
    EINK().forceSlowFullUpdate(true);
    newState = true;  // Triggers einkHandler to redraw
    
    // Show a message on the OLED
    OLED().oledWord("Hello!");
}

// ============================================================================
// KEYBOARD HANDLER
// ============================================================================
// This function is called every loop iteration when your app is active.
// Handle keyboard input here.

void processKB_HELLO() {
    char key = KB().updateKeypress();
    if (key == 0) return;  // No key pressed
    
    switch (CurrentHelloState) {
        case HELLO_MAIN:
            if (key == 13) {  // Enter key
                // Switch to counter screen
                CurrentHelloState = HELLO_COUNTER;
                newState = true;
            }
            else if (key == 12) {  // ESC/Home key
                // Return to home screen
                HOME_INIT();
            }
            break;
            
        case HELLO_COUNTER:
            if (key == 21) {  // Right arrow
                counter++;
                newState = true;
            }
            else if (key == 19) {  // Left arrow
                counter--;
                newState = true;
            }
            else if (key == 12 || key == 8) {  // ESC or Backspace
                // Go back to main screen
                CurrentHelloState = HELLO_MAIN;
                newState = true;
            }
            break;
    }
}

// ============================================================================
// DISPLAY HANDLER
// ============================================================================
// This function is called to update the e-ink display.
// Only redraw when newState is true to avoid unnecessary refreshes.

void einkHandler_HELLO() {
    if (!newState) return;  // Nothing to update
    newState = false;
    
    // Clear the screen
    display.setFullWindow();
    display.fillScreen(GxEPD_WHITE);
    
    switch (CurrentHelloState) {
        case HELLO_MAIN:
            // Draw "Hello World!" centered on screen
            display.setTextColor(GxEPD_BLACK);
            display.setFont(&FreeSansBold18pt7b);
            display.setCursor(60, 100);
            display.print("Hello World!");
            
            display.setFont(&FreeSans9pt7b);
            display.setCursor(50, 140);
            display.print("Press ENTER for counter");
            
            display.setCursor(70, 170);
            display.print("Press ESC to exit");
            break;
            
        case HELLO_COUNTER:
            // Draw counter screen
            display.setTextColor(GxEPD_BLACK);
            display.setFont(&FreeSans9pt7b);
            display.setCursor(80, 60);
            display.print("Counter Demo");
            
            // Draw the counter value large
            display.setFont(&FreeSansBold24pt7b);
            display.setCursor(120, 130);
            display.print(counter);
            
            // Instructions
            display.setFont(&FreeSans9pt7b);
            display.setCursor(40, 180);
            display.print("LEFT/RIGHT to change");
            display.setCursor(60, 210);
            display.print("ESC to go back");
            break;
    }
    
    // Refresh the display
    EINK().refresh();
}
