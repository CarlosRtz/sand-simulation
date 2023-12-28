#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "particle.h"

sand_simulation *simulation;

void init_simulation(int width, int height){
    simulation = (sand_simulation*) malloc(sizeof(sand_simulation));
    if(!simulation) return;

    particle_t *p = (particle_t *) malloc(sizeof(particle_t) * width * height);   
    if(!p) return;

    uint8_t  *tex = (uint8_t *) malloc(sizeof(uint8_t) * width * height * 4);
    if(!tex) return;

    simulation->width = width;
    simulation->height = height;
    simulation->particles = p;
    simulation->texture_buffer = tex;

    for(int i = 0; i < width * height; i++){
        simulation->particles[i] = new_empty();
    }
}

void destroy_simulation(){
    free(simulation->particles);
    free(simulation->texture_buffer);
    free(simulation);
}

void update_simulation(){
    for(int y = 0; y < simulation->height; y++){
        int dir = y % 2 == 0;
        for(int x = dir ? 0 : simulation->width - 1; dir ? x < simulation->width : x >= 0; dir ? x++ : x--){
            int i = get_index(x, y);
            particle_t *p = &simulation->particles[i];
            if(!p->updated){
                p->update(p, x, y);
            }
        }
    }

    for(int i = 0; i < simulation->width * simulation->height; i++){
        simulation->particles[i].updated = 0;
    }
}

int in_bounds(int x, int y){
    return (x >= 0 && x <= simulation->width && y >= 0 && y < simulation->height);    
}

int get_index(int x, int y){
    return y * simulation->width + x;
}

void p_set(particle_t p, int i){
    p.updated = 1;
    simulation->particles[i] = p;

    int j = i * 4;
    simulation->texture_buffer[j] = p.color.r;
    simulation->texture_buffer[j + 1] = p.color.g;
    simulation->texture_buffer[j + 2] = p.color.b;
    simulation->texture_buffer[j + 3] = p.color.a;
}

/* Create particles */
particle_t new_empty(){
    particle_t p = {
        .id = empty_id,
        .color = {.r=0, .g=0, .b=0, .a=255},
        .velocity = {0, 0},
        .life_time = 1,
        .updated = 0,
        .update = update_empty
    };
    return p;
}

particle_t new_sand(){
    particle_t p = {
        .id = sand_id,
        .color = {.r=230, .g=205, .b=50, .a=255},
        .velocity = {0, 0},
        .life_time = 1,
        .updated = 0,
        .update = update_sand
    };
    return p;
}

particle_t new_water(){
    particle_t p = {
        .id = water_id,
        .color = {.r=50, .g=120, .b=170, .a=255},
        .velocity = {0, 0},
        .life_time = 1,
        .updated = 0,
        .update = update_water
    };
    return p;
}

particle_t new_coal(){
    int c = (rand() % (50 - 25 + 1)) + 25;
    particle_t p = {
        .id = coal_id,
        .color = {.r=c, .g=c, .b=c, .a=255},
        .velocity = {0.0, 0.0},
        .life_time = 1.0,
        .updated = 0,
        .update = update_coal
    };
    return p;
}

particle_t new_oil(){
    particle_t p;
    return p;
}

particle_t new_fire(){
    particle_t p;
    return p;
}

particle_t new_smoke(){
    particle_t p;
    return p;
}

particle_t new_steam(){
    particle_t p;
    return p;
}


/*      Update particles    */

/*      UPDATE EMPTY        */
void update_empty(particle_t *p, int x, int y){
    int i = get_index(x, y);
    p_set(*p, i);
}


/*      UPDATE SAND         */
// Try moving bellow or diagonals
// Increse y speed when falling
void update_sand(particle_t *p, int x, int y){
    float max_spread = 5.0;
    float max_speed_fall = -10.0;

    p->velocity[0] = p->velocity[0] < 0.0 ? 0.0 : p->velocity[0];
    p->velocity[1] = p->velocity[1] < max_speed_fall ? max_speed_fall : p->velocity[1];
    p->velocity[1] = p->velocity[1] > 0.0 ? 0.0 : p->velocity[1];

    int i = get_index(x, y);
    int x_off, y_off, x_coord;
    int j;

    // try moving bellow:
    x_off = round(p->velocity[0]);
    y_off = round(p->velocity[1]);

    if(in_bounds(x_off == 0 ? x : x * x_off, y - 1 + y_off)){
        j = get_index(x_off == 0 ? x : x * x_off, y - 1 + y_off);
        particle_t temp = simulation->particles[j];
        
        if(temp.id == empty_id){
            p->velocity[0] -= 1.0;
            p->velocity[1] -= gravity;
            p_set(*p, j);
            p_set(temp, i);
            return;
        }
    }

    p->velocity[0] = ((float)rand())/RAND_MAX * p->velocity[1];
    p->velocity[0] = p->velocity[0] > max_spread ? max_spread : p->velocity[0];

    p->velocity[1] += gravity;

    x_off = round(p->velocity[0]);
    y_off = round(p->velocity[1]);
        
    p->velocity[0] -= 1.0;

    // try first diagonal
    int dir = rand() % 2 ? -1 : 1;
    x_coord = x_off == 0 ? x + dir : x + (dir * x_off);
    if(in_bounds(x_coord, y - 1)){
        j = get_index(x_coord, y - 1);
        particle_t temp = simulation->particles[j];

        if(temp.id == empty_id){
            p_set(*p, j);
            p_set(temp, i);
            return;
        }
    }

    // try second diagonal (same thing with opposite direction)
    x_coord = x_off == 0 ? x - dir : x + (-dir * x_off);    
    if(in_bounds(x_coord, y - 1)){
        j = get_index(x_coord, y - 1);
        particle_t temp = simulation->particles[j];

        if(temp.id == empty_id){
            p_set(*p, j);
            p_set(temp, i);
            return;
        }
    }

    p_set(*p, i);
}

