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

struct Particle {
  char type;
  short energy;
  short durability;

  float vel;
  char spread_dir;
  float spread_vel;
  float saturation;
};

struct ParticleProps {
  short flags;
  short energy;
  short durability;
  float min_saturation;
};

#define IS_EMPTY(p)        (prop_table[p].flags & 0x1)
#define IS_FLAMMABLE(p)    (prop_table[p].flags & 0x10)
#define IS_DISSOLVABLE(p)  (prop_table[p].flags & 0x100)

struct ParticleProps prop_table[] = {
  { // PTYPE_NONE
    0x001,  // flags:  empty
    -1,
    -1,
    0.f,
  },
  { // PTYPE_SAND
    0x000,  // flags:  no props
    -1,
    -1,
    0.75f,
  },
  { // PTYPE_WATER
    0x000,  // flags: no props
    -1,
    -1,
    0.9f,
  },
  { // PTYPE_WOOD
    0x110,  // flags:  flammable, dissolvable
    -1,
    24,
    0.8f,
  },
  { // PTYPE_FIRE
    0x000,  // flags:  no props
    24,
    -1,
    0.4f,
  },
  { // PTYPE_SMOKE
    0x001,  // flags: no props
    -1,
    -1,
    0.75f,
  },
  { // PTYPE_METAL
    0x100,  // flags: dissolvable
    -1,
    64,
    0.9f,
  },
  { // PTYPE_ACID
    0x000,  // flags:  no props
    -1,
    -1,
    0.9f,
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

struct Position {
  short x;
  short y;
};

Particle new_Particle(char type) {
  Particle this = (Particle)malloc(sizeof(struct Particle));
  if (this == NULL) {
    printf("new_Particle NULL\n");
    return NULL;
  }

  this->type = type;
  this->energy = prop_table[type].energy;
  this->durability = prop_table[type].durability;

  this->vel = START_VEL;
  this->spread_vel = START_SPREAD;
  this->spread_dir = randBoolean() ? SPREAD_LEFT : SPREAD_RIGHT;

  this->saturation = randFloatRange(prop_table[type].min_saturation, 1.f);

  return this;
}

char Particle_getType(Particle p) {
  return p->type;
}
void Particle_setType(Particle p, char type) {
  p->type = type;
  p->energy = prop_table[p->type].energy;
  p->durability = prop_table[p->type].durability;
  p->saturation = randFloatRange(prop_table[type].min_saturation, 1.f);
}

char Particle_getLastSpreadDir(Particle p) {
  return p->spread_dir;
}

float Particle_getSaturation(Particle p) {
  return p->saturation;
}

void Particle_reset(Particle p) {
  p->type = PTYPE_NONE;
  p->energy = prop_table[PTYPE_NONE].energy;
  p->durability = prop_table[PTYPE_NONE].durability;

  p->vel = START_VEL;
  p->spread_vel = START_SPREAD;
  p->spread_dir = randBoolean() ? SPREAD_LEFT : SPREAD_RIGHT;
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
    this->cw = width / CELL_SIZE;
    if (width % CELL_SIZE > 0) {
      this->cw += 1;
    }
    this->ch = height / CELL_SIZE;
    if (height % CELL_SIZE > 0) {
      this->ch += 1;
    }
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
    short cx = x / CELL_SIZE;
    short cy = y / CELL_SIZE;

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
    short cx = x / CELL_SIZE;
    short cy = y / CELL_SIZE;

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

void World_resetParticle(World w, short x, short y) {
  World_activateCell(w, x, y);

  Particle_reset(w->p[y * w->w + x]);
}

// World_swapParticle: swap the Particles at the specified positions in the grid.
void World_swapParticle(World w, short x1, short y1, short x2, short y2) {
    World_activateCell(w, x1, y1);
    World_activateCell(w, x2, y2);

    Particle temp = w->p[y1 * w->w + x1];
    w->p[y1 * w->w + x1] = w->p[y2 * w->w + x2];
    w->p[y2 * w->w + x2] = temp;
}

struct Position sim_fall(World w, Particle p, short x, short y) {
  if (y == w->h -1) {
    return (struct Position){x, y};
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
    return (struct Position){x, movey};
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
    int dx = x + p->spread_dir;
    for (int i = 0; i <= 1; i++) {
      if (dx < 0 || dx >= w->w) {
        break;
      }

      if (dy + 1 < w->h && IS_EMPTY(World_getParticle(w, dx, dy + 1)->type)) {
        break;
      }

      if (IS_EMPTY(World_getParticle(w, dx, dy)->type) &&
          dy > besty) {
        bestx = dx;
        besty = dy;
      }
      // check other direction for second iteration
      dx = x - p->spread_dir;
    }
  }

  if (besty != y) {
    World_swapParticle(w, x, y, bestx, besty);
    return (struct Position){bestx, besty};
  }

  // all previous attempts to move failed, it isn't possible right now
  p->vel = START_VEL;
  return (struct Position){x, y};
}

struct Position sim_spread(World w, Particle p, short x, short y) {
  char left_blocked = x <= 0 || !(IS_EMPTY(World_getParticle(w, x - 1, y)->type));
  char right_blocked = x >= w->w - 1 || !(IS_EMPTY(World_getParticle(w, x + 1, y)->type));

  // spread left if right is occupied & left is not
  if (right_blocked && !left_blocked) {
    p->spread_dir = SPREAD_LEFT;
  }

  // spread right if left is occupied & right is not
  if (left_blocked && !right_blocked) {
    p->spread_dir = SPREAD_RIGHT;
  }

  // try to move in spread direction
  int movex = x;
  for (int k = 1; k <= p->spread_vel; k++) {
    int try_pos = x + p->spread_dir * k;

    // moving left but that is out of bounds
    if (p->spread_dir == SPREAD_LEFT &&
        try_pos <= 0) {
      p->spread_dir = SPREAD_RIGHT;
      break;
    }

    // moving right but that is out of bounds
    if (p->spread_dir == SPREAD_RIGHT &&
        try_pos >= w->w - 1) {
      p->spread_dir = SPREAD_LEFT;
      break;
    }

    // can move again
    if (IS_EMPTY(World_getParticle(w, try_pos, y)->type)) {
      movex = try_pos;
    }
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
    p->spread_vel = clamp(p->spread_vel + SPREAD_ACC, 0, MAX_SPREAD);
    return (struct Position){movex, y};
  }

  // no move
  p->vel = START_VEL;
  p->spread_vel = START_VEL;
  return (struct Position){x, y};
}

struct Position sim_burn(World w, Particle p, short x, short y) {
  char spread = 0;
  for (int i = x - 1; i <= x + 1; i++) {
    for (int j = y - 1; j <= y + 1; j++) {
      if (i < 0 || i >= w->w || j < 0 || j >= w->h) {
        continue;
      }

      Particle neighbor = World_getParticle(w, i, j);

      if (IS_FLAMMABLE(neighbor->type)) {
        // wait to spread
        if (p->energy >= prop_table[p->type].energy / 2) {
          continue;
        }
        // random chance to spread (50%)
        if (randFloat() > 0.5f) {
          continue;
        }
        Particle_setType(neighbor, p->type);
        neighbor->energy = prop_table[neighbor->type].energy;
        spread = spread | 0x1; // mark as spread to flammable material
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
        // else, 2% chance to make smoke
        else if (randFloat() <= 0.02) {
          Particle above = World_getParticle(w, i, j);
          above->type = PTYPE_SMOKE;
          above->energy = prop_table[above->type].energy;
        }

        spread = spread | 0x10; // mark as made smoke
        World_swapParticle(w, x, y, i, j);
      }
    }
  }

  if (spread & 0x1) { // spread to something flammable
    p->energy = prop_table[p->type].energy;
  }

  return (struct Position){x, y};
}

struct Position sim_fall_up(World w, Particle p, short x, short y) {
  // top of screen, "keep moving up" (clear smoke)
  if (y == 0) {
    Particle_reset(p);
    return (struct Position){x, y};
  }

  // try to move up at velocity
  int movey = y;
  for (int k = -1; k <= -1; k++) { // TODO: temp
    if (y + k < w->h &&
        IS_EMPTY(World_getParticle(w, x, y + k)->type) && // move into empty
        World_getParticle(w, x, y + k)->type != p->type   // don't move into your own type
       ) {
      movey = y + k;
    }
    else {
      break;
    }
  }

  // made a move up
  if (movey != y) {
    World_swapParticle(w, x, y, x, movey);
    p->vel = clamp(p->vel + GRAVITY_ACC, -MAX_VEL, MAX_VEL);
    return (struct Position){x, movey};
  }

  // did nothing
  return (struct Position){x, movey};
}

struct Position sim_spread_gas(World w, Particle p, short x, short y) {
  char left_blocked = x <= 0 || !(IS_EMPTY(World_getParticle(w, x - 1, y)->type));
  char right_blocked = x >= w->w - 1 || !(IS_EMPTY(World_getParticle(w, x + 1, y)->type));

  // random chance to switch direction
  if (randFloat() <= 0.02f) {
    p->spread_dir = -p->spread_dir;
  }

  // spread left if right is occupied & left is not
  if (right_blocked && !left_blocked) {
    p->spread_dir = SPREAD_LEFT;
  }

  // spread right if left is occupied & right is not
  if (left_blocked && !right_blocked) {
    p->spread_dir = SPREAD_RIGHT;
  }

  // chance to not try to spread
  if (randFloat() <= 0.85f) {
    return (struct Position){x, y};
  }

  // try to move in spread direction
  int movex = x;
  for (int k = 1; k <= 1; k++) {
    int try_pos = x + p->spread_dir * k;

    // moving left but that is out of bounds
    if (p->spread_dir == SPREAD_LEFT &&
        try_pos <= 0) {
      p->spread_dir = SPREAD_RIGHT;
      break;
    }

    // moving right but that is out of bounds
    if (p->spread_dir == SPREAD_RIGHT &&
        try_pos >= w->w - 1) {
      p->spread_dir = SPREAD_LEFT;
      break;
    }

    // can move again
    if (IS_EMPTY(World_getParticle(w, try_pos, y)->type)) {
      movex = try_pos;
    }
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
    p->spread_vel = clamp(p->spread_vel + SPREAD_ACC, 0, MAX_SPREAD);
    return (struct Position){movex, y};
  }

  // no move
  p->vel = START_VEL;
  p->spread_vel = START_VEL;
  return (struct Position){x, y};
}

struct Position sim_dissolve(World w, Particle p, short x, short y) {
  // search all neighbors (below & left/right) for something dissolvable
  for (int i = x - 1; i <= x + 1; i++) {
    for (int j = y; j <= y + 1; j++) {
      // skip out of bounds
      if (i < 0 || i >= w->w || j < 0 || j >= w->h) {
        continue;
      }

      // skip over your own self
      if (i == x && j == y) {
        continue;
      }

      Particle neighbor = World_getParticle(w, i, j);

      if (IS_DISSOLVABLE(neighbor->type) &&
          neighbor->durability > 0) {
        World_activateCell(w, i, j);
        neighbor->durability -= 1; // decrease neighbor's durability
      }
    }
  }

  return (struct Position){x, y};
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
          int rx = i % CELL_SIZE; // relative x in cell
          int ry = j % CELL_SIZE; // relative y in cell
          unsigned long c_mask = ((long)0x1) << (ry * CELL_SIZE + rx);

          // check if particle ticked
          if ((w->cs[c_index] & c_mask) != 0) {
            continue;
          }

          struct Position pos = {i, j};
          // simulate each particle type appropriately
          switch (current->type) {
            case PTYPE_SAND:
              pos = sim_fall(w, current, pos.x, pos.y);
            break;
            case PTYPE_WATER:
              pos = sim_fall(w, current, pos.x, pos.y);
              if (pos.x == i && pos.y == j) { // if falling failed
                pos = sim_spread(w, current, pos.x, pos.y);
              }
            break;
            case PTYPE_WOOD:
              // empty: wood does nothing
            break;
            case PTYPE_FIRE:
              World_activateCell(w, pos.x, pos.y);
              current->energy -= 1;
              pos = sim_burn(w, current, pos.x, pos.y);
            break;
            case PTYPE_SMOKE:
              pos = sim_fall_up(w, current, pos.x, pos.y);
              pos = sim_spread_gas(w, current, pos.x, pos.y);
            break;
            case PTYPE_METAL:
              // empty: metal does nothing
            break;
            case PTYPE_ACID:
              // behave like water
              pos = sim_fall(w, current, pos.x, pos.y);
              if (pos.x == i && pos.y == j) { // if falling failed
                pos = sim_spread(w, current, pos.x, pos.y);
              }
              pos = sim_dissolve(w, current, pos.x, pos.y);
            break;
          }

          if (current->energy == 0 ||
              current->durability == 0) {
            Particle_reset(current);
          }

          // mark new particle position as ticked
          int new_cx = pos.x / CELL_SIZE;
          int new_cy = pos.y / CELL_SIZE;
          int new_rx = pos.x % CELL_SIZE; // relative x in cell
          int new_ry = pos.y % CELL_SIZE; // relative y in cell
          unsigned long new_c_mask = ((long)0x1) << (new_ry * CELL_SIZE + new_rx);

          int new_c_index = new_cy * w->ch + new_cx;
          w->cs[new_c_index] |= new_c_mask;
        }
      }
    }
  }

  // reset all cell status
  for (int i = 0; i < w->cw * w->ch; i++) {
    w->cs[i] = 0;
  }
}
