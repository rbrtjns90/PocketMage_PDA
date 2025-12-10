// PocketMage V3.0 - AstraLua
// Simple Lua-like interpreter / REPL for PocketMage
// @R Jones2025

#include <pocketmage.h>
#include <string>
#include <vector>
#include <map>
#include <cmath>
#include <cstdlib>

static constexpr const char* TAG = "ASTRALUA";

// ===================== INTERPRETER STATE =====================
// Simple expression evaluator with variables
static std::map<std::string, double> variables;
static std::map<std::string, std::string> stringVars;

// Console state
static std::vector<std::string> consoleLines;
static std::string inputLine;
static int scrollOffset = 0;
static volatile bool needsRedraw = true;

// Display constants
static constexpr int MAX_CONSOLE_LINES = 200;
static constexpr int VISIBLE_LINES = 10;
static constexpr int MAX_LINE_WIDTH = 38;

// ===================== HELPER FUNCTIONS =====================
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

// Check if string is a number
static bool isNumber(const std::string& s) {
  if (s.empty()) return false;
  size_t i = 0;
  if (s[0] == '-' || s[0] == '+') i++;
  bool hasDigit = false;
  bool hasDot = false;
  for (; i < s.length(); i++) {
    if (s[i] == '.') {
      if (hasDot) return false;
      hasDot = true;
    } else if (isdigit(s[i])) {
      hasDigit = true;
    } else {
      return false;
    }
  }
  return hasDigit;
}

// Check if valid identifier
static bool isIdentifier(const std::string& s) {
  if (s.empty()) return false;
  if (!isalpha(s[0]) && s[0] != '_') return false;
  for (size_t i = 1; i < s.length(); i++) {
    if (!isalnum(s[i]) && s[i] != '_') return false;
  }
  return true;
}

// ===================== EXPRESSION EVALUATOR =====================
// Simple recursive descent parser for math expressions

static size_t exprPos;
static std::string exprStr;

static double parseExpression();
static double parseTerm();
static double parseFactor();
static double parsePrimary();

static char peek() {
  while (exprPos < exprStr.length() && exprStr[exprPos] == ' ') exprPos++;
  if (exprPos >= exprStr.length()) return '\0';
  return exprStr[exprPos];
}

static char get() {
  char c = peek();
  if (c != '\0') exprPos++;
  return c;
}

static double parsePrimary() {
  char c = peek();
  
  // Parentheses
  if (c == '(') {
    get(); // consume '('
    double val = parseExpression();
    if (peek() == ')') get();
    return val;
  }
  
  // Unary minus
  if (c == '-') {
    get();
    return -parsePrimary();
  }
  
  // Number or identifier
  std::string token;
  while (exprPos < exprStr.length()) {
    c = exprStr[exprPos];
    if (isalnum(c) || c == '.' || c == '_') {
      token += c;
      exprPos++;
    } else {
      break;
    }
  }
  
  if (token.empty()) return 0;
  
  // Check for built-in constants
  if (token == "pi" || token == "PI") return 3.14159265358979;
  if (token == "e" || token == "E") return 2.71828182845905;
  
  // Check for built-in functions
  if (token == "sin" || token == "cos" || token == "tan" ||
      token == "sqrt" || token == "abs" || token == "log" ||
      token == "floor" || token == "ceil") {
    if (peek() == '(') {
      get(); // consume '('
      double arg = parseExpression();
      if (peek() == ')') get();
      
      if (token == "sin") return sin(arg);
      if (token == "cos") return cos(arg);
      if (token == "tan") return tan(arg);
      if (token == "sqrt") return sqrt(arg);
      if (token == "abs") return fabs(arg);
      if (token == "log") return log(arg);
      if (token == "floor") return floor(arg);
      if (token == "ceil") return ceil(arg);
    }
  }
  
  // Check for variable
  if (isIdentifier(token)) {
    auto it = variables.find(token);
    if (it != variables.end()) {
      return it->second;
    }
    return 0; // undefined variable = 0
  }
  
  // Must be a number
  return atof(token.c_str());
}

static double parseFactor() {
  double left = parsePrimary();
  
  while (peek() == '^') {
    get();
    double right = parsePrimary();
    left = pow(left, right);
  }
  
  return left;
}

