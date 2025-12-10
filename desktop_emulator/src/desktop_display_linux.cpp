/**
 * @file desktop_display_linux.cpp
 * @brief Linux-specific display backend implementation
 */

#include "desktop_display_sdl2.h"

#ifdef PLATFORM_LINUX

#include <iostream>
#include <filesystem>
#include <cstdlib>

// ============================================================================
// Linux Platform-Specific Implementation
// ============================================================================

bool DesktopDisplay::platformInit() {
    // Linux-specific initialization
    
    // Set hints for better rendering
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
    
    // X11/Wayland specific hints
    SDL_SetHint(SDL_HINT_VIDEO_X11_NET_WM_BYPASS_COMPOSITOR, "0");
    
    return true;
}

void DesktopDisplay::platformShutdown() {
    // Linux-specific cleanup (if any)
}

std::string DesktopDisplay::platformGetFontPath() {
    // Try multiple font locations on Linux
    std::vector<std::string> fontPaths = {
        // Bundled fonts (preferred)
        "./fonts/DejaVuSans.ttf",
        "../fonts/DejaVuSans.ttf",
        "fonts/DejaVuSans.ttf",
        
        // System fonts (Debian/Ubuntu)
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/TTF/DejaVuSans.ttf",
        
        // System fonts (Fedora/RHEL)
        "/usr/share/fonts/dejavu-sans-fonts/DejaVuSans.ttf",
        "/usr/share/fonts/dejavu/DejaVuSans.ttf",
        
        // System fonts (Arch)
        "/usr/share/fonts/TTF/DejaVuSans.ttf",
        "/usr/share/fonts/truetype/DejaVuSans.ttf",
        
        // Liberation fonts (common alternative)
        "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
        "/usr/share/fonts/liberation-sans/LiberationSans-Regular.ttf",
        
        // Noto fonts
        "/usr/share/fonts/truetype/noto/NotoSans-Regular.ttf",
        "/usr/share/fonts/noto/NotoSans-Regular.ttf",
        
        // User fonts
        std::string(getenv("HOME") ? getenv("HOME") : "") + "/.local/share/fonts/DejaVuSans.ttf",
        std::string(getenv("HOME") ? getenv("HOME") : "") + "/.fonts/DejaVuSans.ttf",
    };
    
    for (const auto& path : fontPaths) {
        if (std::filesystem::exists(path)) {
            std::cout << "[Linux] Found font: " << path << std::endl;
            return path;
        }
    }
    
    std::cerr << "[Linux] Warning: No suitable font found" << std::endl;
    std::cerr << "[Linux] Install dejavu fonts: sudo apt install fonts-dejavu" << std::endl;
    std::cerr << "[Linux] Or run ./fonts/download_fonts.sh to download fonts" << std::endl;
    
    return "";
}

#endif // PLATFORM_LINUX
