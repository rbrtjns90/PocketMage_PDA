/**
 * @file pgmspace.h
 * @brief AVR pgmspace compatibility for desktop emulator
 * 
 * On AVR, PROGMEM stores data in flash memory. On desktop, we just use regular memory.
 */

#ifndef PGMSPACE_H
#define PGMSPACE_H

#include <cstdint>
#include <cstring>

// PROGMEM is not needed on desktop
#ifndef PROGMEM
#define PROGMEM
#endif

// pgm_read functions just read from memory directly
#define pgm_read_byte(addr)   (*(const uint8_t *)(addr))
#define pgm_read_word(addr)   (*(const uint16_t *)(addr))
#define pgm_read_dword(addr)  (*(const uint32_t *)(addr))
#define pgm_read_float(addr)  (*(const float *)(addr))
#define pgm_read_ptr(addr)    (*(const void **)(addr))

// String functions
#define strcpy_P(dest, src)   strcpy(dest, src)
#define strncpy_P(dest, src, n) strncpy(dest, src, n)
#define strcmp_P(s1, s2)      strcmp(s1, s2)
#define strncmp_P(s1, s2, n)  strncmp(s1, s2, n)
#define strlen_P(s)           strlen(s)
#define strcat_P(dest, src)   strcat(dest, src)
#define strncat_P(dest, src, n) strncat(dest, src, n)

// Memory functions
#define memcpy_P(dest, src, n) memcpy(dest, src, n)
#define memcmp_P(s1, s2, n)   memcmp(s1, s2, n)

// Printf functions
#define sprintf_P(s, fmt, ...) sprintf(s, fmt, ##__VA_ARGS__)
#define snprintf_P(s, n, fmt, ...) snprintf(s, n, fmt, ##__VA_ARGS__)

// PSTR just returns the string
#ifndef PSTR
#define PSTR(s) (s)
#endif

#endif // PGMSPACE_H
