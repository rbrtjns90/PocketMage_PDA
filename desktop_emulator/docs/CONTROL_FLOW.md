# PocketMage Control Flow

## Startup Sequence

```
main()
  │
  ├─► g_display = new DesktopDisplay()
  │     └─► SDL2 window created (930x930)
  │         ├─► E-ink area: 0,0 (930x780)
  │         └─► OLED area: 81,795 (768x120)
  │
  ├─► setup()  [PocketMageV3.cpp]
  │     └─► PocketMage_INIT()  [pocketmage_sys.cpp]
  │           ├─► Serial.begin(115200)
  │           ├─► Wire.begin() / SPI.begin()
  │           ├─► setupOled()      → U8g2 OLED init
  │           ├─► setupBZ()        → Buzzer init
  │           ├─► setupKB()        → TCA8418 keyboard init
  │           ├─► setupEink()      → GxEPD2 display init
  │           ├─► setupSD()        → SD_MMC filesystem init
  │           ├─► PowerSystem.init()
  │           ├─► setupTouch()     → MPR121 touch init
  │           ├─► setupClock()     → RTC init
  │           │
  │           └─► pocketmage::power::loadState()
  │                 ├─► prefs.begin("PocketMage")
  │                 ├─► Load settings (TIMEOUT, DEBUG_VERBOSE, etc.)
  │                 ├─► CurrentAppState = HOME (default)
  │                 │
  │                 └─► switch(CurrentAppState)
  │                       ├─► HOME     → HOME_INIT()     ← Sets newState=true
  │                       ├─► TXT      → TXT_INIT()
  │                       ├─► SETTINGS → SETTINGS_INIT()
  │                       ├─► TASKS    → TASKS_INIT()
  │                       ├─► CALENDAR → CALENDAR_INIT()
  │                       ├─► LEXICON  → LEXICON_INIT()
  │                       └─► JOURNAL  → JOURNAL_INIT()
  │
  └─► Main Loop (60 FPS)
        │
        ├─► g_display->handleEvents()   ← SDL2 keyboard/mouse events
        │
        ├─► loop()  [PocketMageV3.cpp]
        │     ├─► pocketmage::time::checkTimeout()
        │     ├─► pocketmage::power::updateBattState()
        │     └─► processKB()
        │           └─► switch(CurrentAppState)
        │                 ├─► HOME     → processKB_HOME()
        │                 ├─► TXT      → processKB_TXT_NEW()
        │                 ├─► FILEWIZ  → processKB_FILEWIZ()
        │                 ├─► TASKS    → processKB_TASKS()
        │                 ├─► SETTINGS → processKB_settings()
        │                 ├─► USB_APP  → processKB_USB()
        │                 ├─► CALENDAR → processKB_CALENDAR()
        │                 ├─► LEXICON  → processKB_LEXICON()
        │                 ├─► JOURNAL  → processKB_JOURNAL()
        │                 └─► APPLOADER→ processKB_APPLOADER()
        │
        ├─► applicationEinkHandler()  [PocketMageV3.cpp]
        │     └─► switch(CurrentAppState)
        │           ├─► HOME     → einkHandler_HOME()
        │           │               └─► if(newState) { drawHome(); EINK().refresh(); }
        │           ├─► TXT      → einkHandler_TXT_NEW()
        │           ├─► FILEWIZ  → einkHandler_FILEWIZ()
        │           ├─► TASKS    → einkHandler_TASKS()
        │           ├─► SETTINGS → einkHandler_settings()
        │           ├─► USB_APP  → einkHandler_USB()
        │           ├─► CALENDAR → einkHandler_CALENDAR()
        │           ├─► LEXICON  → einkHandler_LEXICON()
        │           ├─► JOURNAL  → einkHandler_JOURNAL()
        │           └─► APPLOADER→ einkHandler_APPLOADER()
        │
        ├─► oled_present_if_dirty()   ← Update OLED texture
        │
        └─► g_display->present()      ← Render to SDL2 window
```

## App State Machine

```
                    ┌─────────────────────────────────────────────┐
                    │                                             │
                    ▼                                             │
              ┌──────────┐                                        │
    ┌────────►│   HOME   │◄───────────────────────────────────────┤
    │         └────┬─────┘                                        │
    │              │ (command input)                              │
    │              ▼                                              │
    │    ┌─────────┴─────────┬─────────┬─────────┬─────────┐      │
    │    ▼                   ▼         ▼         ▼         ▼      │
    │ ┌─────┐  ┌─────────┐ ┌─────┐ ┌────────┐ ┌───────┐ ┌─────┐   │
    │ │ TXT │  │ FILEWIZ │ │TASKS│ │CALENDAR│ │JOURNAL│ │ ... │   │
    │ └──┬──┘  └────┬────┘ └──┬──┘ └───┬────┘ └───┬───┘ └──┬──┘   │
    │    │          │         │        │          │        │      │
    │    └──────────┴─────────┴────────┴──────────┴────────┴──────┘
    │                         (ESC / char 12)
    │
    └─── All apps return to HOME via ESC key (char code 12)
```

