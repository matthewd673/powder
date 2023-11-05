#define CELL_SIZE   8   // cell size must be 8 to fit in tick status array
#define LOGMAP_RES  4   // log map res should fit nicely inside cells

#define PTYPE_LEN   12
#define PTYPE_NONE  0
#define PTYPE_SAND  1
#define PTYPE_WATER 2
#define PTYPE_WOOD  3
#define PTYPE_FIRE  4
#define PTYPE_SMOKE 5
#define PTYPE_METAL 6
#define PTYPE_ACID  7
#define PTYPE_DIRT  8
#define PTYPE_SEED  9
#define PTYPE_PLANT 10
#define PTYPE_LAVA  11

#define SPREAD_LEFT   -1
#define SPREAD_RIGHT  1

typedef struct Particle *Particle;
typedef struct World *World;

Particle new_Particle(char type);

char Particle_get_type(Particle p);
void Particle_set_type(Particle p, char type);
char Particle_get_spread_dir(Particle p);
float Particle_get_saturation(Particle p);
char Particle_get_log_type(Particle p);
float Particle_get_log_amount(Particle p);
char Particle_is_fluid(Particle p);

World new_World(short width, short height);
void free_World(World w);

Particle World_getParticle(World w, short x, short y);
short World_getCellWidth(World w);
short World_getCellHeight(World w);
char World_getCellStatusExact(World w, short cx, short cy);

void World_setParticle(World w, short x, short y, Particle p);
void World_resetParticle(World w, short x, short y);

void World_simulate(World w);
