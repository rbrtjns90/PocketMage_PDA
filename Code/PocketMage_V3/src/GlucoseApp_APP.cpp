// PocketMage V3.0 - Glucose Tracker App
// Blood glucose logging and tracking
// @Ashtf 2025

#include <globals.h>
#include <vector>
#include <algorithm>

static constexpr const char* TAG = "GLUCOSE";

// ===================== DATA MODEL =====================
enum class GlucoseUnit : uint8_t {
  MgPerDl,
  MmolPerL
};

enum class ReadingTag : uint8_t {
  None = 0,
  Fasting = 1,
  PreMeal = 2,
  PostMeal = 3,
  Bedtime = 4,
  Exercise = 5,
  Correction = 6
};

struct GlucoseReading {
  int hour;
  int minute;
  int value;          // mg/dL (or mmol*10 for mmol/L)
  GlucoseUnit unit;
  ReadingTag tag;
  String note;
};

// ===================== APP STATE =====================
enum class Screen { Today, NewReading, History, Summary };
static Screen currentScreen = Screen::Today;
static volatile bool needsRedraw = true;

// Current day's readings
static std::vector<GlucoseReading> todayReadings;
static int scrollOffset = 0;
static int selectedIndex = 0;

// History view
static int historyDayOffset = 0;  // 0 = today, -1 = yesterday, etc.
static std::vector<GlucoseReading> historyReadings;

// Summary view
static int summaryDays = 7;  // 7, 14, or 30

// New reading input
static String inputValue = "";
static ReadingTag inputTag = ReadingTag::None;
static String inputNote = "";
static int inputStep = 0;  // 0=value, 1=tag, 2=note, 3=confirm

// Settings
static GlucoseUnit preferredUnit = GlucoseUnit::MgPerDl;
static int rangeMin = 70;   // Low threshold
static int rangeMax = 180;  // High threshold

// ===================== HELPER FUNCTIONS =====================
const char* getTagName(ReadingTag tag) {
  switch(tag) {
    case ReadingTag::Fasting: return "Fasting";
    case ReadingTag::PreMeal: return "Pre-meal";
    case ReadingTag::PostMeal: return "Post-meal";
    case ReadingTag::Bedtime: return "Bedtime";
    case ReadingTag::Exercise: return "Exercise";
    case ReadingTag::Correction: return "Correction";
    default: return "";
  }
}

const char* getTagShort(ReadingTag tag) {
  switch(tag) {
    case ReadingTag::Fasting: return "F";
    case ReadingTag::PreMeal: return "Pre";
    case ReadingTag::PostMeal: return "Pst";
    case ReadingTag::Bedtime: return "Bed";
    case ReadingTag::Exercise: return "Exr";
    case ReadingTag::Correction: return "Cor";
    default: return "";
  }
}

String formatTime(int hour, int minute) {
  char buf[8];
  snprintf(buf, sizeof(buf), "%02d:%02d", hour, minute);
  return String(buf);
}

String getDateString(int dayOffset) {
  // Get current date from RTC and apply offset
  DateTime now = CLOCK().nowDT();
  // Simple day offset (doesn't handle month boundaries perfectly)
  int day = now.day() + dayOffset;
  int month = now.month();
  int year = now.year();
  
  // Basic month boundary handling
  while (day < 1) {
    month--;
    if (month < 1) { month = 12; year--; }
    day += 30;  // Approximate
  }
  while (day > 28) {
    day -= 30;
    month++;
    if (month > 12) { month = 1; year++; }
  }
  
  char buf[16];
  snprintf(buf, sizeof(buf), "%04d-%02d-%02d", year, month, day < 1 ? 1 : day);
  return String(buf);
}

String getTodayFilename() {
  DateTime now = CLOCK().nowDT();
  char buf[32];
  snprintf(buf, sizeof(buf), "/glucose/%04d-%02d-%02d.csv", 
           now.year(), now.month(), now.day());
  return String(buf);
}

