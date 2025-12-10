// PocketMage V3.0 - AstraLua
// Full Lua 5.4 interpreter for PocketMage
// @R Jones 2025

#include <pocketmage.h>
#include <string>
#include <vector>

extern "C" {
  #include "minilua.h"
}

static constexpr const char* TAG = "ASTRALUA";

// ===================== LUA STATE =====================
static lua_State* L = nullptr;

// Console state
static std::vector<std::string> consoleLines;
static std::string inputLine;
static int scrollOffset = 0;
static volatile bool needsRedraw = true;

// Display constants
static constexpr int MAX_CONSOLE_LINES = 200;
static constexpr int VISIBLE_LINES = 10;
static constexpr int MAX_LINE_WIDTH = 38;

// ===================== CONSOLE FUNCTIONS =====================
static void consolePrint(const std::string& line) {
  // Word wrap long lines
  if (line.length() <= MAX_LINE_WIDTH) {
    consoleLines.push_back(line);
  } else {
    size_t pos = 0;
    while (pos < line.length()) {
      consoleLines.push_back(line.substr(pos, MAX_LINE_WIDTH));
      pos += MAX_LINE_WIDTH;
    }
  }
  
  // Limit scrollback
  while (consoleLines.size() > MAX_CONSOLE_LINES) {
    consoleLines.erase(consoleLines.begin());
  }
  
  // Auto-scroll to bottom
  int total = (int)consoleLines.size();
  if (total > VISIBLE_LINES) {
    scrollOffset = total - VISIBLE_LINES;
  } else {
    scrollOffset = 0;
  }
  
  needsRedraw = true;
}

// Trim whitespace
static std::string trim(const std::string& s) {
  size_t start = s.find_first_not_of(" \t\r\n");
  if (start == std::string::npos) return "";
  size_t end = s.find_last_not_of(" \t\r\n");
  return s.substr(start, end - start + 1);
}

// ===================== LUA PRINT OVERRIDE =====================
// Custom print function that outputs to our console
static int lua_pm_print(lua_State* Ls) {
  int n = lua_gettop(Ls);
  std::string out;
  
  for (int i = 1; i <= n; i++) {
    size_t len;
    const char* s = luaL_tolstring(Ls, i, &len);
    if (s) {
      if (!out.empty()) out += "\t";
      out += s;
    }
    lua_pop(Ls, 1);
  }
  
  consolePrint(out);
  return 0;
}

// ===================== LUA INITIALIZATION =====================
static void initLua() {
  if (L) {
    lua_close(L);
    L = nullptr;
  }
  
  L = luaL_newstate();
  luaL_openlibs(L);
  
  // Override print to use our console
  lua_pushcfunction(L, lua_pm_print);
  lua_setglobal(L, "print");
  
  // Add PocketMage-specific functions here in the future
  // e.g., beep(), display functions, etc.
}

