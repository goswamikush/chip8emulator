# CHIP-8 Emulator

A CHIP-8 interpreter written in C with SDL2 for graphics rendering. CHIP-8 is an interpreted programming language developed in the 1970s for simple video games.

## Features

- Full CHIP-8 instruction set implementation
- 64x64 monochrome display
- Keyboard input support
- Configurable execution speed
- Clean, modular codebase with separated concerns

## Prerequisites

- GCC or Clang compiler
- SDL2 library

### Installing SDL2

**macOS (Homebrew):**
```bash
brew install sdl2
```

**Ubuntu/Debian:**
```bash
sudo apt-get install libsdl2-dev
```

**Arch Linux:**
```bash
sudo pacman -S sdl2
```

**Windows:**
Download SDL2 development libraries from [libsdl.org](https://www.libsdl.org/download-2.0.php)

## Building

**macOS (Homebrew):**
```bash
gcc main.c chip8.c -o chip8 -I/opt/homebrew/include -L/opt/homebrew/lib -lSDL2
```

**Linux:**
```bash
gcc main.c chip8.c -o chip8 -lSDL2
```

**Windows (MinGW):**
```bash
gcc main.c chip8.c -o chip8.exe -I<path-to-SDL2>/include -L<path-to-SDL2>/lib -lSDL2
```

## Running

```bash
./chip8 <path-to-rom>
```

**Example:**
```bash
./chip8 roms/pong.ch8
```

## Finding ROMs

Here are some great sources for CHIP-8 ROMs:

- **[Zophar's Domain](https://www.zophar.net/pdroms/chip8.html)** - Large collection of public domain ROMs
- **[CHIP-8 Games Pack](https://github.com/kripod/chip8-roms)** - Curated collection on GitHub
- **[Chip-8 Program Pack](https://johnearnest.github.io/chip8Archive/)** - Archive with 400+ programs
- **[David Winter's CHIP-8 Collection](https://www.pong-story.com/chip8/)** - Classic games and demos

Popular ROMs to try:
- `PONG` - Classic Pong game
- `INVADERS` - Space Invaders clone
- `TETRIS` - Tetris clone
- `BRIX` - Breakout clone
- `TICTAC` - Tic-tac-toe

## Controls

The CHIP-8 keypad is mapped to your keyboard as follows:

```
CHIP-8 Keypad:          Keyboard Mapping:
┌─┬─┬─┬─┐              ┌─┬─┬─┬─┐
│1│2│3│C│              │1│2│3│4│
├─┼─┼─┼─┤              ├─┼─┼─┼─┤
│4│5│6│D│              │Q│W│E│R│
├─┼─┼─┼─┤              ├─┼─┼─┼─┤
│7│8│9│E│              │A│S│D│F│
├─┼─┼─┼─┤              ├─┼─┼─┼─┤
│A│0│B│F│              │Z│X│C│V│
└─┴─┴─┴─┘              └─┴─┴─┴─┘
```

## Project Structure

```
chip8emulator/
├── chip8.h          # CHIP-8 core definitions and API
├── chip8.c          # CHIP-8 emulator implementation
├── main.c           # SDL setup, rendering, and main loop
├── README.md        # This file
└── roms/            # ROM files (not included)
```

## Implementation Details

- **Memory:** 4KB RAM with programs loaded at address 0x200
- **Registers:** 16 general-purpose 8-bit registers (V0-VF)
- **Display:** 64x64 pixel monochrome display
- **Timers:** 60Hz delay and sound timers
- **Stack:** 16 levels for subroutine calls

## Resources

- [CHIP-8 Technical Reference](http://devernay.free.fr/hacks/chip8/C8TECH10.HTM)
- [Cowgod's CHIP-8 Reference](http://devernay.free.fr/hacks/chip8/C8TECH10.HTM)
- [CHIP-8 Wikipedia](https://en.wikipedia.org/wiki/CHIP-8)

## License

This project is open source and available under the MIT License.

## Acknowledgments

- Built as a learning project to understand emulator development
- Thanks to the CHIP-8 community for documentation and test ROMs
