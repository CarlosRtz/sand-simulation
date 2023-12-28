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
    particle_t p;
    return p;
}

particle_t new_coal(){
    particle_t p;
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


/* Update particles */
void update_empty(particle_t *p, int x, int y){
    int i = get_index(x, y);
    p_set(*p, i);
}


/*      UPDATE SAND     */
// Try moving bellow or diagonals
// Increse y speed when falling
void update_sand(particle_t *p, int x, int y){
    float max_speed_x = 5.0;
    float min_speed_y = -10.0;

    p->velocity[0] = p->velocity[0] < 0.0 ? 0.0 : p->velocity[0];
    p->velocity[1] = p->velocity[1] < min_speed_y ? min_speed_y : p->velocity[1];
    
    int i = get_index(x, y);
    int x_off, y_off;
    int j;

    // try moving bellow:
    x_off = round(p->velocity[0]);
    y_off = round(p->velocity[1]);
    if(in_bounds(x + x_off, y - 1 + y_off)){
        j = get_index(x + x_off, y - 1 + y_off);
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
    p->velocity[0] = p->velocity[0] > max_speed_x ? max_speed_x : p->velocity[0];

    p->velocity[1] += gravity;
    if(p->velocity[1] > 0.0){
        p->velocity[1] = 0.0;
    }

    x_off = round(p->velocity[0]);
    y_off = round(p->velocity[1]);
    p->velocity[0] -= 1.0;

    // try first diagonal
    int dir = rand() % 2 ? -1 : 1;
    if(in_bounds(x + (dir * x_off), y - 1)){
        j = get_index(x + (dir * x_off), y - 1);
        particle_t temp = simulation->particles[j];

        if(temp.id == empty_id){
            p_set(*p, j);
            p_set(temp, i);
            return;
        }
    }

    // try second diagonal (same thing with opposite direction)
    if(in_bounds(x + (dir * x_off), y - 1)){
        j = get_index(x + (dir * x_off), y - 1);
        particle_t temp = simulation->particles[j];

        if(temp.id == empty_id){
            p_set(*p, j);
            p_set(temp, i);
            return;
        }
    }

    p_set(*p, i);
}