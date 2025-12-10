// PocketMage V3.0 - FlashCard App
// Load and study flashcards from CSV files
// @Ashtf 2025

#include <pocketmage.h>
#include <vector>

static constexpr const char* TAG = "FLASHCARD";

// ===================== DATA MODEL =====================
struct FlashCard {
  String question;
  String answer;
  bool known;  // Track if user marked as "known"
};

struct Deck {
  String name;
  String filename;
};

// ===================== STATISTICS =====================
struct DeckStats {
  String deckName;
  int totalStudied;
  int totalCorrect;
  int sessionsCount;
  int bestScore;  // percentage
};

static std::vector<DeckStats> allStats;
static int lifetimeStudied = 0;
static int lifetimeCorrect = 0;
static int lifetimeSessions = 0;

// ===================== APP STATE =====================
enum class Screen { DeckList, Study, Results, Stats };
static Screen currentScreen = Screen::DeckList;
static volatile bool needsRedraw = true;

// Deck management
static std::vector<Deck> availableDecks;
static int selectedDeckIndex = 0;

// Current study session
static std::vector<FlashCard> currentCards;
static int currentCardIndex = 0;
static bool showingAnswer = false;
static int correctCount = 0;
static int incorrectCount = 0;
static bool studyComplete = false;
static String currentDeckName = "";

// Display constants
static constexpr int MAX_LINE_CHARS = 36;

// ===================== HELPER FUNCTIONS =====================
// Word wrap text to fit display width
std::vector<String> wrapText(const String& text, int maxChars) {
  std::vector<String> lines;
  String current = "";
  String word = "";
  
  for (size_t i = 0; i <= text.length(); i++) {
    char c = (i < text.length()) ? text.charAt(i) : ' ';
    
    if (c == ' ' || c == '\n' || i == text.length()) {
      if (current.length() + word.length() + 1 <= (size_t)maxChars) {
        if (current.length() > 0) current += " ";
        current += word;
      } else {
        if (current.length() > 0) lines.push_back(current);
        current = word;
      }
      word = "";
      
      if (c == '\n' && current.length() > 0) {
        lines.push_back(current);
        current = "";
      }
    } else {
      word += c;
    }
  }
  
  if (current.length() > 0) {
    lines.push_back(current);
  }
  
  return lines;
}

// ===================== DECK LOADING =====================
void scanForDecks() {
  availableDecks.clear();
  
  // Look for CSV files in /flashcards/ directory
  File dir = SD_MMC.open("/flashcards");
  if (!dir || !dir.isDirectory()) {
    // Create the directory if it doesn't exist
    SD_MMC.mkdir("/flashcards");
    return;
  }
  
  File entry = dir.openNextFile();
  while (entry) {
    String name = entry.name();
    if (name.endsWith(".csv") || name.endsWith(".CSV")) {
      Deck d;
      // Extract just the filename for display
      int lastSlash = name.lastIndexOf('/');
      if (lastSlash >= 0) {
        d.name = name.substring(lastSlash + 1, name.length() - 4);
      } else {
        d.name = name.substring(0, name.length() - 4);
      }
      
      // Store full path for loading
      // If name doesn't start with /, prepend /flashcards/
      if (name.startsWith("/")) {
        d.filename = name;
      } else {
        d.filename = "/flashcards/" + name;
      }
      availableDecks.push_back(d);
    }
    entry = dir.openNextFile();
  }
  dir.close();
}