String getFilenameForOffset(int dayOffset) {
  DateTime now = CLOCK().nowDT();
  int day = now.day() + dayOffset;
  int month = now.month();
  int year = now.year();
  
  while (day < 1) {
    month--;
    if (month < 1) { month = 12; year--; }
    day += 30;
  }
  
  char buf[32];
  snprintf(buf, sizeof(buf), "/glucose/%04d-%02d-%02d.csv", year, month, day < 1 ? 1 : day);
  return String(buf);
}

// ===================== STORAGE =====================
void ensureGlucoseDir() {
  if (!SD_MMC.exists("/glucose")) {
    SD_MMC.mkdir("/glucose");
  }
}

void saveReading(const GlucoseReading& r) {
  ensureGlucoseDir();
  String filename = getTodayFilename();
  
  File file = SD_MMC.open(filename.c_str(), FILE_APPEND);
  if (file) {
    // CSV format: hour,minute,value,unit,tag,note
    char line[64];
    snprintf(line, sizeof(line), "%d,%d,%d,%d,%d,%s\n", 
             r.hour, r.minute, r.value, 
             (int)r.unit, (int)r.tag, r.note.c_str());
    file.print(line);
    file.close();
  }
}

void loadTodayReadings() {
  todayReadings.clear();
  String filename = getTodayFilename();
  
  File file = SD_MMC.open(filename.c_str(), FILE_READ);
  if (!file) return;
  
  while (file.available()) {
    String line = file.readStringUntil('\n');
    if (line.length() < 5) continue;
    
    // Parse CSV: hour,minute,value,unit,tag,note
    GlucoseReading r;
    int pos = 0;
    int field = 0;
    String token = "";
    
    for (size_t i = 0; i <= line.length(); i++) {
      char c = (i < line.length()) ? line.charAt(i) : ',';
      if (c == ',' || c == '\n' || c == '\r') {
        switch(field) {
          case 0: r.hour = token.toInt(); break;
          case 1: r.minute = token.toInt(); break;
          case 2: r.value = token.toInt(); break;
          case 3: r.unit = (GlucoseUnit)token.toInt(); break;
          case 4: r.tag = (ReadingTag)token.toInt(); break;
          case 5: r.note = token; break;
        }
        token = "";
        field++;
      } else {
        token += c;
      }
    }
    
    if (field >= 5) {
      todayReadings.push_back(r);
    }
  }
  file.close();
}

void loadHistoryReadings(int dayOffset) {
  historyReadings.clear();
  String filename = getFilenameForOffset(dayOffset);
  
  File file = SD_MMC.open(filename.c_str(), FILE_READ);
  if (!file) return;
  
  while (file.available()) {
    String line = file.readStringUntil('\n');
    if (line.length() < 5) continue;
    
    GlucoseReading r;
    int field = 0;
    String token = "";
    
    for (size_t i = 0; i <= line.length(); i++) {
      char c = (i < line.length()) ? line.charAt(i) : ',';
      if (c == ',' || c == '\n' || c == '\r') {
        switch(field) {
          case 0: r.hour = token.toInt(); break;
          case 1: r.minute = token.toInt(); break;
          case 2: r.value = token.toInt(); break;
          case 3: r.unit = (GlucoseUnit)token.toInt(); break;
          case 4: r.tag = (ReadingTag)token.toInt(); break;
          case 5: r.note = token; break;
        }
        token = "";
        field++;
      } else {
        token += c;
      }
    }
    
    if (field >= 5) {
      historyReadings.push_back(r);
    }
  }
  file.close();
}

// ===================== STATISTICS =====================
struct GlucoseStats {
  int count;
  int sum;
  int min;
  int max;
  int inRange;
  int low;
  int high;
};

GlucoseStats computeStats(const std::vector<GlucoseReading>& readings) {
  GlucoseStats s = {0, 0, 999, 0, 0, 0, 0};
  
  for (const auto& r : readings) {
    s.count++;
    s.sum += r.value;
    if (r.value < s.min) s.min = r.value;
    if (r.value > s.max) s.max = r.value;
    
    if (r.value < rangeMin) s.low++;
    else if (r.value > rangeMax) s.high++;
    else s.inRange++;
  }
  
  return s;
}

