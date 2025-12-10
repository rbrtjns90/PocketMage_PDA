// PocketMage V3.0 - Terminal App
// @Ashtf 2025

#include <globals.h>
#include <vector>

// SSH support
#ifdef ESP_PLATFORM
#include "libssh_esp32.h"
#include <libssh/libssh.h>
#else
#include <libssh/libssh.h>
#endif

static constexpr const char* TAG = "TERMINAL";

// ===================== TERMINAL STATE =====================
static std::vector<String> scrollback;      // Output history
static String currentLine = "";             // Line being typed
static std::vector<String> cmdHistory;      // Command history
static int historyIndex = -1;               // Current position in history
static int scrollOffset = 0;                // Scroll position in scrollback
static volatile bool needsRedraw = true;    // E-ink needs refresh
static constexpr int MAX_SCROLLBACK = 128;  // Max lines to keep

// Display constants (will be calculated based on font)
static constexpr int TERM_EINK_WIDTH = 320;
static constexpr int TERM_EINK_HEIGHT = 240;
static constexpr int LINE_HEIGHT = 16;      // Pixels per line
static constexpr int MAX_VISIBLE_LINES = 13; // Lines visible on e-ink
static constexpr int CHAR_WIDTH = 8;        // Approximate char width
static constexpr int MAX_CHARS_PER_LINE = 38; // Max chars per line

// Current working directory
static String currentDir = "/";

// ===================== SSH STATE =====================
static ssh_session g_ssh_session = nullptr;
static ssh_channel g_ssh_channel = nullptr;
static bool g_ssh_connected = false;
static bool g_waiting_for_password = false;
static String g_ssh_hostname = "";
static String g_ssh_username = "";

// ===================== HELPER FUNCTIONS =====================

// Split a string into words
std::vector<String> splitWords(const String& s) {
  std::vector<String> result;
  String current = "";
  for (size_t i = 0; i < s.length(); i++) {
    char c = s.charAt(i);
    if (c == ' ') {
      if (current.length() > 0) {
        result.push_back(current);
        current = "";
      }
    } else {
      current += c;
    }
  }
  if (current.length() > 0) result.push_back(current);
  return result;
}

// Append a line to scrollback
void appendLine(const String& text) {
  // Word wrap long lines
  if (text.length() > MAX_CHARS_PER_LINE) {
    String remaining = text;
    while (remaining.length() > MAX_CHARS_PER_LINE) {
      scrollback.push_back(remaining.substring(0, MAX_CHARS_PER_LINE));
      remaining = remaining.substring(MAX_CHARS_PER_LINE);
    }
    if (remaining.length() > 0) {
      scrollback.push_back(remaining);
    }
  } else {
    scrollback.push_back(text);
  }
  
  // Trim if too long
  while (scrollback.size() > MAX_SCROLLBACK) {
    scrollback.erase(scrollback.begin());
  }
  
  // Auto-scroll to bottom
  if (scrollback.size() > MAX_VISIBLE_LINES) {
    scrollOffset = scrollback.size() - MAX_VISIBLE_LINES;
  }
  
  needsRedraw = true;
}

// ===================== BUILT-IN COMMANDS =====================

void cmd_help() {
  appendLine("Available commands:");
  appendLine("  help     - Show this help");
  appendLine("  clear    - Clear the screen");
  appendLine("  echo ... - Print text");
  appendLine("  time     - Show current time");
  appendLine("  date     - Show current date");
  appendLine("  ls       - List directory");
  appendLine("  cd <dir> - Change directory");
  appendLine("  pwd      - Print working dir");
  appendLine("  cat <f>  - Show file contents");
  appendLine("  batt     - Battery status");
  appendLine("  info     - System info");
  appendLine("  ssh <host> - Connect via SSH");
  appendLine("  disconnect - Close SSH");
  appendLine("  exit     - Return to PocketMage");
}

void cmd_clear() {
  scrollback.clear();
  scrollOffset = 0;
  needsRedraw = true;
}

void cmd_echo(const std::vector<String>& args) {
  String msg = "";
  for (size_t i = 1; i < args.size(); i++) {
    if (i > 1) msg += " ";
    msg += args[i];
  }
  appendLine(msg);
}

void cmd_time() {
  DateTime now = CLOCK().nowDT();
  char buf[32];
  snprintf(buf, sizeof(buf), "%02d:%02d:%02d", now.hour(), now.minute(), now.second());
  appendLine(String("Time: ") + buf);
}

