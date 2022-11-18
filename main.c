#include <stdio.h>
#include <SDL2/SDL.h>

#include "sim.h"

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 800

SDL_Window *gWindow = NULL;
SDL_Surface *gScreenSurface = NULL;
SDL_Surface *gCanvas = NULL;

int InitWindow();
int CreateSurface();
void Quit();

void SetPixel(SDL_Surface *surface, int x, int y, Uint32 color) {
    if (x < 0 || x >= surface->w || y < 0 || y >= surface->h) return;

    Uint32 *const target = (Uint32 *) ((Uint8 *) surface->pixels
                                                    + y * surface->pitch
                                                    + x * surface->format->BytesPerPixel);
    *target = color;
}

int main(int argc, char *argv[]) {
    if (!InitWindow()) {
        printf("Failed to initialize window\n");
        return 1;
    }

    if (!CreateSurface()) {
        printf("Failed to create surface\n");
        return 1;
    }

    World w = new_World(WINDOW_WIDTH, WINDOW_HEIGHT);

    SDL_Event e;
    int mX, mY;
    Uint32 mButtons;

    int loop = 1;
    while (loop) {
        // event loop
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                loop = 0;
            }
        }

        // mouse state input
        mButtons = SDL_GetMouseState(&mX, &mY);
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

        if ((mButtons & SDL_BUTTON_LEFT) != 0) {
            Particle_setType(
                World_getParticle(w, mX, mY),
                0x1
            );
            // printf("draw %d %d\n", mX, mY);
        }

        // render
        SDL_FillRect(gScreenSurface, NULL, 0x0);
        SDL_FillRect(gCanvas, NULL, 0x0);

        for (short i = 0; i < WINDOW_WIDTH; i++) {
            for (short j = 0; j < WINDOW_HEIGHT; j++) {
                Uint32 col = 0x0;
                switch (Particle_getType(World_getParticle(w, i, j))) {
                    case 0x1:
                        col = 0xff0000ff;
                        break;
                }

                SetPixel(gCanvas, i, j, col);
            }
        }

        // printf("done draw\n");

        SDL_BlitSurface(gCanvas, NULL, gScreenSurface, NULL);
        SDL_UpdateWindowSurface(gWindow);
    }

    free_World(w);
}

int InitWindow() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("Failed to initialize video: %s\n", SDL_GetError());
        return 0;
    }

    gWindow = SDL_CreateWindow(
        "powder",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        SDL_WINDOW_SHOWN
    );

    if (gWindow == NULL) {
        printf("Failed to create window: %s\n", SDL_GetError());
        return 0;
    }

    gScreenSurface = SDL_GetWindowSurface(gWindow);
    return 1;
}

int CreateSurface() {
    gCanvas = SDL_CreateRGBSurface(
        0,
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        32,
        0xff000000,
        0x00ff0000,
        0x0000ff00,
        0x000000ff
    );

    return gCanvas != NULL;
}

void Quit() {
    SDL_FreeSurface(gCanvas);
    gCanvas = NULL;

    SDL_DestroyWindow(gWindow);
    gWindow = NULL;

    SDL_Quit();
}