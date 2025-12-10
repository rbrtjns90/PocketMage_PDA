// PocketMage StarterApp - Hello World Example
// Based on BlankApp template
// 
// This is a template for creating standalone apps that can be loaded via
// the PocketMage App Launcher. Copy this entire StarterApp folder to create
// your own app.

#include <globals.h>

// Include fonts for e-ink display text
// Available fonts: FreeSans, FreeMono, FreeSerif (9pt, 12pt) and Bold variants
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeMonoBold9pt7b.h>

static constexpr const char* TAG = "STARTER";

// App state
enum AppScreen { SCREEN_MAIN, SCREEN_COUNTER };
static AppScreen currentScreen = SCREEN_MAIN;
static int counter = 0;
static int screenIndex = 0;  // 0 = counter, 1 = square, -1 = circle
static bool needsRedraw = true;

// ADD PROCESS/KEYBOARD APP SCRIPTS HERE

// Auto-generated INIT function
void STARTERAPP_INIT() {
    CurrentAppState = STARTERAPP;
    currentScreen = SCREEN_MAIN;
    needsRedraw = true;
}

void processKB_STARTERAPP() {
  char key = KB().updateKeypress();
  if (key == 0) return;
  
  switch (currentScreen) {
    case SCREEN_MAIN:
      if (key == 13) {  // Enter - go to counter
        currentScreen = SCREEN_COUNTER;
        needsRedraw = true;
      }
      else if (key == 12) {  // ESC - return to PocketMage OS
        CurrentAppState = HOME; HOME_INIT(); return;
      }
      break;
      
    case SCREEN_COUNTER:
      if (key == 21) {  // Right - increment counter
        counter++;
        needsRedraw = true;
      }
      else if (key == 19) {  // Left - decrement counter
        counter--;
        needsRedraw = true;
      }
      else if (key == 28) {  // Up - go to square screen
        screenIndex = 1;
        needsRedraw = true;
      }
      else if (key == 20) {  // Down - go to circle screen
        screenIndex = -1;
        needsRedraw = true;
      }
      else if (key == 12 || key == 8) {  // ESC or Backspace - go back
        currentScreen = SCREEN_MAIN;
        screenIndex = 0;  // Reset to counter screen
        needsRedraw = true;
      }
      break;
  }
  
  // Update OLED
  u8g2.clearBuffer();
  if (currentScreen == SCREEN_MAIN) {
    u8g2.drawStr(0, 12, "Hello World!");
  } else if (screenIndex == 0) {
    u8g2.drawStr(0, 12, ("Count: " + String(counter)).c_str());
  } else if (screenIndex == 1) {
    u8g2.drawStr(0, 12, "Square Demo");
  } else {
    u8g2.drawStr(0, 12, "Circle Demo");
  }
  u8g2.sendBuffer();
}

void einkHandler_STARTERAPP() {
  if (!needsRedraw) return;
  needsRedraw = false;
  
  display.setFullWindow();
  display.fillScreen(GxEPD_WHITE);
  display.setTextColor(GxEPD_BLACK);
  
  switch (currentScreen) {
    case SCREEN_MAIN:
      // Use bold font for title
      display.setFont(&FreeMonoBold9pt7b);
      display.setCursor(70, 80);
      display.print("Hello World!");
      
      // Use regular font for instructions
      display.setFont(&FreeSans9pt7b);
      display.setCursor(45, 120);
      display.print("Press ENTER for counter");
      display.setCursor(65, 150);
      display.print("Press ESC to exit");
      break;
      
    case SCREEN_COUNTER:
      if (screenIndex == 0) {
        // Counter screen
        display.setFont(&FreeSans9pt7b);
        display.setCursor(100, 50);
        display.print("Counter Demo");
        
        display.setFont(&FreeMonoBold9pt7b);
        display.setCursor(140, 110);
        display.print(counter);
        
        display.setFont(&FreeSans9pt7b);
        display.setCursor(50, 160);
        display.print("LEFT/RIGHT to change");
        display.setCursor(50, 190);
        display.print("UP/DOWN for shapes");
      }
      else if (screenIndex == 1) {
        // Square screen - demonstrates drawRect and fillRect
        display.setFont(&FreeSans9pt7b);
        display.setCursor(100, 30);
        display.print("Square Demo");
        
        // Draw outlined square
        display.drawRect(50, 60, 80, 80, GxEPD_BLACK);
        display.setCursor(55, 170);
        display.print("drawRect()");
        
        // Draw filled square
        display.fillRect(180, 60, 80, 80, GxEPD_BLACK);
        display.setCursor(180, 170);
        display.print("fillRect()");
        
        display.setCursor(70, 210);
        display.print("UP/DOWN to navigate");
      }
      else {
        // Circle screen - demonstrates drawCircle and fillCircle
        display.setFont(&FreeSans9pt7b);
        display.setCursor(100, 30);
        display.print("Circle Demo");
        
        // Draw outlined circle
        display.drawCircle(90, 100, 40, GxEPD_BLACK);
        display.setCursor(45, 170);
        display.print("drawCircle()");
        
        // Draw filled circle
        display.fillCircle(220, 100, 40, GxEPD_BLACK);
        display.setCursor(175, 170);
        display.print("fillCircle()");
        
        display.setCursor(70, 210);
        display.print("UP/DOWN to navigate");
      }
      
      display.setFont(&FreeSans9pt7b);
      display.setCursor(85, 230);
      display.print("ESC to go back");
      break;
  }
  
  EINK().refresh();
}

/////////////////////////////////////////////////////////////
//  ooo        ooooo       .o.       ooooo ooooo      ooo  //
//  `88.       .888'      .888.      `888' `888b.     `8'  //
//   888b     d'888      .8"888.      888   8 `88b.    8   //
//   8 Y88. .P  888     .8' `888.     888   8   `88b.  8   //
//   8  `888'   888    .88ooo8888.    888   8     `88b.8   //
//   8    Y     888   .8'     `888.   888   8       `888   //
//  o8o        o888o o88o     o8888o o888o o8o        `8   //
/////////////////////////////////////////////////////////////
// SETUP

// Function removed - handled by main firmware


// Function removed - handled by main firmware


// migrated from einkFunc.cpp

// Function removed - handled by main firmware
