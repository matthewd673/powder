#include <stdlib.h>
#include <stdio.h>
#include "sim.h"
#include "math.h"

// rough approximation
#define GRAVITY_ACC     .1568f
#define START_VEL       1.f
#define MAX_VEL         9.8f
#define SPREAD_ACC      .0157f
#define START_SPREAD    1.f
#define MAX_SPREAD      3.f

#define LIFETIME_FIRE   24

struct Particle {
    char type;
    char ticked;
    float vel;
    float spreadVel;
    char lastSpreadDir;
    short lifetime;
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
    this->spreadVel = START_SPREAD;
    this->lastSpreadDir = 0;
    this->lifetime = 0;

    return this;
}

char Particle_getType(Particle p) {
    return p->type;
}
void Particle_setType(Particle p, char type) {
    p->type = type;
}

void Particle_reset(Particle p) {
    p->type = PTYPE_NONE;
    p->ticked = 0;
    p->vel = START_VEL;
    p->spreadVel = START_SPREAD;
    p->lastSpreadDir = 0;
    p->lifetime = 0;
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

int sim_pile(World w, Particle p, short x, short y) {
    if (y == w->h - 1) {
        return 0;
    }
    
    int moveY = y;
    for (int k = 1; k <= p->vel; k++) {
        if (y + k < w->h &&
            World_getParticle(w, x, y + k)->type == PTYPE_NONE) {
                moveY = y + k;
            }
        else {
            break;
        }
    }
    // MOVE INTO PILE
    if (moveY != y) {
        World_swapParticle(w, x, y, x, moveY);
        p->vel = clamp(p->vel + GRAVITY_ACC, -MAX_VEL, MAX_VEL);
        return 1;
    }
    else if (x > 0 && World_getParticle(w, x - 1, y + 1)->type == PTYPE_NONE) {
        World_swapParticle(w, x, y, x - 1, y + 1);
        p->vel = START_VEL;
        return 1;
    }
    else if (x < w->w - 1 && World_getParticle(w, x + 1, y + 1)->type == PTYPE_NONE) {
        World_swapParticle(w, x, y, x + 1, y + 1);
        p->vel = START_VEL;
        return 1;
    }
    // NO MOVE
    else {
        p->vel = START_VEL;
        return 0;
    }
}

int sim_spread(World w, Particle p, short x, short y) {
    // MOVE INTO SPREAD
    if ((x > 0 && World_getParticle(w, x - 1, y)->type == PTYPE_NONE) && (p->lastSpreadDir == 0 ||
        (p->lastSpreadDir == 1 && (x >= w->w - 1 || World_getParticle(w, x + 1, y)->type != PTYPE_NONE)))) {
        int moveX = x;
        for (int k = 1; k <= p->spreadVel; k++) {
            if (x - k >= 0 &&
                World_getParticle(w, x - k, y)->type == PTYPE_NONE) {
                    moveX = x - k;
                }
            else {
                break;
            }
        }
        if (moveX != x) {
            World_swapParticle(w, x, y, x - 1, y);
            p->vel = START_VEL;
            p->spreadVel = clamp(p->vel + SPREAD_ACC, -MAX_SPREAD, MAX_SPREAD);
            p->lastSpreadDir = 0;
            return 1;
        }
        else {
            return 0;
        }
    }
    else if ((x < w->w - 1 && World_getParticle(w, x + 1, y)->type == PTYPE_NONE) && (p->lastSpreadDir == 1 ||
             (p->lastSpreadDir == 0 && (x <= 0 || World_getParticle(w, x - 1, y)->type != PTYPE_NONE)))) {
        int moveX = x;
        for (int k = 1; k <= p->spreadVel; k++) {
            if (x + k < w->w &&
                World_getParticle(w, x + k, y)->type == PTYPE_NONE) {
                    moveX = x + k;
                }
            else {
                break;
            }
        }
        if (moveX != x) {
            World_swapParticle(w, x, y, x + 1, y);
            p->vel = START_VEL;
            p->spreadVel = clamp(p->vel + SPREAD_ACC, -MAX_SPREAD, MAX_SPREAD);
            p->lastSpreadDir = 1;
            return 1;
        }
        else {
            return 0;
        }
    }
    // NO MOVE
    else {
        p->vel = START_VEL;
        p->spreadVel = START_VEL;
        return 0;
    }
}

int sim_burn(World w, Particle p, short x, short y) {
    int spread = 0;
    for (int i = x - 1; i <= x + 1; i++) {
        for (int j = y - 1; j <= y + 1; j++) {

            if (i < 0 || i >= w->w || j < 0 || j >= w->h) {
                continue;
            }

            Particle neighbor = World_getParticle(w, i, j);

            if (neighbor->type == PTYPE_WOOD) {
                // wait to spread
                if (p->lifetime < LIFETIME_FIRE / 2) {
                    continue;
                }
                // random chance to spread
                if (randFloat() > 0.5f) {
                    continue;
                }
                neighbor->type = PTYPE_FIRE;
                neighbor->lifetime = 0;
                spread = 1;
            }
            else if (neighbor->type == PTYPE_NONE) {
                if (randFloat() > 0.2) {
                    continue;
                }
                // only upwards
                if (j >= y) {
                    continue;
                }
                // sparky version
                World_swapParticle(w, x, y, i, j);
                // spready version
                // if (p->lifetime < LIFETIME_FIRE/3) {
                //     neighbor->type = PTYPE_FIRE;
                //     neighbor->lifetime = p->lifetime * 3;
                //     p->lifetime += LIFETIME_FIRE/2;
                // }
                // neighbor->lifetime = p->lifetime * 2;
                // p->lifetime += 4;
            }
        }
    }

    if (spread) {
        p->lifetime = 0;
    }

    return 1;
    // Particle_reset(p);
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

            switch (current->type) {
                case PTYPE_SAND:
                    // skip sand at bottom of screen
                    if (j == w->h - 1) {
                        break;
                    }

                    sim_pile(w, current, i, j);
                break;
                case PTYPE_WATER:
                    if (!sim_pile(w, current, i, j)) {
                        sim_spread(w, current, i, j);
                    }
                break;
                case PTYPE_WOOD:
                break;
                case PTYPE_FIRE:
                    current->lifetime++;
                    if (current->lifetime >= LIFETIME_FIRE) {
                        Particle_reset(current);
                    }
                    sim_burn(w, current, i, j);
                break;
            }
            current->ticked = 1;
        }
    }
}