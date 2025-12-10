#ifndef GLOBALS_H
#define GLOBALS_H

// LIBRARIES
#include <USB.h>
#include <USBMSC.h>
#include <SD_MMC.h>
#include <Preferences.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <pocketmage.h>
#include <assets.h>

// ===================== USB & STORAGE =====================
// USB mass storage controller
extern USBMSC msc;           // USB MSC object
extern sdmmc_card_t* card;   // SD card pointer

// ===================== SYSTEM STATE =====================
extern Preferences prefs;        // NVS preferences
extern int OLEDFPSMillis;            // Last OLED FPS update time
extern int KBBounceMillis;           // Last keyboard debounce time
extern volatile bool TCA8418_event;  // Keypad interrupt event
extern volatile bool PWR_BTN_event;  // Power button event
extern volatile bool newState;       // App state changed
extern volatile bool OLEDPowerSave;  // OLED power save mode
extern volatile bool disableTimeout; // Disable timeout globally
extern bool fileLoaded;

// extern volatile int prevBattState;   // Previous battery state
extern unsigned int flashMillis;     // Flash timing
extern int prevTime;                 // Previous time (minutes)
extern uint8_t prevSec;              // Previous seconds
extern TaskHandle_t einkHandlerTaskHandle; // E-Ink handler task

extern String OTA1_APP;
extern String OTA2_APP;
extern String OTA3_APP;
extern String OTA4_APP;

// ===================== KEYBOARD STATE =====================
extern volatile bool SDCARD_INSERT;  // SD card inserted event
enum KBState { NORMAL, SHIFT, FUNC };    // Keyboard state

// ===================== APP STATES =====================
enum AppState { HOME, TXT, FILEWIZ, USB_APP, BT, SETTINGS, TASKS, CALENDAR, JOURNAL, LEXICON, APPLOADER, HELLO,
    ASTRALUAAPP,
    FLASHCARDAPP,
    GLUCOSEAPP,
    MUSICAPP,
    TERMINALAPP,
    STARTERAPP, APPLAUNCHER };
extern const String appStateNames[];     // App state names
extern const unsigned char *appIcons[11]; // App icons
extern AppState CurrentAppState;         // Current app state

// ===================== TASKS APP =====================
extern std::vector<std::vector<String>> tasks; // Task list

// ===================== HOME APP =====================
enum HOMEState { HOME_HOME, NOWLATER };       // Home app states
extern HOMEState CurrentHOMEState;            // Current home state

// <FILEWIZ.cpp>
void FILEWIZ_INIT();
void processKB_FILEWIZ();
void einkHandler_FILEWIZ();
String fileWizardMini(bool allowRecentSelect = false, String rootDir = "/");

// <TXT.cpp>
void TXT_INIT();
void processKB_TXT_NEW();
void einkHandler_TXT_NEW();
void saveMarkdownFile(const String& path);

// <HOME.cpp>
void HOME_INIT();
void einkHandler_HOME();
void processKB_HOME();
void mageIdle(bool internalRefresh = true);
void resetIdle();

// <TASKS.cpp>
void TASKS_INIT();
void sortTasksByDueDate(std::vector<std::vector<String>> &tasks);
void updateTaskArray();
void einkHandler_TASKS();
void processKB_TASKS();

// <settings.cpp>
void SETTINGS_INIT();
void processKB_settings();
void einkHandler_settings();
void settingCommandSelect(String command);

// <USB.cpp>
void USB_INIT();
void processKB_USB();
void einkHandler_USB();

// <CALENDAR.cpp>
void CALENDAR_INIT();
void processKB_CALENDAR();
void einkHandler_CALENDAR();

// <LEXICON.cpp>
void LEXICON_INIT();
void processKB_LEXICON();
void einkHandler_LEXICON();

// <JOURNAL.cpp>
void JOURNAL_INIT();
void processKB_JOURNAL();
void einkHandler_JOURNAL();

// <APPLOADER.cpp>
void APPLOADER_INIT();
void processKB_APPLOADER();
void einkHandler_APPLOADER();
void rebootToAppSlot(int otaIndex);
void loadAndDrawAppIcon(int x, int y, int otaIndex, bool showName = true, int maxNameChars = 10);

// <HELLO_WORLD.cpp> - Example app for developers
void HELLO_INIT();
void processKB_HELLO();
void einkHandler_HELLO();

// ASTRALUAAPP App
void ASTRALUAAPP_INIT();
void processKB_ASTRALUAAPP();
void einkHandler_ASTRALUAAPP();


// FLASHCARDAPP App
void FLASHCARDAPP_INIT();
void processKB_FLASHCARDAPP();
void einkHandler_FLASHCARDAPP();


// GLUCOSEAPP App
void GLUCOSEAPP_INIT();
void processKB_GLUCOSEAPP();
void einkHandler_GLUCOSEAPP();


// MUSICAPP App
void MUSICAPP_INIT();
void processKB_MUSICAPP();
void einkHandler_MUSICAPP();


// TERMINALAPP App
void TERMINALAPP_INIT();
void processKB_TERMINALAPP();
void einkHandler_TERMINALAPP();


// STARTERAPP App
void STARTERAPP_INIT();
void processKB_STARTERAPP();
void einkHandler_STARTERAPP();

// APPLAUNCHER - Desktop Emulator App Launcher
void APPLAUNCHER_INIT();
void processKB_APPLAUNCHER();
void einkHandler_APPLAUNCHER();


// <PocketMage>
void einkHandler(void *parameter); // moved from EinkFunc.cpp
void applicationEinkHandler();
void processKB();

#endif // GLOBALS_H