#pragma once
#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <vector>
#include <GxEPD2_BW.h>

// ===================== FRAME CLASS =====================
# define MAX_FRAMES 100
# define X_OFFSET 4
#pragma region textSource
// bit flags for alignment or future options
enum LineFlags : uint8_t { LF_NONE=0, LF_RIGHT= 1<<0, LF_CENTER= 1<<1 };
struct LineView {
  const char* ptr;   // points to NUL-terminated string in RAM or PROGMEM
  uint16_t    len;   // byte length (no need to include '\0')
  uint8_t     flags; // LineFlags
};
// read-only interface for any line list (PROGMEM table, arena, etc.)
struct TextSource {
  virtual ~TextSource() {}
  virtual size_t   size() const = 0;
  virtual LineView line(size_t i) const = 0; 
};
template<size_t MAX_LINES, size_t BUF_BYTES>
struct FixedArenaSource : TextSource {
  char     buf[BUF_BYTES];
  uint16_t off[MAX_LINES];
  uint16_t len_[MAX_LINES];
  uint8_t  flags_[MAX_LINES];
  size_t   nLines = 0;
  size_t   used   = 0;

  size_t size() const override { return nLines; }

  LineView line(size_t i) const override {
    return { buf + off[i], len_[i], flags_[i] };
  }

  void clear() { nLines = 0; used = 0; }

  // Returns false if out of capacity; caller can choose to drop the oldest, etc.
  bool pushLine(const char* s, uint16_t L, uint8_t flags = LF_NONE) {
    if (nLines >= MAX_LINES || used + L + 1 > BUF_BYTES) return false;
    memcpy(buf + used, s, L);
    buf[used + L] = '\0';
    off[nLines]   = (uint16_t)used;
    len_[nLines]  = L;
    flags_[nLines]= flags;
    used         += L + 1;
    nLines++;
    return true;
  }
};
struct ProgmemTableSource : TextSource {
  // table is a PROGMEM array of PROGMEM pointers to '\0'-terminated strings
  const char* const* table; // PROGMEM
  size_t count;

  ProgmemTableSource(const char* const* t, size_t n) : table(t), count(n) {}

  size_t size() const override { return count; }

  LineView line(size_t i) const override {
    const char* p = (const char*)pgm_read_ptr(&table[i]);
    // length from PROGMEM
    uint16_t L = (uint16_t)strlen_P(p);
    return { p, L, LF_NONE };
  }
};

extern const char* const HELP_LINES[] PROGMEM;
extern const size_t HELP_COUNT;
extern const char* const UNIT_TYPES_LINES[] PROGMEM;
extern const size_t UNIT_TYPES_COUNT;
extern const char* const CONV_DIR_LINES[] PROGMEM;
extern const size_t CONV_DIR_COUNT;
extern const char* const CONV_LENGTH_LINES[] PROGMEM;
extern const size_t CONV_LENGTH_COUNT;
extern const char* const CONV_AREA_LINES[] PROGMEM;
extern const size_t CONV_AREA_COUNT;
extern const char* const CONV_VOLUME_LINES[] PROGMEM;
extern const size_t CONV_VOLUME_COUNT;
extern const char* const CONV_MASS_LINES[] PROGMEM;
extern const size_t CONV_MASS_COUNT;
extern const char* const CONV_TEMPERATURE_LINES[] PROGMEM;
extern const size_t CONV_TEMPERATURE_COUNT;
extern const char* const CONV_ENERGY_LINES[] PROGMEM;
extern const size_t CONV_ENERGY_COUNT;
extern const char* const CONV_SPEED_LINES[] PROGMEM;
extern const size_t CONV_SPEED_COUNT;
extern const char* const CONV_PRESSURE_LINES[] PROGMEM;
extern const size_t CONV_PRESSURE_COUNT;
extern const char* const CONV_DATA_LINES[] PROGMEM;
extern const size_t CONV_DATA_COUNT;
extern const char* const CONV_ANGLE_LINES[] PROGMEM;
extern const size_t CONV_ANGLE_COUNT;
extern const char* const CONV_TIME_LINES[] PROGMEM;
extern const size_t CONV_TIME_COUNT;
extern const char* const CONV_POWER_LINES[] PROGMEM;
extern const size_t CONV_POWER_COUNT;
extern const char* const CONV_FORCE_LINES[] PROGMEM;
extern const size_t CONV_FORCE_COUNT;
extern const char* const CONV_FREQUENCY_LINES[] PROGMEM;
extern const size_t CONV_FREQUENCY_COUNT;


