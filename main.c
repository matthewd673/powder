#include <stdio.h>

#include "raylib.h"

#include "sim.h"

#define WINDOW_WIDTH    800
#define WINDOW_HEIGHT   800
#define WINDOW_FPS      120

#define WORLD_WIDTH     400
#define WORLD_HEIGHT    400

#define PARTICLE_SCALE  2

// rendering color map
struct Color color_map[] = {
  { 0, 0, 0, 255 },       // PTYPE_NONE
  { 247, 213, 134, 255 }, // PTYPE_SAND
  { 137, 209, 245, 255 }, // PTYPE_WATER
  { 138, 91, 51, 255 },   // PTYPE_WOOD
  { 247, 83, 47, 255 },   // PTYPE_FIRE
  { 74, 74, 74, 255 },    // PTYPE_SMOKE
};

int main(int argc, char *argv[]) {
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "powder");
    SetTargetFPS(WINDOW_FPS);

    World w = new_World(WORLD_WIDTH, WORLD_HEIGHT);

    int brushSize = 1;
    char newParticleType = 0x1;

    while (!WindowShouldClose()) {
        // INPUT
        // change brush type
        if (IsKeyDown(KEY_ONE)) {
            newParticleType = PTYPE_SAND;
        }
        else if (IsKeyDown(KEY_TWO)) {
            newParticleType = PTYPE_WATER;
        }
        else if (IsKeyDown(KEY_THREE)) {
            newParticleType = PTYPE_WOOD;
        }
        else if (IsKeyDown(KEY_FOUR)) {
            newParticleType = PTYPE_FIRE;
        }
        // change brush size
        else if (IsKeyPressed(KEY_UP)) {
            brushSize++;
        }
        else if (IsKeyPressed(KEY_DOWN)) {
            brushSize = brushSize <= 1 ? 1 : brushSize - 1;
        }

        // mouse state input
        int mX = GetMouseX();
        int mY = GetMouseY();

        if (mX < 0) {
            mX = 0;
        }
        if (mX >= WINDOW_WIDTH) {
            mX = WINDOW_WIDTH - 1;
        }
        if (mY < 0) {
            mY = 0;
        }
        if (mY >= WINDOW_HEIGHT) {
            mY = WINDOW_HEIGHT - 1;
        }

        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            for (int i = 0; i < brushSize; i++) {
                for (int j = 0; j < brushSize; j++) {
                    int pX = mX / PARTICLE_SCALE + i;
                    int pY = mY / PARTICLE_SCALE + j;

                    if (pX >= WORLD_WIDTH ||
                        pY >= WORLD_HEIGHT) {
                            continue;
                        }

                    Particle existing = World_getParticle(w, pX, pY);
                    if (Particle_getType(existing) == PTYPE_NONE) {
                        Particle_setType(
                            existing,
                            newParticleType
                        );
                        World_setParticle(w, pX, pY, existing); // trigger cell activation
                    }
                }
            }
        }

        // UPDATE
        World_simulate(w);

        // DRAWING
        BeginDrawing();

        // loop through every cell
        for (short ci = 0; ci < World_getCellWidth(w); ci++) {
            for (short cj = 0; cj < World_getCellHeight(w); cj++) {
                if (!World_getCellStatusExact(w, ci, cj)) {
                    continue;
                }

                for (short i = ci * CELL_SIZE; i < (ci + 1) * CELL_SIZE; i++) {
                    for (short j = cj * CELL_SIZE; j < (cj + 1) * CELL_SIZE; j++) {
                        Color col = color_map[Particle_getType(World_getParticle(w, i, j))];
                        DrawRectangle(i * PARTICLE_SCALE, j * PARTICLE_SCALE,
                                      PARTICLE_SCALE, PARTICLE_SCALE,
                                      col);
                    }
                }
            }
        }

        EndDrawing();
    }

    // clean up
    CloseWindow();
    free_World(w);
}