GlucoseStats computeMultiDayStats(int days) {
  GlucoseStats total = {0, 0, 999, 0, 0, 0, 0};
  
  for (int d = 0; d > -days; d--) {
    loadHistoryReadings(d);
    GlucoseStats dayStats = computeStats(historyReadings);
    
    total.count += dayStats.count;
    total.sum += dayStats.sum;
    if (dayStats.min < total.min) total.min = dayStats.min;
    if (dayStats.max > total.max) total.max = dayStats.max;
    total.inRange += dayStats.inRange;
    total.low += dayStats.low;
    total.high += dayStats.high;
  }
  
  return total;
}

// ===================== INPUT HANDLER =====================

// Forward declaration for app init
void appInit_GLUCOSEAPP();

// Auto-generated INIT function
void GLUCOSEAPP_INIT() {
    CurrentAppState = GLUCOSEAPP;
    newState = true;
    appInit_GLUCOSEAPP();  // Call app's init function
}

void processKB_GLUCOSEAPP() {
  if (OLEDPowerSave) {
    u8g2.setPowerSave(0);
    OLEDPowerSave = false;
  }
  
  char inchar = KB().updateKeypress();
  if (inchar == 0) return;
  
  // HOME key - go back or exit
  if (inchar == 12) {
    if (currentScreen == Screen::Today) {
      CurrentAppState = HOME; HOME_INIT(); return;
    } else if (currentScreen == Screen::NewReading) {
      // Cancel new reading
      inputValue = "";
      inputTag = ReadingTag::None;
      inputNote = "";
      inputStep = 0;
      currentScreen = Screen::Today;
      needsRedraw = true;
    } else {
      currentScreen = Screen::Today;
      loadTodayReadings();
      needsRedraw = true;
    }
    return;
  }
  
  switch(currentScreen) {
    case Screen::Today:
      // N = New reading
      if (inchar == 'n' || inchar == 'N') {
        currentScreen = Screen::NewReading;
        inputValue = "";
        inputTag = ReadingTag::None;
        inputNote = "";
        inputStep = 0;
        needsRedraw = true;
      }
      // H = History
      else if (inchar == 'h' || inchar == 'H') {
        currentScreen = Screen::History;
        historyDayOffset = 0;
        loadHistoryReadings(0);
        needsRedraw = true;
      }
      // S = Summary
      else if (inchar == 's' || inchar == 'S') {
        currentScreen = Screen::Summary;
        summaryDays = 7;
        needsRedraw = true;
      }
      // Up arrow - scroll up
      else if (inchar == 16 || inchar == 28) {
        if (selectedIndex > 0) {
          selectedIndex--;
          needsRedraw = true;
        }
      }
      // Down arrow - scroll down
      else if (inchar == 15 || inchar == 20) {
        if (selectedIndex < (int)todayReadings.size() - 1) {
          selectedIndex++;
          needsRedraw = true;
        }
      }
      break;
      
    case Screen::NewReading:
      if (inputStep == 0) {
        // Entering value
        if (inchar >= '0' && inchar <= '9') {
          if (inputValue.length() < 4) {
            inputValue += inchar;
            needsRedraw = true;
          }
        }
        else if (inchar == 8) {  // Backspace
          if (inputValue.length() > 0) {
            inputValue.remove(inputValue.length() - 1);
            needsRedraw = true;
          }
        }
        else if (inchar == 13) {  // Enter
          if (inputValue.length() > 0) {
            inputStep = 1;
            needsRedraw = true;
          }
        }
      }
      else if (inputStep == 1) {
        // Selecting tag
        if (inchar >= '0' && inchar <= '6') {
          inputTag = (ReadingTag)(inchar - '0');
          inputStep = 2;
          needsRedraw = true;
        }
        else if (inchar == 13) {  // Enter = skip tag
          inputStep = 2;
          needsRedraw = true;
        }
        // Quick tag shortcuts
        else if (inchar == 'f' || inchar == 'F') {
          inputTag = ReadingTag::Fasting;
          inputStep = 2;
          needsRedraw = true;
        }
        else if (inchar == 'p' || inchar == 'P') {
          inputTag = ReadingTag::PreMeal;
          inputStep = 2;
          needsRedraw = true;
        }
        else if (inchar == 'a' || inchar == 'A') {  // After meal
          inputTag = ReadingTag::PostMeal;
          inputStep = 2;
          needsRedraw = true;
        }
        else if (inchar == 'b' || inchar == 'B') {
          inputTag = ReadingTag::Bedtime;
          inputStep = 2;
          needsRedraw = true;
        }
        else if (inchar == 'e' || inchar == 'E') {
          inputTag = ReadingTag::Exercise;
          inputStep = 2;
          needsRedraw = true;
        }
      }
      else if (inputStep == 2) {
        // Entering note
        if (inchar == 13) {  // Enter = save
          // Save the reading
          DateTime now = CLOCK().nowDT();
          GlucoseReading r;
          r.hour = now.hour();
          r.minute = now.minute();
          r.value = inputValue.toInt();
          r.unit = preferredUnit;
          r.tag = inputTag;
          r.note = inputNote;
          
          saveReading(r);
          todayReadings.push_back(r);
          
          // Reset and go back
          inputValue = "";
          inputTag = ReadingTag::None;
          inputNote = "";
          inputStep = 0;
          currentScreen = Screen::Today;
          needsRedraw = true;
        }
        else if (inchar == 8) {  // Backspace
          if (inputNote.length() > 0) {
            inputNote.remove(inputNote.length() - 1);
            needsRedraw = true;
          }
        }
        else if (inputNote.length() < 20) {
          inputNote += inchar;
          needsRedraw = true;
        }
      }
      break;
      
    case Screen::History:
      // Left arrow - previous day
      if (inchar == 19) {
        historyDayOffset--;
        loadHistoryReadings(historyDayOffset);
        selectedIndex = 0;
        needsRedraw = true;
      }
      // Right arrow - next day
      else if (inchar == 21) {
        if (historyDayOffset < 0) {
          historyDayOffset++;
          loadHistoryReadings(historyDayOffset);
          selectedIndex = 0;
          needsRedraw = true;
        }
      }
      // Up/Down - scroll entries
      else if (inchar == 16 || inchar == 28) {
        if (selectedIndex > 0) {
          selectedIndex--;
          needsRedraw = true;
        }
      }
      else if (inchar == 15 || inchar == 20) {
        if (selectedIndex < (int)historyReadings.size() - 1) {
          selectedIndex++;
          needsRedraw = true;
        }
      }
      break;
      
    case Screen::Summary:
      // Number keys to change range
      if (inchar == '7') {
        summaryDays = 7;
        needsRedraw = true;
      }
      else if (inchar == '1') {
        summaryDays = 14;
        needsRedraw = true;
      }
      else if (inchar == '3') {
        summaryDays = 30;
        needsRedraw = true;
      }
      break;
  }
  
  // Update OLED status
  u8g2.clearBuffer();
  switch(currentScreen) {
    case Screen::Today:
      u8g2.drawStr(0, 12, "Glucose - Today");
      break;
    case Screen::NewReading:
      u8g2.drawStr(0, 12, ("New: " + inputValue + " mg/dL").c_str());
      break;
    case Screen::History:
      u8g2.drawStr(0, 12, ("History: " + getDateString(historyDayOffset)).c_str());
      break;
    case Screen::Summary:
      u8g2.drawStr(0, 12, (String(summaryDays) + "-day Summary").c_str());
      break;
  }
  u8g2.sendBuffer();
}