// ===================== COMMAND EXECUTION =====================
static void executeCommand(const std::string& cmd) {
  std::string line = trim(cmd);
  if (line.empty()) return;
  
  consolePrint("> " + line);
  
  // Built-in commands
  if (line == "help" || line == "?") {
    consolePrint("AstraLua 2.0 - Full Lua 5.4");
    consolePrint("");
    consolePrint("Commands:");
    consolePrint("  help    - Show this help");
    consolePrint("  clear   - Clear console");
    consolePrint("  files   - List .lua files");
    consolePrint("  run X   - Run file X.lua");
    consolePrint("  exit    - Return to OS");
    consolePrint("");
    consolePrint("This is real Lua! Try:");
    consolePrint("  print('Hello!')");
    consolePrint("  for i=1,5 do print(i) end");
    consolePrint("  t = {1,2,3}");
    consolePrint("  function f(x) return x*2 end");
    return;
  }
  
  if (line == "clear" || line == "cls") {
    consoleLines.clear();
    scrollOffset = 0;
    consolePrint("AstraLua 2.0 - Lua 5.4");
    return;
  }
  
  if (line == "exit" || line == "quit") {
    if (L) {
      lua_close(L);
      L = nullptr;
    }
    rebootToPocketMage();
    return;
  }
  
  // List files command
  if (line == "files" || line == "ls" || line == "dir") {
    File dir = SD_MMC.open("/lua");
    if (!dir || !dir.isDirectory()) {
      SD_MMC.mkdir("/lua");
      consolePrint("No files in /lua/");
      return;
    }
    
    consolePrint("Files in /lua/:");
    int count = 0;
    File entry = dir.openNextFile();
    while (entry) {
      String entryName = entry.name();
      std::string name(entryName.c_str());
      if (name.find(".lua") != std::string::npos) {
        size_t lastSlash = name.rfind('/');
        if (lastSlash != std::string::npos) {
          name = name.substr(lastSlash + 1);
        }
        consolePrint("  " + name);
        count++;
      }
      entry = dir.openNextFile();
    }
    dir.close();
    
    if (count == 0) {
      consolePrint("  (no .lua files)");
    }
    return;
  }
  
  // Run file command
  if (line.substr(0, 4) == "run " || line.substr(0, 5) == "load ") {
    std::string filename = trim(line.substr(line.find(' ') + 1));
    
    if (filename.find(".lua") == std::string::npos) {
      filename += ".lua";
    }
    
    std::string path = "/lua/" + filename;
    
    File file = SD_MMC.open(path.c_str(), FILE_READ);
    if (!file) {
      consolePrint("[error] File not found: " + filename);
      return;
    }
    
    // Read entire file
    std::string code;
    while (file.available()) {
      code += (char)file.read();
    }
    file.close();
    
    consolePrint("Running: " + filename);
    
    // Execute with Lua
    int status = luaL_dostring(L, code.c_str());
    if (status != LUA_OK) {
      const char* err = lua_tostring(L, -1);
      consolePrint(std::string("[error] ") + (err ? err : "unknown"));
      lua_pop(L, 1);
    }
    return;
  }
  
  // Execute as Lua code
  // First try as expression (to print result)
  std::string exprCode = "return " + line;
  int status = luaL_loadstring(L, exprCode.c_str());
  
  if (status == LUA_OK) {
    status = lua_pcall(L, 0, LUA_MULTRET, 0);
    if (status == LUA_OK) {
      int nresults = lua_gettop(L);
      if (nresults > 0) {
        std::string result;
        for (int i = 1; i <= nresults; i++) {
          if (i > 1) result += "\t";
          const char* s = luaL_tolstring(L, i, nullptr);
          if (s) result += s;
          lua_pop(L, 1);
        }
        if (!result.empty()) {
          consolePrint(result);
        }
      }
      lua_settop(L, 0);
      return;
    }
  }
  
  // Clear stack and try as statement
  lua_settop(L, 0);
  status = luaL_dostring(L, line.c_str());
  
  if (status != LUA_OK) {
    const char* err = lua_tostring(L, -1);
    consolePrint(std::string("[error] ") + (err ? err : "syntax error"));
    lua_pop(L, 1);
  }
}

// ===================== APP INIT =====================
void appInit() {
  consoleLines.clear();
  inputLine.clear();
  scrollOffset = 0;
  needsRedraw = true;
  
  initLua();
  
  consolePrint("AstraLua 2.0 - Lua 5.4.7");
  consolePrint("Type 'help' for commands");
  consolePrint("");
}