static double parseTerm() {
  double left = parseFactor();
  
  while (true) {
    char op = peek();
    if (op == '*') {
      get();
      left *= parseFactor();
    } else if (op == '/') {
      get();
      double right = parseFactor();
      if (right != 0) left /= right;
      else { consolePrint("[error] Division by zero"); return 0; }
    } else if (op == '%') {
      get();
      double right = parseFactor();
      if (right != 0) left = fmod(left, right);
      else { consolePrint("[error] Modulo by zero"); return 0; }
    } else {
      break;
    }
  }
  
  return left;
}

static double parseExpression() {
  double left = parseTerm();
  
  while (true) {
    char op = peek();
    if (op == '+') {
      get();
      left += parseTerm();
    } else if (op == '-') {
      get();
      left -= parseTerm();
    } else {
      break;
    }
  }
  
  return left;
}

static double evaluate(const std::string& expr) {
  exprStr = expr;
  exprPos = 0;
  return parseExpression();
}

// ===================== COMMAND INTERPRETER =====================
static void executeCommand(const std::string& cmd) {
  std::string line = trim(cmd);
  if (line.empty()) return;
  
  consolePrint("> " + line);
  
  // Help command
  if (line == "help" || line == "?") {
    consolePrint("AstraLua 1.0 - Commands:");
    consolePrint("  help     - Show this help");
    consolePrint("  files    - List .lua files");
    consolePrint("  run NAME - Run a .lua file");
    consolePrint("  vars     - List variables");
    consolePrint("  clear    - Clear console");
    consolePrint("  exit     - Return to OS");
    consolePrint("");
    consolePrint("Math: +, -, *, /, ^, %");
    consolePrint("Funcs: sin,cos,tan,sqrt,");
    consolePrint("       abs,log,floor,ceil");
    consolePrint("Consts: pi, e");
    return;
  }
  
  // List files command
  if (line == "files" || line == "ls" || line == "dir") {
    File dir = SD_MMC.open("/lua");
    if (!dir || !dir.isDirectory()) {
      SD_MMC.mkdir("/lua");
      consolePrint("No files in /lua/");
      consolePrint("Add .lua files to run");
      return;
    }
    
    consolePrint("Files in /lua/:");
    int count = 0;
    File entry = dir.openNextFile();
    while (entry) {
      String entryName = entry.name();
      std::string name(entryName.c_str());
      if (name.find(".lua") != std::string::npos) {
        // Extract just filename
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
    
    // Add .lua extension if not present
    if (filename.find(".lua") == std::string::npos) {
      filename += ".lua";
    }
    
    // Build full path
    std::string path = "/lua/" + filename;
    
    File file = SD_MMC.open(path.c_str(), FILE_READ);
    if (!file) {
      consolePrint("[error] File not found: " + filename);
      consolePrint("Use 'files' to list available");
      return;
    }
    
    consolePrint("Running: " + filename);
    consolePrint("---");
    
    // Read and execute each line
    while (file.available()) {
      String fileLine = file.readStringUntil('\n');
      const char* linePtr = fileLine.c_str();
      std::string stdLine(linePtr);
      stdLine = trim(stdLine);
      
      // Skip empty lines and comments
      if (stdLine.empty()) continue;
      if (stdLine[0] == '-' && stdLine.length() > 1 && stdLine[1] == '-') {
        continue;  // Skip -- comments
      }
      
      // Execute the line (don't print the > prompt for file lines)
      // We'll call a helper that doesn't echo
      std::string execLine = stdLine;
      
      // Handle the line directly without recursion issues
      // Check for print
      if (execLine.substr(0, 6) == "print(" && execLine.back() == ')') {
        std::string arg = execLine.substr(6, execLine.length() - 7);
        arg = trim(arg);
        if (arg.length() >= 2 && arg[0] == '"' && arg.back() == '"') {
          consolePrint(arg.substr(1, arg.length() - 2));
        } else {
          double val = evaluate(arg);
          char buf[64];
          if (val == floor(val) && fabs(val) < 1e9) {
            snprintf(buf, sizeof(buf), "%.0f", val);
          } else {
            snprintf(buf, sizeof(buf), "%.6g", val);
          }
          consolePrint(buf);
        }
        continue;
      }
      
      // Handle assignment
      size_t eqP = execLine.find('=');
      if (eqP != std::string::npos && eqP > 0 && 
          (eqP + 1 >= execLine.length() || execLine[eqP + 1] != '=')) {
        std::string varName = trim(execLine.substr(0, eqP));
        std::string expr = trim(execLine.substr(eqP + 1));
        if (isIdentifier(varName)) {
          if (expr.length() >= 2 && expr[0] == '"' && expr.back() == '"') {
            stringVars[varName] = expr.substr(1, expr.length() - 2);
          } else {
            variables[varName] = evaluate(expr);
          }
          continue;
        }
      }
      
      // Handle for loop
      if (execLine.substr(0, 4) == "for ") {
        size_t eqPos = execLine.find('=');
        size_t commaPos = execLine.find(',');
        size_t doPos = execLine.find(" do ");
        size_t endPos = execLine.rfind(" end");
        
        if (eqPos != std::string::npos && commaPos != std::string::npos && 
            doPos != std::string::npos && endPos != std::string::npos) {
          std::string vn = trim(execLine.substr(4, eqPos - 4));
          int sv = (int)evaluate(trim(execLine.substr(eqPos + 1, commaPos - eqPos - 1)));
          int ev = (int)evaluate(trim(execLine.substr(commaPos + 1, doPos - commaPos - 1)));
          std::string body = trim(execLine.substr(doPos + 4, endPos - doPos - 4));
          
          for (int i = sv; i <= ev; i++) {
            variables[vn] = i;
            // Execute body (simple version - just handle print)
            if (body.substr(0, 6) == "print(" && body.back() == ')') {
              std::string arg = body.substr(6, body.length() - 7);
              double val = evaluate(trim(arg));
              char buf[64];
              if (val == floor(val) && fabs(val) < 1e9) {
                snprintf(buf, sizeof(buf), "%.0f", val);
              } else {
                snprintf(buf, sizeof(buf), "%.6g", val);
              }
              consolePrint(buf);
            }
          }
        }
        continue;
      }
      
      // Plain expression - just evaluate (don't print result for file execution)
      evaluate(execLine);
    }
    
    file.close();
    consolePrint("---");
    consolePrint("Done.");
    return;
  }
  
  // Clear command
  if (line == "clear" || line == "cls") {
    consoleLines.clear();
    scrollOffset = 0;
    consolePrint("AstraLua 1.0");
    return;
  }
  
  // Exit command
  if (line == "exit" || line == "quit") {
    rebootToPocketMage();
    return;
  }
  
  // List variables
  if (line == "vars" || line == "variables") {
    if (variables.empty() && stringVars.empty()) {
      consolePrint("No variables defined");
    } else {
      for (const auto& v : variables) {
        char buf[64];
        snprintf(buf, sizeof(buf), "  %s = %.6g", v.first.c_str(), v.second);
        consolePrint(buf);
      }
      for (const auto& v : stringVars) {
        consolePrint("  " + v.first + " = \"" + v.second + "\"");
      }
    }
    return;
  }
  
  // Print command
  if (line.substr(0, 6) == "print(" && line.back() == ')') {
    std::string arg = line.substr(6, line.length() - 7);
    arg = trim(arg);
    
    // String literal
    if (arg.length() >= 2 && arg[0] == '"' && arg.back() == '"') {
      consolePrint(arg.substr(1, arg.length() - 2));
    } else {
      // Expression
      double val = evaluate(arg);
      char buf[64];
      if (val == floor(val) && fabs(val) < 1e9) {
        snprintf(buf, sizeof(buf), "%.0f", val);
      } else {
        snprintf(buf, sizeof(buf), "%.6g", val);
      }
      consolePrint(buf);
    }
    return;
  }
  
  // Assignment: var = expr
  size_t eqPos = line.find('=');
  if (eqPos != std::string::npos && eqPos > 0) {
    // Make sure it's not == 
    if (eqPos + 1 < line.length() && line[eqPos + 1] == '=') {
      // Comparison - evaluate as expression
    } else {
      std::string varName = trim(line.substr(0, eqPos));
      std::string expr = trim(line.substr(eqPos + 1));
      
      if (isIdentifier(varName)) {
        // Check for string assignment
        if (expr.length() >= 2 && expr[0] == '"' && expr.back() == '"') {
          stringVars[varName] = expr.substr(1, expr.length() - 2);
          consolePrint(varName + " = \"" + stringVars[varName] + "\"");
        } else {
          double val = evaluate(expr);
          variables[varName] = val;
          char buf[64];
          if (val == floor(val) && fabs(val) < 1e9) {
            snprintf(buf, sizeof(buf), "%s = %.0f", varName.c_str(), val);
          } else {
            snprintf(buf, sizeof(buf), "%s = %.6g", varName.c_str(), val);
          }
          consolePrint(buf);
        }
        return;
      }
    }
  }
  
  // For loop: for i=1,5 do print(i) end
  if (line.substr(0, 4) == "for ") {
    // Simple for loop parsing
    size_t eqP = line.find('=');
    size_t commaP = line.find(',');
    size_t doP = line.find(" do ");
    size_t endP = line.rfind(" end");
    
    if (eqP != std::string::npos && commaP != std::string::npos && 
        doP != std::string::npos && endP != std::string::npos) {
      std::string varName = trim(line.substr(4, eqP - 4));
      std::string startStr = trim(line.substr(eqP + 1, commaP - eqP - 1));
      std::string endStr = trim(line.substr(commaP + 1, doP - commaP - 1));
      std::string body = trim(line.substr(doP + 4, endP - doP - 4));
      
      int startVal = (int)evaluate(startStr);
      int endVal = (int)evaluate(endStr);
      
      for (int i = startVal; i <= endVal; i++) {
        variables[varName] = i;
        executeCommand(body);
      }
      return;
    }
    consolePrint("[error] Invalid for syntax");
    consolePrint("Use: for i=1,5 do cmd end");
    return;
  }
  
  // If statement: if expr then cmd end
  if (line.substr(0, 3) == "if ") {
    size_t thenP = line.find(" then ");
    size_t endP = line.rfind(" end");
    
    if (thenP != std::string::npos && endP != std::string::npos) {
      std::string cond = trim(line.substr(3, thenP - 3));
      std::string body = trim(line.substr(thenP + 6, endP - thenP - 6));
      
      double val = evaluate(cond);
      if (val != 0) {
        executeCommand(body);
      }
      return;
    }
    consolePrint("[error] Invalid if syntax");
    consolePrint("Use: if expr then cmd end");
    return;
  }
  
  // Plain expression - evaluate and print result
  double result = evaluate(line);
  char buf[64];
  if (result == floor(result) && fabs(result) < 1e9) {
    snprintf(buf, sizeof(buf), "= %.0f", result);
  } else {
    snprintf(buf, sizeof(buf), "= %.6g", result);
  }
  consolePrint(buf);
}

// ===================== APP INIT =====================
void appInit() {
  consoleLines.clear();
  inputLine.clear();
  scrollOffset = 0;
  variables.clear();
  stringVars.clear();
  needsRedraw = true;
  
  consolePrint("AstraLua 1.0");
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
      // Don't redraw e-ink for typing, just OLED
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
      // Don't redraw e-ink for typing, just OLED
    }
  }
  
  // Update OLED with current input
  u8g2.clearBuffer();
  u8g2.drawStr(0, 12, "AstraLua");
  
  // Show input line on OLED (truncate if too long)
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
  display.print("AstraLua 1.0 - Lua-like REPL");
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
  // Check battery
  pocketmage::power::updateBattState();
  
  // Run KB loop
  processKB();

  // Yield to watchdog
  vTaskDelay(50 / portTICK_PERIOD_MS);
  yield();
}

// migrated from einkFunc.cpp
void einkHandler(void* parameter) {
  vTaskDelay(pdMS_TO_TICKS(250)); 
  for (;;) {
    applicationEinkHandler();

    vTaskDelay(pdMS_TO_TICKS(50));
    yield();
  }
}