# PocketMage Icon Editor
*Working title - name TBD*

A Qt6-based pixel art editor designed for creating PocketMage icons and general pixel art.

## Features

### Drawing Tools
- **Pencil** (P) - Draw individual pixels
- **Eraser** (E) - Erase to white
- **Fill** (F) - Flood fill an area
- **Line** (L) - Draw straight lines
- **Rectangle** (R) - Draw rectangles
- **Ellipse** (O) - Draw ellipses/circles
- **Eyedropper** (I) - Pick color from canvas
- **Text** (T) - Type text (3x5 pixel font)
- **Select** (S) - Select and move regions

### Image Import
**File → Import Image to Pixel Art** (Ctrl+Shift+I)

Convert any image (PNG, JPG, etc.) to pixel art with options:
- **Target Size**: Resize to specific dimensions (default 40x40 for icons)
- **Color Mode**: Monochrome, Preserve Colors, Grayscale (4/8 levels)
- **Auto-crop**: Remove whitespace borders before scaling
- **Invert colors**: Swap light and dark

### Open with Color Options
**File → Open** (Ctrl+O)

When opening any image, choose how to process it:
- Keep Original Colors
- Monochrome (Black & White) with adjustable threshold
- Grayscale (4 or 8 levels)
- Quantize Colors (reduce palette)
- Invert colors

### Export Formats
- **PNG/BMP** - Standard image formats
- **Export BMP (PocketMage)** (Ctrl+E) - 1-bit BMP for SD card icons
- **Export Arduino Header** - C++ PROGMEM array for embedding in firmware
- **Export Raw Binary** - 4-byte header + packed 1-bit pixels

### Zoom Controls
| Button | Shortcut | Action |
|--------|----------|--------|
| 🔍− | `-` | Zoom out (25% relative) |
| 🔍+ | `+` | Zoom in (25% relative) |
| 1:1 | `1` | Actual size (1 screen pixel = 1 image pixel) |
| ⊡ Fit | `0` | Fit entire image in window |

Also supports:
- Scroll wheel zoom
- Pinch-to-zoom gesture (trackpad)

### Color Palette
Quick-access palette with common pixel art colors. Click any color to select it.

## Building

### Requirements
- Qt6 (Widgets module)
- CMake 3.16+
- C++17 compiler

### Build Commands
```bash
cd tools/IconEditor
mkdir -p build && cd build
cmake ..
make
```

### Run
```bash
# macOS
./IconEditor.app/Contents/MacOS/IconEditor

# Linux
./IconEditor
```

## Usage for PocketMage Icons

### Creating a New Icon
1. Set size to 40x40 (default)
2. Draw your icon using the tools
3. Export as BMP, Header, or Raw Binary for firmware

### Converting an Existing Image
1. **File → Import Image to Pixel Art** (Ctrl+Shift+I)
2. Set target size to 40x40
3. Choose **Monochrome** for e-ink display
4. Adjust threshold if needed
5. Check **Auto-crop** to remove whitespace
6. Click OK and edit the result
7. Export as needed

### Export for App Loader
```bash
# Export as BMP, then copy to SD card
/apps/myapp_ICON.bmp
```

### Export for Firmware
1. **File → Export Arduino Header**
2. Copy the generated array to `assets.cpp`
3. Reference in `homeIconsAllArray`

## Keyboard Shortcuts

| Action | Shortcut |
|--------|----------|
| New | Ctrl+N |
| Open | Ctrl+O |
| Save | Ctrl+S |
| Export BMP | Ctrl+E |
| Import Image | Ctrl+Shift+I |
| Invert Colors | Ctrl+I |
| Clear | Ctrl+Delete |
| Quit | Ctrl+Q |
| Zoom In | + |
| Zoom Out | - |
| Actual Size | 1 |
| Fit to Window | 0 |

## File Formats

### Arduino Header (.h)
```c
// 'iconName', 40x40px
const unsigned char iconName [] PROGMEM = {
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  // ... 200 bytes total for 40x40
};
```

### Raw Binary (.bin)
- Bytes 0-1: Width (uint16_t, little-endian)
- Bytes 2-3: Height (uint16_t, little-endian)
- Bytes 4+: Packed 1-bit pixels (8 pixels per byte, MSB first)

## Tips

- **High contrast** works best on e-ink displays
- Use **Monochrome** mode for PocketMage icons
- **Auto-crop** helps maximize icon visibility
- Adjust **threshold** (50-200) to control black/white balance
- **Invert** is useful for dark subjects on light backgrounds
