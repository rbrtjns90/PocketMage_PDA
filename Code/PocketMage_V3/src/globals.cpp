#include "globals.h"
#include "sdmmc_cmd.h"

// ===================== USB & STORAGE =====================
// USB mass storage controller
USBMSC msc;
sdmmc_card_t* card = nullptr;     // SD card pointer

// ===================== SYSTEM STATE =====================
// E-Ink refresh control
// volatile int einkRefresh = FULL_REFRESH_AFTER; // Partial/full refresh counter
Preferences prefs;        // NVS preferences
int OLEDFPSMillis = 0;            // Last OLED FPS update time
int KBBounceMillis = 0;           // Last keyboard debounce time
volatile bool TCA8418_event = false;  // Keypad interrupt event
volatile bool PWR_BTN_event = false;  // Power button event
volatile bool newState = false;       // App state changed
volatile bool OLEDPowerSave = false;  // OLED power save mode
volatile bool disableTimeout = false; // Disable timeout globally
bool fileLoaded = false;

// volatile int prevBattState = 0;       // Previous battery state
unsigned int flashMillis = 0;         // Flash timing
int prevTime = 0;                     // Previous time (minutes)
uint8_t prevSec = 0;                  // Previous seconds
uint8_t partialCounter = 0;           // Counter for partial refreshes
volatile bool forceSlowFullUpdate = false; // Force slow full update

int TIMEOUT;              // Auto sleep timeout (seconds)
bool DEBUG_VERBOSE;       // Extra debug output
bool SYSTEM_CLOCK;        // Show clock on screen
bool SHOW_YEAR;           // Show year in clock
bool SAVE_POWER;          // Enable power saving mode
bool ALLOW_NO_MICROSD;    // Allow running without SD card
bool HOME_ON_BOOT;        // Start home app on boot
int OLED_BRIGHTNESS = 255;  // OLED brightness (0-255)
int OLED_MAX_FPS = 30;      // OLED max FPS

String OTA1_APP;
String OTA2_APP;
String OTA3_APP;
String OTA4_APP;

// ===================== FILES & TEXT =====================
volatile bool SDCARD_INSERT = false;  // SD card inserted event

// ===================== APP STATES =====================
const String appStateNames[] = { "txt", "filewiz", "usb", "bt", "settings", "tasks", "calendar", "journal", "lexicon", "script" , "loader" }; // App state names
const unsigned char *appIcons[11] = { _homeIcons2, _homeIcons3, _homeIcons4, _homeIcons5, _homeIcons6, taskIconTasks0, _homeIcons7, _homeIcons8, _homeIcons9, _homeIcons11, _homeIcons10}; // App icons
AppState CurrentAppState;             // Current app state

// ===================== TASKS APP =====================
std::vector<std::vector<String>> tasks;      // Task list

// ===================== HOME APP =====================
HOMEState CurrentHOMEState = HOME_HOME;      // Current home state