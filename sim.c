#include <stdlib.h>
#include <stdio.h>
#include "sim.h"

struct Particle {
    char type;
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
    Particle *grid = (Particle *)malloc(width * height * sizeof(struct Particle));
    if (grid == NULL) {
        return NULL;
    }

    // for (int i = 0; i < width * height; i++) {
    //     grid[i] = new_Particle(0x0);
    // }
    printf("done init\n");
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