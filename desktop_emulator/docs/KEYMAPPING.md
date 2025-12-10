# PocketMage Emulator Key Mappings

This document describes how desktop keyboard keys map to PocketMage's internal key codes.

## Quick Reference

| Desktop Key | Action |
|-------------|--------|
| **ESC** | Return to home screen |
| **Enter** | Confirm / Submit |
| **Backspace** | Delete character |
| **Tab** | Tab / Indent |
| **Space** | Space |
| **Arrow Keys** | Navigation |
| **Shift** | Toggle SHIFT mode |
| **Alt / F1** | Toggle FUNC mode |
| **A-Z, 0-9** | Text input |

## Detailed Key Codes

### Navigation Keys

| SDL Key | PocketMage Code | Description |
|---------|-----------------|-------------|
| `SDLK_ESCAPE` | `12` | Return to home screen |
| `SDLK_HOME` | `12` | Return to home screen |
| `SDLK_RETURN` | `13` | Enter / Confirm |
| `SDLK_KP_ENTER` | `13` | Enter (numpad) |
| `SDLK_BACKSPACE` | `8` | Delete character |
| `SDLK_DELETE` | `8` | Delete (mapped to backspace) |
| `SDLK_TAB` | `9` | Tab |
| `SDLK_SPACE` | `32` | Space |

### Arrow Keys

| SDL Key | PocketMage Code | Description |
|---------|-----------------|-------------|
| `SDLK_LEFT` | `19` | Move cursor left |
| `SDLK_RIGHT` | `21` | Move cursor right |
| `SDLK_UP` | `28` | Move cursor up |
| `SDLK_DOWN` | `20` | Move cursor down / Select |

### Modifier Keys

| SDL Key | PocketMage Code | Description |
|---------|-----------------|-------------|
| `SDLK_LSHIFT` / `SDLK_RSHIFT` | `17` | Toggle SHIFT keyboard mode |
| `SDLK_LALT` / `SDLK_RALT` | `18` | Toggle FUNC keyboard mode |
| `SDLK_F1` | `18` | Toggle FUNC keyboard mode |

### Text Input

Regular letter and number keys (A-Z, 0-9) are handled via SDL's `SDL_TEXTINPUT` event for proper character handling including shifted characters.

## PocketMage Keyboard Modes

The physical PocketMage device has three keyboard modes:

### NORMAL Mode (default)
```
q w e r t y u i o p
a s d f g h j k l [BKSP]
[TAB] z x c v b n m . [ENTER]
    [SHFT] [FN] [SPACE] [←] [SEL] [→]
```

### SHIFT Mode (press Shift to toggle)
```
Q W E R T Y U I O P
A S D F G H J K L [BKSP]
[TAB] Z X C V B N M , [ENTER]
    [SHFT] [FN] [SPACE] [↑] [SEL] [↓]
```

### FUNC Mode (press Alt/F1 to toggle)
```
1 2 3 4 5 6 7 8 9 0
# ! $ : ; ( ) ' " [BKSP]
[ESC] % _ & + - / ? , [ENTER]
    [SHFT] [FN] [SPACE] [HOME] [7] [6]
```

## Special Key Codes Reference

These are the internal key codes used by PocketMage:

| Code | Meaning |
|------|---------|
| `6` | FUNC mode: code 6 |
| `7` | FUNC mode: code 7 |
| `8` | Backspace |
| `9` | Tab |
| `12` | ESC / Home (return to home screen) |
| `13` | Enter / Carriage Return |
| `14` | FUNC mode: ESC |
| `17` | Shift toggle |
| `18` | Func toggle |
| `19` | Left arrow |
| `20` | Select / Down |
| `21` | Right arrow |
| `28` | Up arrow (SHIFT mode) |
| `29` | Select (SHIFT mode) |
| `30` | Down arrow (SHIFT mode) |

## Implementation

Key mapping is implemented in:
- `desktop_emulator/src/desktop_display_sdl2.cpp` - `sdlKeyToChar()` function
- `desktop_emulator/src/pocketmage_stubs.cpp` - `PocketmageKB::updateKeypress()`

The flow is:
1. SDL captures keyboard event
2. `DesktopDisplay::handleEvents()` processes the event
3. `sdlKeyToChar()` converts SDL keycode to PocketMage code
4. Key is queued in `_keyQueue`
5. `PocketmageKB::updateKeypress()` retrieves the key
6. App's `processKB_*()` function handles the key
