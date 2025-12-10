#!/bin/bash
# Load App Script for PocketMage Desktop Emulator
# Usage: ./load_app.sh path/to/app.tar
#
# This script extracts a PocketMage app tar file and integrates it
# into the desktop emulator for testing.

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
EMULATOR_DIR="$SCRIPT_DIR"
APPS_SRC_DIR="$EMULATOR_DIR/apps"
BUILD_DIR="$EMULATOR_DIR/build"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

usage() {
    echo "Usage: $0 <app.tar>"
    echo ""
    echo "Loads a PocketMage app tar file into the desktop emulator."
    echo "The emulator will be rebuilt with the app integrated."
    exit 1
}

if [ $# -lt 1 ]; then
    usage
fi

TAR_FILE="$1"

if [ ! -f "$TAR_FILE" ]; then
    echo -e "${RED}Error: File not found: $TAR_FILE${NC}"
    exit 1
fi

# Get app name from tar filename
APP_NAME=$(basename "$TAR_FILE" .tar)
APP_NAME_UPPER=$(echo "$APP_NAME" | tr '[:lower:]' '[:upper:]' | tr '-' '_' | tr ' ' '_')
APP_NAME_LOWER=$(echo "$APP_NAME" | tr '[:upper:]' '[:lower:]' | tr '-' '_' | tr ' ' '_')

echo -e "${GREEN}Loading app: $APP_NAME${NC}"

# Create apps directory if it doesn't exist
mkdir -p "$APPS_SRC_DIR"

# Extract tar to temp directory
TEMP_DIR=$(mktemp -d)
echo "Extracting to temp: $TEMP_DIR"
tar -xf "$TAR_FILE" -C "$TEMP_DIR"

# Find the .cpp or source files
APP_CPP=""
APP_BIN=""
APP_ICON=""

for f in "$TEMP_DIR"/*; do
    case "$f" in
        *.cpp) APP_CPP="$f" ;;
        *.bin) APP_BIN="$f" ;;
        *icon*.bmp|*ICON*.bin) APP_ICON="$f" ;;
    esac
done

# Check if we have source code
if [ -n "$APP_CPP" ]; then
    echo -e "${GREEN}Found source: $(basename "$APP_CPP")${NC}"
    cp "$APP_CPP" "$APPS_SRC_DIR/${APP_NAME_LOWER}.cpp"
    
    # Copy icon if present
    if [ -n "$APP_ICON" ]; then
        mkdir -p "$EMULATOR_DIR/data/apps"
        cp "$APP_ICON" "$EMULATOR_DIR/data/apps/${APP_NAME_LOWER}_icon.bin"
        echo "Copied icon to data/apps/"
    fi
    
    echo -e "${YELLOW}Source app loaded. You need to:${NC}"
    echo "1. Add to CMakeLists.txt: apps/${APP_NAME_LOWER}.cpp"
    echo "2. Add AppState enum: ${APP_NAME_UPPER}"
    echo "3. Add to globals.h declarations"
    echo "4. Add switch cases in PocketMageV3.cpp"
    echo "5. Rebuild: cd build && make"
    
elif [ -n "$APP_BIN" ]; then
    echo -e "${YELLOW}Found binary only: $(basename "$APP_BIN")${NC}"
    echo "Binary apps cannot be loaded into the emulator."
    echo "The emulator requires source code to compile."
    echo ""
    echo "To test this app, you need real hardware."
else
    echo -e "${RED}No .cpp or .bin found in tar file${NC}"
    ls -la "$TEMP_DIR"
fi

# Cleanup
rm -rf "$TEMP_DIR"

echo ""
echo -e "${GREEN}Done!${NC}"
