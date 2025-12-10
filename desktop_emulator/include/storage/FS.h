/**
 * @file FS.h
 * @brief ESP32 FS (File System) library mock for desktop emulator
 * 
 * This provides the FS interface that PocketMage expects.
 * The actual implementation is in SD_MMC.h
 */

#ifndef FS_H
#define FS_H

// Forward declaration - SD_MMC.h includes FS.h

// FS is already defined in SD_MMC.h via the fs namespace
// This header just ensures compatibility with #include <FS.h>

#endif // FS_H
