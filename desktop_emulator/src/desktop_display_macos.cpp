/**
 * @file desktop_display_macos.cpp
 * @brief macOS-specific display backend implementation
 */

#include "desktop_display_sdl2.h"

#ifdef PLATFORM_MACOS

#include <iostream>
#include <filesystem>
#include <cstdlib>

// ============================================================================
// macOS Platform-Specific Implementation
// ============================================================================

bool DesktopDisplay::platformInit() {
    // macOS-specific initialization
    // SDL handles most things automatically on macOS
    
    // Set hint for better rendering on Retina displays
    SDL_SetHint(SDL_HINT_VIDEO_HIGHDPI_DISABLED, "0");
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
    
    return true;
}

void DesktopDisplay::platformShutdown() {
    // macOS-specific cleanup (if any)
}

std::string DesktopDisplay::platformGetFontPath() {
    // Try multiple font locations on macOS
    std::vector<std::string> fontPaths = {
        // Bundled fonts (preferred)
        "./fonts/DejaVuSans.ttf",
        "../fonts/DejaVuSans.ttf",
        "fonts/DejaVuSans.ttf",
        
        // Homebrew installed fonts
        "/opt/homebrew/share/fonts/dejavu/DejaVuSans.ttf",
        "/usr/local/share/fonts/dejavu/DejaVuSans.ttf",
        
        // System fonts
        "/System/Library/Fonts/Helvetica.ttc",
        "/System/Library/Fonts/SFNSMono.ttf",
        "/Library/Fonts/Arial.ttf",
        
        // User fonts
        std::string(getenv("HOME") ? getenv("HOME") : "") + "/Library/Fonts/DejaVuSans.ttf",
    };
    
    for (const auto& path : fontPaths) {
        if (std::filesystem::exists(path)) {
            std::cout << "[macOS] Found font: " << path << std::endl;
            return path;
        }
    }
    
    std::cerr << "[macOS] Warning: No suitable font found" << std::endl;
    std::cerr << "[macOS] Run ./fonts/download_fonts.sh to download fonts" << std::endl;
    
    return "";
}

#endif // PLATFORM_MACOS
