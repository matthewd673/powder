typedef struct Particle *Particle;
typedef struct World *World;

Particle new_Particle(char type);

char Particle_getType(Particle p);
void Particle_setType(Particle p, char type);

World new_World(short width, short height);
void free_World(World w);

Particle World_getParticle(World w, short x, short y);