
#include <globals.h>
#if !OTA_APP // POCKETMAGE_OS
enum LexState {MENU, DEF};
LexState CurrentLexState = MENU;

static String currentLine = "";

// Vector to hold the definitions
std::vector<std::pair<String, String>> defList;
int definitionIndex = 0;

void LEXICON_INIT() {
  currentLine = "";
  CurrentAppState = LEXICON;
  CurrentLexState = MENU;
  KB().setKeyboardState(NORMAL);
  newState = true;
  definitionIndex = 0;
}

void loadDefinitions(String word) {
  OLED().oledWord("Loading Definitions");
  SDActive = true;
  pocketmage::setCpuSpeed(240);
  delay(50);

  defList.clear();  // Clear previous results

  if (word.length() == 0 || SD().getNoSD()) return;

  char firstChar = tolower(word[0]);
  if (firstChar < 'a' || firstChar > 'z') return;

  String filePath = "/dict/" + String((char)toupper(firstChar)) + ".txt";

  File file = SD_MMC.open(filePath);
  if (!file) {
    OLED().oledWord("Missing Dictionary!");
    delay(2000);
    return;
  }

  word.toLowerCase();

  while (file.available()) {
    String line = file.readStringUntil('\n');
    line.trim();
    if (line.length() == 0) continue;

    int defSplit = line.indexOf(')');
    if (defSplit == -1) continue;

    // Extract key and definition
    String key = line.substring(0, defSplit + 1);
    String def = line.substring(defSplit + 1);
    def.trim();

    String keyLower = key;
    keyLower.toLowerCase();

    if (keyLower.startsWith(word)) {
      defList.push_back({key, def});
    }
    else if (defList.size() > 0) {
      // No more definitions
      break;
    }
  }

  file.close();

  if (defList.empty()) {
    OLED().oledWord("No definitions found");
    delay(2000);
  }
  else {
    CurrentLexState = DEF;
    KB().setKeyboardState(NORMAL);
    definitionIndex = 0;
    newState = true;
  }

  if (SAVE_POWER) pocketmage::setCpuSpeed(POWER_SAVE_FREQ);
  SDActive = false;
}

void processKB_LEXICON() {
  int currentMillis = millis();

  switch (CurrentLexState) {
    case MENU:
      if (currentMillis - KBBounceMillis >= KB_COOLDOWN) {  
        char inchar = KB().updateKeypress();
        // HANDLE INPUTS
        //No char recieved
        if (inchar == 0);   
        //CR Recieved
        else if (inchar == 13) {                          
          loadDefinitions(currentLine);
          currentLine = "";
        }                                      
        //SHIFT Recieved
        else if (inchar == 17) {                                  
          if (KB().getKeyboardState() == SHIFT) KB().setKeyboardState(NORMAL);
          else KB().setKeyboardState(SHIFT);
        }
        //FN Recieved
        else if (inchar == 18) {                                  
          if (KB().getKeyboardState() == FUNC) KB().setKeyboardState(NORMAL);
          else KB().setKeyboardState(FUNC);
        }
        //Space Recieved
        else if (inchar == 32) {                                  
          currentLine += " ";
        }
        //ESC / CLEAR Recieved
        else if (inchar == 20) {                                  
          currentLine = "";
        }
        //BKSP Recieved
        else if (inchar == 8) {                  
          if (currentLine.length() > 0) {
            currentLine.remove(currentLine.length() - 1);
          }
        }
        // Home recieved
        else if (inchar == 12) {
          HOME_INIT();
        }
        else {
          currentLine += inchar;
          if (inchar >= 48 && inchar <= 57) {}  //Only leave FN on if typing numbers
          else if (KB().getKeyboardState() != NORMAL) {
            KB().setKeyboardState(NORMAL);
          }
        }

        currentMillis = millis();
        //Make sure oled only updates at OLED_MAX_FPS
        if (currentMillis - OLEDFPSMillis >= (1000/OLED_MAX_FPS)) {
          OLEDFPSMillis = currentMillis;
          OLED().oledLine(currentLine, false);
        }
      }
      break;

    case DEF:
      if (currentMillis - KBBounceMillis >= KB_COOLDOWN) {  
        char inchar = KB().updateKeypress();
        // HANDLE INPUTS
        //No char recieved
        if (inchar == 0);   
        //CR Recieved
        else if (inchar == 13) {                          
          loadDefinitions(currentLine);
          currentLine = "";
        }                                      
        //SHIFT Recieved
        else if (inchar == 17) {                                  
          if (KB().getKeyboardState() == SHIFT) KB().setKeyboardState(NORMAL);
          else KB().setKeyboardState(SHIFT);
        }
        //FN Recieved
        else if (inchar == 18) {                                  
          if (KB().getKeyboardState() == FUNC) KB().setKeyboardState(NORMAL);
          else KB().setKeyboardState(FUNC);
        }
        //Space Recieved
        else if (inchar == 32) {                                  
          currentLine += " ";
        }
        //ESC / CLEAR Recieved
        else if (inchar == 20) {                                  
          currentLine = "";
        }
        //BKSP Recieved
        else if (inchar == 8) {                  
          if (currentLine.length() > 0) {
            currentLine.remove(currentLine.length() - 1);
          }
        }
        // Home recieved
        else if (inchar == 12) {
          HOME_INIT();
        }

        // LEFT Recieved
        else if (inchar == 19) {
          definitionIndex--;
          if (definitionIndex < 0) definitionIndex = 0;
          newState = true;
        }
        // RIGHT Received
        else if (inchar == 21) {
          definitionIndex++;
          if (definitionIndex >= defList.size()) definitionIndex = defList.size() - 1;
          newState = true;
        }

        else {
          currentLine += inchar;
          if (inchar >= 48 && inchar <= 57) {}  //Only leave FN on if typing numbers
          else if (KB().getKeyboardState() != NORMAL) {
            KB().setKeyboardState(NORMAL);
          }
        }

        currentMillis = millis();
        //Make sure oled only updates at OLED_MAX_FPS
        if (currentMillis - OLEDFPSMillis >= (1000/OLED_MAX_FPS)) {
          OLEDFPSMillis = currentMillis;
          OLED().oledLine(currentLine, false);
        }
      }
      break;
  }
}

void einkHandler_LEXICON() {
  switch (CurrentLexState) {
    case MENU:
      if (newState) {
        newState = false;
        EINK().resetDisplay(false);
        display.drawBitmap(0, 0, _lex0, 320, 218, GxEPD_BLACK);

        EINK().drawStatusBar("Type a Word:");

        EINK().multiPassRefresh(2);
      }
      break;
    case DEF:
      if (newState) {
        newState = false;

        display.drawBitmap(0, 0, _lex1, 320, 218, GxEPD_BLACK);

        display.setTextColor(GxEPD_BLACK);

        // Draw Word
        display.setFont(&FreeSerif12pt7b);
        display.setCursor(12, 50);
        display.print(defList[definitionIndex].first);

        // Draw Definition
        display.setFont(&FreeSerif9pt7b);
        display.setCursor(8, 87);
        // ADD WORD WRAP
        display.print(defList[definitionIndex].second);

        EINK().drawStatusBar("Type a New Word:");

        EINK().forceSlowFullUpdate(true);
        EINK().refresh();
      }
      break;
  }
  
}
#endif