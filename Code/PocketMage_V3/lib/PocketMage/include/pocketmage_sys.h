//  888888ba                    dP                  dP                                          //
//  88    `8b                   88                  88                                          //
// a88aaaa8P' .d8888b. .d8888b. 88  .dP  .d8888b. d8888P 88d8b.d8b. .d8888b. .d8888b. .d8888b.  //
//  88        88'  `88 88'  `"" 88888"   88ooood8   88   88'`88'`88 88'  `88 88'  `88 88ooood8  //
//  88        88.  .88 88.  ... 88  `8b. 88.  ...   88   88  88  88 88.  .88 88.  .88 88.  ...  //
//  dP        `88888P' `88888P' dP   `YP `88888P'   dP   dP  dP  dP `88888P8 `8888P88 `88888P'  //
//                                                                                .88           //
//                                                                            d8888P            //
                                                     
#pragma once
#include <Arduino.h>
#include <vector>

class String;

extern bool mscEnabled;
extern bool sinkEnabled;
extern volatile bool SDActive;
extern volatile int battState;       // Battery state
extern volatile bool PWR_BTN_event;  // Power button event **shared with library**

extern bool rebootToPocketMage();

namespace pocketmage{
  void setCpuSpeed(int newFreq);
  void deepSleep(bool alternateScreenSaver = false);
  bool setRebootFlagOTA();
  void checkRebootOTA();
  void IRAM_ATTR PWR_BTN_irq();
}

// ===================== SYSTEM SETUP =====================
void PocketMage_INIT();
// ===================== GLOBAL TEXT HELPERS =====================
String vectorToString();
void stringToVector(String inputText);
String removeChar(String str, char character);
int stringToInt(String str);
extern volatile bool newLineAdded;           // New line added in TXT
extern std::vector<String> allLines;                // All lines in TXT
extern bool noTimeout;               // Disable timeout