/*      UPDATE WATER        */
// Try to move bellow or diagonals, like sand
// Try to move directly to the side
void update_water(particle_t *p, int x, int y){
    float max_speed_fall = -10.0;
    float max_spread = 10.0;

    p->velocity[0] = p->velocity[0] > max_spread ? max_spread : p->velocity[0];
    p->velocity[1] = p->velocity[1] < max_speed_fall ? max_speed_fall : p->velocity[1];
    p->velocity[1] = p->velocity[1] > 0.0 ? 0.0 : p->velocity[1];

    int i = get_index(x, y);
    int x_off, y_off, x_coord;
    int j;

    int dir = p->velocity[0] > 0 ? 1 : -1;

    // Try moving bellow
    x_coord = x_off == 0 ? x : x;
    y_off = round(p->velocity[1]);
    if(in_bounds(x_coord, y - 1 + y_off)){
        int j = get_index(x_coord, y - 1 + y_off);
        particle_t temp = simulation->particles[j];

        if(temp.id == empty_id){
            p->velocity[0] -= dir;
            p->velocity[1] -= gravity;   

            p_set(*p, j);
            p_set(temp, i);
            return;
        }
    }

    // Try first diagonal
    p->velocity[0] += 2 * dir * ((float)rand())/RAND_MAX * p->velocity[1];
    p->velocity[0] = p->velocity[0] > max_spread ? max_spread : p->velocity[0];

    p->velocity[1] += gravity;

    x_coord = x_off == 0 ? x + dir : x + x_off;
    if(in_bounds(x_coord, y - 1)){
        j = get_index(x_coord, y - 1);
        particle_t temp = simulation->particles[j];

        if(temp.id == empty_id){
            p->velocity[0] += dir;

            p_set(*p, j);
            p_set(temp, i);
            return;
        }
    }

    // Try other diagonal (same thing with opposite direction)
    float old_vel = p->velocity[0];
    p->velocity[0] *= -0.5;
    x_off = round(p->velocity[0]);
    x_coord = x_off == 0 ? x - dir : x + x_off;
    if(in_bounds(x_coord, y - 1)){
        j = get_index(x_coord, y - 1);
        particle_t temp = simulation->particles[j];
        
        if(temp.id == empty_id){
            p->velocity[0] += -dir;

            p_set(*p, j);
            p_set(temp, i);
            return;
        }
    }

    // Try to move to the side
    p->velocity[0] = old_vel;
    x_off = round(p->velocity[0]);
    x_coord = x_off == 0 ? x + dir : x + x_off;
    if(in_bounds(x_coord, y)){
        j = get_index(x_coord, y);
        particle_t temp = simulation->particles[j];
        
        if(temp.id == empty_id){
            p->velocity[0] += dir;

            p_set(*p, j);
            p_set(temp, i);
            return;
        }
    }

    // Try other side
    p->velocity[0] *= -0.5;
    x_coord = x_off == 0 ? x - dir : x + x_off;
    if(in_bounds(x_coord, y)){
        j = get_index(x_coord, y);
        particle_t temp = simulation->particles[j];

        if(temp.id == empty_id){
            p->velocity[0] -= dir;

            p_set(*p, j);
            p_set(temp, i);
            return;
        }
    }

    p_set(*p, i);
}

void update_coal(particle_t *p, int x, int y){
    float max_spread = 5.0;
    float max_speed_fall = -10.0;

    p->velocity[0] = p->velocity[0] < 0.0 ? 0.0 : p->velocity[0];
    p->velocity[1] = p->velocity[1] < max_speed_fall ? max_speed_fall : p->velocity[1];
    p->velocity[1] = p->velocity[1] > 0.0 ? 0.0 : p->velocity[1];

    int i = get_index(x, y);
    int x_off, y_off, x_coord;
    int j;

    // try moving bellow:
    x_off = round(p->velocity[0]);
    y_off = round(p->velocity[1]);

    if(in_bounds(x_off == 0 ? x : x * x_off, y - 1 + y_off)){
        j = get_index(x_off == 0 ? x : x * x_off, y - 1 + y_off);
        particle_t temp = simulation->particles[j];
        
        if(temp.id == empty_id){
            p->velocity[0] -= 1.0;
            p->velocity[1] -= gravity;
            p_set(*p, j);
            p_set(temp, i);
            return;
        }
    }

    p->velocity[0] = 0.3 * ((float)rand())/RAND_MAX * p->velocity[1];
    p->velocity[0] = p->velocity[0] > max_spread ? max_spread : p->velocity[0];

    p->velocity[1] += gravity;

    x_off = round(p->velocity[0]);
    y_off = round(p->velocity[1]);
        
    p->velocity[0] -= 1.0;

    // try first diagonal
    int dir = rand() % 2 ? -1 : 1;
    x_coord = x_off == 0 ? x + dir : x + (dir * x_off);
    if(in_bounds(x_coord, y - 1)){
        j = get_index(x_coord, y - 1);
        particle_t temp = simulation->particles[j];

        if(temp.id == empty_id){
            p_set(*p, j);
            p_set(temp, i);
            return;
        }
    }

    // try second diagonal (same thing with opposite direction)
    x_coord = x_off == 0 ? x - dir : x + (-dir * x_off);    
    if(in_bounds(x_coord, y - 1)){
        j = get_index(x_coord, y - 1);
        particle_t temp = simulation->particles[j];

        if(temp.id == empty_id){
            p_set(*p, j);
            p_set(temp, i);
            return;
        }
    }

    p_set(*p, i);

}