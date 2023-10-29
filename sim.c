#include <stdlib.h>
#include <stdio.h>
#include <math.h>
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
#define STRENGTH_ACID   3

struct Particle {
  char type;
  char lastSpreadDir;
  short lifetime;
  float vel;
  float spreadVel;
  float saturation;
};

struct ParticleProps {
  short flags;
};

#define IS_EMPTY(p)        (prop_table[p].flags & 0x1)
#define IS_FLAMMABLE(p)    (prop_table[p].flags & 0x10)
#define IS_DISSOLVABLE(p)  (prop_table[p].flags & 0x100)

struct ParticleProps prop_table[] = {
  { // PTYPE_NONE
    0x001,  // flags:  empty
  },
  { // PTYPE_SAND
    0x000,  // flags:  no props
  },
  { // PTYPE_WATER
    0x000,  // flags: no props
  },
  { // PTYPE_WOOD
    0x110,  // flags:  flammable, dissolvable
  },
  { // PTYPE_FIRE
    0x000,  // flags:  no props
  },
  { // PTYPE_SMOKE
    0x001,  // flags: no props
  },
  { // PTYPE_METAL
    0x100,  // flags: dissolvable
  },
  { // PTYPE_ACID
    0x000,  // flags:  no props
  },
};

struct World {
    Particle *p;
    short w;
    short h;

    char *c;
    short cw;
    short ch;

    unsigned long *cs;
};

Particle new_Particle(char type) {
  Particle this = (Particle)malloc(sizeof(struct Particle));
  if (this == NULL) {
    printf("new_Particle NULL\n");
    return NULL;
  }

  this->type = type;
  this->vel = START_VEL;
  this->spreadVel = START_SPREAD;
  this->lastSpreadDir = randBoolean() ? SPREAD_LEFT : SPREAD_RIGHT;
  this->lifetime = 0;

  this->saturation = randFloatRange(0.8f, 1.f);

  return this;
}

char Particle_getType(Particle p) {
  return p->type;
}
void Particle_setType(Particle p, char type) {
  p->type = type;
}

char Particle_getLastSpreadDir(Particle p) {
  return p->lastSpreadDir;
}

float Particle_getSaturation(Particle p) {
  return p->saturation;
}

void Particle_reset(Particle p) {
  p->type = PTYPE_NONE;
  p->vel = START_VEL;
  p->spreadVel = START_SPREAD;
  p->lastSpreadDir = SPREAD_LEFT;
  p->lifetime = 0;
}

World new_World(short width, short height) {
    printf("Beginning world init\n");
    World this = (World)malloc(sizeof(struct World));
    if (this == NULL) {
        return NULL;
    }

    this->w = width;
    this->h = height;

    printf("World size: %dx%d (%d)\n", width, height, width * height);
    printf("Particle struct size: %lu\n", sizeof(struct Particle));
    Particle *grid = (Particle *)malloc(width * height * sizeof(Particle));
    if (grid == NULL) {
        return NULL;
    }

    for (int i = 0; i < width * height; i++) {
        grid[i] = new_Particle(0x0);
    }
    printf("Done world init\n");
    this->p = grid;

    // generate cells
    printf("Beginning world cell init\n");
    this->cw = (int)ceil(width / CELL_SIZE);
    this->ch = (int)ceil(height / CELL_SIZE);
    printf("World cell size: %dx%d (%d)\n", this->cw, this->ch, this->cw * this->ch);

    this->c = (char *)malloc(this->cw * this->ch * sizeof(short));
    for (int i = 0; i < this->cw * this->ch; i++) {
        this->c[i] = 1; // default all cells to on
    }

    this->cs = (unsigned long *)malloc(this->cw * this->ch * sizeof(unsigned long));

    printf("Done world cell init\n");

    return this;
}

void free_World(World w) {
    free(w->p);
    free(w->c);
    free(w->cs);
    free(w);
}

short World_getCellWidth(World w) {
    return w->cw;
}

short World_getCellHeight(World w) {
    return w->ch;
}

void World_activateCell(World w, short x, short y) {
    short cx = (short)floor(x / CELL_SIZE);
    short cy = (short)floor(y / CELL_SIZE);

    short i = cy * w->cw + cx;
    w->c[i] = 2; // 2 chances to stay active

    // activate surrounding cells (no diagonals)
    if (cx > 0) {
      w->c[i - 1] = 2;
    }
    if (cx < w->cw - 1) {
      w->c[i + 1] = 2;
    }
    if (cy > 0) {
      w->c[i - w->cw] = 2;
    }
    if (cy < w->ch - 1) {
      w->c[i + w->cw] = 2;
    }
}

char World_getCellStatus(World w, short x, short y) {
    short cx = (short)floor(x / CELL_SIZE);
    short cy = (short)floor(y / CELL_SIZE);

    return w->c[cy * w->cw + cx];
}

