# Desktop Emulator Header Files

This directory contains stub/mock implementations of ESP32 and Arduino libraries
for the desktop emulator. Headers are organized by theme below.

## Display Libraries

| File | Description |
|------|-------------|
| `Adafruit_GFX.h` | Graphics primitives (drawPixel, drawLine, drawBitmap, etc.) |
| `GxEPD2_BW.h` | E-ink display driver (black/white) |
| `U8g2lib.h` | OLED display driver |
| `desktop_display_sdl2.h` | SDL2 display backend for emulator |
| `oled_service.h` | OLED rendering service |

## Input Libraries

| File | Description |
|------|-------------|
| `Adafruit_TCA8418.h` | Keyboard matrix controller |
| `Adafruit_MPR121.h` | Capacitive touch sensor |
| `hid_host.h` | USB HID host support |
| `hid_usage_keyboard.h` | HID keyboard usage codes |
| `hid_usage_mouse.h` | HID mouse usage codes |

## Storage Libraries

| File | Description |
|------|-------------|
| `FS.h` | Filesystem base class |
| `SD_MMC.h` | SD card (MMC mode) filesystem |
| `Preferences.h` | NVS key-value storage (JSON file in emulator) |
| `sdmmc_cmd.h` | SD/MMC command definitions |

## ESP32 Platform

| File | Description |
|------|-------------|
| `Arduino.h` | Arduino core compatibility |
| `esp_log.h` | ESP-IDF logging macros |
| `esp32-hal-log.h` | ESP32 HAL logging |
| `esp_cpu.h` | CPU control functions |
| `esp_ota_ops.h` | OTA update operations |
| `pgmspace.h` | Program memory macros (PROGMEM) |
| `Update.h` | Firmware update support |

## Hardware Peripherals

| File | Description |
|------|-------------|
| `SPI.h` | SPI bus interface |
| `Wire.h` | I2C (Wire) bus interface |
| `RTClib.h` | Real-time clock (DS3231) |
| `Buzzer.h` | Piezo buzzer control |
| `MP2722.h` | Power management IC |
| `USB.h` | USB device support |
| `USBMSC.h` | USB mass storage class |
| `ESP32-targz.h` | Tar/gzip extraction |

## PocketMage Library

| File | Description |
|------|-------------|
| `pocketmage_compat.h` | Core compatibility layer (String, millis, etc.) |
| `pocketmage_stubs.h` | Singleton accessors (EINK, OLED, KB, SD, etc.) |
| `pocketmage_eink.h` | E-ink display wrapper |
| `pocketmage_oled.h` | OLED display wrapper |
| `pocketmage_kb.h` | Keyboard input wrapper |
| `pocketmage_sd.h` | SD card wrapper |
| `pocketmage_clock.h` | RTC clock wrapper |
| `pocketmage_touch.h` | Touch sensor wrapper |
| `pocketmage_bz.h` | Buzzer wrapper |
| `pocketmage_sys.h` | System functions |

## Subdirectories

| Directory | Contents |
|-----------|----------|
| `Fonts/` | Symlink to `../fonts/gfx` (Adafruit GFX fonts) |
| `freertos/` | FreeRTOS stubs (FreeRTOS.h, task.h) |
| `driver/` | ESP-IDF driver stubs |
| `usb/` | USB library stubs |

## Notes

- All headers provide mock/stub implementations for desktop compatibility
- The emulator uses SDL2 for display rendering instead of actual hardware
- Preferences are stored as JSON files in `./data/sys/prefs_*.txt`
- SD card files are read from `./data/` directory