void loadDeck(const String& filename) {
  currentCards.clear();
  
  File file = SD_MMC.open(filename.c_str(), FILE_READ);
  if (!file) return;
  
  // Skip header line if present
  bool firstLine = true;
  
  while (file.available()) {
    String line = file.readStringUntil('\n');
    line.trim();
    if (line.length() < 3) continue;
    
    // Check if this looks like a header
    if (firstLine) {
      firstLine = false;
      String lower = line;
      lower.toLowerCase();
      if (lower.startsWith("question") || lower.startsWith("front")) {
        continue;  // Skip header
      }
    }
    
    // Parse CSV: question,answer
    int commaPos = line.indexOf(',');
    if (commaPos > 0) {
      FlashCard card;
      card.question = line.substring(0, commaPos);
      card.answer = line.substring(commaPos + 1);
      card.known = false;
      
      // Remove quotes if present
      if (card.question.startsWith("\"")) {
        card.question = card.question.substring(1);
      }
      if (card.question.endsWith("\"")) {
        card.question = card.question.substring(0, card.question.length() - 1);
      }
      if (card.answer.startsWith("\"")) {
        card.answer = card.answer.substring(1);
      }
      if (card.answer.endsWith("\"")) {
        card.answer = card.answer.substring(0, card.answer.length() - 1);
      }
      
      currentCards.push_back(card);
    }
  }
  file.close();
}

// ===================== STATS PERSISTENCE =====================
void loadStats() {
  allStats.clear();
  lifetimeStudied = 0;
  lifetimeCorrect = 0;
  lifetimeSessions = 0;
  
  File file = SD_MMC.open("/flashcards/stats.csv", FILE_READ);
  if (!file) return;
  
  // First line: lifetime stats
  if (file.available()) {
    String line = file.readStringUntil('\n');
    // Format: lifetime,studied,correct,sessions
    int pos1 = line.indexOf(',');
    int pos2 = line.indexOf(',', pos1 + 1);
    int pos3 = line.indexOf(',', pos2 + 1);
    if (pos1 > 0 && pos2 > 0 && pos3 > 0) {
      lifetimeStudied = line.substring(pos1 + 1, pos2).toInt();
      lifetimeCorrect = line.substring(pos2 + 1, pos3).toInt();
      lifetimeSessions = line.substring(pos3 + 1).toInt();
    }
  }
  
  // Remaining lines: per-deck stats
  while (file.available()) {
    String line = file.readStringUntil('\n');
    line.trim();
    if (line.length() < 3) continue;
    
    // Format: deckname,studied,correct,sessions,best
    DeckStats ds;
    int p1 = line.indexOf(',');
    int p2 = line.indexOf(',', p1 + 1);
    int p3 = line.indexOf(',', p2 + 1);
    int p4 = line.indexOf(',', p3 + 1);
    
    if (p1 > 0) {
      ds.deckName = line.substring(0, p1);
      ds.totalStudied = line.substring(p1 + 1, p2).toInt();
      ds.totalCorrect = line.substring(p2 + 1, p3).toInt();
      ds.sessionsCount = line.substring(p3 + 1, p4).toInt();
      ds.bestScore = line.substring(p4 + 1).toInt();
      allStats.push_back(ds);
    }
  }
  file.close();
}

void saveStats() {
  File file = SD_MMC.open("/flashcards/stats.csv", FILE_WRITE);
  if (!file) return;
  
  // Lifetime stats
  char line[128];
  snprintf(line, sizeof(line), "lifetime,%d,%d,%d\n", 
           lifetimeStudied, lifetimeCorrect, lifetimeSessions);
  file.print(line);
  
  // Per-deck stats
  for (const auto& ds : allStats) {
    snprintf(line, sizeof(line), "%s,%d,%d,%d,%d\n",
             ds.deckName.c_str(), ds.totalStudied, ds.totalCorrect,
             ds.sessionsCount, ds.bestScore);
    file.print(line);
  }
  file.close();
}

void recordSession(const String& deckName, int studied, int correct) {
  // Update lifetime stats
  lifetimeStudied += studied;
  lifetimeCorrect += correct;
  lifetimeSessions++;
  
  int pct = (studied > 0) ? (correct * 100 / studied) : 0;
  
  // Find or create deck stats
  bool found = false;
  for (auto& ds : allStats) {
    if (ds.deckName == deckName) {
      ds.totalStudied += studied;
      ds.totalCorrect += correct;
      ds.sessionsCount++;
      if (pct > ds.bestScore) ds.bestScore = pct;
      found = true;
      break;
    }
  }
  
  if (!found) {
    DeckStats ds;
    ds.deckName = deckName;
    ds.totalStudied = studied;
    ds.totalCorrect = correct;
    ds.sessionsCount = 1;
    ds.bestScore = pct;
    allStats.push_back(ds);
  }
  
  saveStats();
}