void cmd_date() {
  DateTime now = CLOCK().nowDT();
  char buf[32];
  snprintf(buf, sizeof(buf), "%04d-%02d-%02d", now.year(), now.month(), now.day());
  appendLine(String("Date: ") + buf);
  const char* days[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
  appendLine(String("Day: ") + days[now.dayOfTheWeek()]);
}

void cmd_ls() {
  File dir = SD_MMC.open(currentDir.c_str());
  if (!dir) {
    appendLine("Error: Cannot open " + currentDir);
    return;
  }
  
  appendLine("Contents of " + currentDir + ":");
  File entry = dir.openNextFile();
  while (entry) {
    String name = entry.name();
    // Extract just the filename from full path
    int lastSlash = name.lastIndexOf('/');
    if (lastSlash >= 0) {
      name = name.substring(lastSlash + 1);
    }
    
    if (entry.isDirectory()) {
      appendLine("  [" + name + "]");
    } else {
      appendLine("  " + name);
    }
    entry = dir.openNextFile();
  }
  dir.close();
}

void cmd_cat(const std::vector<String>& args) {
  if (args.size() < 2) {
    appendLine("Usage: cat <filename>");
    return;
  }
  
  String filename = args[1];
  if (!filename.startsWith("/")) {
    // Relative path - prepend current directory
    if (currentDir.endsWith("/")) {
      filename = currentDir + filename;
    } else {
      filename = currentDir + "/" + filename;
    }
  }
  
  File file = SD_MMC.open(filename.c_str(), FILE_READ);
  if (!file) {
    appendLine("Error: Cannot open " + filename);
    return;
  }
  
  appendLine("--- " + filename + " ---");
  while (file.available()) {
    String line = file.readStringUntil('\n');
    line.trim();
    appendLine(line);
  }
  file.close();
  appendLine("--- EOF ---");
}

void cmd_pwd() {
  appendLine("Current directory: " + currentDir);
}

void cmd_cd(const std::vector<String>& args) {
  if (args.size() < 2) {
    // No argument - go to root
    currentDir = "/";
    appendLine("Changed to /");
    return;
  }
  
  String target = args[1];
  String newDir;
  
  // Handle special cases
  if (target == "/") {
    newDir = "/";
  }
  else if (target == "..") {
    // Go up one directory
    if (currentDir == "/") {
      appendLine("Already at root");
      return;
    }
    // Remove trailing slash if present
    String temp = currentDir;
    if (temp.endsWith("/") && temp.length() > 1) {
      temp = temp.substring(0, temp.length() - 1);
    }
    // Find last slash
    int lastSlash = temp.lastIndexOf('/');
    if (lastSlash <= 0) {
      newDir = "/";
    } else {
      newDir = temp.substring(0, lastSlash);
      if (newDir.length() == 0) newDir = "/";
    }
  }
  else if (target.startsWith("/")) {
    // Absolute path
    newDir = target;
  }
  else {
    // Relative path
    if (currentDir.endsWith("/")) {
      newDir = currentDir + target;
    } else {
      newDir = currentDir + "/" + target;
    }
  }
  
  // Ensure path ends without trailing slash (except root)
  if (newDir.length() > 1 && newDir.endsWith("/")) {
    newDir = newDir.substring(0, newDir.length() - 1);
  }
  
  // Verify directory exists
  File dir = SD_MMC.open(newDir.c_str());
  if (!dir) {
    appendLine("Error: Directory not found: " + newDir);
    return;
  }
  if (!dir.isDirectory()) {
    appendLine("Error: Not a directory: " + newDir);
    dir.close();
    return;
  }
  dir.close();
  
  currentDir = newDir;
  appendLine("Changed to " + currentDir);
}

void cmd_batt() {
#ifdef ESP_PLATFORM
  pocketmage::power::updateBattState();
  appendLine(String("Battery: ") + String(battState) + "%");
#else
  appendLine("Battery: N/A (emulator)");
#endif
}

void cmd_info() {
  appendLine("PocketMage Terminal v1.0");
#ifdef ESP_PLATFORM
  appendLine(String("CPU Freq: ") + String(getCpuFrequencyMhz()) + " MHz");
  appendLine(String("Free Heap: ") + String(ESP.getFreeHeap()) + " bytes");
  appendLine(String("Chip Model: ") + String(ESP.getChipModel()));
#else
  appendLine("Running on Desktop Emulator");
#endif
  if (g_ssh_connected) {
    appendLine(String("SSH: Connected to ") + g_ssh_hostname);
  }
}

// ===================== SSH FUNCTIONS =====================

void ssh_close_session() {
  if (g_ssh_channel) {
    ssh_channel_send_eof(g_ssh_channel);
    ssh_channel_close(g_ssh_channel);
    ssh_channel_free(g_ssh_channel);
    g_ssh_channel = nullptr;
  }
  if (g_ssh_session) {
    ssh_disconnect(g_ssh_session);
    ssh_free(g_ssh_session);
    g_ssh_session = nullptr;
  }
  g_ssh_connected = false;
  g_waiting_for_password = false;
  appendLine("--- SSH Session Closed ---");
}

void ssh_read_output() {
  if (!g_ssh_connected || g_ssh_channel == nullptr) return;
  
  char buffer[256];
  int nbytes = ssh_channel_read_nonblocking(g_ssh_channel, buffer, sizeof(buffer) - 1, 0);
  
  if (nbytes > 0) {
    buffer[nbytes] = '\0';
    String output = String(buffer);
    int start = 0;
    int end = output.indexOf('\n');
    
    while (end >= 0) {
      String line = output.substring(start, end);
      line.trim();
      if (line.length() > 0) appendLine(line);
      start = end + 1;
      end = output.indexOf('\n', start);
    }
    if (start < (int)output.length()) {
      String partial = output.substring(start);
      partial.trim();
      if (partial.length() > 0) appendLine(partial);
    }
  }
  
  if (ssh_channel_is_eof(g_ssh_channel)) {
    ssh_close_session();
  }
}

void ssh_send_command(const String& command) {
  if (!g_ssh_connected || g_ssh_channel == nullptr) {
    appendLine("Error: Not connected");
    return;
  }
  String cmd = command + "\n";
  ssh_channel_write(g_ssh_channel, cmd.c_str(), cmd.length());
}

void ssh_open_shell() {
  g_ssh_channel = ssh_channel_new(g_ssh_session);
  if (!g_ssh_channel) {
    appendLine("Error: Failed to create channel");
    return;
  }
  
  if (ssh_channel_open_session(g_ssh_channel) != SSH_OK) {
    appendLine(String("Error: ") + ssh_get_error(g_ssh_session));
    ssh_channel_free(g_ssh_channel);
    g_ssh_channel = nullptr;
    return;
  }
  
  ssh_channel_request_pty(g_ssh_channel);
  
  if (ssh_channel_request_shell(g_ssh_channel) != SSH_OK) {
    appendLine("Error starting shell");
    return;
  }
  
  appendLine("--- SSH Session Active ---");
  appendLine("Type 'exit' to disconnect");
}

void handle_password_input(const String& password) {
  g_waiting_for_password = false;
  
  if (ssh_userauth_password(g_ssh_session, nullptr, password.c_str()) == SSH_AUTH_SUCCESS) {
    appendLine("Authentication successful");
    g_ssh_connected = true;
    ssh_open_shell();
  } else {
    appendLine("Authentication failed");
    ssh_disconnect(g_ssh_session);
    ssh_free(g_ssh_session);
    g_ssh_session = nullptr;
  }
}

void cmd_ssh(const std::vector<String>& args) {
  if (g_ssh_connected) {
    appendLine("Already connected. Use 'disconnect' first.");
    return;
  }
  
  if (args.size() < 2) {
    appendLine("Usage: ssh [user@]hostname [-p port]");
    appendLine("Example: ssh user@example.com");
    return;
  }
  
  String target = args[1];
  String username = "root";
  String hostname = "";
  int port = 22;
  
  int atPos = target.indexOf('@');
  if (atPos > 0) {
    username = target.substring(0, atPos);
    hostname = target.substring(atPos + 1);
  } else {
    hostname = target;
  }
  
  for (size_t i = 2; i < args.size(); i++) {
    if (args[i] == "-p" && i + 1 < args.size()) {
      port = args[++i].toInt();
    }
  }
  
  g_ssh_username = username;
  g_ssh_hostname = hostname;
  
  appendLine(String("Connecting to ") + hostname + "...");
  
  g_ssh_session = ssh_new();
  if (!g_ssh_session) {
    appendLine("Error: Failed to create SSH session");
    return;
  }
  
  ssh_options_set(g_ssh_session, SSH_OPTIONS_HOST, hostname.c_str());
  ssh_options_set(g_ssh_session, SSH_OPTIONS_USER, username.c_str());
  ssh_options_set(g_ssh_session, SSH_OPTIONS_PORT, &port);
  
  if (ssh_connect(g_ssh_session) != SSH_OK) {
    appendLine(String("Connection failed: ") + ssh_get_error(g_ssh_session));
    ssh_free(g_ssh_session);
    g_ssh_session = nullptr;
    return;
  }
  
  appendLine("Connected. Authenticating...");
  
  // Try public key auth first
  if (ssh_userauth_publickey_auto(g_ssh_session, nullptr, nullptr) == SSH_AUTH_SUCCESS) {
    appendLine("Public key authentication successful");
    g_ssh_connected = true;
    ssh_open_shell();
    return;
  }
  
  // Fall back to password
  appendLine("Password: ");
  g_waiting_for_password = true;
}

void cmd_disconnect() {
  if (g_ssh_connected) {
    ssh_close_session();
  } else {
    appendLine("Not connected");
  }
}

// ===================== COMMAND DISPATCHER =====================

void executeCommand(const String& cmdLine) {
  auto tokens = splitWords(cmdLine);
  if (tokens.empty()) return;
  
  String cmd = tokens[0];
  cmd.toLowerCase();
  
  if (cmd == "help" || cmd == "?") {
    cmd_help();
  }
  else if (cmd == "clear" || cmd == "cls") {
    cmd_clear();
  }
  else if (cmd == "echo") {
    cmd_echo(tokens);
  }
  else if (cmd == "time") {
    cmd_time();
  }
  else if (cmd == "date") {
    cmd_date();
  }
  else if (cmd == "ls" || cmd == "dir") {
    cmd_ls();
  }
  else if (cmd == "cd") {
    cmd_cd(tokens);
  }
  else if (cmd == "cat" || cmd == "type") {
    cmd_cat(tokens);
  }
  else if (cmd == "pwd") {
    cmd_pwd();
  }
  else if (cmd == "batt" || cmd == "battery") {
    cmd_batt();
  }
  else if (cmd == "info" || cmd == "sysinfo") {
    cmd_info();
  }
  else if (cmd == "ssh") {
    cmd_ssh(tokens);
  }
  else if (cmd == "disconnect" || cmd == "disc") {
    cmd_disconnect();
  }
  else if (cmd == "exit" || cmd == "quit" || cmd == "q") {
    if (g_ssh_connected) {
      ssh_close_session();
    } else {
      CurrentAppState = HOME; HOME_INIT(); return;
    }
  }
  else {
    appendLine("Unknown command: " + cmd);
    appendLine("Type 'help' for available commands");
  }
}

// ===================== KEYBOARD HANDLER =====================


// Auto-generated INIT function
void TERMINALAPP_INIT() {
    CurrentAppState = TERMINALAPP;
    newState = true;
}

void processKB_TERMINALAPP() {
  if (OLEDPowerSave) {
    u8g2.setPowerSave(0);
    OLEDPowerSave = false;
  }
  
  disableTimeout = true;  // Keep terminal active
  
  unsigned long currentMillis = millis();
  if (currentMillis - KBBounceMillis >= KB_COOLDOWN) {
    char inchar = KB().updateKeypress();
    
    // No char received
    if (inchar == 0) {
      // Do nothing
    }
    // HOME key - exit to PocketMage
    else if (inchar == 12) {
      CurrentAppState = HOME; HOME_INIT(); return;
    }
    // SHIFT received
    else if (inchar == 17) {
      if (KB().getKeyboardState() == SHIFT) KB().setKeyboardState(NORMAL);
      else KB().setKeyboardState(SHIFT);
    }
    // FN received
    else if (inchar == 18) {
      if (KB().getKeyboardState() == FUNC) KB().setKeyboardState(NORMAL);
      else KB().setKeyboardState(FUNC);
    }
    // TAB received - autocomplete (future)
    else if (inchar == 9) {
      currentLine += "    ";
    }
    // ENTER received - execute command
    else if (inchar == 13) {
      if (g_waiting_for_password) {
        // Don't echo password
        handle_password_input(currentLine);
        currentLine = "";
        return;
      } else if (g_ssh_connected) {
        // Send to remote server
        appendLine("> " + currentLine);
        ssh_send_command(currentLine);
      } else {
        // Execute local command
        appendLine("> " + currentLine);
        if (currentLine.length() > 0) {
          executeCommand(currentLine);
        }
      }
      // Add to history (but not passwords)
      if (currentLine.length() > 0 && !g_waiting_for_password) {
        cmdHistory.push_back(currentLine);
        if (cmdHistory.size() > 50) {
          cmdHistory.erase(cmdHistory.begin());
        }
      }
      historyIndex = -1;
      currentLine = "";
    }
    // BACKSPACE received
    else if (inchar == 8) {
      if (currentLine.length() > 0) {
        currentLine.remove(currentLine.length() - 1);
      }
    }
    // UP arrow - scroll up through output (16 on hardware, 28 on emulator)
    else if (inchar == 16 || inchar == 28) {
      if (scrollOffset > 0) {
        scrollOffset--;
        needsRedraw = true;
      }
    }
    // DOWN arrow - scroll down through output (15 on hardware, 20 on emulator)
    else if (inchar == 15 || inchar == 20) {
      int maxOffset = (int)scrollback.size() - MAX_VISIBLE_LINES;
      if (maxOffset < 0) maxOffset = 0;
      if (scrollOffset < maxOffset) {
        scrollOffset++;
        needsRedraw = true;
      }
    }
    // LEFT arrow (19) - history previous
    else if (inchar == 19) {
      if (!cmdHistory.empty()) {
        if (historyIndex < 0) {
          historyIndex = cmdHistory.size() - 1;
        } else if (historyIndex > 0) {
          historyIndex--;
        }
        currentLine = cmdHistory[historyIndex];
      }
    }
    // RIGHT arrow (21) - history next
    else if (inchar == 21) {
      if (historyIndex >= 0) {
        historyIndex++;
        if (historyIndex >= (int)cmdHistory.size()) {
          historyIndex = -1;
          currentLine = "";
        } else {
          currentLine = cmdHistory[historyIndex];
        }
      }
    }
    // Regular character
    else {
      currentLine += inchar;
      if (inchar >= 48 && inchar <= 57) {
        // Keep FN on for numbers
      } else if (KB().getKeyboardState() != NORMAL) {
        KB().setKeyboardState(NORMAL);
      }
    }
    
    // Update OLED with current input line
    currentMillis = millis();
    if (currentMillis - OLEDFPSMillis >= (1000 / OLED_MAX_FPS)) {
      OLEDFPSMillis = currentMillis;
      OLED().oledLine("> " + currentLine, false);
    }
  }
  
  // Handle touch scroll
  TOUCH().updateScrollFromTouch();
  long scroll = TOUCH().getDynamicScroll();
  if (scroll != 0) {
    int newOffset = scrollOffset - (scroll / 10);
    int maxOffset = (int)scrollback.size() - MAX_VISIBLE_LINES;
    if (maxOffset < 0) maxOffset = 0;
    
    if (newOffset < 0) newOffset = 0;
    if (newOffset > maxOffset) newOffset = maxOffset;
    
    if (newOffset != scrollOffset) {
      scrollOffset = newOffset;
      needsRedraw = true;
    }
    TOUCH().setDynamicScroll(0);
  }
}

// ===================== E-INK HANDLER =====================

void einkHandler_TERMINALAPP() {
  // Poll for SSH output
  if (g_ssh_connected) {
    ssh_read_output();
  }
  
  if (!needsRedraw) return;
  needsRedraw = false;
  
  display.setRotation(3);
  display.fillScreen(GxEPD_WHITE);
  display.setFont(&FreeMono9pt7b);
  display.setTextColor(GxEPD_BLACK);
  
  // Draw title bar
  display.fillRect(0, 0, TERM_EINK_WIDTH, 18, GxEPD_BLACK);
  display.setTextColor(GxEPD_WHITE);
  display.setCursor(5, 13);
  if (g_ssh_connected) {
    display.print("Terminal [SSH]");
  } else {
    display.print("Terminal");
  }
  
  // Draw scroll indicator if needed
  if (scrollback.size() > MAX_VISIBLE_LINES) {
    int scrollbarHeight = 200;
    int thumbHeight = (MAX_VISIBLE_LINES * scrollbarHeight) / scrollback.size();
    if (thumbHeight < 10) thumbHeight = 10;
    int thumbPos = 20 + (scrollOffset * (scrollbarHeight - thumbHeight)) / 
                   (scrollback.size() - MAX_VISIBLE_LINES);
    
    display.drawRect(TERM_EINK_WIDTH - 8, 20, 6, scrollbarHeight, GxEPD_BLACK);
    display.fillRect(TERM_EINK_WIDTH - 7, thumbPos, 4, thumbHeight, GxEPD_BLACK);
  }
  
  // Draw scrollback lines
  display.setTextColor(GxEPD_BLACK);
  int y = 35;
  int startLine = scrollOffset;
  int endLine = std::min((int)scrollback.size(), startLine + MAX_VISIBLE_LINES);
  
  for (int i = startLine; i < endLine; i++) {
    display.setCursor(5, y);
    display.print(scrollback[i].c_str());
    y += LINE_HEIGHT;
  }
  
  // Note: Current input line is shown on OLED, not e-ink
  // E-ink only shows the scrollback/output history
  
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

// Function removed - handled by main firmware


// Function removed - handled by main firmware


// migrated from einkFunc.cpp

// Function removed - handled by main firmware
