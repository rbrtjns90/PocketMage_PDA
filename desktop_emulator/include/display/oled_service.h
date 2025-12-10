/**
 * @file oled_service.h
 * @brief OLED display service for desktop emulator
 * 
 * Provides a simple interface for displaying text on the emulated OLED.
 */

#ifndef OLED_SERVICE_H
#define OLED_SERVICE_H

#include <string>

// Set OLED display content (up to 3 lines)
void oled_set_lines(const char* line1, const char* line2, const char* line3);

// Set a single OLED line (0-2)
void oled_set_line(int lineNum, const char* text);

// Clear OLED display
void oled_clear();

// Present OLED if dirty (call from main loop)
void oled_present_if_dirty();

// Get current OLED lines
const char* oled_get_line(int lineNum);

#endif // OLED_SERVICE_H
