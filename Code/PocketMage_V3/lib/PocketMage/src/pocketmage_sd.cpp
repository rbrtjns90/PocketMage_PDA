//  .d88888b  888888ba   //
//  88.    "' 88    `8b  //
//  `Y88888b. 88     88  //
//        `8b 88     88  //
//  d8'   .8P 88    .8P  //
//   Y88888P  8888888P   //

#include <pocketmage.h>
#include <config.h> // for FULL_REFRESH_AFTER
#include <SD_MMC.h>

static constexpr const char* TAG = "SD";

extern bool SAVE_POWER;

// Initialization of sd class
static PocketmageSD pm_sd;

// Helpers
static int countVisibleChars(String input) {
  int count = 0;

  for (size_t i = 0; i < input.length(); i++) {
    char c = input[i];
    // Check if the character is a visible character or space
    if (c >= 32 && c <= 126) {  // ASCII range for printable characters and space
      count++;
    }
  }

  return count;
}

// Setup for SD Class
// @ dependencies:
//   - setupOled()
//   - setupBZ()
//   - setupEINK()
void setupSD() {
  SD_MMC.setPins(SD_CLK, SD_CMD, SD_D0);
  if (!SD_MMC.begin("/sdcard", true) || SD_MMC.cardType() == CARD_NONE) {
    ESP_LOGE(TAG, "MOUNT FAILED");

    OLED().oledWord("SD Card Not Detected!");
    delay(2000);
    if (ALLOW_NO_MICROSD) {
      OLED().oledWord("All Work Will Be Lost!");
      delay(5000);
      SD().setNoSD(true);
    }
    else {
      OLED().oledWord("Insert SD Card and Reboot!");
      delay(5000);
      // Put OLED to sleep
      OLED().setPowerSave(1);
      // Shut Down Jingle
      BZ().playJingle(Jingles::Shutdown);
      // Sleep
      esp_deep_sleep_start();
      return;
    }
  }

  pocketmage::setCpuSpeed(240);
  // Create folders and files if needed
  if (!SD_MMC.exists("/sys"))                 SD_MMC.mkdir( "/sys"                );
  if (!SD_MMC.exists("/notes"))               SD_MMC.mkdir( "/notes"              );
  if (!SD_MMC.exists("/journal"))             SD_MMC.mkdir( "/journal"            );
  if (!SD_MMC.exists("/dict"))                SD_MMC.mkdir( "/dict"               );
  if (!SD_MMC.exists("/apps"))                SD_MMC.mkdir( "/apps"               );
  if (!SD_MMC.exists("/apps/temp"))           SD_MMC.mkdir( "/apps/temp"          );
  if (!SD_MMC.exists("/notes"))               SD_MMC.mkdir( "/notes"              );
  if (!SD_MMC.exists("/assets"))              SD_MMC.mkdir( "/assets"             );
  if (!SD_MMC.exists("/assets/backgrounds"))  SD_MMC.mkdir( "/assets/backgrounds" );

  if (!SD_MMC.exists("/assets/backgrounds/HOWTOADDBACKGROUNDS.txt")) {
    File f = SD_MMC.open("/assets/backgrounds/HOWTOADDBACKGROUNDS.txt", FILE_WRITE);
    if (f) {
      f.print("How to add custom backgrounds:\n1. Make a background that is 1 bit (black OR white) and 320x240 pixels.\n2. Export your background as a .bmp file.\n3. Use image2cpp to convert your image to a .bin file. Use the settings: Invert Image Colors (TRUE), Swap Bits in Byte (FALSE). Select the \"Download as Binary File (.bin)\" button.\n4. Place the .bin file in this folder.\n5. Enjoy your new custom wallpapers!");
      f.close();
    }
  }
  
  if (!SD_MMC.exists("/sys/events.txt")) {
    File f = SD_MMC.open("/sys/events.txt", FILE_WRITE);
    if (f) f.close();
  }
  if (!SD_MMC.exists("/sys/tasks.txt")) {
    File f = SD_MMC.open("/sys/tasks.txt", FILE_WRITE);
    if (f) f.close();
  }
  if (!SD_MMC.exists("/sys/SDMMC_META.txt")) {
    File f = SD_MMC.open("/sys/SDMMC_META.txt", FILE_WRITE);
    if (f) f.close();
  }
}

