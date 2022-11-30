#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "sim.h"

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 800

#define WORLD_WIDTH 400
#define WORLD_HEIGHT 400

#define PARTICLE_SCALE 2

SDL_Window *gWindow = NULL;
SDL_Renderer *gRenderer = NULL;

int InitWindow();
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

    // create texture
    SDL_Texture *texture = SDL_CreateTexture(gRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, WINDOW_WIDTH, WINDOW_HEIGHT);
    if (texture == NULL) {
        printf("Failed to create texture: %s\n", SDL_GetError());
        return 1;
    }

    // Uint32 pixels[WINDOW_WIDTH * WINDOW_HEIGHT];
    Uint32 *pixels = (Uint32 *)calloc(WINDOW_WIDTH * WINDOW_HEIGHT, sizeof(Uint32));
    if (pixels == NULL) {
        printf("Failed to create pixel array\n");
    }

    // create font
    if (TTF_Init() < 0) {
        printf("Failed to initialize SDL_ttf: %s\n", TTF_GetError());
    }
    TTF_Font *font = TTF_OpenFont("arial.ttf", 12);
    if (font == NULL) {
        printf("Failed to load font: %s\n", TTF_GetError());
    }
    SDL_Color textColor = {255, 255, 255};
    SDL_Surface *textSurface = TTF_RenderText_Solid(font, "powder", textColor);
    SDL_Texture *textTexture = SDL_CreateTextureFromSurface(gRenderer, textSurface);
    SDL_Rect textRect;
    textRect.x = 10;
    textRect.y = 10;
    TTF_SizeText(font, "powder", &textRect.w, &textRect.h);

    World w = new_World(WORLD_WIDTH, WORLD_HEIGHT);

    SDL_Event e;
    int mX, mY;
    Uint32 mButtons;

    int brushSize = 1;
    char newParticleType = 0x1;

    int loop = 1;
    Uint64 now_time = SDL_GetPerformanceCounter();
    Uint64 last_time = 0;
    double delta_time = 0;
    char time_string[6];

    while (loop) {
        last_time = now_time;
        now_time = SDL_GetPerformanceCounter();
        delta_time = (double)((now_time - last_time)*1000 / (double)SDL_GetPerformanceFrequency());
        SDL_SetWindowTitle(gWindow, gcvt(delta_time, 5, time_string));

        // event loop
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                loop = 0;
            }
            else if (e.type == SDL_KEYDOWN) {
                switch (e.key.keysym.sym) {
                    // exit
                    case SDLK_ESCAPE:
                        loop = 0;
                    break;
                    // switch particle type
                    case SDLK_1:
                        newParticleType = PTYPE_SAND;
                    break;
                    case SDLK_2:
                        newParticleType = PTYPE_WATER;
                    break;
                    case SDLK_3:
                        newParticleType = PTYPE_WOOD;
                    break;
                    case SDLK_4:
                        newParticleType = PTYPE_FIRE;
                    break;
                    // switch brush size
                    case SDLK_UP:
                        brushSize++;
                    break;
                    case SDLK_DOWN:
                        brushSize--;
                        if (brushSize <= 0) {
                            brushSize = 1;
                        }
                    break;
                }
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
            for (int i = 0; i < brushSize; i++) {
                for (int j = 0; j < brushSize; j++) {
                    if (mX / PARTICLE_SCALE + i >= WORLD_WIDTH ||
                        mY / PARTICLE_SCALE + j >= WORLD_HEIGHT) {
                            continue;
                        }
                    if (Particle_getType(World_getParticle(w, mX / PARTICLE_SCALE + i, mY / PARTICLE_SCALE + j)) == PTYPE_NONE) {
                        Particle_setType(
                            World_getParticle(w, mX / PARTICLE_SCALE + i, mY / PARTICLE_SCALE + j),
                            newParticleType
                        );
                    }
                }
            }
        }

        // update
        World_simulate(w);

        // render
        if (SDL_UpdateTexture(texture, NULL, pixels, WINDOW_WIDTH * sizeof(Uint32)) < 0) {
            printf("Failed to update texture: %s\n", SDL_GetError());
        }

        for (short i = 0; i < WORLD_WIDTH; i++) {
            for (short j = 0; j < WORLD_HEIGHT; j++) {
                Uint32 col = 0x0;
                switch (Particle_getType(World_getParticle(w, i, j))) {
                    case PTYPE_SAND:
                        col = 0xffdab163;
                        break;
                    case PTYPE_WATER:
                        col = 0xff3388de;
                        break;
                    case PTYPE_WOOD:
                        col = 0xff94493a;
                        break;
                    case PTYPE_FIRE:
                        col = 0xffde5d3a;
                        break;
                    case PTYPE_SMOKE:
                        col = 0xff2c1e31;
                        break;
                }

                for (int k = 0; k < PARTICLE_SCALE; k++) {
                    for (int l = 0; l < PARTICLE_SCALE; l++) {
                        // SetPixel(gCanvas, i*PARTICLE_SCALE + k, j*PARTICLE_SCALE + l, col);
                        pixels[((j * PARTICLE_SCALE + l) * WINDOW_WIDTH) + (i * PARTICLE_SCALE + k)] = col;
                    }
                }
            }
        }

        SDL_RenderClear(gRenderer);
        SDL_RenderCopy(gRenderer, texture, NULL, NULL);
        SDL_RenderCopy(gRenderer, textTexture, NULL, &textRect);
        SDL_RenderPresent(gRenderer);
    }

    // clean up texture
    SDL_DestroyTexture(texture);
    free(pixels);

    // clean up font
    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);
    TTF_CloseFont(font);

    free_World(w);
}

int InitWindow() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("Failed to initialize video: %s\n", SDL_GetError());
        return 0;
    }

    // create window
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

    // create renderer
    gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED);
    if (gRenderer == NULL) {
        printf("Failed to create renderer: %s\n", SDL_GetError());
        return 0;
    }
    return 1;
}

void Quit() {
    SDL_DestroyRenderer(gRenderer);
    SDL_DestroyWindow(gWindow);
    gWindow = NULL;

    SDL_Quit();
}