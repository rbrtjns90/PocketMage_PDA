#pragma once
#include <Arduino.h>
#include <vector>
#include <FS.h>

class Adafruit_MPR121;   
class PocketmageEink;

extern Adafruit_MPR121 cap; // Touch slider

// ===================== CAPACATIVE TOUCH CLASS =====================
class PocketmageTOUCH {
public:
  explicit PocketmageTOUCH(Adafruit_MPR121 &cap) : cap_(cap) {}

  // Main methods
  void updateScrollFromTouch();
  bool updateScroll(int maxScroll, ulong& lineScroll);
  // getters 
  long int getDynamicScroll() const { return dynamicScroll_; }
  void setDynamicScroll(long int val) { dynamicScroll_ = val; }
  void setPrevDynamicScroll(long int val) { prev_dynamicScroll_ = val; }
  void setLastTouch(int val) { lastTouch_ = val; }
  void setLastTouchTime(unsigned long val) { lastTouchTime_ = val; }
  void resetLastTouch()  { lastTouch_ = -1; }
  long int getPrevDynamicScroll() const { return prev_dynamicScroll_; }
  int getLastTouch() const { return lastTouch_; }
  int getLastTouchTime() const { return lastTouchTime_; }
  int getDiff() const { return dynamicScroll_ - prev_dynamicScroll_; }
private:
  Adafruit_MPR121      &cap_;                          // class reference to hardware touch object
  volatile long int dynamicScroll_ = 0;         // Dynamic scroll offset
  volatile long int prev_dynamicScroll_ = 0;    // Previous scroll offset
  int lastTouch_ = -1;                          // Last touch event
  unsigned long lastTouchTime_ = 0;             // Last touch time
};

void setupTouch();
PocketmageTOUCH& TOUCH();