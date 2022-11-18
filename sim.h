typedef struct Particle *Particle;
typedef struct World *World;

Particle new_Particle(char type);

char Particle_getType(Particle p);
void Particle_setType(Particle p, char type);

char Particle_getTicked(Particle p);
void Particle_setTicked(Particle p, char ticked);

World new_World(short width, short height);
void free_World(World w);

Particle World_getParticle(World w, short x, short y);
void World_setParticle(World w, short x, short y, Particle p);
void World_swapParticle(World w, short x1, short y1, short x2, short y2);