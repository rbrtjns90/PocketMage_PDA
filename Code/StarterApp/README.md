# PocketMage Starter App

This is a Hello World example app for creating standalone apps that can be loaded via the PocketMage App Launcher.

See [docs/APP_DEVELOPMENT.md](docs/APP_DEVELOPMENT.md) for a complete guide on creating your own PocketMage apps. The `StarterApp/src/appMaim.cpp` example demonstrates the basic app structure.

## Quick Start

1. Copy this entire `StarterApp` folder and rename it to your app name
2. Edit `src/appMain.cpp` with your app code
3. Build with PlatformIO
4. Install on your PocketMage device

## Project Structure

```
StarterApp/
├── src/
│   ├── appMain.cpp          # Your app code goes here
│   ├── assets.cpp           # App icons and bitmaps
│   ├── globals.cpp          # Global variables
│   └── setup/               # Hardware initialization
├── include/
│   ├── globals.h            # Global declarations
│   ├── pocketmage.h         # Main PocketMage include
│   ├── assets.h             # Asset declarations
│   └── Fonts/               # GFX fonts
├── lib/                     # PocketMage libraries
│   ├── pocketmage_eink/     # E-ink display
│   ├── pocketmage_oled/     # OLED display
│   ├── pocketmage_kb/       # Keyboard input
│   ├── pocketmage_sd/       # SD card access
│   ├── pocketmage_bz/       # Buzzer
│   ├── pocketmage_touch/    # Touch slider
│   ├── pocketmage_clock/    # RTC clock
│   └── pocketmage_sys/      # System functions
├── platformio.ini           # Build configuration
└── appPartition.csv         # Flash partition table
```

## App Structure

Your app needs two main functions in `appMain.cpp`:

```cpp
#include <pocketmage.h>

// Called every loop - handle keyboard input
void processKB() {
    char key = KB().updateKeypress();
    if (key == 0) return;
    
    if (key == 12) {  // ESC - return to PocketMage OS
        rebootToPocketMage();
    }
    
    // Handle other keys...
}

// Called to update e-ink display
void applicationEinkHandler() {
    // Draw your screen here
    display.setFullWindow();
    display.fillScreen(GxEPD_WHITE);
    display.setTextColor(GxEPD_BLACK);
    display.setFont(&FreeSans9pt7b);
    display.setCursor(10, 30);
    display.print("Hello!");
    EINK().refresh();
}
```

## Key Codes

| Key | Code | Description |
|-----|------|-------------|
| ESC/Home | 12 | Return to PocketMage OS |
| Enter | 13 | Confirm/Select |
| Backspace | 8 | Delete/Back |
| Left Arrow | 19 | Navigate left |
| Right Arrow | 21 | Navigate right |
| Up Arrow | 28 | Navigate up |
| Down Arrow | 20 | Navigate down |

## PocketMage API

### Keyboard
```cpp
char key = KB().updateKeypress();  // Get key press (0 if none)
```

### E-ink Display
```cpp
display.setFullWindow();
display.fillScreen(GxEPD_WHITE);
display.setTextColor(GxEPD_BLACK);
display.setFont(&FreeSans9pt7b);
display.setCursor(x, y);
display.print("Text");
EINK().refresh();
```

### OLED Status Bar
```cpp
OLED().oledWord("Status");     // Show word on OLED
OLED().infoBar();              // Show default info bar
```

### File System
```cpp
pocketmage::file::saveFile();
pocketmage::file::loadFile();
pocketmage::file::delFile("/path/to/file.txt");
pocketmage::file::appendToFile("/path/to/file.txt", "content");
```

### System
```cpp
rebootToPocketMage();          // Return to PocketMage OS
pocketmage::power::deepSleep();
pocketmage::power::updateBattState();
```

## Building

### Prerequisites
- [PlatformIO](https://platformio.org/) (VS Code extension or CLI)

### Build Steps
1. Open this folder in VS Code with PlatformIO
2. Click Build (checkmark icon) or run `pio run`
3. The firmware is created at `.pio/build/PM_V3/firmware.bin`

## Installing on PocketMage

### Method 1: App Loader (Recommended)
1. Copy `firmware.bin` to SD card `/apps/` folder
2. Rename it to your app name (e.g., `myapp.bin`)
3. Create a 40x40 pixel BMP icon named `myapp.bmp`
4. On PocketMage, type `app` or `loader` and press Enter
5. Select your app to install it

### Method 2: Direct Flash
1. Connect PocketMage via USB
2. Use PlatformIO Upload or `pio run -t upload`

## Creating Your App Icon

Your app needs a 40x40 pixel BMP icon:
- Size: 40x40 pixels
- Format: BMP (1-bit or 24-bit)
- Name: Same as your .bin file (e.g., `myapp.bmp`)
- Location: SD card `/apps/` folder

## Example App

See `src/appMain.cpp` for a complete "Hello World" example with:
- Two screens (main + counter)
- Keyboard navigation
- E-ink display updates
- OLED status updates

## Tips

1. **Always handle ESC key** - Let users return to PocketMage OS
2. **Use needsRedraw flag** - Only update e-ink when necessary
3. **Update OLED** - Show status on the small OLED display
4. **Test with emulator** - Use the desktop emulator first (see `desktop_emulator/`)

## Available Fonts

Include fonts at the top of your file:
```cpp
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeMonoBold9pt7b.h>
```

Available: FreeSans, FreeMono, FreeSerif (9pt, 12pt) and Bold variants.
