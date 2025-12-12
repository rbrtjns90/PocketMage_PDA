//  888888ba  d888888P  a88888b. //
//  88    `8b    88    d8'   `88 //
// a88aaaa8P'    88    88        //
//  88   `8b.    88    88        //
//  88     88    88    Y8.   .88 //
//  dP     dP    dP     Y88888P' //

#include "pocketmage.h"

static constexpr const char* TAG = "CLOCK";

RTC_PCF8563 rtc;

const char daysOfTheWeek[7][12] = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };

// Initialization of clock class
static PocketmageCLOCK pm_clock(rtc);

// Setup for Clock Class
void setupClock(){
  pinMode(RTC_INT, INPUT);
  if (!CLOCK().begin()) {
    ESP_LOGE(TAG, "Couldn't find RTC");
    delay(1000);
  }
  // SET CLOCK IF NEEDED
  if (SET_CLOCK_ON_UPLOAD || CLOCK().getRTC().lostPower()) {
    CLOCK().setToCompileTimeUTC();
  }
  CLOCK().getRTC().start();
  wireClock();
}

// Wire function  for Clock class
// add any global references here + add set function to class header file
void wireClock(){
    
}

// Access for other apps
PocketmageCLOCK& CLOCK() { return pm_clock; }

bool PocketmageCLOCK::begin() {
  if (!rtc_.begin()) { begun_ = false; return false; }
  begun_ = true;
  return true;
}

void PocketmageCLOCK::setTimeFromString(String timeStr) {
  if (timeStr.length() != 5 || timeStr[2] != ':') {
      ESP_LOGE(TAG, "Invalid format! Use HH:MM. Provided str: %s", timeStr.c_str());
      return;
  }

  int hours = timeStr.substring(0, 2).toInt();
  int minutes = timeStr.substring(3, 5).toInt();

  if (hours < 0 || hours > 23 || minutes < 0 || minutes > 59) {
      OLED().oledWord("Invalid");
      delay(500);
      return;
  }

  DateTime now = CLOCK().nowDT();  // Get current date
  CLOCK().getRTC().adjust(DateTime(now.year(), now.month(), now.day(), hours, minutes, 0));

  ESP_LOGI(TAG, "Time updated!");
}
    
bool PocketmageCLOCK::isValid() {
  if (!begun_) return false;
  DateTime t = rtc_.now();
  const bool saneYear = t.year() >= 2020 && t.year() < 2099;  // check for reasonable year for DateTime t
  return saneYear;
}