void startStudySession() {
  if (selectedDeckIndex >= 0 && selectedDeckIndex < (int)availableDecks.size()) {
    currentDeckName = availableDecks[selectedDeckIndex].name;
    loadDeck(availableDecks[selectedDeckIndex].filename);
    currentCardIndex = 0;
    showingAnswer = false;
    correctCount = 0;
    incorrectCount = 0;
    studyComplete = false;
    
    // Only switch to study if we have cards
    if (!currentCards.empty()) {
      currentScreen = Screen::Study;
    }
    // If no cards loaded, stay on deck list (will show error)
    needsRedraw = true;
  }
}

void finishSession() {
  // Record the session stats
  int total = correctCount + incorrectCount;
  if (total > 0) {
    recordSession(currentDeckName, total, correctCount);
  }
}

void nextCard(bool correct) {
  if (correct) {
    correctCount++;
    currentCards[currentCardIndex].known = true;
  } else {
    incorrectCount++;
  }
  
  currentCardIndex++;
  showingAnswer = false;
  
  if (currentCardIndex >= (int)currentCards.size()) {
    studyComplete = true;
    finishSession();
    currentScreen = Screen::Results;
  }
  
  needsRedraw = true;
}

// ===================== APP INIT =====================
void appInit() {
  currentScreen = Screen::DeckList;
  needsRedraw = true;
  selectedDeckIndex = 0;
  currentCards.clear();
  currentCardIndex = 0;
  showingAnswer = false;
  correctCount = 0;
  incorrectCount = 0;
  studyComplete = false;
  currentDeckName = "";
  
  loadStats();
  scanForDecks();
}

// ===================== INPUT HANDLER =====================
void processKB() {
  if (OLEDPowerSave) {
    u8g2.setPowerSave(0);
    OLEDPowerSave = false;
  }
  
  char inchar = KB().updateKeypress();
  if (inchar == 0) return;
  
  // HOME key - go back or exit
  if (inchar == 12) {
    if (currentScreen == Screen::DeckList) {
      rebootToPocketMage();
    } else {
      currentScreen = Screen::DeckList;
      scanForDecks();
      needsRedraw = true;
    }
    return;
  }
  
  switch(currentScreen) {
    case Screen::DeckList:
      // Up arrow
      if (inchar == 16 || inchar == 28) {
        if (selectedDeckIndex > 0) {
          selectedDeckIndex--;
          needsRedraw = true;
        }
      }
      // Down arrow
      else if (inchar == 15 || inchar == 20) {
        if (selectedDeckIndex < (int)availableDecks.size() - 1) {
          selectedDeckIndex++;
          needsRedraw = true;
        }
      }
      // Enter - start studying
      else if (inchar == 13) {
        if (!availableDecks.empty()) {
          startStudySession();
        }
      }
      // R - refresh deck list
      else if (inchar == 'r' || inchar == 'R') {
        scanForDecks();
        needsRedraw = true;
      }
      // T - view stats
      else if (inchar == 't' || inchar == 'T') {
        currentScreen = Screen::Stats;
        needsRedraw = true;
      }
      break;
      
    case Screen::Stats:
      // Any key returns to deck list
      if (inchar == 13 || inchar == ' ' || inchar == 12) {
        currentScreen = Screen::DeckList;
        needsRedraw = true;
      }
      break;
      
    case Screen::Study:
      if (!showingAnswer) {
        // Space or Enter to reveal answer
        if (inchar == ' ' || inchar == 13) {
          showingAnswer = true;
          needsRedraw = true;
        }
      } else {
        // Y or Right arrow = correct
        if (inchar == 'y' || inchar == 'Y' || inchar == 21) {
          nextCard(true);
        }
        // N or Left arrow = incorrect
        else if (inchar == 'n' || inchar == 'N' || inchar == 19) {
          nextCard(false);
        }
        // Space = incorrect (quick mode)
        else if (inchar == ' ') {
          nextCard(false);
        }
      }
      break;
      
    case Screen::Results:
      // Any key returns to deck list
      if (inchar == 13 || inchar == ' ') {
        currentScreen = Screen::DeckList;
        needsRedraw = true;
      }
      break;
  }
  
  // Update OLED
  u8g2.clearBuffer();
  switch(currentScreen) {
    case Screen::DeckList:
      u8g2.drawStr(0, 12, "FlashCards");
      break;
    case Screen::Study:
      {
        char status[32];
        snprintf(status, sizeof(status), "Card %d/%d", 
                 currentCardIndex + 1, (int)currentCards.size());
        u8g2.drawStr(0, 12, status);
      }
      break;
    case Screen::Results:
      u8g2.drawStr(0, 12, "Study Complete!");
      break;
    case Screen::Stats:
      u8g2.drawStr(0, 12, "Statistics");
      break;
  }
  u8g2.sendBuffer();
}