## Drawing Pipeline

```
App Code (e.g., drawHome())
    │
    ├─► display.fillScreen(GxEPD_WHITE)
    │     └─► GxEPD2_BW::fillScreen()
    │           └─► g_display->einkClear()
    │                 └─► _einkBuffer filled with 0 (white)
    │
    ├─► display.drawBitmap(x, y, bitmap, w, h, GxEPD_BLACK)
    │     └─► Adafruit_GFX::drawBitmap()  [inherited]
    │           └─► for each pixel: drawPixel(x, y, color)
    │                 └─► GxEPD2_BW::drawPixel()  [virtual override]
    │                       └─► g_display->einkSetPixel(x, y, black)
    │                             └─► _einkBuffer[y*WIDTH+x] = 1
    │
    ├─► display.print("text")
    │     └─► Adafruit_GFX::write() → drawChar() → drawPixel()
    │
    └─► EINK().refresh()
          └─► g_display->einkRefresh()
                └─► _needsEinkRefresh = true
                      │
                      ▼ (next frame)
                g_display->present()
                      └─► updateEinkTexture()
                            └─► Copy _einkBuffer to SDL texture
                                  └─► SDL_RenderCopy() to window
```

## Key Classes & Singletons

```
┌─────────────────────────────────────────────────────────────────┐
│                     Desktop Emulator Layer                       │
├─────────────────────────────────────────────────────────────────┤
│  DesktopDisplay (g_display)                                      │
│    ├─► SDL2 window management                                    │
│    ├─► _einkBuffer[310*240] - E-ink pixel buffer                │
│    ├─► _oledBuffer[256*32]  - OLED pixel buffer                 │
│    └─► Keyboard event handling                                   │
├─────────────────────────────────────────────────────────────────┤
│                     PocketMage Library Layer                     │
├─────────────────────────────────────────────────────────────────┤
│  EINK()  → PocketmageEink   - E-ink display control             │
│  OLED()  → PocketmageOLED   - OLED display control              │
│  KB()    → PocketmageKB     - Keyboard input                    │
│  SD()    → PocketmageSD     - SD card filesystem                │
│  CLOCK() → PocketmageClock  - RTC time                          │
│  TOUCH() → PocketmageTOUCH  - Capacitive touch                  │
│  BZ()    → PocketmageBZ     - Buzzer/audio                      │
├─────────────────────────────────────────────────────────────────┤
│  display  → GxEPD2_BW<...>  - E-ink drawing (Adafruit_GFX)      │
│  u8g2     → U8G2_...        - OLED drawing                      │
│  keypad   → Adafruit_TCA8418- Hardware keyboard matrix          │
│  SD_MMC   → SD_MMCClass     - SD card access                    │
│  prefs    → Preferences     - NVS storage (JSON file in emu)    │
└─────────────────────────────────────────────────────────────────┘
```

## Global State Variables

| Variable | Type | Purpose |
|----------|------|---------|
| `CurrentAppState` | `AppState` enum | Which app is active |
| `CurrentHOMEState` | `HomeState` enum | Sub-state within HOME |
| `newState` | `bool` | Triggers screen redraw when true |
| `currentLine` | `String` | Current text input buffer |
| `noTimeout` | `bool` | Disable sleep timeout |
| `SDActive` | `bool` | SD card operation in progress |

## File Locations

```
Code/PocketMage_V3/
├── src/
│   ├── PocketMageV3.cpp      ← Main entry (setup/loop)
│   ├── globals.cpp           ← Global variable definitions
│   └── OS_APPS/
│       ├── HOME.cpp          ← Home screen app
│       ├── TXT_NEW.cpp       ← Text editor
│       ├── FILEWIZ.cpp       ← File browser
│       ├── TASKS.cpp         ← Task manager
│       ├── CALENDAR.cpp      ← Calendar app
│       ├── JOURNAL.cpp       ← Journal app
│       ├── LEXICON.cpp       ← Dictionary
│       ├── SETTINGS.cpp      ← Settings menu
│       ├── APPLOADER.cpp     ← Sideloaded apps
│       └── USB.cpp           ← USB mass storage
│
├── lib/PocketMage/
│   ├── src/
│   │   ├── pocketmage_sys.cpp   ← PocketMage_INIT, loadState
│   │   ├── pocketmage_eink.cpp  ← E-ink helpers
│   │   ├── pocketmage_oled.cpp  ← OLED helpers
│   │   ├── pocketmage_kb.cpp    ← Keyboard handling
│   │   ├── pocketmage_sd.cpp    ← SD card helpers
│   │   └── ...
│   └── include/
│       └── pocketmage.h         ← Main include
│
└── include/
    ├── globals.h                ← Enums, externs, constants
    ├── config.h                 ← Pin definitions
    └── assets.h                 ← Bitmap assets
```
