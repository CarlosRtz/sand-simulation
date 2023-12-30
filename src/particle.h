#ifndef __PARTICLEH__
#define __PARTICLEH__

#include <stdint.h>

typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
} color_t;

typedef struct particle_t {
    uint8_t id;
    color_t color;
    struct velocity {float x; float y;} velocity;
    float life_time;
    uint8_t updated;

    void (*update)(struct particle_t*, int, int);
} particle_t;

typedef struct {
    int width;
    int height;
    particle_t *particles;
    uint8_t *texture_buffer;
} sand_simulation;

#define gravity 1.0

extern sand_simulation *simulation;

void init_simulation(int width, int height);
void destroy_simulation();
void update_simulation();

int in_bounds(int x, int y);
int get_index(int x, int y);
void p_set(particle_t p, int i);

#define empty_id    (uint8_t)0
#define sand_id     (uint8_t)1
#define water_id    (uint8_t)2
#define coal_id     (uint8_t)3
#define oil_id      (uint8_t)4
#define fire_id     (uint8_t)5
#define smoke_id    (uint8_t)6
#define steam_id    (uint8_t)7

particle_t new_empty(); 
particle_t new_sand();  
particle_t new_water(); 
particle_t new_coal(); 
particle_t new_oil();
particle_t new_fire();
particle_t new_smoke();
particle_t new_steam();

void update_empty(particle_t *p, int x, int y);
void update_sand(particle_t *p, int x, int y);
void update_water(particle_t *p, int x, int y);
void update_coal(particle_t *p, int x, int y);
void update_oil(particle_t *p, int x, int y);
void update_fire(particle_t *p, int x, int y);
void update_smoke(particle_t *p, int x, int y);
void update_steam(particle_t *p, int x, int y);

#endif