// ===================== E-INK DISPLAY =====================
void drawDeckListScreen() {
  display.fillRect(0, 0, 320, 20, GxEPD_BLACK);
  display.setTextColor(GxEPD_WHITE);
  display.setCursor(5, 15);
  display.print("FlashCards - Select Deck");
  display.setTextColor(GxEPD_BLACK);
  
  if (availableDecks.empty()) {
    display.setCursor(20, 60);
    display.print("No decks found!");
    display.setCursor(20, 85);
    display.print("Add CSV files to /flashcards/");
    display.setCursor(20, 110);
    display.print("Format: question,answer");
    display.setCursor(20, 150);
    display.print("Example:");
    display.setCursor(20, 170);
    display.print("  What is 2+2?,4");
    display.setCursor(20, 190);
    display.print("  Capital of France?,Paris");
  } else {
    display.setCursor(10, 45);
    display.print("Available Decks:");
    
    int y = 70;
    for (size_t i = 0; i < availableDecks.size() && y < 200; i++) {
      display.setCursor(15, y);
      if ((int)i == selectedDeckIndex) {
        display.print("> ");
      } else {
        display.print("  ");
      }
      display.print(availableDecks[i].name.c_str());
      y += 22;
    }
  }
  
  display.setCursor(5, 230);
  display.print("ENTER:Study  T:Stats  HOME:Exit");
}

void drawStatsScreen() {
  display.fillRect(0, 0, 320, 20, GxEPD_BLACK);
  display.setTextColor(GxEPD_WHITE);
  display.setCursor(5, 15);
  display.print("Statistics");
  display.setTextColor(GxEPD_BLACK);
  
  // Lifetime stats
  display.setCursor(10, 45);
  display.print("Lifetime Stats:");
  
  char line[64];
  display.setCursor(20, 68);
  snprintf(line, sizeof(line), "Sessions: %d", lifetimeSessions);
  display.print(line);
  
  display.setCursor(20, 88);
  snprintf(line, sizeof(line), "Cards Studied: %d", lifetimeStudied);
  display.print(line);
  
  display.setCursor(20, 108);
  if (lifetimeStudied > 0) {
    int pct = (lifetimeCorrect * 100) / lifetimeStudied;
    snprintf(line, sizeof(line), "Overall Accuracy: %d%%", pct);
  } else {
    snprintf(line, sizeof(line), "Overall Accuracy: --");
  }
  display.print(line);
  
  // Per-deck stats
  display.setCursor(10, 135);
  display.print("Per-Deck Best Scores:");
  
  int y = 155;
  if (allStats.empty()) {
    display.setCursor(20, y);
    display.print("No deck stats yet");
  } else {
    for (size_t i = 0; i < allStats.size() && y < 210; i++) {
      display.setCursor(20, y);
      snprintf(line, sizeof(line), "%s: %d%% (%d sessions)",
               allStats[i].deckName.c_str(),
               allStats[i].bestScore,
               allStats[i].sessionsCount);
      display.print(line);
      y += 18;
    }
  }
  
  display.setCursor(5, 230);
  display.print("ENTER: Back to Decks");
}