// Access for other apps
PocketmageSD& SD() { return pm_sd; }

    
void PocketmageSD::saveFile() {
  if (SD().getNoSD()) {
      OLED().oledWord("SAVE FAILED - No SD!");
      delay(5000);
      return;
  } else {
      SDActive = true;
      pocketmage::setCpuSpeed(240);
      delay(50);

      String textToSave = vectorToString();
      ESP_LOGV(TAG, "Text to save: %s", textToSave.c_str());

      if (SD().getEditingFile() == "" || SD().getEditingFile() == "-")
      SD().setEditingFile("/temp.txt");
      keypad.disableInterrupts();
      if (!SD().getEditingFile().startsWith("/"))
      SD().setEditingFile("/" + SD().getEditingFile());
      //OLED().oledWord("Saving File: "+ editingFile);
      SD().writeFile(SD_MMC, (SD().getEditingFile()).c_str(), textToSave.c_str());
      //OLED().oledWord("Saved: "+ editingFile);

      // Write MetaData
      SD().writeMetadata(SD().getEditingFile());

      // delay(1000);
      keypad.enableInterrupts();
      if (SAVE_POWER)
      pocketmage::setCpuSpeed(POWER_SAVE_FREQ);
      SDActive = false;
  }
}
  
void PocketmageSD::writeMetadata(const String& path) {
  SDActive = true;
  pocketmage::setCpuSpeed(240);
  delay(50);

  File file = SD_MMC.open(path);
  if (!file || file.isDirectory()) {
      OLED().oledWord("META WRITE ERR");
      delay(1000);
      ESP_LOGE(TAG, "Invalid file for metadata: %s", path);
      return;
  }
  // Get file size
  size_t fileSizeBytes = file.size();
  file.close();

  // Format size string
  String fileSizeStr = String(fileSizeBytes) + " Bytes";

  // Get line and char counts
  int charCount = countVisibleChars(SD().readFileToString(SD_MMC, path.c_str()));

  String charStr = String(charCount) + " Char";
  // Get current time from RTC
  DateTime now = CLOCK().nowDT();
  char timestamp[20];
  sprintf(timestamp, "%04d%02d%02d-%02d%02d", now.year(), now.month(), now.day(), now.hour(),
          now.minute());

  // Compose new metadata line
  String newEntry = path + "|" + timestamp + "|" + fileSizeStr + "|" + charStr;

  const char* metaPath = SYS_METADATA_FILE;
  // Read existing entries and rebuild the file without duplicates
  File metaFile = SD_MMC.open(metaPath, FILE_READ);
  String updatedMeta = "";
  bool replaced = false;

  if (metaFile) {
      while (metaFile.available()) {
      String line = metaFile.readStringUntil('\n');
      if (line.startsWith(path + "|")) {
          updatedMeta += newEntry + "\n";
          replaced = true;
      } else if (line.length() > 1) {
          updatedMeta += line + "\n";
      }
      }
      metaFile.close();
  }

  if (!replaced) {
      updatedMeta += newEntry + "\n";
  }
  // Write back the updated metadata
  metaFile = SD_MMC.open(metaPath, FILE_WRITE);
  if (!metaFile) {
      ESP_LOGE(TAG, "Failed to open metadata file for writing: %s", metaPath);
      return;
  }
  metaFile.print(updatedMeta);
  metaFile.close();
  ESP_LOGI(TAG, "Metadata updated");

  if (SAVE_POWER)
  pocketmage::setCpuSpeed(POWER_SAVE_FREQ);
  SDActive = false;
}
  
void PocketmageSD::loadFile(bool showOLED) {
  SDActive = true;
  pocketmage::setCpuSpeed(240);
  delay(50);

  if (SD().getNoSD()) {
      OLED().oledWord("LOAD FAILED - No SD!");
      delay(5000);
      return;
  } else {
      SDActive = true;
      pocketmage::setCpuSpeed(240);
      delay(50);

      keypad.disableInterrupts();
      if (showOLED)
      OLED().oledWord("Loading File");
      if (!SD().getEditingFile().startsWith("/"))
      SD().setEditingFile("/" + SD().getEditingFile());
      String textToLoad = SD().readFileToString(SD_MMC, (SD().getEditingFile()).c_str());
      ESP_LOGV(TAG, "Text to load: %s", textToLoad.c_str());

      stringToVector(textToLoad);
      keypad.enableInterrupts();
      if (showOLED) {
      OLED().oledWord("File Loaded");
      delay(200);
      }
      if (SAVE_POWER)
      pocketmage::setCpuSpeed(POWER_SAVE_FREQ);
      SDActive = false;
  }
}
  