// ===================== E-INK DISPLAY =====================
void drawTodayScreen() {
  display.fillRect(0, 0, 320, 20, GxEPD_BLACK);
  display.setTextColor(GxEPD_WHITE);
  display.setCursor(5, 15);
  
  DateTime now = CLOCK().nowDT();
  char header[64];
  snprintf(header, sizeof(header), "Glucose Tracker | %02d:%02d", now.hour(), now.minute());
  display.print(header);
  
  display.setTextColor(GxEPD_BLACK);
  
  // Last reading summary
  if (!todayReadings.empty()) {
    const auto& last = todayReadings.back();
    display.setCursor(5, 38);
    char lastStr[48];
    snprintf(lastStr, sizeof(lastStr), "Last: %d mg/dL %s", last.value, getTagShort(last.tag));
    display.print(lastStr);
    
    // Range indicator
    if (last.value < rangeMin) {
      display.print(" [LOW]");
    } else if (last.value > rangeMax) {
      display.print(" [HIGH]");
    }
  }
  
  // Today's entries
  display.setCursor(5, 58);
  display.print("Today's Readings:");
  
  int y = 75;
  int maxVisible = 8;
  int start = std::max(0, (int)todayReadings.size() - maxVisible);
  
  for (size_t i = start; i < todayReadings.size() && y < 220; i++) {
    const auto& r = todayReadings[i];
    display.setCursor(10, y);
    
    char line[48];
    snprintf(line, sizeof(line), "%s  %3d  %-8s %s",
             formatTime(r.hour, r.minute).c_str(),
             r.value,
             getTagName(r.tag),
             r.note.c_str());
    
    if ((int)i == selectedIndex) {
      display.print("> ");
    } else {
      display.print("  ");
    }
    display.print(line);
    y += 18;
  }
  
  if (todayReadings.empty()) {
    display.setCursor(20, 100);
    display.print("No readings yet today");
  }
  
  // Controls
  display.setCursor(5, 230);
  display.print("N:New  H:History  S:Summary  HOME:Exit");
}

