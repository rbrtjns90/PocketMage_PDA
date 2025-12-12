//  .d88888b  888888ba   //
//  88.    "' 88    `8b  //
//  `Y88888b. 88     88  //
//        `8b 88     88  //
//  d8'   .8P 88    .8P  //
//   Y88888P  8888888P   //


#pragma once
#include <Arduino.h>
#include <FS.h>

// forward-declaration to avoid including U8g2lib.h, GxEPD2_BW.h, pocketmage_oled.h, and pocketmage_eink.h
class PocketmageOled;
class PocketmageEink;

// ===================== SD CLASS =====================
class PocketmageSD {
public:
  explicit PocketmageSD() {}

  void saveFile();
  void writeMetadata(const String& path);
  void loadFile(bool showOLED = true);
  void delFile(String fileName);
  void deleteMetadata(String path);
  void renFile(String oldFile, String newFile);
  void renMetadata(String oldPath, String newPath);
  void copyFile(String oldFile, String newFile);
  void appendToFile(String path, String inText);

  // Getters / Setters
  bool getNoSD()  {return noSD_;}
  void setNoSD(bool in) {noSD_ = in;}

  String getWorkingFile()  {return workingFile_;}
  void setWorkingFile(String in) {workingFile_ = in;}

  String getEditingFile()  {return editingFile_;}
  void setEditingFile(String in) {editingFile_ = in;}

  String getFilesListIndex(int index) {return filesList_[index];}
  void setFilesListIndex(int index, String content) {filesList_[index] = content;}

  // low level methods  To Do: remove arguments for fs::FS &fs and reference internal fs::FS* instead
  void listDir(fs::FS &fs, const char *dirname);
  void readFile(fs::FS &fs, const char *path);
  String readFileToString(fs::FS &fs, const char *path);
  void writeFile(fs::FS &fs, const char *path, const char *message);
  void appendFile(fs::FS &fs, const char *path, const char *message);
  void renameFile(fs::FS &fs, const char *path1, const char *path2);
  void deleteFile(fs::FS &fs, const char *path);
  
private:

  static constexpr const char*  tag               = "MAGE_SD";

  String editingFile_ = "";
  String filesList_[MAX_FILES];
  String workingFile_ = "";

  uint8_t                       fileIndex_        = 0;
  String                        excludedFiles_[3] = { "/temp.txt", "/settings.txt", "/tasks.txt" };

  // Flags / counters
  bool                          noSD_              = false;
};

void setupSD();
PocketmageSD& SD();
