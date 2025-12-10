# PocketMage PDA Desktop Emulator

A cross-platform SDL2-based desktop emulator for the PocketMage PDA device, allowing development and testing without physical hardware.

## Features

- **Dual Display Emulation**: E-ink (310x240) and OLED (256x32) displays
- **E-ink Refresh Animation**: Realistic black/white flash on screen transitions
- **Keyboard Input**: Full keyboard support with PocketMage key mappings
- **File System**: Virtual SD card using local `data/` directory
- **Cross-Platform**: Supports macOS, Linux, and Windows

## Quick Start

### Prerequisites

- **CMake** 3.16 or later
- **C++17** compatible compiler
- **SDL2** and **SDL2_ttf** development libraries

### macOS

```bash
# Install dependencies
brew install sdl2 sdl2_ttf cmake

# Build
./build.sh

# Run
./build/PocketMage_PDA_Emulator
```

### Linux (Ubuntu/Debian)

```bash
# Install dependencies
sudo apt-get install libsdl2-dev libsdl2-ttf-dev cmake g++

# Build
./build.sh

# Run
./build/PocketMage_PDA_Emulator
```

### Windows

1. Install [vcpkg](https://github.com/Microsoft/vcpkg):
```cmd
git clone https://github.com/Microsoft/vcpkg.git C:\vcpkg
C:\vcpkg\bootstrap-vcpkg.bat
C:\vcpkg\vcpkg integrate install
```

2. Install SDL2:
```cmd
C:\vcpkg\vcpkg install sdl2 sdl2-ttf:x64-windows
```

3. Build:
```cmd
build.bat
```

4. Run:
```cmd
build\Release\PocketMage_PDA_Emulator.exe
```

## Command Line Options

```bash
./build/PocketMage_PDA_Emulator [options]

Options:
  -t, --test       Run screen test mode (displays test patterns)
  -f, --no-flash   Disable e-ink flash animation (faster transitions)
  -h, --help       Show help
```

## Controls

| Key | Function |
|-----|----------|
| **ESC** | Return to home screen |
| **Enter** | Select/Confirm |
| **Backspace** | Delete character |
| **Arrow Keys** | Navigate |
| **Shift** | Toggle SHIFT keyboard mode |
| **Alt / F1** | Toggle FUNC keyboard mode |
| **Letters/Numbers** | Text input |
| **Close Window** | Quit emulator |

See [docs/KEYMAPPING.md](docs/KEYMAPPING.md) for detailed key mappings.

## App Commands

Type these commands on the home screen and press Enter:

| Command | App |
|---------|-----|
| `hello` or `0` | Hello World (Example App) |
| `txt` or `1` | Text Editor |
| `file` or `2` | File Wizard |
| `usb` or `3` | USB Transfer |
| `set` or `5` | Settings |
| `tasks` or `6` | Tasks |
| `cal` or `7` | Calendar |
| `journ` or `8` | Journal |
| `lex` or `9` | Lexicon |

See [docs/COMMANDS.md](docs/COMMANDS.md) for full command list.

## For App Developers

See [docs/APP_DEVELOPMENT.md](docs/APP_DEVELOPMENT.md) for a complete guide on creating your own PocketMage apps. The `StarterApp/src/appMaim.cpp` example demonstrates the basic app structure.

## Project Structure

```
desktop_emulator/
├── CMakeLists.txt              # Build configuration
├── build.sh                    # macOS/Linux build script
├── build.bat                   # Windows build script
├── README.md                   # This file
├── docs/
│   ├── COMMANDS.md             # App launch commands
│   ├── KEYMAPPING.md           # Keyboard mappings
│   └── CONTROL_FLOW.md         # Architecture documentation
├── include/
│   ├── display/                # Display-related headers
│   │   ├── desktop_display_sdl2.h
│   │   ├── GxEPD2_BW.h
│   │   └── U8g2lib.h
│   ├── input/                  # Input device headers
│   │   ├── Adafruit_TCA8418.h
│   │   └── Adafruit_MPR121.h
│   ├── storage/                # Storage headers
│   │   ├── SD_MMC.h
│   │   ├── FS.h
│   │   └── Preferences.h
│   ├── esp32/                  # ESP32 compatibility
│   │   ├── Arduino.h
│   │   └── esp_log.h
│   ├── hardware/               # Hardware mocks
│   │   ├── Wire.h
│   │   ├── SPI.h
│   │   ├── RTClib.h
│   │   └── Buzzer.h
│   └── pocketmage/             # PocketMage stubs
│       ├── pocketmage_compat.h
│       └── pocketmage_stubs.h
├── src/
│   ├── main.cpp                # Entry point
│   ├── desktop_display_sdl2.cpp # Common display code
│   ├── desktop_display_macos.cpp # macOS-specific
│   ├── desktop_display_linux.cpp # Linux-specific
│   ├── desktop_display_windows.cpp # Windows-specific
│   ├── hardware_shim.cpp       # Hardware mock implementations
│   ├── pocketmage_shim.cpp     # Library singleton mocks
│   └── pocketmage_stubs.cpp    # PocketMage class stubs
├── data/                       # Virtual SD card
│   ├── sys/                    # System files
│   ├── journal/                # Journal entries
│   └── dict/                   # Dictionary files
└── fonts/                      # TTF fonts
    └── DejaVuSans.ttf          # Default font
```

## Data Directory

The emulator uses the `data/` directory as a virtual SD card. Place your files here:

- `data/` - Root of virtual SD card
- `data/sys/` - System files (tasks, events, metadata)
- `data/journal/` - Journal entries
- `data/dict/` - Dictionary files

## Development

### Build Options

```bash
# Debug build
./build.sh --debug

# Clean build
./build.sh --clean

# Check dependencies only
./build.sh --dry-run
```

### Adding New Features

1. Hardware mocks go in `include/` and `src/hardware_shim.cpp`
2. Library shims go in `src/pocketmage_shim.cpp`
3. Platform-specific code goes in `src/desktop_display_<platform>.cpp`

## Installing Apps with install_app.py

The `install_app.py` script automates integrating standalone PocketMage apps into the desktop emulator.

### Usage

```bash
# Install from app folder
python3 install_app.py ../Code/MyApp

# Install from tar file (includes icon)
python3 install_app.py myapp.tar
```

### What It Does

1. **Converts standalone app code** to integrated format:
   - `processKB()` → `processKB_MYAPP()`
   - `applicationEinkHandler()` → `einkHandler_MYAPP()`
   - Removes `setup()`, `loop()`, `einkHandler(void*)` (handled by main firmware)
   - Replaces `rebootToPocketMage()` with state change to HOME

2. **Updates emulator source files**:
   - `globals.h` - Adds app to `AppState` enum and function declarations
   - `PocketMageV3.cpp` - Adds switch cases for einkHandler and processKB
   - `APPLAUNCHER.cpp` - Adds app entry with icon for the App Launcher
   - `CMakeLists.txt` - Adds source file to build

3. **Copies icon** to `data/apps/` for the App Launcher

4. **Rebuilds the emulator** automatically

### Launching Your App

After installation:
- Type `apps` or `launcher` at the HOME prompt
- Select your app from the App Launcher
- Press Enter to launch

### Example Workflow

```bash
# 1. Create app from StarterApp template
cp -r ../Code/StarterApp ../Code/MyNewApp

# 2. Edit your app code
# (modify ../Code/MyNewApp/src/appMain.cpp)

# 3. Install into emulator
python3 install_app.py ../Code/MyNewApp

# 4. Run emulator and test
./build/PocketMage_PDA_Emulator
# Type: apps
# Select your app and press Enter
```

## Exporting Apps with export_app.py

The `export_app.py` script converts an integrated emulator app back to standalone format for building and flashing to the real PocketMage device.

### Usage

```bash
# Export to default location (Code/AppName_Standalone/)
python3 export_app.py StarterApp

# Export to specific folder
python3 export_app.py MyApp ../Code/MyApp_Standalone
```

### What It Does

Reverses the transformations done by `install_app.py`:

| Integrated (Emulator) | Standalone (Real Device) |
|-----------------------|--------------------------|
| `#include <globals.h>` | `#include <pocketmage.h>` |
| `processKB_MYAPP()` | `processKB()` |
| `einkHandler_MYAPP()` | `applicationEinkHandler()` |
| `CurrentAppState = HOME` | `rebootToPocketMage()` |

Also adds back the standalone functions: `setup()`, `loop()`, `einkHandler(void*)`

### Output

Creates a complete PlatformIO project:
- `src/appMain.cpp` - Converted standalone code
- `platformio.ini` - Build configuration
- `appPartition.csv` - Partition table
- `README.md` - Build instructions

### Building for Real Device

```bash
cd Code/MyApp_Standalone
pio run              # Build
pio run -t upload    # Flash to device
```

### Creating App Package for App Loader

```bash
cp .pio/build/myapp/firmware.bin myapp.bin
tar -cvf myapp.tar myapp.bin myapp_ICON.bin
# Copy myapp.tar to SD card /apps/ folder
```

## Troubleshooting

### Font Issues

If text doesn't appear, ensure the `fonts/` directory contains TTF fonts (DejaVuSans.ttf is bundled).

### Build Errors

- Ensure SDL2 and SDL2_ttf are installed
- Check CMake version: `cmake --version` (need 3.16+)
- On Windows, ensure vcpkg is properly integrated

### Display Issues

- Both windows should appear (E-ink and OLED)
- If windows are black, check font loading in console output

## License

See the main PocketMage repository for license information.