void PocketmageSD::delFile(String fileName) {
  if (SD().getNoSD()) {
      OLED().oledWord("DELETE FAILED - No SD!");
      delay(5000);
      return;
  } else {
      SDActive = true;
      pocketmage::setCpuSpeed(240);
      delay(50);

      keypad.disableInterrupts();
      // OLED().oledWord("Deleting File: "+ fileName);
      if (!fileName.startsWith("/"))
      fileName = "/" + fileName;
      SD().deleteFile(SD_MMC, fileName.c_str());
      // OLED().oledWord("Deleted: "+ fileName);

      // Delete MetaData
      SD().deleteMetadata(fileName);

      delay(1000);
      keypad.enableInterrupts();
      if (SAVE_POWER)
      pocketmage::setCpuSpeed(POWER_SAVE_FREQ);
      SDActive = false;
  }
}
  
void PocketmageSD::deleteMetadata(String path) {
  SDActive = true;
  pocketmage::setCpuSpeed(240);
  delay(50);

  const char* metaPath = SYS_METADATA_FILE;

  // Open metadata file for reading
  File metaFile = SD_MMC.open(metaPath, FILE_READ);
  if (!metaFile) {
      ESP_LOGE(TAG, "Metadata file not found: %s", metaPath);
      return;
  }

  // Store lines that don't match the given path
  std::vector<String> keptLines;
  while (metaFile.available()) {
      String line = metaFile.readStringUntil('\n');
      if (!line.startsWith(path + "|")) {
      keptLines.push_back(line);
      }
  }
  metaFile.close();

  // Delete the original metadata file
  SD_MMC.remove(metaPath);

  // Recreate the file and write back the kept lines
  File writeFile = SD_MMC.open(metaPath, FILE_WRITE);
  if (!writeFile) {
      ESP_LOGE(TAG, "Failed to recreate metadata file. %s", writeFile.path());
      return;
  }

  for (const String& line : keptLines) {
      writeFile.println(line);
  }

  writeFile.close();
  ESP_LOGI(TAG, "Metadata entry deleted (if it existed).");
  }
  
void PocketmageSD::renFile(String oldFile, String newFile) {
  if (SD().getNoSD()) {
      OLED().oledWord("RENAME FAILED - No SD!");
      delay(5000);
      return;
  } else {
      SDActive = true;
      pocketmage::setCpuSpeed(240);
      delay(50);

      keypad.disableInterrupts();
      // OLED().oledWord("Renaming "+ oldFile + " to " + newFile);
      if (!oldFile.startsWith("/"))
      oldFile = "/" + oldFile;
      if (!newFile.startsWith("/"))
      newFile = "/" + newFile;
      SD().renameFile(SD_MMC, oldFile.c_str(), newFile.c_str());
      OLED().oledWord(oldFile + " -> " + newFile);
      delay(1000);

      // Update MetaData
      SD().renMetadata(oldFile, newFile);

      keypad.enableInterrupts();
      if (SAVE_POWER)
      pocketmage::setCpuSpeed(POWER_SAVE_FREQ);
      SDActive = false;
  }
}
  
