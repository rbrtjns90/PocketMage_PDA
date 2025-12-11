#!/bin/bash
# PocketMage PDA Desktop Emulator Build Script
# Customized for Fedora / Red Hat / CentOS / Rocky Linux / AlmaLinux

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${GREEN}=== PocketMage PDA Desktop Emulator Build (Fedora/RHEL) ===${NC}"

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
        --help|-h)
            echo "Usage: $0 [OPTIONS]"
            echo ""
            echo "Options:"
            echo "  --clean     Remove build directory before building"
            echo "  --debug     Build with debug symbols"
            echo "  --dry-run   Check dependencies without building"
            echo "  --help, -h  Show this help message"
            exit 0
            ;;
        *)
            echo -e "${RED}Unknown option: $1${NC}"
            echo "Use --help for usage information"
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

# Detect package manager (dnf is preferred, yum as fallback for older RHEL/CentOS)
detect_package_manager() {
    if command -v dnf &> /dev/null; then
        echo "dnf"
    elif command -v yum &> /dev/null; then
        echo "yum"
    else
        echo ""
    fi
}

PKG_MGR=$(detect_package_manager)

if [ -z "$PKG_MGR" ]; then
    echo -e "${RED}Error: Neither dnf nor yum package manager found.${NC}"
    echo "This script is designed for Fedora/RHEL-based distributions."
    exit 1
fi

echo -e "${YELLOW}Detected package manager: ${PKG_MGR}${NC}"

echo -e "${YELLOW}Checking dependencies...${NC}"

# Check cmake
if ! check_command cmake; then
    echo -e "${YELLOW}Installing cmake via ${PKG_MGR}...${NC}"
    if $DRY_RUN; then
        echo "[DRY RUN] Would run: sudo ${PKG_MGR} install -y cmake"
    else
        sudo ${PKG_MGR} install -y cmake
    fi
fi

# Check for g++ (C++ compiler)
if ! check_command g++; then
    echo -e "${YELLOW}Installing C++ compiler via ${PKG_MGR}...${NC}"
    if $DRY_RUN; then
        echo "[DRY RUN] Would run: sudo ${PKG_MGR} install -y gcc-c++"
    else
        sudo ${PKG_MGR} install -y gcc-c++
    fi
fi

# Check for pkg-config
if ! check_command pkg-config; then
    echo -e "${YELLOW}Installing pkg-config via ${PKG_MGR}...${NC}"
    if $DRY_RUN; then
        echo "[DRY RUN] Would run: sudo ${PKG_MGR} install -y pkgconf-pkg-config"
    else
        sudo ${PKG_MGR} install -y pkgconf-pkg-config
    fi
fi

# Check for SDL2
# Fedora/RHEL package names: SDL2-devel, SDL2_ttf-devel
if ! pkg-config --exists sdl2 2>/dev/null; then
    echo -e "${YELLOW}Installing SDL2 development libraries via ${PKG_MGR}...${NC}"
    if $DRY_RUN; then
        echo "[DRY RUN] Would run: sudo ${PKG_MGR} install -y SDL2-devel SDL2_ttf-devel"
    else
        sudo ${PKG_MGR} install -y SDL2-devel SDL2_ttf-devel
    fi
fi

# Check for make (sometimes not installed by default)
if ! check_command make; then
    echo -e "${YELLOW}Installing make via ${PKG_MGR}...${NC}"
    if $DRY_RUN; then
        echo "[DRY RUN] Would run: sudo ${PKG_MGR} install -y make"
    else
        sudo ${PKG_MGR} install -y make
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
            # Alternative: try to install DejaVu fonts from system packages
            echo -e "${YELLOW}Attempting to install system fonts...${NC}"
            sudo ${PKG_MGR} install -y dejavu-sans-fonts dejavu-sans-mono-fonts dejavu-serif-fonts || true
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

# Build using all available cores
echo -e "${YELLOW}Building...${NC}"
cmake --build . -j$(nproc)

echo -e "${GREEN}=== Build Complete ===${NC}"
echo -e "Run the emulator with: ${YELLOW}./build/PocketMage_PDA_Emulator${NC}"