char World_getCellStatusExact(World w, short cx, short cy) {
    return w->c[cy * w->cw + cx];
}

Particle World_getParticle(World w, short x, short y) {
    return w->p[y * w->w + x];
}
void World_setParticle(World w, short x, short y, Particle p) {
    World_activateCell(w, x, y);

    w->p[y * w->w + x] = p;
}
void World_swapParticle(World w, short x1, short y1, short x2, short y2) {
    World_activateCell(w, x1, y1);
    World_activateCell(w, x2, y2);

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
            IS_EMPTY(World_getParticle(w, x, y + k)->type)) {
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
    else if (x > 0 && IS_EMPTY(World_getParticle(w, x - 1, y + 1)->type)) {
        World_swapParticle(w, x, y, x - 1, y + 1);
        p->vel = START_VEL;
        return 1;
    }
    else if (x < w->w - 1 && IS_EMPTY(World_getParticle(w, x + 1, y + 1)->type)) {
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

int sim_fall(World w, Particle p, short x, short y) {
  if (y == w->h -1) {
    return 0;
  }

  // try to fall down at velocity
  int movey = y;
  for (int k = 1; k <= p->vel; k++) {
    if (y + k < w->h &&
        IS_EMPTY(World_getParticle(w, x, y + k)->type)
       ) {
      movey = y + k;
    }
    else {
      break;
    }
  }

  // made a move down
  if (movey != y) {
    World_swapParticle(w, x, y, x, movey);
    p->vel = clamp(p->vel + GRAVITY_ACC, -MAX_VEL, MAX_VEL);
    return 1;
  }

  // didn't move down, try a diagonal down in both directions
  int bestx = x;
  int besty = y;

  for (int k = 1; k <= p->vel; k++) {
    int dy = y + k;
    if (dy >= w->h) {
      break;
    }

    // begin with the last spread direction
    int dx = x + p->lastSpreadDir;
    for (int i = 0; i <= 1; i++) {
      if (dx < 0 || dx >= w->w) {
        break;
      }

      if (IS_EMPTY(World_getParticle(w, dx, dy)->type) &&
          dy > besty) {
        bestx = dx;
        besty = dy;
      }
      // check other direction for second iteration
      dx = x - p->lastSpreadDir;
    }
  }

  if (besty != y) {
    World_swapParticle(w, x, y, bestx, besty);
    return 1;
  }

  // all previous attempts to move failed, it isn't possible right now
  p->vel = START_VEL;
  return 0;
}

int sim_spread(World w, Particle p, short x, short y) {
    // spread left if right is occupied
    if (x >= w->w - 1 || // right is out of bounds
        x < w->w - 1 && !(IS_EMPTY(World_getParticle(w, x + 1, y)->type)) // right is occupied
        ) {
      p->lastSpreadDir = SPREAD_LEFT;
    }

    // spread right if left is occupied
    if (x <= 0 || // left is out of bounds
        x > 0 && !(IS_EMPTY(World_getParticle(w, x - 1, y)->type)) // left is occupied
        ) {
      p->lastSpreadDir = SPREAD_RIGHT;
    }

    // try to move in spread direction
    int movex = x;
    for (int k = 1; k <= p->spreadVel; k++) {
      int try_pos = x + p->lastSpreadDir * k;

      // moving left but that is out of bounds
      if (p->lastSpreadDir == SPREAD_LEFT &&
          try_pos <= 0) {
        p->lastSpreadDir = SPREAD_RIGHT;
        break;
      }

      // moving right but that is out of bounds
      if (p->lastSpreadDir == SPREAD_RIGHT &&
          try_pos >= w->w - 1) {
        p->lastSpreadDir = SPREAD_LEFT;
        break;
      }

      // can move again
      if (IS_EMPTY(World_getParticle(w, try_pos, y)->type)) {
        movex = try_pos;
      }
      // blocked by the same type of particle
//      else if (World_getParticle(w, try_pos, y)->type == p->type) {
//        continue;
//      }
      // cannot move anywhere
      else {
        break;
      }
    }

    // found a move
    if (movex != x)
    {
      World_swapParticle(w, x, y, movex, y);
      p->vel = START_VEL;
      p->spreadVel = clamp(p->spreadVel + SPREAD_ACC, 0, MAX_SPREAD);
      return 1;
    }

    // no move
    p->vel = START_VEL;
    p->spreadVel = START_VEL;
    return 0;
}

int sim_burn(World w, Particle p, short x, short y) {
    char spread = 0;
    for (int i = x - 1; i <= x + 1; i++) {
        for (int j = y - 1; j <= y + 1; j++) {

            if (i < 0 || i >= w->w || j < 0 || j >= w->h) {
                continue;
            }

            Particle neighbor = World_getParticle(w, i, j);

            if (IS_FLAMMABLE(neighbor->type)) {
                // wait to spread
                if (p->lifetime < LIFETIME_FIRE / 2) {
                    continue;
                }
                // random chance to spread (50%)
                if (randFloat() > 0.5f) {
                    continue;
                }
                neighbor->type = PTYPE_FIRE;
                neighbor->lifetime = 0;
                spread = spread | 0x1;
            }
            else if (IS_EMPTY(neighbor->type)) {
                // only upwards
                if (j >= y) {
                    continue;
                }

                // 20% chance to spread
                if (randFloat() > 0.2) {
                    continue;
                }
                // else, 20% chance to make smoke
                else if (randFloat() < 0.2) {
                    Particle above = World_getParticle(w, i, j);
                    above->lifetime = 0;
                    above->type = PTYPE_SMOKE;
                }

                spread = spread | 0x2;
                World_swapParticle(w, x, y, i, j);
            }
        }
    }

    if (spread & 0x1) { // spread to wood
        p->lifetime = 0;
    }

    return 1;
}

int sim_spread_up(World w, Particle p, short x, short y) {
    // attempt to go up
    if (y > 0 &&
        IS_EMPTY(World_getParticle(w, x, y - 1)->type) && // empty space
        World_getParticle(w, x, y - 1)->type != p->type // not swapping with same type
        ) {
        World_swapParticle(w, x, y, x, y - 1);
        return 1;
    }

    // if already at top, "keep floating up" (clear smoke)
    if (y == 0) {
        Particle_reset(p);
    }
    // try to spread
    else if (x > 0 && IS_EMPTY(World_getParticle(w, x - 1, y)->type)) {
        World_swapParticle(w, x, y, x - 1, y);
        return 1;
    }
    else if (x < w->w - 1 && IS_EMPTY(World_getParticle(w, x + 1, y)->type)) {
        World_swapParticle(w, x, y, x + 1, y);
        return 1;
    }

    return 0;
}

int sim_dissolve(World w, Particle p, short x, short y) {
  // search all neighbors for something dissolvable
  for (int i = x - 1; i <= x + 1; i++) {
    for (int j = y - 1; j <= y + 1; j++) {
      // skip out of bounds
      if (i < 0 || i >= w->w || j < 0 || j >= w->h) {
        continue;
      }

      Particle neighbor = World_getParticle(w, i, j);

      if (IS_DISSOLVABLE(neighbor->type)) {
        neighbor->type = PTYPE_NONE; // dissolves instantly
        p->lifetime++; // increase "lifetime"
      }
    }
  }

  return 1;
}

void World_simulate(World w) {
  // loop through every cell
  short active_ct = 0;
  for (short ci = 0; ci < w->cw; ci++) {
    for (short cj = w->ch - 1; cj >= 0; cj--) {
      int c_index = cj * w->ch + ci;
      char active = w->c[c_index];

      if (!active) {
        continue;
      }
      active_ct++;

      // deactivate cell
      // cells have 2 chances to stay active before they're turned off
      // (so that particles falling into a cell and activating it don't get stuck)
      w->c[c_index]--;

      // simulate particles within cell
      for (short i = ci * CELL_SIZE; i < (ci + 1) * CELL_SIZE; i++) {
        for (short j = (cj + 1) * CELL_SIZE - 1; j >= cj * CELL_SIZE; j--) {
          Particle current = World_getParticle(w, i, j);

          // find particle in cell map
          int rx = i - ci * CELL_SIZE; // relative x in cell
          int ry = j - cj * CELL_SIZE; // relative y in cell
          unsigned long c_mask = ((long)0x1) << (ry * CELL_SIZE + rx);

          // check if particle ticked
          if ((w->cs[c_index] & c_mask) != 0) {
            continue;
          }

          // simulate each particle type appropriately
          switch (current->type) {
            case PTYPE_SAND:
              sim_fall(w, current, i, j);
            break;
            case PTYPE_WATER:
              //sim_fall(w, current, i, j);
              sim_spread(w, current, i, j);
            break;
            case PTYPE_WOOD:
              // empty: wood does nothing
            break;
            case PTYPE_FIRE:
              World_activateCell(w, i, j);
              current->lifetime++;
              if (current->lifetime >= LIFETIME_FIRE) {
                Particle_reset(current);
              }
              sim_burn(w, current, i, j);
            break;
            case PTYPE_SMOKE:
              sim_spread_up(w, current, i, j);
            break;
            case PTYPE_METAL:
              // empty: metal does nothing
            break;
            case PTYPE_ACID:
              // behave like water
              if (!sim_pile(w, current, i, j)) {
                sim_spread(w, current, i, j);
              }
              sim_dissolve(w, current, i, j);

              // TODO: better acid behavior
              // lifetime is based on amount of things dissolved
              if (current->lifetime >= STRENGTH_ACID) {
                Particle_reset(current);
              }
            break;
          }
          w->cs[c_index] |= c_mask;
        }
      }
    }
  }

  // reset all cell status
  for (int i = 0; i < w->cw * w->ch; i++) {
    w->cs[i] = 0;
  }
}