void drawNewReadingScreen() {
  display.fillRect(0, 0, 320, 20, GxEPD_BLACK);
  display.setTextColor(GxEPD_WHITE);
  display.setCursor(5, 15);
  display.print("New Reading");
  display.setTextColor(GxEPD_BLACK);
  
  // Step 1: Value
  display.setCursor(20, 50);
  if (inputStep == 0) {
    display.print("> ");
  } else {
    display.print("  ");
  }
  display.print("Value: ");
  display.print(inputValue.c_str());
  display.print(" mg/dL");
  if (inputStep == 0) {
    display.print("_");
  }
  
  // Step 2: Tag
  display.setCursor(20, 80);
  if (inputStep == 1) {
    display.print("> ");
  } else {
    display.print("  ");
  }
  display.print("Tag: ");
  display.print(getTagName(inputTag));
  
  if (inputStep == 1) {
    display.setCursor(30, 105);
    display.print("[0]None [1]Fast [2]Pre [3]Post");
    display.setCursor(30, 125);
    display.print("[4]Bed [5]Exer [6]Corr");
    display.setCursor(30, 145);
    display.print("Or: F P A B E keys");
  }
  
  // Step 3: Note
  display.setCursor(20, 170);
  if (inputStep == 2) {
    display.print("> ");
  } else {
    display.print("  ");
  }
  display.print("Note: ");
  display.print(inputNote.c_str());
  if (inputStep == 2) {
    display.print("_");
  }
  
  // Instructions
  display.setCursor(20, 210);
  if (inputStep == 0) {
    display.print("Type value, press ENTER");
  } else if (inputStep == 1) {
    display.print("Select tag or ENTER to skip");
  } else if (inputStep == 2) {
    display.print("Type note or ENTER to save");
  }
  
  display.setCursor(5, 230);
  display.print("HOME: Cancel");
}

void drawHistoryScreen() {
  display.fillRect(0, 0, 320, 20, GxEPD_BLACK);
  display.setTextColor(GxEPD_WHITE);
  display.setCursor(5, 15);
  display.print("History - ");
  display.print(getDateString(historyDayOffset).c_str());
  display.setTextColor(GxEPD_BLACK);
  
  // Day navigation
  display.setCursor(20, 40);
  display.print("<< LEFT  |  ");
  display.print(getDateString(historyDayOffset).c_str());
  display.print("  |  RIGHT >>");
  
  // Entries
  int y = 65;
  for (size_t i = 0; i < historyReadings.size() && y < 210; i++) {
    const auto& r = historyReadings[i];
    display.setCursor(10, y);
    
    char line[48];
    snprintf(line, sizeof(line), "%s  %3d  %-8s %s",
             formatTime(r.hour, r.minute).c_str(),
             r.value,
             getTagName(r.tag),
             r.note.c_str());
    
    if ((int)i == selectedIndex) {
      display.print("> ");
    } else {
      display.print("  ");
    }
    display.print(line);
    y += 18;
  }
  
  if (historyReadings.empty()) {
    display.setCursor(20, 100);
    display.print("No readings for this day");
  }
  
  display.setCursor(5, 230);
  display.print("LEFT/RIGHT: Day  UP/DOWN: Scroll  HOME: Back");
}

