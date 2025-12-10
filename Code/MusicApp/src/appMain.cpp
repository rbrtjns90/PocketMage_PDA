// PocketMage V3.0 - Music App
// Chiptune player and piano keyboard
// @R Jones 2025

#include <pocketmage.h>
#include <Buzzer.h>

static constexpr const char* TAG = "MUSIC";

// ===================== NOTE DEFINITIONS =====================
// Standard note frequencies (Hz)
#define NOTE_C4  262
#define NOTE_CS4 277
#define NOTE_D4  294
#define NOTE_DS4 311
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_FS4 370
#define NOTE_G4  392
#define NOTE_GS4 415
#define NOTE_A4  440
#define NOTE_AS4 466
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_CS5 554
#define NOTE_D5  587
#define NOTE_DS5 622
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_FS5 740
#define NOTE_G5  784
#define NOTE_GS5 831
#define NOTE_A5  880
#define NOTE_AS5 932
#define NOTE_B5  988
#define NOTE_C6  1047
#define NOTE_D6  1175
#define NOTE_E6  1319
#define NOTE_F6  1397
#define NOTE_G6  1568
#define NOTE_A6  1760
#define NOTE_REST 0

// ===================== APP STATE =====================
enum MusicMode { MODE_MENU, MODE_PIANO, MODE_PLAYER };
static MusicMode currentMode = MODE_MENU;
static int menuSelection = 0;
static int songSelection = 0;
static bool isPlaying = false;
static int currentNoteIndex = 0;
static unsigned long lastNoteTime = 0;
static volatile bool needsRedraw = true;

// ===================== SONG DATA =====================
struct SongNote {
  int freq;
  int duration;  // milliseconds
};

// Melody 1: Original "Adventure Theme" (upbeat exploration tune)
static const SongNote adventureTheme[] = {
  {NOTE_E5, 150}, {NOTE_G5, 150}, {NOTE_A5, 300}, {NOTE_G5, 150}, {NOTE_E5, 150},
  {NOTE_D5, 300}, {NOTE_E5, 150}, {NOTE_G5, 150}, {NOTE_A5, 150}, {NOTE_B5, 150},
  {NOTE_C6, 450}, {NOTE_REST, 150},
  {NOTE_B5, 150}, {NOTE_A5, 150}, {NOTE_G5, 300}, {NOTE_E5, 150}, {NOTE_D5, 150},
  {NOTE_E5, 600}, {NOTE_REST, 300},
  {NOTE_C5, 150}, {NOTE_D5, 150}, {NOTE_E5, 300}, {NOTE_G5, 150}, {NOTE_A5, 150},
  {NOTE_G5, 300}, {NOTE_E5, 150}, {NOTE_D5, 150}, {NOTE_C5, 600},
};
static const int adventureLen = sizeof(adventureTheme) / sizeof(adventureTheme[0]);

// Melody 2: Original "Victory Fanfare" (triumphant short jingle)
static const SongNote victoryFanfare[] = {
  {NOTE_G5, 100}, {NOTE_G5, 100}, {NOTE_G5, 100}, {NOTE_G5, 400},
  {NOTE_DS5, 400}, {NOTE_F5, 400},
  {NOTE_G5, 200}, {NOTE_REST, 100}, {NOTE_F5, 100}, {NOTE_G5, 600},
  {NOTE_REST, 200},
  {NOTE_C6, 150}, {NOTE_C6, 150}, {NOTE_C6, 150}, {NOTE_C6, 500},
};
static const int victoryLen = sizeof(victoryFanfare) / sizeof(victoryFanfare[0]);

// Melody 3: Original "Mystery Cave" (eerie dungeon music)
static const SongNote mysteryCave[] = {
  {NOTE_E4, 400}, {NOTE_REST, 100}, {NOTE_G4, 200}, {NOTE_REST, 100},
  {NOTE_A4, 300}, {NOTE_GS4, 300}, {NOTE_G4, 400}, {NOTE_REST, 200},
  {NOTE_E4, 200}, {NOTE_D4, 200}, {NOTE_E4, 600}, {NOTE_REST, 400},
  {NOTE_A4, 300}, {NOTE_G4, 200}, {NOTE_E4, 200}, {NOTE_D4, 400},
  {NOTE_E4, 800}, {NOTE_REST, 400},
  {NOTE_G4, 200}, {NOTE_A4, 200}, {NOTE_B4, 400}, {NOTE_A4, 200},
  {NOTE_G4, 200}, {NOTE_E4, 600},
};
static const int mysteryLen = sizeof(mysteryCave) / sizeof(mysteryCave[0]);

