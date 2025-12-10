// PocketMage V3.0
// @Ashtf 2025

#include <globals.h>

static constexpr const char* TAG = "MAIN"; // TODO: Come up with a better tag

//        .o.       ooooooooo.   ooooooooo.    .oooooo..o  //
//       .888.      `888   `Y88. `888   `Y88. d8P'    `Y8  //
//      .8"888.      888   .d88'  888   .d88' Y88bo.       //
//     .8' `888.     888ooo88P'   888ooo88P'   `"Y8888o.   //
//    .88ooo8888.    888          888              `"Y88b  //
//   .8'     `888.   888          888         oo     .d8P  //
//  o88o     o8888o o888o        o888o        8""88888P'   //


// ADD E-INK HANDLER APP SCRIPTS HERE
void applicationEinkHandler() {
  switch (CurrentAppState) {
    case HOME:
      einkHandler_HOME();
      break;
    case TXT:
      einkHandler_TXT_NEW();
      break;
    case FILEWIZ:
      einkHandler_FILEWIZ();
      break;
    case TASKS:
      einkHandler_TASKS();
      break;
    case SETTINGS:
      einkHandler_settings();
      break;
    case USB_APP:
      einkHandler_USB();
      break;
    case CALENDAR:
      einkHandler_CALENDAR();
      break;
    case LEXICON:
      einkHandler_LEXICON();
      break;
    case JOURNAL:
      einkHandler_JOURNAL();
      break;
    case APPLOADER:
      einkHandler_APPLOADER();
      break;
    case HELLO:
      einkHandler_HELLO();
      break;

        case ASTRALUAAPP:
            einkHandler_ASTRALUAAPP();
            break;

        case FLASHCARDAPP:
            einkHandler_FLASHCARDAPP();
            break;

        case GLUCOSEAPP:
            einkHandler_GLUCOSEAPP();
            break;

        case MUSICAPP:
            einkHandler_MUSICAPP();
            break;

        case STARTERAPP:
            einkHandler_STARTERAPP();
            break;
        case APPLAUNCHER:
            einkHandler_APPLAUNCHER();
            break;
    // ADD APP CASES HERE
    default:
      einkHandler_HOME();
      break;
  }
}

// ADD PROCESS/KEYBOARD APP SCRIPTS HERE
void processKB() {
  // Check for USB KB
  KB().checkUSBKB();

  switch (CurrentAppState) {
    case HOME:
      processKB_HOME();
      break;
    case TXT:
      processKB_TXT_NEW();
      break;
    case FILEWIZ:
      processKB_FILEWIZ();
      break;
    case TASKS:
      processKB_TASKS();
      break;
    case SETTINGS:
      processKB_settings();
      break;
    case USB_APP:
      processKB_USB();
      break;
    case CALENDAR:
      processKB_CALENDAR();
      break;
    case LEXICON:
      processKB_LEXICON();
      break;
    case JOURNAL:
      processKB_JOURNAL();
      break;
    case APPLOADER:
      processKB_APPLOADER();
      break;
    case HELLO:
      processKB_HELLO();
      break;

        case ASTRALUAAPP:
            processKB_ASTRALUAAPP();
            break;

        case FLASHCARDAPP:
            processKB_FLASHCARDAPP();
            break;

        case GLUCOSEAPP:
            processKB_GLUCOSEAPP();
            break;

        case MUSICAPP:
            processKB_MUSICAPP();
            break;

        case STARTERAPP:
            processKB_STARTERAPP();
            break;
        case APPLAUNCHER:
            processKB_APPLAUNCHER();
            break;
    // ADD APP CASES HERE
    default:
      processKB_HOME();
      break;
  }
}

//  ooo        ooooo       .o.       ooooo ooooo      ooo  //
//  `88.       .888'      .888.      `888' `888b.     `8'  //
//   888b     d'888      .8"888.      888   8 `88b.    8   //
//   8 Y88. .P  888     .8' `888.     888   8   `88b.  8   //
//   8  `888'   888    .88ooo8888.    888   8     `88b.8   //
//   8    Y     888   .8'     `888.   888   8       `888   //
//  o8o        o888o o88o     o8888o o888o o8o        `8   //

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////|
// SETUP
void setup() {
  PocketMage_INIT();
}

// Keyboard / OLED Loop
void loop() {
  if (!noTimeout)  pocketmage::time::checkTimeout();
  if (DEBUG_VERBOSE) pocketmage::debug::printDebug();

  PowerSystem.printDiagnostics(); // power diag
  
  pocketmage::power::updateBattState();
  processKB();

  // Yield to watchdog
  vTaskDelay(50 / portTICK_PERIOD_MS);
  yield();
}

// E-Ink Loop
void einkHandler(void* parameter) {
  vTaskDelay(pdMS_TO_TICKS(250)); 
  for (;;) {
    applicationEinkHandler();

    vTaskDelay(pdMS_TO_TICKS(50));
    yield();
  }
}