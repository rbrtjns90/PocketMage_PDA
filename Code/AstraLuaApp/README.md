# AstraLua 1.0

A Lua-like interpreter / REPL for PocketMage.

## Features

- **Math expressions** - Full expression evaluator with proper operator precedence
- **Variables** - Store and use numeric and string variables
- **Built-in functions** - sin, cos, tan, sqrt, abs, log, floor, ceil
- **Constants** - pi, e
- **Control flow** - Simple for loops and if statements
- **Print command** - Output strings and expressions
- **Scrollable console** - View command history

## Commands

| Command | Description |
|---------|-------------|
| `help` or `?` | Show help |
| `vars` | List all variables |
| `clear` or `cls` | Clear console |
| `exit` or `quit` | Return to PocketMage OS |

## Math Operations

| Operator | Description |
|----------|-------------|
| `+` | Addition |
| `-` | Subtraction |
| `*` | Multiplication |
| `/` | Division |
| `^` | Power/Exponent |
| `%` | Modulo |

## Built-in Functions

- `sin(x)` - Sine (radians)
- `cos(x)` - Cosine (radians)
- `tan(x)` - Tangent (radians)
- `sqrt(x)` - Square root
- `abs(x)` - Absolute value
- `log(x)` - Natural logarithm
- `floor(x)` - Round down
- `ceil(x)` - Round up

## Constants

- `pi` - 3.14159265358979
- `e` - 2.71828182845905

## Examples

### Basic Math
```lua
2 + 2
= 4

10 * 5 + 3
= 53

2 ^ 10
= 1024

sqrt(144)
= 12
```

### Variables
```lua
x = 10
x = 10

y = x * 2 + 5
y = 25

sqrt(x^2 + y^2)
= 26.9258
```

### Print
```lua
print("Hello World")
Hello World

print(pi * 2)
6.28319
```

### For Loop
```lua
for i=1,5 do print(i) end
> print(1)
1
> print(2)
2
...
```

### If Statement
```lua
x = 10
if x then print("x is set") end
x is set
```

## Controls

- **Type** - Enter Lua-like commands
- **ENTER** - Execute command
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

## Future Enhancements

To add full Lua support:
1. Add Lua source files to the project
2. Include lua.h, lauxlib.h, lualib.h
3. Replace the expression evaluator with luaL_dostring()
4. Add PocketMage-specific Lua bindings (display, buzzer, etc.)
