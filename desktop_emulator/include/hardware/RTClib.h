/**
 * @file RTClib.h
 * @brief RTC library mock for desktop emulator
 * 
 * Uses system time to provide real-time clock functionality.
 */

#ifndef RTCLIB_H
#define RTCLIB_H

#include "pocketmage/pocketmage_compat.h"
#include <ctime>

// Forward declaration
class TimeSpan;

// ============================================================================
// DateTime Class
// ============================================================================
class DateTime {
public:
    DateTime() {
        time_t now = time(nullptr);
        struct tm* t = localtime(&now);
        _year = t->tm_year + 1900;
        _month = t->tm_mon + 1;
        _day = t->tm_mday;
        _hour = t->tm_hour;
        _minute = t->tm_min;
        _second = t->tm_sec;
    }
    
    DateTime(uint16_t year, uint8_t month, uint8_t day,
             uint8_t hour = 0, uint8_t minute = 0, uint8_t second = 0) 
        : _year(year), _month(month), _day(day),
          _hour(hour), _minute(minute), _second(second) {}
    
    DateTime(const char* date, const char* time) {
        // Parse __DATE__ format: "Dec  7 2025"
        // Parse __TIME__ format: "12:34:56"
        _year = 2025;
        _month = 12;
        _day = 7;
        _hour = 12;
        _minute = 0;
        _second = 0;
        
        if (date) {
            char monthStr[4];
            int day, year;
            if (sscanf(date, "%3s %d %d", monthStr, &day, &year) == 3) {
                _year = year;
                _day = day;
                const char* months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                                        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
                for (int i = 0; i < 12; i++) {
                    if (strncmp(monthStr, months[i], 3) == 0) {
                        _month = i + 1;
                        break;
                    }
                }
            }
        }
        
        if (time) {
            int h, m, s;
            if (sscanf(time, "%d:%d:%d", &h, &m, &s) == 3) {
                _hour = h;
                _minute = m;
                _second = s;
            }
        }
    }
    
    uint16_t year() const { return _year; }
    uint8_t month() const { return _month; }
    uint8_t day() const { return _day; }
    uint8_t hour() const { return _hour; }
    uint8_t minute() const { return _minute; }
    uint8_t second() const { return _second; }
    
    uint8_t dayOfTheWeek() const {
        // Zeller's congruence
        int y = _year;
        int m = _month;
        if (m < 3) {
            m += 12;
            y--;
        }
        int k = y % 100;
        int j = y / 100;
        int h = (_day + (13 * (m + 1)) / 5 + k + k / 4 + j / 4 - 2 * j) % 7;
        return ((h + 6) % 7); // Convert to 0=Sunday
    }
    
    uint32_t unixtime() const {
        struct tm t;
        t.tm_year = _year - 1900;
        t.tm_mon = _month - 1;
        t.tm_mday = _day;
        t.tm_hour = _hour;
        t.tm_min = _minute;
        t.tm_sec = _second;
        t.tm_isdst = -1;
        return static_cast<uint32_t>(mktime(&t));
    }
    
    DateTime operator+(const TimeSpan& span) const;
    DateTime operator-(const TimeSpan& span) const;
    TimeSpan operator-(const DateTime& other) const;
    
    bool operator<(const DateTime& other) const { return unixtime() < other.unixtime(); }
    bool operator>(const DateTime& other) const { return unixtime() > other.unixtime(); }
    bool operator<=(const DateTime& other) const { return unixtime() <= other.unixtime(); }
    bool operator>=(const DateTime& other) const { return unixtime() >= other.unixtime(); }
    bool operator==(const DateTime& other) const { return unixtime() == other.unixtime(); }
    bool operator!=(const DateTime& other) const { return unixtime() != other.unixtime(); }
    
    String timestamp(uint8_t opt = 0) const {
        char buf[32];
        snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d",
                 _year, _month, _day, _hour, _minute, _second);
        return String(buf);
    }
    
private:
    uint16_t _year;
    uint8_t _month, _day, _hour, _minute, _second;
};

// ============================================================================
// TimeSpan Class
// ============================================================================
class TimeSpan {
public:
    TimeSpan(int32_t seconds = 0) : _seconds(seconds) {}
    
    TimeSpan(int16_t days, int8_t hours, int8_t minutes, int8_t seconds)
        : _seconds(days * 86400L + hours * 3600L + minutes * 60L + seconds) {}
    
    int16_t days() const { return _seconds / 86400L; }
    int8_t hours() const { return (_seconds % 86400L) / 3600L; }
    int8_t minutes() const { return (_seconds % 3600L) / 60L; }
    int8_t seconds() const { return _seconds % 60L; }
    int32_t totalseconds() const { return _seconds; }
    
    TimeSpan operator+(const TimeSpan& other) const {
        return TimeSpan(_seconds + other._seconds);
    }
    
    TimeSpan operator-(const TimeSpan& other) const {
        return TimeSpan(_seconds - other._seconds);
    }
    
private:
    int32_t _seconds;
};

// DateTime operators implementation
inline DateTime DateTime::operator+(const TimeSpan& span) const {
    uint32_t t = unixtime() + span.totalseconds();
    time_t tt = static_cast<time_t>(t);
    struct tm* tm = localtime(&tt);
    return DateTime(tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
                    tm->tm_hour, tm->tm_min, tm->tm_sec);
}

inline DateTime DateTime::operator-(const TimeSpan& span) const {
    uint32_t t = unixtime() - span.totalseconds();
    time_t tt = static_cast<time_t>(t);
    struct tm* tm = localtime(&tt);
    return DateTime(tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
                    tm->tm_hour, tm->tm_min, tm->tm_sec);
}

inline TimeSpan DateTime::operator-(const DateTime& other) const {
    return TimeSpan(static_cast<int32_t>(unixtime()) - static_cast<int32_t>(other.unixtime()));
}

// ============================================================================
// RTC Classes
// ============================================================================
class RTC_PCF8563 {
public:
    bool begin(TwoWire* wire = nullptr) { return true; }
    
    DateTime now() { return DateTime(); }
    
    void adjust(const DateTime& dt) {
        // In emulator, we don't actually adjust system time
    }
    
    void start() {}
    void stop() {}
    
    bool lostPower() { return false; }
    
    void writeSqwPinMode(uint8_t mode) {}
    uint8_t readSqwPinMode() { return 0; }
};

// Alias for DS3231 (same interface)
using RTC_DS3231 = RTC_PCF8563;
using RTC_DS1307 = RTC_PCF8563;

#endif // RTCLIB_H
