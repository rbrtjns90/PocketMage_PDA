# PocketMage App Development Guide

This guide explains how to create apps for the PocketMage PDA using the desktop emulator.

## Quick Start

1. Copy `APP_TEMPLATE.cpp` to create your new app file
2. Implement the three core functions
3. Register your app in the system
4. Test with the desktop emulator

## Required Includes

Every app needs at minimum:

```cpp
#include <globals.h>                    // Required - system globals and API
```

For text rendering, include the fonts you need:

```cpp
#include <Fonts/FreeSans9pt7b.h>        // Standard text
#include <Fonts/FreeSansBold18pt7b.h>   // Headers/titles
```

## App Structure

Every PocketMage app consists of three core functions:

```cpp
void MYAPP_INIT();           // Called when app launches
void processKB_MYAPP();      // Called every loop to handle input
void einkHandler_MYAPP();    // Called to update the display
```

### Example: Hello World

See `HELLO_WORLD.cpp` for a complete working example.

```cpp
#include <globals.h>

// Include fonts you want to use
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSansBold18pt7b.h>
#include <Fonts/FreeSansBold24pt7b.h>

// App state
enum MyAppState { SCREEN1, SCREEN2 };
static MyAppState currentState = SCREEN1;

void MYAPP_INIT() {
    currentState = SCREEN1;
    EINK().forceSlowFullUpdate(true);
    newState = true;
    OLED().oledWord("My App");
}

void processKB_MYAPP() {
    char key = KB().updateKeypress();
    if (key == 0) return;
    
    if (key == 12) {  // ESC - return home
        HOME_INIT();
        return;
    }
    
    // Handle other keys...
    if (key == 13) {  // Enter
        currentState = SCREEN2;
        newState = true;
    }
}

void einkHandler_MYAPP() {
    if (!newState) return;
    newState = false;
    
    display.setFullWindow();
    display.fillScreen(GxEPD_WHITE);
    display.setTextColor(GxEPD_BLACK);
    display.setFont(&FreeSans9pt7b);
    display.setCursor(10, 30);
    display.print("Hello from my app!");
    
    EINK().refresh();
}
```

## Key Codes

| Key | Code | Description |
|-----|------|-------------|
| ESC/Home | 12 | Return to home screen |
| Enter | 13 | Confirm/Select |
| Backspace | 8 | Delete/Back |
| Left Arrow | 19 | Navigate left |
| Right Arrow | 21 | Navigate right |
| Up Arrow | 28 | Navigate up (in SHIFT mode) |
| Down Arrow | 20 | Navigate down/Select |
| Shift | 17 | Toggle shift mode |
| Func | 18 | Toggle function mode |

## Display API

### Screen Setup
```cpp
display.setFullWindow();           // Use full screen
display.fillScreen(GxEPD_WHITE);   // Clear to white
display.fillScreen(GxEPD_BLACK);   // Clear to black
```

### Text Drawing
```cpp
display.setTextColor(GxEPD_BLACK);
display.setFont(&FreeSans9pt7b);   // Set font
display.setCursor(x, y);           // Position (y is baseline)
display.print("Text");             // Draw text
display.println("Line");           // Draw text with newline
```

### Available Fonts

**Important:** You must include the font header files you want to use at the top of your app:

```cpp
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSansBold18pt7b.h>
```

Available fonts (located in `desktop_emulator/include/Fonts/`):

| Font Family | Sizes Available |
|-------------|-----------------|
| FreeSans | 9pt, 12pt |
| FreeSansBold | 9pt, 12pt, 18pt, 24pt |
| FreeSansOblique | 9pt |
| FreeSansBoldOblique | 9pt, 12pt, 18pt, 24pt |
| FreeMono | 9pt, 12pt |
| FreeMonoBold | 9pt, 12pt, 18pt, 24pt |
| FreeMonoOblique | 9pt |
| FreeMonoBoldOblique | 9pt, 12pt, 18pt, 24pt |
| FreeSerif | 9pt, 12pt |
| FreeSerifBold | 9pt, 12pt, 18pt, 24pt |
| FreeSerifItalic | 9pt |
| FreeSerifBoldItalic | 9pt, 12pt, 18pt, 24pt |

Font naming pattern: `<Family><Size>7b` (e.g., `FreeSansBold18pt7b`)

### Graphics
```cpp
display.drawLine(x0, y0, x1, y1, GxEPD_BLACK);
display.drawRect(x, y, w, h, GxEPD_BLACK);
display.fillRect(x, y, w, h, GxEPD_BLACK);
display.drawCircle(x, y, r, GxEPD_BLACK);
display.fillCircle(x, y, r, GxEPD_BLACK);
display.drawBitmap(x, y, bitmap, w, h, GxEPD_BLACK);
```

### Screen Refresh
```cpp
EINK().refresh();                  // Normal refresh
EINK().forceSlowFullUpdate(true);  // Request full refresh (cleaner)
EINK().multiPassRefresh(2);        // Multi-pass for better contrast
```

## OLED Status Bar

The small OLED display shows status info:

```cpp
OLED().oledWord("Status");         // Show single word
OLED().oledLine("Longer message"); // Show line with progress
OLED().infoBar();                  // Show default info bar
```

## File System

Access the SD card via the `pocketmage::file` namespace:

```cpp
// Read file
String content = pocketmage::file::readFile("/myapp/data.txt");

// Write file
pocketmage::file::writeFile("/myapp/data.txt", "content");

// Append to file
pocketmage::file::appendToFile("/myapp/log.txt", "new line");

// Delete file
pocketmage::file::delFile("/myapp/temp.txt");

// Check if file exists
if (SD_MMC.exists("/myapp/config.txt")) { ... }
```

### File Picker
```cpp
String selectedFile = fileWizardMini(false, "/");  // Opens file picker
if (selectedFile.length() > 0) {
    // User selected a file
}
```

## Buzzer

Play sounds with the buzzer:

```cpp
BZ().playJingle(Jingles::Startup);   // Play startup jingle
BZ().playJingle(Jingles::Shutdown);  // Play shutdown jingle
```

## Best Practices

### 1. Use `newState` Flag
Only redraw the screen when `newState` is true. This prevents unnecessary e-ink refreshes.

```cpp
void einkHandler_MYAPP() {
    if (!newState) return;  // Skip if nothing changed
    newState = false;
    // ... draw screen
}
```

### 2. Handle ESC Key
Always let users return to the home screen:

```cpp
if (key == 12) {  // ESC
    HOME_INIT();
    return;
}
```

### 3. Use App States
Track which screen/mode your app is in:

```cpp
enum MyAppState { MENU, EDITOR, SETTINGS };
static MyAppState currentState = MENU;
```

### 4. Show OLED Feedback
Update the OLED when the user does something:

```cpp
OLED().oledWord("Saved!");
```

### 5. Screen Dimensions
- E-ink display: 310 x 240 pixels
- OLED display: 256 x 32 pixels

## Registering Your App

To make your app accessible from the home screen:

### 1. Add to AppState enum (globals.h)
```cpp
enum AppState { HOME, TXT, FILEWIZ, ..., MYAPP };
```

### 2. Add command in HOME.cpp
```cpp
else if (command == "myapp" || command == "ma") {
    MYAPP_INIT();
}
```

### 3. Add to app dispatcher (sysFunc.cpp)
```cpp
case MYAPP:
    processKB_MYAPP();
    break;
```

### 4. Add to eink handler dispatcher
```cpp
case MYAPP:
    einkHandler_MYAPP();
    break;
```

### 5. Declare functions in globals.h
```cpp
void MYAPP_INIT();
void processKB_MYAPP();
void einkHandler_MYAPP();
```

### 6. Add to CMakeLists.txt (for desktop emulator)

Open `desktop_emulator/CMakeLists.txt` and find the `POCKETMAGE_SOURCES` section (around line 83). Add your app's source file:

```cmake
# ---------------------------
# PocketMage source files
# ---------------------------
set(POCKETMAGE_SOURCES
    ${POCKETMAGE_SRC}/PocketMageV3.cpp
    ${POCKETMAGE_SRC}/globals.cpp
    ${POCKETMAGE_SRC}/assets.cpp
    ${POCKETMAGE_APPS}/HOME.cpp
    ${POCKETMAGE_APPS}/TXT.cpp
    ${POCKETMAGE_APPS}/TXT_NEW.cpp
    ${POCKETMAGE_APPS}/FILEWIZ.cpp
    ${POCKETMAGE_APPS}/TASKS.cpp
    ${POCKETMAGE_APPS}/CALENDAR.cpp
    ${POCKETMAGE_APPS}/SETTINGS.cpp
    ${POCKETMAGE_APPS}/JOURNAL.cpp
    ${POCKETMAGE_APPS}/LEXICON.cpp
    ${POCKETMAGE_SRC}/HELLO_WORLD.cpp      # Example app
    ${POCKETMAGE_SRC}/MYAPP.cpp            # <-- ADD YOUR APP HERE
    # APPLOADER.cpp excluded - requires ESP32 OTA functionality
)
```

**Note:** Use `${POCKETMAGE_SRC}` for files in `Code/PocketMage_V3/src/` or `${POCKETMAGE_APPS}` for files in `Code/PocketMage_V3/src/OS_APPS/`.

## Testing with the Emulator

1. Build the emulator: `./build.sh`
2. Run: `./build/PocketMage_PDA_Emulator`
3. Type your app command and press Enter
4. Use keyboard to test your app
5. Press ESC to return home

## Debugging Tips

- Use `Serial.println()` for debug output (shows in terminal)
- Check the OLED for status messages
- Use `--no-flash` flag to speed up testing
- The emulator's `data/` folder is your virtual SD card

## Standalone Apps (App Launcher)

For apps that run independently (loaded via App Launcher), see the **`Code/StarterApp/`** template. Standalone apps:

- Are separate PlatformIO projects
- Can be installed via the App Loader
- Run independently from the main PocketMage OS
- Can return to PocketMage OS with `rebootToPocketMage()`

See `Code/StarterApp/README.md` for full documentation on creating standalone apps.