extern FixedArenaSource<512, 16384> frameLines;
extern ProgmemTableSource helpSrc;


#pragma endregion
#pragma region frameSetup
class Frame {
public:
  // what kind of content this frame holds
  enum class Kind : uint8_t { none, text, bitmap };

  // geometry
  int left, right, top, bottom;
  int origLeft, origRight, origTop, origBottom;
  int extendLeft, extendRight, extendTop, extendBottom;
  int bitmapW = 0;
  int bitmapH = 0;
  // flags/state switch to uint8_t flag
  bool cursor   = false;
  bool box      = false;
  bool invert   = false;
  bool overlap  = false;


  int   choice     = -1;
  long  scroll     = 0;
  long  prevScroll = -1;
  int   maxLines   = 0;
  long  lastTotal  = -1;

  // content (only one valid at a time)
  Kind kind = Kind::none;
  const TextSource* source = nullptr;  // for text frames
  const uint8_t* bitmap    = nullptr;  // for bitmap frames
  const GFXfont *font = (GFXfont *)&FreeSerif9pt7b;

  
  // base constructor for common fields
  Frame(int left, int right, int top, int bottom, 
        bool cursor=false, bool box=false)
  : left(left), right(right), top(top), bottom(bottom),
    extendBottom(bottom), origBottom(bottom), cursor(cursor), box(box) {}

  // constructor for text frames
  Frame(int left, int right, int top, int bottom,
        const TextSource* linesPtr,
        bool cur=false, bool bx=false)
  : Frame(left, right, top, bottom, cur, bx) {
    kind   = Kind::text;
    source = linesPtr;
  }

  // constructor for bitmap frames
  Frame(int left, int right, int top, int bottom,
        const uint8_t* bitmapPtr, int width, int height,
        bool cursor=false, bool box=false)
  : Frame(left, right, top, bottom, cursor, box) {
    kind      = Kind::bitmap;
    bitmap    = bitmapPtr;
    bitmapW      = width;
    bitmapH      = height;
  }

  bool hasText()   const { return kind == Kind::text   && source; }
  bool hasBitmap() const { return kind == Kind::bitmap && bitmap; }
};

extern Frame testBitmapScreen;
extern Frame testBitmapScreen1;
extern Frame testBitmapScreen2;
extern Frame testTextScreen;
extern Frame *CurrentFrameState;
extern int currentFrameChoice;
extern int frameSelection;
extern std::vector<Frame*> frames;
#pragma endregion

// <FRAMES.cpp>
  // main functions
void einkFramesDynamic(std::vector<Frame*> &frames, bool doFull_);
  // text boxes
std::vector<String> formatText(Frame &frame,int maxTextWidth);
void drawLineInFrame(String &srcLine, int lineIndex, Frame &frame, int usableY, bool clearLine, bool isPartial);
void drawFrameBox(int usableX, int usableY, int usableWidth, int usableHeight,bool invert);
int computeCursorX(Frame &frame, bool rightAlign, bool centerAlign, int16_t x1, uint16_t lineWidth);
  // String formatting
static size_t sliceThatFits(const char* s, size_t n, int maxTextWidth);
std::vector<String> sourceToVector(const TextSource* src);
String frameChoiceString(const Frame& f);  
  //scroll
void updateScroll(Frame *currentFrameState,int prevScroll,int currentScroll, bool reset = false);
void updateScrollFromTouch_Frame();
void oledScrollFrame(); 
//void updateScroll(Frame *currentFrameState,int prevScroll,int currentScroll, bool reset);
void getVisibleRange(Frame *f, long totalLines, long &startLine, long &endLine);
