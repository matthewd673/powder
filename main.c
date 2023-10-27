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
  { 11, 78, 120, 255 },   // PTYPE_WATER
  { 138, 91, 51, 255 },   // PTYPE_WOOD
  { 247, 83, 47, 255 },   // PTYPE_FIRE
  { 74, 74, 74, 255 },    // PTYPE_SMOKE
  { 196, 196, 196, 255 }, // PTYPE_METAL
  { 105, 217, 50, 255 },  // PTYPE_ACID
};

int main(int argc, char *argv[]) {
  InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "powder");
  SetTargetFPS(WINDOW_FPS);

  RenderTexture2D worldTexture = LoadRenderTexture(WINDOW_WIDTH, WINDOW_HEIGHT);
  RenderTexture2D glowTexture = LoadRenderTexture(WINDOW_WIDTH, WINDOW_HEIGHT);

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
    else if (IsKeyDown(KEY_FIVE)) {
      newParticleType = PTYPE_METAL;
    }
    else if (IsKeyDown(KEY_SIX)) {
      newParticleType = PTYPE_ACID;
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
    // draw world texture
    BeginTextureMode(worldTexture);

    // loop through every cell
    for (short ci = 0; ci < World_getCellWidth(w); ci++) {
      for (short cj = 0; cj < World_getCellHeight(w); cj++) {
        if (World_getCellStatusExact(w, ci, cj) == 0) {
          continue;
        }

        for (short i = ci * CELL_SIZE; i < (ci + 1) * CELL_SIZE; i++) {
          for (short j = cj * CELL_SIZE; j < (cj + 1) * CELL_SIZE; j++) {
            Particle p = World_getParticle(w, i, j);
            // determine particle color
            Color col = color_map[Particle_getType(p)];
            col.r *= Particle_getSaturation(p);
            col.g *= Particle_getSaturation(p);
            col.b *= Particle_getSaturation(p);

            if (Particle_getType(p) == PTYPE_WATER &&
                Particle_getLastSpreadDir(p) == SPREAD_LEFT) {
              col.r = 0;
              col.g = 0;
              col.b = 255;
            }
            else if (Particle_getType(p) == PTYPE_WATER) {
              col.r = 255;
              col.g = 0;
              col.b = 0;
            }

            DrawRectangle(i * PARTICLE_SCALE, -j * PARTICLE_SCALE + WINDOW_HEIGHT,
                          PARTICLE_SCALE, PARTICLE_SCALE,
                          col);
          }
        }
      }
    }

    EndTextureMode();

    // draw to screen
    BeginDrawing();

    // draw heatmap
    for (short i = 0; i < World_getCellWidth(w); i++) {
      for (short j = 0; j < World_getCellHeight(w); j++) {
        Color col = { 0, 0, 50, 255 };
        // lifetime = 2
        if (World_getCellStatusExact(w, i, j) & 0x10) {
          col.r = 255;
        }
        // lifetime = 1
        else if (World_getCellStatusExact(w, i, j) & 0x01) {
          col.r = 127;
        }

        DrawRectangle(i * 3, j * 3, 3, 3, col);
      }
    }

    DrawTexture(worldTexture.texture, 0, 0, WHITE);

    EndDrawing();
  }

  // clean up
  CloseWindow();
  free_World(w);
}
