/**
 * @file desktop_display_windows.cpp
 * @brief Windows-specific display backend implementation
 */

#include "desktop_display_sdl2.h"

#ifdef PLATFORM_WINDOWS

#include <iostream>
#include <filesystem>
#include <cstdlib>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <shlobj.h>

// ============================================================================
// Windows Platform-Specific Implementation
// ============================================================================

bool DesktopDisplay::platformInit() {
    // Windows-specific initialization
    
    // Set hints for better rendering
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
    
    // Windows-specific hints
    SDL_SetHint(SDL_HINT_WINDOWS_DPI_AWARENESS, "permonitorv2");
    SDL_SetHint(SDL_HINT_WINDOWS_DPI_SCALING, "1");
    
    return true;
}

void DesktopDisplay::platformShutdown() {
    // Windows-specific cleanup (if any)
}

std::string DesktopDisplay::platformGetFontPath() {
    // Get Windows fonts directory
    char windowsFontsPath[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_FONTS, NULL, 0, windowsFontsPath))) {
        // Windows fonts path obtained
    } else {
        strcpy(windowsFontsPath, "C:\\Windows\\Fonts");
    }
    
    // Try multiple font locations on Windows
    std::vector<std::string> fontPaths = {
        // Bundled fonts (preferred)
        ".\\fonts\\DejaVuSans.ttf",
        "..\\fonts\\DejaVuSans.ttf",
        "fonts\\DejaVuSans.ttf",
        
        // Windows system fonts
        std::string(windowsFontsPath) + "\\segoeui.ttf",    // Segoe UI
        std::string(windowsFontsPath) + "\\arial.ttf",       // Arial
        std::string(windowsFontsPath) + "\\tahoma.ttf",      // Tahoma
        std::string(windowsFontsPath) + "\\verdana.ttf",     // Verdana
        std::string(windowsFontsPath) + "\\calibri.ttf",     // Calibri
        std::string(windowsFontsPath) + "\\consola.ttf",     // Consolas
        
        // DejaVu if installed
        std::string(windowsFontsPath) + "\\DejaVuSans.ttf",
        
        // User fonts
        std::string(getenv("LOCALAPPDATA") ? getenv("LOCALAPPDATA") : "") + 
            "\\Microsoft\\Windows\\Fonts\\DejaVuSans.ttf",
    };
    
    for (const auto& path : fontPaths) {
        if (std::filesystem::exists(path)) {
            std::cout << "[Windows] Found font: " << path << std::endl;
            return path;
        }
    }
    
    std::cerr << "[Windows] Warning: No suitable font found" << std::endl;
    std::cerr << "[Windows] Run fonts\\download_fonts.bat or copy DejaVuSans.ttf to fonts\\" << std::endl;
    
    return "";
}

#endif // PLATFORM_WINDOWS
