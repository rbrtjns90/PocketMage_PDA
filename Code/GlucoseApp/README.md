# PocketMage Glucose Tracker

A blood glucose logging and tracking app for PocketMage.

## Features

### Manual Entry
- Blood glucose value (mg/dL)
- Time automatically captured from RTC
- Optional notes (up to 20 characters)

### Tagging
- **Fasting** - Morning/fasting readings
- **Pre-meal** - Before eating
- **Post-meal** - After eating
- **Bedtime** - Before sleep
- **Exercise** - During/after workout
- **Correction** - Correction bolus

### Views

#### Today View
- Current time and last reading
- List of today's readings with time, value, tag, and notes
- LOW/HIGH indicators for out-of-range values

#### History View
- Navigate by day with Left/Right arrows
- View past readings
- Scroll through entries

#### Summary View
- Statistics over 7, 14, or 30 days
- Average, min, max values
- In-range percentage
- Low/High percentages

### Storage
- CSV files stored in `/glucose/` folder
- One file per day: `/glucose/YYYY-MM-DD.csv`
- Easy to export via SD card

## Controls

### Today View
- **N** - New reading
- **H** - History view
- **S** - Summary view
- **Up/Down** - Scroll readings
- **HOME** - Exit to PocketMage

### New Reading
1. Type glucose value (numbers)
2. Press **ENTER**
3. Select tag: **0-6** or quick keys (**F**asting, **P**re, **A**fter, **B**ed, **E**xercise)
4. Type optional note
5. Press **ENTER** to save
- **HOME** - Cancel

### History View
- **Left/Right** - Previous/next day
- **Up/Down** - Scroll entries
- **HOME** - Back to Today

### Summary View
- **7** - 7-day summary
- **1** - 14-day summary
- **3** - 30-day summary
- **HOME** - Back to Today

## Settings

Default thresholds (can be modified in code):
- Low: < 70 mg/dL
- High: > 180 mg/dL

## CSV Format

```
hour,minute,value,unit,tag,note
9,2,92,0,1,
12,18,132,0,2,chicken wrap
13,22,168,0,3,pizza
```

## Building

```bash
cd Code/GlucoseApp
pio run
```

## Installing to Desktop Emulator

```bash
cd desktop_emulator
python3 install_app.py ../Code/GlucoseApp
```

## Future Enhancements

- Carb + insulin logging
- Reminders / alarms
- Bluetooth CGM integration
- Export to standard formats
- Trend graphs
