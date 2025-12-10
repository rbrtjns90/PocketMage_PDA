/**
 * @file desktop_display_sdl2_windows.cpp
 * @brief Windows-specific display backend implementation
 */

#include "desktop_display_sdl2.h"

#ifdef PLATFORM_WINDOWS

#include <iostream>
#include <filesystem>
#include <cstdlib>

// ============================================================================
// Windows Platform-Specific Implementation
// ============================================================================

bool DesktopDisplay::platformInit() {
    // Windows-specific SDL hints (tuned for desktop use)

    // Prefer high-quality scaling if available
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");

    // Disable HiDPI auto-scaling; we control logical size explicitly
    SDL_SetHint(SDL_HINT_VIDEO_HIGHDPI_DISABLED, "1");

    return true;
}

void DesktopDisplay::platformShutdown() {
    // Windows-specific cleanup can go here if needed in the future
}

std::string DesktopDisplay::platformGetFontPath() {
    // Search order:
    //  1. Bundled fonts (in the build/run directory)
    //  2. System fonts in C:\Windows\Fonts

    std::vector<std::string> fontPaths = {
        // Bundled DejaVu fonts (relative paths)
        "./fonts/DejaVuSans.ttf",
        "../fonts/DejaVuSans.ttf",
        "../../fonts/DejaVuSans.ttf",

        // Windows system fonts (fallbacks)
        "C:/Windows/Fonts/segoeui.ttf",
        "C:/Windows/Fonts/calibri.ttf",
        "C:/Windows/Fonts/arial.ttf",
        "C:/Windows/Fonts/tahoma.ttf",
    };

    for (const auto& path : fontPaths) {
        if (std::filesystem::exists(path)) {
            std::cout << "[Windows] Found font: " << path << std::endl;
            return path;
        }
    }

    std::cerr << "[Windows] Warning: No suitable font found" << std::endl;
    std::cerr << "[Windows] Ensure fonts/DejaVuSans.ttf exists or a common system font is installed." << std::endl;

    return "";
}

#endif // PLATFORM_WINDOWS