void drawSummaryScreen() {
  display.fillRect(0, 0, 320, 20, GxEPD_BLACK);
  display.setTextColor(GxEPD_WHITE);
  display.setCursor(5, 15);
  char title[32];
  snprintf(title, sizeof(title), "Summary - Last %d Days", summaryDays);
  display.print(title);
  display.setTextColor(GxEPD_BLACK);
  
  // Compute stats
  GlucoseStats stats = computeMultiDayStats(summaryDays);
  
  // Display stats
  display.setCursor(20, 50);
  display.print("Readings: ");
  display.print(stats.count);
  
  if (stats.count > 0) {
    int avg = stats.sum / stats.count;
    
    display.setCursor(20, 75);
    char avgStr[32];
    snprintf(avgStr, sizeof(avgStr), "Average: %d mg/dL", avg);
    display.print(avgStr);
    
    display.setCursor(20, 95);
    char rangeStr[48];
    snprintf(rangeStr, sizeof(rangeStr), "Range: %d - %d mg/dL", stats.min, stats.max);
    display.print(rangeStr);
    
    // Percentages
    int inRangePct = (stats.inRange * 100) / stats.count;
    int lowPct = (stats.low * 100) / stats.count;
    int highPct = (stats.high * 100) / stats.count;
    
    display.setCursor(20, 125);
    char pctStr[48];
    snprintf(pctStr, sizeof(pctStr), "In Range (%d-%d): %d%%", rangeMin, rangeMax, inRangePct);
    display.print(pctStr);
    
    display.setCursor(20, 145);
    snprintf(pctStr, sizeof(pctStr), "Low (<%d): %d%%", rangeMin, lowPct);
    display.print(pctStr);
    
    display.setCursor(20, 165);
    snprintf(pctStr, sizeof(pctStr), "High (>%d): %d%%", rangeMax, highPct);
    display.print(pctStr);
  } else {
    display.setCursor(20, 80);
    display.print("No data available");
  }
  
  // Range selector
  display.setCursor(20, 200);
  display.print("[7] 7 days  [1] 14 days  [3] 30 days");
  
  display.setCursor(5, 230);
  display.print("HOME: Back");
}

void einkHandler_GLUCOSEAPP() {
  if (!needsRedraw) return;
  needsRedraw = false;
  
  display.setRotation(3);
  display.fillScreen(GxEPD_WHITE);
  display.setFont(&FreeMono9pt7b);
  display.setTextColor(GxEPD_BLACK);
  
  switch(currentScreen) {
    case Screen::Today:
      drawTodayScreen();
      break;
    case Screen::NewReading:
      drawNewReadingScreen();
      break;
    case Screen::History:
      drawHistoryScreen();
      break;
    case Screen::Summary:
      drawSummaryScreen();
      break;
  }
  
  EINK().refresh();
}

// ===================== APP INIT =====================
void appInit_GLUCOSEAPP() {
  // Reset all state when app starts
  currentScreen = Screen::Today;
  needsRedraw = true;
  scrollOffset = 0;
  selectedIndex = 0;
  historyDayOffset = 0;
  summaryDays = 7;
  inputValue = "";
  inputTag = ReadingTag::None;
  inputNote = "";
  inputStep = 0;
  
  // Load today's readings
  loadTodayReadings();
}

/////////////////////////////////////////////////////////////
//  ooo        ooooo       .o.       ooooo ooooo      ooo  //
//  `88.       .888'      .888.      `888' `888b.     `8'  //
//   888b     d'888      .8"888.      888   8 `88b.    8   //
//   8 Y88. .P  888     .8' `888.     888   8   `88b.  8   //
//   8  `888'   888    .88ooo8888.    888   8     `88b.8   //
//   8    Y     888   .8'     `888.   888   8       `888   //
//  o8o        o888o o88o     o8888o o888o o8o        `8   //
/////////////////////////////////////////////////////////////
// SETUP

// Function removed - handled by main firmware


// Function removed - handled by main firmware


// migrated from einkFunc.cpp

// Function removed - handled by main firmware
