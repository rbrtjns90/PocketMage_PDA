# PocketMage Music App

A chiptune music player and piano keyboard for PocketMage.

## Features

### Piano Mode
Play notes using your keyboard like a piano:

**Lower Octave (C4-B4):**
```
White keys: Z X C V B N M
Black keys:  S D   G H J
```

**Upper Octave (C5-C6):**
```
White keys: Q W E R T Y U I
Black keys:  2 3   5 6 7
```

### Song Player
Play pre-loaded chiptune melodies:
- **Adventure Theme** - Upbeat exploration music
- **Victory Fanfare** - Triumphant short jingle
- **Mystery Cave** - Eerie dungeon atmosphere
- **Battle Ready** - Intense battle intro

## Controls

### Menu
- **Up/Down** - Select option
- **Enter** - Choose option
- **HOME** - Exit to PocketMage

### Piano Mode
- **Letter/Number keys** - Play notes
- **HOME** - Back to menu

### Song Player
- **Up/Down** - Select song
- **Enter** - Play/Stop
- **Space** - Stop
- **HOME** - Back to menu

## Building

```bash
cd Code/MusicApp
pio run
```

## Installing to Desktop Emulator

```bash
cd desktop_emulator
python3 install_app.py ../Code/MusicApp
```

## Technical Notes

- Uses ESP32 LEDC for tone generation
- Songs loop automatically
- Non-blocking playback allows UI updates during music
