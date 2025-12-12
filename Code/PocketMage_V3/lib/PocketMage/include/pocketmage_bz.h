//  888888ba  dP     dP d8888888P d8888888P  88888888b  888888ba  //
//  88    `8b 88     88      .d8'      .d8'  88         88    `8b //
// a88aaaa8P' 88     88    .d8'      .d8'   a88aaaa    a88aaaa8P' //
//  88   `8b. 88     88  .d8'      .d8'      88         88   `8b. //
//  88    .88 Y8.   .8P d8'       d8'        88         88     88 //
//  88888888P `Y88888P' Y8888888P Y8888888P  88888888P  dP     dP //

#pragma once
#include <Arduino.h>
#include <Buzzer.h>
#include <driver/ledc.h>

#define PWM_CHANNEL LEDC_CHANNEL_1
#define PWM_RESOLUTION LEDC_TIMER_10_BIT // 10-bit resolution (0-1023)



// To Do: copy instructions to readme
/* ADDING NEW JINGLES

   constexpr static const Note Jingles::myMelodyNotes[] = {
      {NOTE_A8, 120}, {NOTE_B8, 120}, {NOTE_C8, 120}, {NOTE_D8, 120}
   }; // This is constexpr so its compile time, not run time
   const Jingles::myMelody = { myMelodyNotes, sizeof(myMelodyNotes) / sizeof(myMelodyNotes[0]);

 * */

struct Note {
  int key;
  int duration;
};

struct Jingle {
  const Note* notes;  // const because we don't want to modify the tune
  size_t len;
};

namespace Jingles {
constexpr static const Note startupNotes[] = {
    {NOTE_A8, 120}, {NOTE_B8, 120}, {NOTE_C8, 120}, {NOTE_D8, 120}};

constexpr static const Note shutdownNotes[] = {
    {NOTE_D8, 120}, {NOTE_C8, 120}, {NOTE_B8, 120}, {NOTE_A8, 120}};

const Jingle Startup = {startupNotes, sizeof(startupNotes) / sizeof(startupNotes[0])};
const Jingle Shutdown = {shutdownNotes, sizeof(shutdownNotes) / sizeof(shutdownNotes[0])};

};  // namespace jingle

// ===================== BZ CLASS =====================
class PocketmageBZ {
 public:
  PocketmageBZ();

  // Main methods
  bool begin(int channel = 0);
  void end();

  void playJingle(const Jingle& jingle);

 private:
  Buzzer buzzer_;
  int channel_ = 0;
  bool begun_ = false;
};

void setupBZ();
PocketmageBZ& BZ();