void drawStudyScreen() {
  display.fillRect(0, 0, 320, 20, GxEPD_BLACK);
  display.setTextColor(GxEPD_WHITE);
  display.setCursor(5, 15);
  
  char header[48];
  snprintf(header, sizeof(header), "Card %d of %d | Correct: %d", 
           currentCardIndex + 1, (int)currentCards.size(), correctCount);
  display.print(header);
  display.setTextColor(GxEPD_BLACK);
  
  if (currentCardIndex < (int)currentCards.size()) {
    const FlashCard& card = currentCards[currentCardIndex];
    
    // Question
    display.setCursor(10, 50);
    display.print("Q:");
    
    std::vector<String> qLines = wrapText(card.question, MAX_LINE_CHARS);
    int y = 50;
    for (const auto& line : qLines) {
      display.setCursor(30, y);
      display.print(line.c_str());
      y += 18;
    }
    
    // Answer (if revealed)
    if (showingAnswer) {
      y += 15;
      display.setCursor(10, y);
      display.print("A:");
      
      std::vector<String> aLines = wrapText(card.answer, MAX_LINE_CHARS);
      for (const auto& line : aLines) {
        display.setCursor(30, y);
        display.print(line.c_str());
        y += 18;
      }
      
      // Instructions
      display.setCursor(5, 210);
      display.print("Did you know it?");
      display.setCursor(5, 230);
      display.print("Y/RIGHT:Yes  N/LEFT:No");
    } else {
      display.setCursor(5, 230);
      display.print("SPACE/ENTER: Show Answer");
    }
  } else {
    // No cards loaded
    display.setCursor(20, 80);
    display.print("No cards in this deck!");
    display.setCursor(20, 110);
    display.print("Check CSV format:");
    display.setCursor(20, 130);
    display.print("  question,answer");
    display.setCursor(5, 230);
    display.print("HOME: Back to decks");
  }
}

void drawResultsScreen() {
  display.fillRect(0, 0, 320, 20, GxEPD_BLACK);
  display.setTextColor(GxEPD_WHITE);
  display.setCursor(5, 15);
  display.print("Study Session Complete!");
  display.setTextColor(GxEPD_BLACK);
  
  int total = correctCount + incorrectCount;
  int pct = (total > 0) ? (correctCount * 100 / total) : 0;
  
  display.setCursor(40, 70);
  char line[48];
  snprintf(line, sizeof(line), "Total Cards: %d", total);
  display.print(line);
  
  display.setCursor(40, 100);
  snprintf(line, sizeof(line), "Correct: %d", correctCount);
  display.print(line);
  
  display.setCursor(40, 130);
  snprintf(line, sizeof(line), "Incorrect: %d", incorrectCount);
  display.print(line);
  
  display.setCursor(40, 170);
  snprintf(line, sizeof(line), "Score: %d%%", pct);
  display.print(line);
  
  // Rating
  display.setCursor(40, 200);
  if (pct >= 90) {
    display.print("Excellent!");
  } else if (pct >= 70) {
    display.print("Good job!");
  } else if (pct >= 50) {
    display.print("Keep practicing!");
  } else {
    display.print("More study needed");
  }
  
  display.setCursor(5, 230);
  display.print("ENTER: Back to Decks");
}

void applicationEinkHandler() {
  if (!needsRedraw) return;
  needsRedraw = false;
  
  display.setRotation(3);
  display.fillScreen(GxEPD_WHITE);
  display.setFont(&FreeMono9pt7b);
  display.setTextColor(GxEPD_BLACK);
  
  switch(currentScreen) {
    case Screen::DeckList:
      drawDeckListScreen();
      break;
    case Screen::Study:
      drawStudyScreen();
      break;
    case Screen::Results:
      drawResultsScreen();
      break;
    case Screen::Stats:
      drawStatsScreen();
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