void PocketmageSD::renMetadata(String oldPath, String newPath) {
  SDActive = true;
  pocketmage::setCpuSpeed(240);
  delay(50);
  const char* metaPath = SYS_METADATA_FILE;

  // Open metadata file for reading
  File metaFile = SD_MMC.open(metaPath, FILE_READ);
  if (!metaFile) {
      ESP_LOGE(TAG, "Metadata file not found: %s", metaPath);
      return;
  }

  std::vector<String> updatedLines;

  while (metaFile.available()) {
      String line = metaFile.readStringUntil('\n');
      if (line.startsWith(oldPath + "|")) {
      // Replace old path with new path at the start of the line
      int separatorIndex = line.indexOf('|');
      if (separatorIndex != -1) {
          // Keep rest of line after '|'
          String rest = line.substring(separatorIndex);
          line = newPath + rest;
      } else {
          // Just replace whole line with new path if malformed
          line = newPath;
      }
      }
      updatedLines.push_back(line);
  }

  metaFile.close();

  // Delete old metadata file
  SD_MMC.remove(metaPath);

  // Recreate file and write updated lines
  File writeFile = SD_MMC.open(metaPath, FILE_WRITE);
  if (!writeFile) {
      ESP_LOGE(TAG, "Failed to recreate metadata file. %s", writeFile.path());
      return;
  }

  for (const String& l : updatedLines) {
      writeFile.println(l);
  }

  writeFile.close();
  ESP_LOGI(TAG, "Metadata updated for renamed file.");

  if (SAVE_POWER)
      pocketmage::setCpuSpeed(POWER_SAVE_FREQ);
  }
  
  void PocketmageSD::copyFile(String oldFile, String newFile) {
  if (SD().getNoSD()) {
      OLED().oledWord("COPY FAILED - No SD!");
      delay(5000);
      return;
  } else {
      SDActive = true;
      pocketmage::setCpuSpeed(240);
      delay(50);

      keypad.disableInterrupts();
      OLED().oledWord("Loading File");
      if (!oldFile.startsWith("/"))
      oldFile = "/" + oldFile;
      if (!newFile.startsWith("/"))
      newFile = "/" + newFile;
      String textToLoad = SD().readFileToString(SD_MMC, (oldFile).c_str());
      SD().writeFile(SD_MMC, (newFile).c_str(), textToLoad.c_str());
      OLED().oledWord("Saved: " + newFile);

      // Write MetaData
      SD().writeMetadata(newFile);

      delay(1000);
      keypad.enableInterrupts();

      if (SAVE_POWER)
      pocketmage::setCpuSpeed(POWER_SAVE_FREQ);
      SDActive = false;
  }
}
  
void PocketmageSD::appendToFile(String path, String inText) {
  if (SD().getNoSD()) {
      OLED().oledWord("OP FAILED - No SD!");
      delay(5000);
      return;
  } else {
      SDActive = true;
      pocketmage::setCpuSpeed(240);
      delay(50);

      keypad.disableInterrupts();
      SD().appendFile(SD_MMC, path.c_str(), inText.c_str());

      // Write MetaData
      SD().writeMetadata(path);

      keypad.enableInterrupts();

      if (SAVE_POWER)
      pocketmage::setCpuSpeed(POWER_SAVE_FREQ);
      SDActive = false;
  }
}

