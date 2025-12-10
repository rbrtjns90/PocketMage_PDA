/**
 * @file gfx_fonts.cpp
 * @brief Stub GFX font definitions for desktop emulator
 * 
 * The emulator uses SDL2_ttf for text rendering, so we just need
 * stub font definitions to satisfy the linker.
 */

#include "pocketmage_compat.h"
#include "Adafruit_GFX.h"

// Linux build: Use same approach as Windows (extern const definitions here, extern declarations in headers)
// macOS build: Uses inline definitions in headers (clang handles this correctly)
#if defined(_WIN32) || defined(__LINUX__) || defined(__linux__)

#ifdef _WIN32
#pragma message("gfx_fonts.cpp: Compiling Windows font definitions")
#else
#pragma message("gfx_fonts.cpp: Compiling Linux font definitions")
#endif

// Windows/Linux: Define stub fonts with extern const for external linkage
// (const at file scope has internal linkage by default in C++)
// The actual font data is not used since SDL2_ttf handles text rendering
extern const GFXfont FreeMono9pt7b = {nullptr, nullptr, 0, 0, 16};
extern const GFXfont FreeMono12pt7b = {nullptr, nullptr, 0, 0, 20};
extern const GFXfont FreeMonoBold9pt7b = {nullptr, nullptr, 0, 0, 16};
extern const GFXfont FreeMonoBold12pt7b = {nullptr, nullptr, 0, 0, 20};
extern const GFXfont FreeMonoBold18pt7b = {nullptr, nullptr, 0, 0, 28};
extern const GFXfont FreeMonoBold24pt7b = {nullptr, nullptr, 0, 0, 36};
extern const GFXfont FreeMonoOblique9pt7b = {nullptr, nullptr, 0, 0, 16};
extern const GFXfont FreeMonoBoldOblique9pt7b = {nullptr, nullptr, 0, 0, 16};
extern const GFXfont FreeMonoBoldOblique12pt7b = {nullptr, nullptr, 0, 0, 20};
extern const GFXfont FreeMonoBoldOblique18pt7b = {nullptr, nullptr, 0, 0, 28};
extern const GFXfont FreeMonoBoldOblique24pt7b = {nullptr, nullptr, 0, 0, 36};
extern const GFXfont FreeSans9pt7b = {nullptr, nullptr, 0, 0, 16};
extern const GFXfont FreeSans12pt7b = {nullptr, nullptr, 0, 0, 20};
extern const GFXfont FreeSansBold9pt7b = {nullptr, nullptr, 0, 0, 16};
extern const GFXfont FreeSansBold12pt7b = {nullptr, nullptr, 0, 0, 20};
extern const GFXfont FreeSansBold18pt7b = {nullptr, nullptr, 0, 0, 28};
extern const GFXfont FreeSansBold24pt7b = {nullptr, nullptr, 0, 0, 36};
extern const GFXfont FreeSansOblique9pt7b = {nullptr, nullptr, 0, 0, 16};
extern const GFXfont FreeSansBoldOblique9pt7b = {nullptr, nullptr, 0, 0, 16};
extern const GFXfont FreeSansBoldOblique12pt7b = {nullptr, nullptr, 0, 0, 20};
extern const GFXfont FreeSansBoldOblique18pt7b = {nullptr, nullptr, 0, 0, 28};
extern const GFXfont FreeSansBoldOblique24pt7b = {nullptr, nullptr, 0, 0, 36};
extern const GFXfont FreeSerif9pt7b = {nullptr, nullptr, 0, 0, 16};
extern const GFXfont FreeSerif12pt7b = {nullptr, nullptr, 0, 0, 20};
extern const GFXfont FreeSerifBold9pt7b = {nullptr, nullptr, 0, 0, 16};
extern const GFXfont FreeSerifBold12pt7b = {nullptr, nullptr, 0, 0, 20};
extern const GFXfont FreeSerifBold18pt7b = {nullptr, nullptr, 0, 0, 28};
extern const GFXfont FreeSerifBold24pt7b = {nullptr, nullptr, 0, 0, 36};
extern const GFXfont FreeSerifItalic9pt7b = {nullptr, nullptr, 0, 0, 16};
extern const GFXfont FreeSerifBoldItalic9pt7b = {nullptr, nullptr, 0, 0, 16};
extern const GFXfont FreeSerifBoldItalic12pt7b = {nullptr, nullptr, 0, 0, 20};
extern const GFXfont FreeSerifBoldItalic18pt7b = {nullptr, nullptr, 0, 0, 28};
extern const GFXfont FreeSerifBoldItalic24pt7b = {nullptr, nullptr, 0, 0, 36};

