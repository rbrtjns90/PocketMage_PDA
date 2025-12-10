# AstraLua 2.0

A full Lua 5.4 interpreter for PocketMage.

## Features

- **Full Lua 5.4** - Complete Lua interpreter using minilua
- **Interactive REPL** - Execute Lua code line by line
- **Script loading** - Load and run .lua files from SD card
- **PocketMage bindings** - Access display, buzzer, and system functions
- **Scrollable console** - View output history with Up/Down keys
- **Word wrapping** - Long output automatically wraps

## Commands

| Command | Description |
|---------|-------------|
| `help` | Show help |
| `clear` | Clear console |
| `exit` | Return to PocketMage OS |
| `load <file>` | Load and run a .lua file |

## Lua Standard Library

All standard Lua 5.4 functions are available:

- **Math** - math.sin, math.cos, math.sqrt, math.abs, math.floor, math.ceil, math.random, etc.
- **String** - string.sub, string.len, string.upper, string.lower, string.format, etc.
- **Table** - table.insert, table.remove, table.sort, etc.
- **I/O** - print, io.open, io.read, io.write (redirected to console)

## PocketMage Bindings

```lua
-- Display
pm.oled("Hello")       -- Show text on OLED
pm.eink("Hello")       -- Show text on E-ink

-- Buzzer
pm.beep()              -- Short beep
pm.tone(440, 100)      -- Play 440Hz for 100ms

-- System
pm.delay(1000)         -- Delay 1 second
pm.millis()            -- Get milliseconds since boot
```

## Examples

### Basic Math
```lua
print(2 + 2)
-- 4

print(math.sqrt(144))
-- 12

print(math.pi * 2)
-- 6.283185307179586
```

### Variables and Functions
```lua
x = 10
y = 20
print(x + y)
-- 30

function square(n)
  return n * n
end
print(square(5))
-- 25
```

### Loops
```lua
for i = 1, 5 do
  print(i)
end
-- 1 2 3 4 5

local sum = 0
for i = 1, 100 do
  sum = sum + i
end
print(sum)
-- 5050
```

### Tables
```lua
t = {10, 20, 30}
for i, v in ipairs(t) do
  print(i, v)
end

person = {name = "Alice", age = 30}
print(person.name)
-- Alice
```

### Loading Scripts
```lua
load bmi.lua
-- Runs /lua/bmi.lua from SD card
```

## Controls

- **Type** - Enter Lua code
- **ENTER** - Execute code
- **BACKSPACE** - Delete character
- **Up/Down** - Scroll console history
- **HOME** - Exit to PocketMage OS

## Building

```bash
cd Code/AstraLuaApp
pio run
```

## Installing to Desktop Emulator

```bash
cd desktop_emulator
python3 install_app.py ../Code/AstraLuaApp
```

## Script Location

Place .lua scripts in `/lua/` on the SD card.

## Example Scripts

The following demo scripts are included:

### Math & Science
- **`bmi.lua`** - Body Mass Index calculator. Calculates BMI from weight/height and shows health status.
- **`circle.lua`** - Circle calculations. Computes circumference and area from radius.
- **`distance.lua`** - Distance formula. Calculates distance between two points.
- **`pythagoras.lua`** - Pythagorean theorem. Finds hypotenuse of a right triangle.
- **`quadratic.lua`** - Quadratic formula solver. Solves ax² + bx + c = 0.
- **`trig.lua`** - Trigonometry demo. Shows sin, cos, tan at various angles.

### Finance
- **`compound.lua`** - Compound interest calculator. Calculates future value of investments.
- **`loan.lua`** - Loan payment calculator. Computes monthly payments, total paid, and interest.

### Programming Examples
- **`factorial.lua`** - Factorial calculator using a for loop.
- **`squares.lua`** - Prints squares of numbers 1-10.
- **`temperature.lua`** - Temperature conversion (Fahrenheit to Celsius).
