#include <stdlib.h>
#include <stdio.h>
#include "sim.h"
#include "math.h"

// rough approximation
#define GRAVITY_ACC     .1568f
#define START_VEL       1.f
#define MAX_VEL         9.8f
#define SPREAD_ACC      .0157f
#define START_SPREAD    0.f
#define MAX_SPREAD      2.f

struct Particle {
    char type;
    char ticked;
    float vel;
    float spreadVel;
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
    this->vel = START_VEL;

    return this;
}

char Particle_getType(Particle p) {
    return p->type;
}
void Particle_setType(Particle p, char type) {
    p->type = type;
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
            
            if (current->ticked) {
                current->ticked = 0;
                continue;
            }

            int moveX;
            int moveY;
            switch (current->type) {
                case PTYPE_SAND:
                    // skip sand at bottom of screen
                    if (j == w->h - 1) {
                        break;
                    }

                    moveY = j;
                    for (int k = 1; k <= current->vel; k++) {
                        if (j + k < w->h &&
                            World_getParticle(w, i, j + k)->type == PTYPE_NONE) {
                                moveY = j + k;
                            }
                        else {
                            break;
                        }
                    }
                    // MOVE INTO PILE
                    if (moveY != j) {
                        World_swapParticle(w, i, j, i, moveY);
                        current->vel = clamp(current->vel + GRAVITY_ACC, -MAX_VEL, MAX_VEL);
                    }
                    else if (i > 0 && World_getParticle(w, i - 1, j + 1)->type == PTYPE_NONE) {
                        World_swapParticle(w, i, j, i - 1, j + 1);
                        current->vel = START_VEL;
                    }
                    else if (i < w->w - 1 && World_getParticle(w, i + 1, j + 1)->type == PTYPE_NONE) {
                        World_swapParticle(w, i, j, i + 1, j + 1);
                        current->vel = START_VEL;
                    }
                    // NO MOVE
                    else {
                        current->vel = START_VEL;
                    }
                break;
                case PTYPE_WATER:
                    moveY = j;

                    for (int k = 1; k <= current->vel; k++) {
                        if (j + k < w->h &&
                            World_getParticle(w, i, j + k)->type == PTYPE_NONE) {
                                moveY = j + k;
                            }
                        else {
                            break;
                        }
                    }

                    // MOVE INTO PILE (LIKE SAND)
                    if (moveY != j) {
                        World_swapParticle(w, i, j, i, moveY);
                        current->vel = clamp(current->vel + GRAVITY_ACC, -MAX_VEL, MAX_VEL);
                    }
                    else if (j < w->h - 1 && i > 0 && World_getParticle(w, i - 1, j + 1)->type == PTYPE_NONE) {
                        World_swapParticle(w, i, j, i - 1, j + 1);
                        current->vel = START_VEL;
                    }
                    else if (j < w->h - 1 && i < w->w - 1 && World_getParticle(w, i + 1, j + 1)->type == PTYPE_NONE) {
                        World_swapParticle(w, i, j, i + 1, j + 1);
                        current->vel = START_VEL;
                    }
                    // MOVE INTO SPREAD
                    else if (i > 0 && World_getParticle(w, i - 1, j)->type == PTYPE_NONE) {
                        moveX = i;
                        for (int k = 1; k <= current->spreadVel; k++) {
                            if (i - k >= 0 &&
                                World_getParticle(w, i - k, j)->type == PTYPE_NONE) {
                                    moveX = i - k;
                                }
                            else {
                                break;
                            }
                        }
                        if (moveX != i) {
                            World_swapParticle(w, i, j, i - 1, j);
                            current->vel = START_VEL;
                        }
                    }
                    else if (i < w->w - 1 && World_getParticle(w, i + 1, j)->type == PTYPE_NONE) {
                        moveX = i;
                        for (int k = 1; k <= current->spreadVel; k++) {
                            if (i + k < w->w &&
                                World_getParticle(w, i + k, j)->type == PTYPE_NONE) {
                                    moveX = i + k;
                                }
                            else {
                                break;
                            }
                        }
                        if (moveX != i) {
                            World_swapParticle(w, i, j, i + 1, j);
                            current->vel = START_VEL;
                        }
                    }
                    // NO MOVE
                    else {
                        current->vel = START_VEL;
                        current->spreadVel = START_VEL;
                    }

                break;
                case PTYPE_WOOD:
                break;
            }
            current->ticked = 1;
        }
    }
}