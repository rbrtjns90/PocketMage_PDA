/**
 * @file Buzzer.h
 * @brief Buzzer/speaker emulation using SDL2 audio
 * 
 * Generates square wave tones to simulate a piezo buzzer.
 */

#ifndef BUZZER_H
#define BUZZER_H

#pragma message("Using emulator Buzzer.h")

#include "pocketmage/pocketmage_compat.h"
#include <SDL.h>
#include <cmath>
#include <atomic>

class Buzzer {
public:
    Buzzer() : _pin(-1), _playing(false), _audioInitialized(false), _deviceId(0) {}
    Buzzer(int pin) : _pin(pin), _playing(false), _audioInitialized(false), _deviceId(0) {}
    
    ~Buzzer() {
        if (_audioInitialized && _deviceId != 0) {
            SDL_CloseAudioDevice(_deviceId);
        }
    }
    
    void begin(int channel = 0) {
        _channel = channel;
        initAudio();
    }
    
    void end(int channel = 0) {
        noTone();
    }
    
    // Main sound method used by PocketMage
    void sound(unsigned int frequency, unsigned long duration) {
        std::cout << "[Buzzer] sound(" << frequency << "Hz, " << duration << "ms)" << std::endl;
        if (frequency == 0) {
            noTone();
            if (duration > 0) delay(duration);
            return;
        }
        tone(frequency, duration);
    }
    
    void tone(unsigned int frequency, unsigned long duration = 0) {
        if (!_audioInitialized) initAudio();
        
        _frequency = frequency;
        _playing = true;
        _sampleIndex = 0;
        
        if (_deviceId != 0) {
            SDL_PauseAudioDevice(_deviceId, 0);  // Start playing
        }
        
        // If duration specified, play for that time then stop
        if (duration > 0) {
            delay(duration);
            noTone();
        }
    }
    
    void noTone() {
        _playing = false;
        _frequency = 0;
        if (_deviceId != 0) {
            SDL_PauseAudioDevice(_deviceId, 1);  // Pause
        }
    }
    
    void playNote(int note, int duration) {
        tone(note, duration);
    }
    
    void playMelody(const int* notes, const int* durations, int length) {
        for (int i = 0; i < length; i++) {
            if (notes[i] > 0) {
                playNote(notes[i], durations[i]);
            } else {
                delay(durations[i]); // Rest
            }
            delay(50); // Small gap between notes
        }
    }
    
    bool isPlaying() const { return _playing; }
    unsigned int getFrequency() const { return _frequency; }
    
    // Volume control (0.0 to 1.0)
    void setVolume(float vol) { _volume = std::max(0.0f, std::min(1.0f, vol)); }
    float getVolume() const { return _volume; }
    
    // Enable/disable audio
    void setEnabled(bool enabled) { _enabled = enabled; if (!enabled) noTone(); }
    bool isEnabled() const { return _enabled; }
    
private:
    [[maybe_unused]] int _pin;
    [[maybe_unused]] int _channel = 0;
    std::atomic<bool> _playing;
    std::atomic<unsigned int> _frequency;
    bool _audioInitialized;
    SDL_AudioDeviceID _deviceId;
    float _volume = 0.3f;  // Default 30% volume (piezo buzzers are loud!)
    bool _enabled = true;
    uint32_t _sampleIndex = 0;
    
    static constexpr int SAMPLE_RATE = 44100;
    static constexpr int AMPLITUDE = 28000;  // Max amplitude for int16
    
    void initAudio() {
        if (_audioInitialized) return;
        
        SDL_AudioSpec want, have;
        SDL_memset(&want, 0, sizeof(want));
        want.freq = SAMPLE_RATE;
        want.format = AUDIO_S16SYS;
        want.channels = 1;
        want.samples = 512;
        want.callback = audioCallback;
        want.userdata = this;
        
        _deviceId = SDL_OpenAudioDevice(nullptr, 0, &want, &have, 0);
        if (_deviceId == 0) {
            std::cerr << "[Buzzer] Failed to open audio: " << SDL_GetError() << std::endl;
            return;
        }
        
        _audioInitialized = true;
        std::cout << "[Buzzer] Audio initialized (freq=" << have.freq << "Hz)" << std::endl;
    }
    
    static void audioCallback(void* userdata, Uint8* stream, int len) {
        Buzzer* buzzer = static_cast<Buzzer*>(userdata);
        int16_t* buffer = reinterpret_cast<int16_t*>(stream);
        int samples = len / sizeof(int16_t);
        
        if (!buzzer->_playing || !buzzer->_enabled || buzzer->_frequency == 0) {
            // Silence
            SDL_memset(stream, 0, len);
            return;
        }
        
        // Generate square wave
        float freq = static_cast<float>(buzzer->_frequency.load());
        float samplesPerCycle = SAMPLE_RATE / freq;
        float amplitude = AMPLITUDE * buzzer->_volume;
        
        for (int i = 0; i < samples; i++) {
            // Square wave: high for half cycle, low for other half
            float pos = fmod(buzzer->_sampleIndex++, samplesPerCycle);
            buffer[i] = (pos < samplesPerCycle / 2) ? 
                        static_cast<int16_t>(amplitude) : 
                        static_cast<int16_t>(-amplitude);
        }
    }
};

// Global buzzer instance
extern Buzzer buzzer;

// Jingle player function
void playJingle(const String& name);
void playJingle(const char* name);

#endif // BUZZER_H