// ===================== INPUT HANDLER =====================
void processKB() {
  if (OLEDPowerSave) {
    u8g2.setPowerSave(0);
    OLEDPowerSave = false;
  }
  
  char inchar = KB().updateKeypress();
  if (inchar == 0) return;
  
  // HOME key - exit to OS
  if (inchar == 12) {
    if (L) {
      lua_close(L);
      L = nullptr;
    }
    rebootToPocketMage();
    return;
  }
  
  // Enter - execute command
  if (inchar == 13) {
    executeCommand(inputLine);
    inputLine.clear();
    needsRedraw = true;
    return;
  }
  
  // Backspace
  if (inchar == 8 || inchar == 127) {
    if (!inputLine.empty()) {
      inputLine.pop_back();
      // Update OLED to show change
      u8g2.clearBuffer();
      u8g2.drawStr(0, 12, "AstraLua 2.0");
      std::string oledInput = "> " + inputLine;
      if (oledInput.length() > 21) {
        oledInput = oledInput.substr(oledInput.length() - 21);
      }
      u8g2.drawStr(0, 24, oledInput.c_str());
      u8g2.sendBuffer();
    }
    return;
  }
  
  // Up arrow - scroll up
  if (inchar == 16 || inchar == 28) {
    if (scrollOffset > 0) {
      scrollOffset--;
      needsRedraw = true;
    }
    return;
  }
  
  // Down arrow - scroll down
  if (inchar == 15 || inchar == 20) {
    int maxScroll = (int)consoleLines.size() - VISIBLE_LINES;
    if (maxScroll < 0) maxScroll = 0;
    if (scrollOffset < maxScroll) {
      scrollOffset++;
      needsRedraw = true;
    }
    return;
  }
  
  // Printable character
  if (inchar >= 32 && inchar <= 126) {
    if (inputLine.length() < 100) {
      inputLine += inchar;
    }
  }
  
  // Update OLED with current input
  u8g2.clearBuffer();
  u8g2.drawStr(0, 12, "AstraLua 2.0");
  
  std::string oledInput = "> " + inputLine;
  if (oledInput.length() > 21) {
    oledInput = oledInput.substr(oledInput.length() - 21);
  }
  u8g2.drawStr(0, 24, oledInput.c_str());
  u8g2.sendBuffer();
}

// ===================== E-INK DISPLAY =====================
void applicationEinkHandler() {
  if (!needsRedraw) return;
  needsRedraw = false;
  
  display.setRotation(3);
  display.fillScreen(GxEPD_WHITE);
  display.setFont(&FreeMono9pt7b);
  display.setTextColor(GxEPD_BLACK);
  
  // Header
  display.fillRect(0, 0, 320, 20, GxEPD_BLACK);
  display.setTextColor(GxEPD_WHITE);
  display.setCursor(5, 15);
  display.print("AstraLua 2.0 - Lua 5.4.7");
  display.setTextColor(GxEPD_BLACK);
  
  // Console output
  int y = 40;
  int total = (int)consoleLines.size();
  int start = scrollOffset;
  int end = start + VISIBLE_LINES;
  if (end > total) end = total;
  
  for (int i = start; i < end; i++) {
    display.setCursor(5, y);
    display.print(consoleLines[i].c_str());
    y += 18;
  }
  
  // Scroll indicator
  if (total > VISIBLE_LINES) {
    int barHeight = 160;
    int thumbHeight = (VISIBLE_LINES * barHeight) / total;
    if (thumbHeight < 10) thumbHeight = 10;
    int thumbY = 30 + (scrollOffset * (barHeight - thumbHeight)) / (total - VISIBLE_LINES);
    display.drawRect(310, 30, 8, barHeight, GxEPD_BLACK);
    display.fillRect(311, thumbY, 6, thumbHeight, GxEPD_BLACK);
  }
  
  // Input line at bottom
  display.drawLine(0, 205, 320, 205, GxEPD_BLACK);
  display.setCursor(5, 225);
  std::string prompt = "> " + inputLine + "_";
  if (prompt.length() > MAX_LINE_WIDTH) {
    prompt = prompt.substr(prompt.length() - MAX_LINE_WIDTH);
  }
  display.print(prompt.c_str());
  
  EINK().refresh();
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
void setup() {
  PocketMage_INIT();
}

void loop() {
  pocketmage::power::updateBattState();
  processKB();
  vTaskDelay(50 / portTICK_PERIOD_MS);
  yield();
}

void einkHandler(void* parameter) {
  vTaskDelay(pdMS_TO_TICKS(250)); 
  for (;;) {
    applicationEinkHandler();
    vTaskDelay(pdMS_TO_TICKS(50));
    yield();
  }
}