// Custom PocketMage fonts - need extern for external linkage
extern const GFXfont Font5x7Fixed = {nullptr, nullptr, 0, 0, 10};
extern const GFXfont Font3x7FixedNum = {nullptr, nullptr, 0, 0, 10};
extern const GFXfont Font4x5Fixed = {nullptr, nullptr, 0, 0, 8};

#else
// macOS: provide stub font definitions (inline from headers works with clang)
GFXfont FreeMono9pt7b = {nullptr, nullptr, 0, 0, 16};
GFXfont FreeMono12pt7b = {nullptr, nullptr, 0, 0, 20};
GFXfont FreeMonoBold9pt7b = {nullptr, nullptr, 0, 0, 16};
GFXfont FreeMonoBold12pt7b = {nullptr, nullptr, 0, 0, 20};
GFXfont FreeMonoBold18pt7b = {nullptr, nullptr, 0, 0, 28};
GFXfont FreeMonoBold24pt7b = {nullptr, nullptr, 0, 0, 36};
GFXfont FreeMonoOblique9pt7b = {nullptr, nullptr, 0, 0, 16};
GFXfont FreeMonoBoldOblique9pt7b = {nullptr, nullptr, 0, 0, 16};
GFXfont FreeMonoBoldOblique12pt7b = {nullptr, nullptr, 0, 0, 20};
GFXfont FreeMonoBoldOblique18pt7b = {nullptr, nullptr, 0, 0, 28};
GFXfont FreeMonoBoldOblique24pt7b = {nullptr, nullptr, 0, 0, 36};
GFXfont FreeSans9pt7b = {nullptr, nullptr, 0, 0, 16};
GFXfont FreeSans12pt7b = {nullptr, nullptr, 0, 0, 20};
GFXfont FreeSansBold9pt7b = {nullptr, nullptr, 0, 0, 16};
GFXfont FreeSansBold12pt7b = {nullptr, nullptr, 0, 0, 20};
GFXfont FreeSansBold18pt7b = {nullptr, nullptr, 0, 0, 28};
GFXfont FreeSansBold24pt7b = {nullptr, nullptr, 0, 0, 36};
GFXfont FreeSansOblique9pt7b = {nullptr, nullptr, 0, 0, 16};
GFXfont FreeSansBoldOblique9pt7b = {nullptr, nullptr, 0, 0, 16};
GFXfont FreeSansBoldOblique12pt7b = {nullptr, nullptr, 0, 0, 20};
GFXfont FreeSansBoldOblique18pt7b = {nullptr, nullptr, 0, 0, 28};
GFXfont FreeSansBoldOblique24pt7b = {nullptr, nullptr, 0, 0, 36};
GFXfont FreeSerif9pt7b = {nullptr, nullptr, 0, 0, 16};
GFXfont FreeSerif12pt7b = {nullptr, nullptr, 0, 0, 20};
GFXfont FreeSerifBold9pt7b = {nullptr, nullptr, 0, 0, 16};
GFXfont FreeSerifBold12pt7b = {nullptr, nullptr, 0, 0, 20};
GFXfont FreeSerifBold18pt7b = {nullptr, nullptr, 0, 0, 28};
GFXfont FreeSerifBold24pt7b = {nullptr, nullptr, 0, 0, 36};
GFXfont FreeSerifItalic9pt7b = {nullptr, nullptr, 0, 0, 16};
GFXfont FreeSerifBoldItalic9pt7b = {nullptr, nullptr, 0, 0, 16};
GFXfont FreeSerifBoldItalic12pt7b = {nullptr, nullptr, 0, 0, 20};
GFXfont FreeSerifBoldItalic18pt7b = {nullptr, nullptr, 0, 0, 28};
GFXfont FreeSerifBoldItalic24pt7b = {nullptr, nullptr, 0, 0, 36};

// Custom PocketMage fonts
extern const GFXfont Font5x7Fixed = {nullptr, nullptr, 0, 0, 10};
extern const GFXfont Font3x7FixedNum = {nullptr, nullptr, 0, 0, 10};
extern const GFXfont Font4x5Fixed = {nullptr, nullptr, 0, 0, 8};
#endif