// Melody 4: Super Mario Bros Theme (Nintendo, 1985)
static const SongNote battleReady[] = {
  {NOTE_E5, 100}, {NOTE_E5, 100}, {NOTE_REST, 50}, {NOTE_E5, 100}, {NOTE_REST, 50},
  {NOTE_C5, 100}, {NOTE_E5, 200}, {NOTE_G5, 400}, {NOTE_REST, 200},
  {NOTE_G4, 400}, {NOTE_REST, 200},
  {NOTE_C5, 200}, {NOTE_REST, 100}, {NOTE_G4, 200}, {NOTE_REST, 100}, {NOTE_E4, 300},
  {NOTE_REST, 100}, {NOTE_A4, 200}, {NOTE_B4, 200}, {NOTE_AS4, 100}, {NOTE_A4, 200},
  {NOTE_G4, 150}, {NOTE_E5, 150}, {NOTE_G5, 150}, {NOTE_A5, 200},
  {NOTE_F5, 200}, {NOTE_G5, 200}, {NOTE_REST, 100}, {NOTE_E5, 200},
  {NOTE_C5, 150}, {NOTE_D5, 150}, {NOTE_B4, 300},
};
static const int battleLen = sizeof(battleReady) / sizeof(battleReady[0]);

// Song list
static const char* songNames[] = {
  "Adventure Theme",
  "Victory Fanfare", 
  "Mystery Cave",
  "Super Mario Bros"
};
static const int numSongs = 4;

// ===================== PIANO KEY MAPPING =====================
// Map keyboard keys to notes (QWERTY layout as piano)
int getKeyFrequency(char key) {
  switch(key) {
    // Bottom row - lower octave
    case 'z': return NOTE_C4;
    case 's': return NOTE_CS4;  // black key
    case 'x': return NOTE_D4;
    case 'd': return NOTE_DS4;  // black key
    case 'c': return NOTE_E4;
    case 'v': return NOTE_F4;
    case 'g': return NOTE_FS4;  // black key
    case 'b': return NOTE_G4;
    case 'h': return NOTE_GS4;  // black key
    case 'n': return NOTE_A4;
    case 'j': return NOTE_AS4;  // black key
    case 'm': return NOTE_B4;
    
    // Top row - higher octave
    case 'q': return NOTE_C5;
    case '2': return NOTE_CS5;  // black key
    case 'w': return NOTE_D5;
    case '3': return NOTE_DS5;  // black key
    case 'e': return NOTE_E5;
    case 'r': return NOTE_F5;
    case '5': return NOTE_FS5;  // black key
    case 't': return NOTE_G5;
    case '6': return NOTE_GS5;  // black key
    case 'y': return NOTE_A5;
    case '7': return NOTE_AS5;  // black key
    case 'u': return NOTE_B5;
    case 'i': return NOTE_C6;
    
    default: return 0;
  }
}

// ===================== BUZZER HELPER =====================
// Use the global buzzer which works on both hardware and emulator
extern Buzzer buzzer;

void playTone(int freq, int duration) {
  if (freq > 0) {
    buzzer.sound(freq, duration);
  } else {
    delay(duration);  // Rest
  }
}

void playToneNonBlocking(int freq) {
  if (freq > 0) {
    buzzer.tone(freq, 0);  // 0 duration = continuous
  } else {
    buzzer.noTone();
  }
}

void stopTone() {
  buzzer.noTone();
}

// ===================== SONG PLAYER =====================
const SongNote* getCurrentSong() {
  switch(songSelection) {
    case 0: return adventureTheme;
    case 1: return victoryFanfare;
    case 2: return mysteryCave;
    case 3: return battleReady;
    default: return adventureTheme;
  }
}

int getCurrentSongLength() {
  switch(songSelection) {
    case 0: return adventureLen;
    case 1: return victoryLen;
    case 2: return mysteryLen;
    case 3: return battleLen;
    default: return adventureLen;
  }
}

void updatePlayer() {
  if (!isPlaying) return;
  
  const SongNote* song = getCurrentSong();
  int songLen = getCurrentSongLength();
  
  unsigned long now = millis();
  if (now - lastNoteTime >= (unsigned long)song[currentNoteIndex].duration) {
    currentNoteIndex++;
    if (currentNoteIndex >= songLen) {
      // Song finished - loop
      currentNoteIndex = 0;
    }
    playToneNonBlocking(song[currentNoteIndex].freq);
    lastNoteTime = now;
    // Don't redraw on every note - causes flashing
  }
}

void startSong() {
  isPlaying = true;
  currentNoteIndex = 0;
  lastNoteTime = millis();
  const SongNote* song = getCurrentSong();
  playToneNonBlocking(song[0].freq);
  needsRedraw = true;
}

void stopSong() {
  isPlaying = false;
  stopTone();
  needsRedraw = true;
}

