# powder

## Requirements
`powder` requires
- [SDL2](https://github.com/libsdl-org/SDL/releases/tag/release-2.24.2)
- [SDL_ttf](https://github.com/libsdl-org/SDL_ttf/releases)

You'll need to copy the `lib` and `include` directories into the project directory, as well as any DLLs.

Additionally, you'll need a copy of Arial (or any TTF named "arial.ttf", I suppose) in the root directory of the project.

## Build & Run

To build and run (on mingw):
```
gcc *.c -Iinclude -Llib -lmingw32 -lSDL2main -lSDL2 -lSDL2_ttf -o powder
./powder
```

## Game

**Particle Types:** `SAND`, `WATER`, `WOOD`, `FIRE`

Left mouse button: place particles

Digit keys (1-4): change particle type