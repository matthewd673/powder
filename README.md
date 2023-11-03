# powder

## Requirements
`powder` requires
- [raylib](https://www.raylib.com/index.html)

## Build & Run

To build and run:
```
cc *.c `pkg-config --libs --cflags raylib` -o powder
./powder
```

## Game

**Particle Types:** `SAND`, `WATER`, `WOOD`, `FIRE`, `SMOKE`, `METAL`, `ACID`,
`DIRT`, `SEED`, `PLANT`, `LAVA`

Left mouse button: place particles

Digit keys (1-9): change particle type
