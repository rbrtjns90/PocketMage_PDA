# Pocketmage Library
This is a basic overview of pocketmage library. Any suggestions/edits to further improve the modularity, readability, and safety of the library are welcome.

### To Do:

1. Progressively move globals out of globals.h replace them with variables owned by library. 
2. Refactor pocketmage:: to use only pocketmage::method().
3. Simplify library usage and document methods
4. Improved compatibility with emulator

## Essentials:

1. Access each library class methods with:

> CLASS().method()

2. Pocketmage system is a namespace

> pocketmage::typeOfMethod::method()<br />
> current pocketmage method types:
> - file
> - time
> - power
> - debug

## Classes:

- buzzer BZ()
- real time clock CLOCK()
- eink EINK()
- keyboard KB()
- oled OLED()
- sd SD()
- capacitive touch TOUCH()
- MP2722 
