#include <stdlib.h>
#include <stdio.h>
#include "sim.h"

struct Particle {
    char type;
    char ticked;
};

struct World {
    Particle *p;
    short w;
    short h;
};

Particle new_Particle(char type) {
    Particle this = (Particle)malloc(sizeof(struct Particle));
    if (this == NULL) {
        printf("new_Particle NULL\n");
        return NULL;
    }

    this->type = type;
    this->ticked = 0;

    return this;
}

char Particle_getType(Particle p) {
    return p->type;
}
void Particle_setType(Particle p, char type) {
    p->type = type;
}

char Particle_getTicked(Particle p) {
    return p->ticked;
}
void Particle_setTicked(Particle p, char ticked) {
    p->ticked = ticked;
}

World new_World(short width, short height) {
    printf("try make world\n");
    World this = (World)malloc(sizeof(struct World));
    if (this == NULL) {
        return NULL;
    }

    this->w = width;
    this->h = height;

    printf("world size: %d\n", width * height);
    Particle *grid = (Particle *)malloc(width * height * sizeof(Particle));
    if (grid == NULL) {
        return NULL;
    }

    for (int i = 0; i < width * height; i++) {
        grid[i] = new_Particle(0x0);
    }
    printf("done world init\n");
    this->p = grid;

    return this;
}

void free_World(World w) {
    free(w->p);
    free(w);
}

Particle World_getParticle(World w, short x, short y) {
    // printf("try get on %d %d = %d\n", x, y, y * x + x);
    return w->p[y * w->w + x];
}
void World_setParticle(World w, short x, short y, Particle p) {
    w->p[y * w->w + x] = p;
}
void World_swapParticle(World w, short x1, short y1, short x2, short y2) {
    Particle temp = w->p[y1 * w->w + x1];
    w->p[y1 * w->w + x1] = w->p[y2 * w->w + x2];
    w->p[y2 * w->w + x2] = temp;
}

void World_simulate(World w) {
    // update
    Particle current;
    for (short i = 0; i < w->w; i++) {
        for (short j = 0; j < w->h; j++) {
            current = World_getParticle(w, i, j);
            
            if (Particle_getTicked(current)) {
                Particle_setTicked(current, 0);
                continue;
            }

            switch (Particle_getType(current)) {
                case PTYPE_SAND:
                    if (j < w->h - 1 &&
                        Particle_getType(World_getParticle(w, i, j + 1)) == PTYPE_NONE) {
                            World_swapParticle(w, i, j, i, j + 1);
                        }
                    else if (j < w->h - 1 && i > 0 &&
                                Particle_getType(World_getParticle(w, i - 1, j + 1)) == PTYPE_NONE) {
                                World_swapParticle(w, i, j, i - 1, j + 1);
                                }
                    else if (j < w->h - 1 && i < w->w - 1 &&
                                Particle_getType(World_getParticle(w, i + 1, j + 1)) == PTYPE_NONE) {
                                World_swapParticle(w, i, j, i + 1, j + 1);
                                }
                break;
                case PTYPE_WATER:
                    if (j < w->h - 1 &&
                        Particle_getType(World_getParticle(w, i, j + 1)) == PTYPE_NONE) {
                            World_swapParticle(w, i, j, i, j + 1);
                        }
                    else if (i > 0 &&
                                Particle_getType(World_getParticle(w, i - 1, j)) == PTYPE_NONE) {
                                World_swapParticle(w, i, j, i - 1, j);
                                }
                    else if (i < w->w - 1 &&
                                Particle_getType(World_getParticle(w, i + 1, j)) == PTYPE_NONE) {
                                World_swapParticle(w, i, j, i + 1, j);
                                }
                break;
                case PTYPE_WOOD:
                break;
            }
            Particle_setTicked(current, 1);
        }
    }
}