// ===================== INPUT HANDLER =====================
void processKB() {
  if (OLEDPowerSave) {
    u8g2.setPowerSave(0);
    OLEDPowerSave = false;
  }
  
  char inchar = KB().updateKeypress();
  if (inchar == 0) {
    // Update song player if in player mode
    if (currentMode == MODE_PLAYER) {
      updatePlayer();
    }
    return;
  }
  
  // HOME key - exit or go back
  if (inchar == 12) {
    if (currentMode == MODE_MENU) {
      rebootToPocketMage();
    } else {
      stopSong();
      currentMode = MODE_MENU;
      needsRedraw = true;
    }
    return;
  }
  
  switch(currentMode) {
    case MODE_MENU:
      // Up arrow
      if (inchar == 16 || inchar == 28) {
        menuSelection = (menuSelection - 1 + 3) % 3;
        needsRedraw = true;
      }
      // Down arrow
      else if (inchar == 15 || inchar == 20) {
        menuSelection = (menuSelection + 1) % 3;
        needsRedraw = true;
      }
      // Enter
      else if (inchar == 13) {
        if (menuSelection == 0) {
          currentMode = MODE_PIANO;
        } else if (menuSelection == 1) {
          currentMode = MODE_PLAYER;
          songSelection = 0;
        } else {
          rebootToPocketMage();
        }
        needsRedraw = true;
      }
      break;
      
    case MODE_PIANO:
      {
        int freq = getKeyFrequency(inchar);
        if (freq > 0) {
          playTone(freq, 150);
        }
      }
      break;
      
    case MODE_PLAYER:
      // Up/Down to select song
      if (inchar == 16 || inchar == 28) {
        if (!isPlaying) {
          songSelection = (songSelection - 1 + numSongs) % numSongs;
          needsRedraw = true;
        }
      }
      else if (inchar == 15 || inchar == 20) {
        if (!isPlaying) {
          songSelection = (songSelection + 1) % numSongs;
          needsRedraw = true;
        }
      }
      // Enter to play/stop
      else if (inchar == 13) {
        if (isPlaying) {
          stopSong();
        } else {
          startSong();
        }
      }
      // Space to stop
      else if (inchar == ' ') {
        stopSong();
      }
      break;
  }
  
  // Update OLED
  u8g2.clearBuffer();
  switch(currentMode) {
    case MODE_MENU:
      u8g2.drawStr(0, 12, "Music App");
      break;
    case MODE_PIANO:
      u8g2.drawStr(0, 12, "Piano Mode");
      break;
    case MODE_PLAYER:
      if (isPlaying) {
        u8g2.drawStr(0, 12, ("Playing: " + String(songNames[songSelection])).c_str());
      } else {
        u8g2.drawStr(0, 12, songNames[songSelection]);
      }
      break;
  }
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
  
  // Title bar
  display.fillRect(0, 0, 320, 20, GxEPD_BLACK);
  display.setTextColor(GxEPD_WHITE);
  display.setCursor(5, 15);
  display.print("Music App");
  display.setTextColor(GxEPD_BLACK);
  
  switch(currentMode) {
    case MODE_MENU:
      display.setCursor(20, 60);
      display.print(menuSelection == 0 ? "> Piano Mode" : "  Piano Mode");
      display.setCursor(20, 90);
      display.print(menuSelection == 1 ? "> Song Player" : "  Song Player");
      display.setCursor(20, 120);
      display.print(menuSelection == 2 ? "> Exit" : "  Exit");
      
      display.setCursor(20, 180);
      display.print("Up/Down: Select");
      display.setCursor(20, 200);
      display.print("Enter: Choose");
      break;
      
    case MODE_PIANO:
      display.setCursor(20, 50);
      display.print("Piano Keyboard");
      
      // Draw keyboard layout
      display.setCursor(20, 90);
      display.print("Lower: Z X C V B N M");
      display.setCursor(20, 110);
      display.print("       S D   G H J");
      display.setCursor(20, 140);
      display.print("Upper: Q W E R T Y U I");
      display.setCursor(20, 160);
      display.print("       2 3   5 6 7");
      
      display.setCursor(20, 200);
      display.print("HOME: Back to menu");
      break;
      
    case MODE_PLAYER:
      display.setCursor(20, 50);
      display.print("Song Player");
      
      // Song list
      for (int i = 0; i < numSongs; i++) {
        display.setCursor(20, 80 + i * 25);
        if (i == songSelection) {
          display.print("> ");
        } else {
          display.print("  ");
        }
        display.print(songNames[i]);
      }
      
      display.setCursor(20, 190);
      if (isPlaying) {
        display.print("Playing... ENTER=Stop");
      } else {
        display.print("ENTER: Play  HOME: Back");
      }
      break;
  }
  
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