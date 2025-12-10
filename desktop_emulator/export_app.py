#!/usr/bin/env python3
"""
export_app.py - Convert integrated emulator app back to standalone format

This script reverses the transformations done by install_app.py, converting
an integrated app back to standalone format for building and flashing to
the real PocketMage device.

Usage:
    python3 export_app.py APPNAME [output_folder]
    
Examples:
    python3 export_app.py StarterApp
    python3 export_app.py MyApp ../Code/MyApp_Standalone
"""

import os
import re
import sys
import shutil

# Paths relative to this script
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
POCKETMAGE_SRC = os.path.join(SCRIPT_DIR, '..', 'Code', 'PocketMage_V3', 'src')


def read_file(path):
    """Read file contents."""
    with open(path, 'r') as f:
        return f.read()


def write_file(path, content):
    """Write content to file."""
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, 'w') as f:
        f.write(content)


def find_integrated_app(app_name):
    """Find the integrated app source file."""
    app_upper = app_name.upper()
    
    # Try different naming patterns
    patterns = [
        os.path.join(POCKETMAGE_SRC, f'{app_upper}_APP.cpp'),
        os.path.join(POCKETMAGE_SRC, f'{app_name}_APP.cpp'),
        os.path.join(POCKETMAGE_SRC, f'{app_name}.cpp'),
        os.path.join(POCKETMAGE_SRC, 'OS_APPS', f'{app_upper}.cpp'),
    ]
    
    for path in patterns:
        if os.path.exists(path):
            return path
    
    return None


def convert_to_standalone(source_code, app_name):
    """Convert integrated app code back to standalone format."""
    app_upper = app_name.upper()
    
    code = source_code
    
    # Replace includes
    code = re.sub(r'#include\s*<globals\.h>', '#include <pocketmage.h>', code)
    
    # Rename functions back to standalone names
    code = re.sub(rf'\bvoid\s+processKB_{app_upper}\s*\(\s*\)', 'void processKB()', code)
    code = re.sub(rf'\bvoid\s+einkHandler_{app_upper}\s*\(\s*\)', 'void applicationEinkHandler()', code)
    
    # Replace state change back to HOME with rebootToPocketMage()
    code = re.sub(
        r'CurrentAppState\s*=\s*HOME\s*;\s*HOME_INIT\s*\(\s*\)\s*;\s*return\s*;',
        'rebootToPocketMage();',
        code
    )
    
    # Also handle simpler patterns
    code = re.sub(
        r'CurrentAppState\s*=\s*HOME\s*;',
        'rebootToPocketMage();',
        code
    )
    
    # Remove the auto-generated INIT function
    init_pattern = rf'// Auto-generated INIT function\s*\nvoid\s+{app_upper}_INIT\s*\(\s*\)\s*\{{[^}}]*\}}\s*\n'
    code = re.sub(init_pattern, '', code)
    
    # Add back the standalone functions that were removed
    standalone_functions = '''
// ============================================================================
// MAIN FUNCTIONS (required for standalone app)
// ============================================================================

void setup() {
    // Initialize hardware
    initPocketMage();
    
    // Initialize app state
    currentScreen = SCREEN_MAIN;
    needsRedraw = true;
}

void loop() {
    // Main loop - handled by PocketMage library
    delay(10);
}

void einkHandler(void* parameter) {
    // E-ink refresh task
    while (true) {
        if (needsRedraw) {
            applicationEinkHandler();
            needsRedraw = false;
        }
        vTaskDelay(pdMS_TO_TICKS(100));
        yield();
    }
}
'''
    
    # Add standalone functions at the end of the file
    code = code.rstrip() + '\n' + standalone_functions
    
    return code


def create_standalone_project(app_name, source_code, output_dir):
    """Create a complete standalone project structure."""
    
    # Create directory structure
    os.makedirs(os.path.join(output_dir, 'src'), exist_ok=True)
    os.makedirs(os.path.join(output_dir, 'include'), exist_ok=True)
    
    # Write the converted source
    write_file(os.path.join(output_dir, 'src', 'appMain.cpp'), source_code)
    
    # Create platformio.ini
    platformio_ini = f'''[env:{app_name.lower()}]
platform = espressif32
board = esp32dev
framework = arduino
monitor_speed = 115200
upload_speed = 921600

lib_deps =
    adafruit/Adafruit GFX Library
    adafruit/Adafruit BusIO
    zinggjm/GxEPD2
    adafruit/RTClib

board_build.partitions = appPartition.csv
'''
    write_file(os.path.join(output_dir, 'platformio.ini'), platformio_ini)
    
    # Create partition table
    partition_csv = '''# Name,   Type, SubType, Offset,  Size, Flags
nvs,      data, nvs,     0x9000,  0x5000,
otadata,  data, ota,     0xe000,  0x2000,
app0,     app,  ota_0,   0x10000, 0x1E0000,
spiffs,   data, spiffs,  0x1F0000,0x10000,
'''
    write_file(os.path.join(output_dir, 'appPartition.csv'), partition_csv)
    
    # Create README
    readme = f'''# {app_name}

Standalone PocketMage app exported from desktop emulator.

## Building

```bash
pio run
```

## Flashing

```bash
pio run -t upload
```

## Creating App Package

After building, create a tar file for the App Loader:

```bash
cp .pio/build/{app_name.lower()}/firmware.bin {app_name.lower()}.bin
tar -cvf {app_name.lower()}.tar {app_name.lower()}.bin {app_name.lower()}_ICON.bin
```

Copy the tar file to the SD card `/apps/` folder.
'''
    write_file(os.path.join(output_dir, 'README.md'), readme)
    
    print(f"Created standalone project in: {output_dir}")


def main():
    if len(sys.argv) < 2:
        print("Usage: python3 export_app.py APPNAME [output_folder]")
        print("\nExamples:")
        print("  python3 export_app.py StarterApp")
        print("  python3 export_app.py MyApp ../Code/MyApp_Standalone")
        sys.exit(1)
    
    app_name = sys.argv[1]
    
    # Determine output directory
    if len(sys.argv) >= 3:
        output_dir = sys.argv[2]
    else:
        output_dir = os.path.join(SCRIPT_DIR, '..', 'Code', f'{app_name}_Standalone')
    
    output_dir = os.path.abspath(output_dir)
    
    print(f"Exporting app: {app_name}")
    print(f"Output directory: {output_dir}")
    
    # Find the integrated app source
    source_path = find_integrated_app(app_name)
    if not source_path:
        print(f"Error: Could not find integrated app '{app_name}'")
        print(f"Looked in: {POCKETMAGE_SRC}")
        sys.exit(1)
    
    print(f"Found source: {source_path}")
    
    # Read and convert
    source_code = read_file(source_path)
    standalone_code = convert_to_standalone(source_code, app_name)
    
    # Create standalone project
    create_standalone_project(app_name, standalone_code, output_dir)
    
    print(f"\n{'='*60}")
    print(f"SUCCESS! Exported '{app_name}' to standalone format.")
    print(f"{'='*60}")
    print(f"\nNext steps:")
    print(f"  1. cd {output_dir}")
    print(f"  2. pio run")
    print(f"  3. pio run -t upload")
    print(f"\nOr create a tar package for the App Loader.")


if __name__ == '__main__':
    main()
