#define CELL_SIZE   10.f

#define PTYPE_LEN   8
#define PTYPE_NONE  0x0
#define PTYPE_SAND  0x1
#define PTYPE_WATER 0x2
#define PTYPE_WOOD  0x3
#define PTYPE_FIRE  0x4
#define PTYPE_SMOKE 0x5
#define PTYPE_METAL 0x6
#define PTYPE_ACID  0x7

#define SPREAD_LEFT   -1
#define SPREAD_RIGHT  1

typedef struct Particle *Particle;
typedef struct World *World;

Particle new_Particle(char type);

char Particle_getType(Particle p);
void Particle_setType(Particle p, char type);
char Particle_getLastSpreadDir(Particle p);
float Particle_getSaturation(Particle p);

World new_World(short width, short height);
void free_World(World w);

Particle World_getParticle(World w, short x, short y);
short World_getCellWidth(World w);
short World_getCellHeight(World w);
char World_getCellStatusExact(World w, short cx, short cy);

void World_setParticle(World w, short x, short y, Particle p);
void World_swapParticle(World w, short x1, short y1, short x2, short y2);

void World_simulate(World w);
