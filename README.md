# powder

## Build & Run
`powder` requires [SDL2](https://github.com/libsdl-org/SDL/releases/tag/release-2.24.2). Once SDL2 is downloaded, you'll need to copy `lib`, `include`, and `SDL2.dll` into the project directory.

To actually build and run (on mingw):
```
gcc *.c -Iinclude -Llib -lmingw32 -lSDL2main -lSDL2 -o powder
./powder
```

## Game

**Particle Types:** `SAND`, `WATER`, `WOOD`

Left mouse button: place particles

Digit keys (1-3): change particle type