// ===================== low level functions =====================
// Low-Level SDMMC Operations switch to using internal fs::FS*
void PocketmageSD::listDir(fs::FS &fs, const char *dirname) {
  if (noSD_) {
    OLED().oledWord("OP FAILED - No SD!");
    delay(5000);
    return;
  }
  else {
    pocketmage::setCpuSpeed(240);
    delay(50);
    noTimeout = true;
    ESP_LOGI(tag, "Listing directory %s\r\n", dirname);

    File root = fs.open(dirname);
    if (!root) {
      noTimeout = false;
      ESP_LOGE(tag, "Failed to open directory: %s", root.path());
      return;
    }
    if (!root.isDirectory()) {
      noTimeout = false;
      ESP_LOGE(tag, "Not a directory: %s", root.path());
      
      return;
    }

    // Reset fileIndex and initialize filesList with "-"
    fileIndex_ = 0; // Reset fileIndex
    for (int i = 0; i < MAX_FILES; i++) {
      filesList_[i] = "-";
    }

    File file = root.openNextFile();
    while (file && fileIndex_ < MAX_FILES) {
      if (!file.isDirectory()) {
        String fileName = String(file.name());
        
        // Check if file is in the exclusion list
        bool excluded = false;
        for (const String &excludedFile : excludedFiles_) {
          if (fileName.equals(excludedFile) || ("/"+fileName).equals(excludedFile)) {
            excluded = true;
            break;
          }
        }

        if (!excluded) {
          filesList_[fileIndex_++] = fileName; // Store file name if not excluded
        }
      }
      file = root.openNextFile();
    }

    // for (int i = 0; i < fileIndex_; i++) { // Only print valid entries
    //   Serial.println(filesList_[i]);       // NOTE: This prints out valid files
    // }

    noTimeout = false;
    if (SAVE_POWER) pocketmage::setCpuSpeed(POWER_SAVE_FREQ);
  }
}
void PocketmageSD::readFile(fs::FS &fs, const char *path) {
  if (noSD_) {
    OLED().oledWord("OP FAILED - No SD!");
    delay(5000);
    return;
  }
  else {
    pocketmage::setCpuSpeed(240);
    delay(50);
    noTimeout = true;
    ESP_LOGI(tag, "Reading file %s\r\n", path);

    File file = fs.open(path);
    if (!file || file.isDirectory()) {
      noTimeout = false;
      ESP_LOGE(tag, "Failed to open file for reading: %s", file.path());
      return;
    }

    file.close();
    noTimeout = false;
    if (SAVE_POWER) pocketmage::setCpuSpeed(POWER_SAVE_FREQ);
  }
}
String PocketmageSD::readFileToString(fs::FS &fs, const char *path) {
  if (noSD_) {
    OLED().oledWord("OP FAILED - No SD!");
    delay(5000);
    return "";
  }
  else { 
    pocketmage::setCpuSpeed(240);
    delay(50);

    noTimeout = true;
    ESP_LOGI(tag, "Reading file: %s\r\n", path);

    File file = fs.open(path);
    if (!file || file.isDirectory()) {
      noTimeout = false;
      ESP_LOGE(tag, "Failed to open file for reading: %s", path);
      OLED().oledWord("Load Failed");
      delay(500);
      return "";  // Return an empty string on failure
    }

    ESP_LOGI(tag, "Reading from file: %s", file.path());
    String content = file.readString();

    file.close();
    EINK().setFullRefreshAfter(FULL_REFRESH_AFTER); //Force a full refresh
    noTimeout = false;
    return content;  // Return the complete String
  }
}
void PocketmageSD::writeFile(fs::FS &fs, const char *path, const char *message) {
  if (noSD_) {
    OLED().oledWord("OP FAILED - No SD!");
    delay(5000);
    return;
  }
  else {
    pocketmage::setCpuSpeed(240);
    delay(50);
    noTimeout = true;
    ESP_LOGI(tag, "Writing file: %s\r\n", path);
    delay(200);

    File file = fs.open(path, FILE_WRITE);
    if (!file) {
      noTimeout = false;
      ESP_LOGE(tag, "Failed to open %s for writing", path);
      return;
    }
    if (file.print(message)) {
      ESP_LOGV(tag, "File written %s", path);
    } 
    else {
      ESP_LOGE(tag, "Write failed for %s", path);
    }
    file.close();
    noTimeout = false;
    if (SAVE_POWER) pocketmage::setCpuSpeed(POWER_SAVE_FREQ);
  }
}
void PocketmageSD::appendFile(fs::FS &fs, const char *path, const char *message) {
  if (noSD_) {
    OLED().oledWord("OP FAILED - No SD!");
    delay(5000);
    return;
  }
  else {
    pocketmage::setCpuSpeed(240);
    delay(50);
    noTimeout = true;
    ESP_LOGI(tag, "Appending to file: %s\r\n", path);

    File file = fs.open(path, FILE_APPEND);
    if (!file) {
      noTimeout = false;
      ESP_LOGE(tag, "Failed to open for appending: %s", path);
      return;
    }
    if (file.println(message)) {
      ESP_LOGV(tag, "Message appended to %s", path);
    } 
    else {
      ESP_LOGE(tag, "Append failed: %s", path);
    }
    file.close();
    noTimeout = false;
    if (SAVE_POWER) pocketmage::setCpuSpeed(POWER_SAVE_FREQ);
  }
}
void PocketmageSD::renameFile(fs::FS &fs, const char *path1, const char *path2) {
  if (noSD_) {
    OLED().oledWord("OP FAILED - No SD!");
    delay(5000);
    return;
  }
  else {
    pocketmage::setCpuSpeed(240);
    delay(50);
    noTimeout = true;
    ESP_LOGI(tag, "Renaming file %s to %s\r\n", path1, path2);

    if (fs.rename(path1, path2)) {
      ESP_LOGV(tag, "Renamed %s to %s\r\n", path1, path2);
    } 
    else {
      ESP_LOGE(tag, "Rename failed: %s to %s", path1, path2);
    }
    noTimeout = false;
    if (SAVE_POWER) pocketmage::setCpuSpeed(POWER_SAVE_FREQ);
  }
}
void PocketmageSD::deleteFile(fs::FS &fs, const char *path) {
  if (noSD_) {
    OLED().oledWord("OP FAILED - No SD!");
    delay(5000);
    return;
  }
  else {
    pocketmage::setCpuSpeed(240);
    delay(50);
    noTimeout = true;
    ESP_LOGI(tag, "Deleting file: %s\r\n", path);
    if (fs.remove(path)) {
      ESP_LOGV(tag, "File deleted: %s", path);
    } 
    else {
      ESP_LOGE(tag, "Delete failed for %s", path);
    }
    noTimeout = false;
    if (SAVE_POWER) pocketmage::setCpuSpeed(POWER_SAVE_FREQ);
  }
}

