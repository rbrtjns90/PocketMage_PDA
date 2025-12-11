#!/bin/bash
# PocketMage PDA Desktop Emulator Build Script

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${GREEN}=== PocketMage PDA Desktop Emulator Build ===${NC}"

# Parse arguments
CLEAN_BUILD=false
DRY_RUN=false
BUILD_TYPE="Release"

while [[ $# -gt 0 ]]; do
    case $1 in
        --clean)
            CLEAN_BUILD=true
            shift
            ;;
        --debug)
            BUILD_TYPE="Debug"
            shift
            ;;
        --dry-run)
            DRY_RUN=true
            shift
            ;;
        *)
            echo -e "${RED}Unknown option: $1${NC}"
            exit 1
            ;;
    esac
done

# Check for required tools
check_command() {
    if ! command -v $1 &> /dev/null; then
        echo -e "${RED}Error: $1 is required but not installed.${NC}"
        return 1
    fi
    return 0
}

echo -e "${YELLOW}Checking dependencies...${NC}"

# Check cmake
if ! check_command cmake; then
    echo "Please install cmake: brew install cmake (macOS) or apt install cmake (Linux)"
    exit 1
fi

# Check for SDL2
if [[ "$OSTYPE" == "darwin"* ]]; then
    # macOS
    if ! brew list sdl2 &>/dev/null; then
        echo -e "${YELLOW}Installing SDL2 via Homebrew...${NC}"
        if $DRY_RUN; then
            echo "[DRY RUN] Would run: brew install sdl2 sdl2_ttf"
        else
            brew install sdl2 sdl2_ttf
        fi
    fi
elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
    # Linux
    if ! pkg-config --exists sdl2; then
        echo -e "${YELLOW}Installing SDL2 via apt...${NC}"
        if $DRY_RUN; then
            echo "[DRY RUN] Would run: sudo apt-get install libsdl2-dev libsdl2-ttf-dev"
        else
            sudo apt-get install -y libsdl2-dev libsdl2-ttf-dev
        fi
    fi
fi

# Download fonts if needed
if [ ! -f "fonts/DejaVuSans.ttf" ]; then
    echo -e "${YELLOW}Downloading fonts...${NC}"
    if $DRY_RUN; then
        echo "[DRY RUN] Would download fonts"
    else
        if [ -f "fonts/download_fonts.sh" ]; then
            chmod +x fonts/download_fonts.sh
            ./fonts/download_fonts.sh
        else
            echo -e "${YELLOW}Font download script not found, fonts may be missing${NC}"
        fi
    fi
fi

if $DRY_RUN; then
    echo -e "${GREEN}Dry run complete. All dependencies appear to be available.${NC}"
    exit 0
fi

# Clean build if requested
if $CLEAN_BUILD; then
    echo -e "${YELLOW}Cleaning build directory...${NC}"
    rm -rf build
fi

# Create build directory
mkdir -p build
cd build

# Configure with CMake
echo -e "${YELLOW}Configuring with CMake...${NC}"
cmake .. -DCMAKE_BUILD_TYPE=$BUILD_TYPE

# Build
echo -e "${YELLOW}Building...${NC}"
if [[ "$OSTYPE" == "darwin"* ]]; then
    cmake --build . -j$(sysctl -n hw.ncpu)
elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
    cmake --build . -j$(nproc)
else
    cmake --build .
fi

echo -e "${GREEN}=== Build Complete ===${NC}"
echo -e "Run the emulator with: ${YELLOW}./build/PocketMage_PDA_Emulator${NC}"
