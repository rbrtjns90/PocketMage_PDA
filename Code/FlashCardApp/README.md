# PocketMage FlashCard App

A flashcard study app that loads question/answer pairs from CSV files.

## Features

- Load flashcard decks from CSV files on SD card
- Study mode with question/answer reveal
- Track correct/incorrect answers
- Score and rating at end of session
- Word wrapping for long questions/answers

## CSV Format

Place CSV files in `/flashcards/` directory on the SD card.

### Simple format:
```csv
question,answer
What is 2+2?,4
Capital of France?,Paris
Who wrote Hamlet?,Shakespeare
```

### With header (auto-detected and skipped):
```csv
Question,Answer
What is the speed of light?,299792458 m/s
What year did WW2 end?,1945
```

### With quotes (for commas in text):
```csv
"What is the formula for water?","H2O"
"Who said ""I think, therefore I am""?","Descartes"
```

## Controls

### Deck Selection
- **Up/Down** - Select deck
- **ENTER** - Start studying
- **R** - Refresh deck list
- **HOME** - Exit to PocketMage

### Study Mode
- **SPACE/ENTER** - Reveal answer
- **Y** or **RIGHT** - Mark as correct
- **N** or **LEFT** - Mark as incorrect
- **HOME** - Back to deck list

### Results Screen
- **ENTER** - Back to deck list

## Example Decks

Create these files in `/flashcards/`:

### vocabulary.csv
```csv
ephemeral,lasting for a very short time
ubiquitous,present everywhere
pragmatic,dealing with things sensibly
```

### capitals.csv
```csv
France,Paris
Germany,Berlin
Japan,Tokyo
Australia,Canberra
```

### math.csv
```csv
Square root of 144,12
15 x 15,225
What is pi to 2 decimal places?,3.14
```

## Building

```bash
cd Code/FlashCardApp
pio run
```

## Installing to Desktop Emulator

```bash
cd desktop_emulator
python3 install_app.py ../Code/FlashCardApp
```

Then create a `flashcards` folder in the emulator's `data` directory and add CSV files.
