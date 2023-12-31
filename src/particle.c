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

/*          Create particles            */
particle_t new_empty(){
    particle_t p = {
        .id = empty_id,
        .color = {.r=0, .g=0, .b=0, .a=255},
        .velocity = {.x=0, .y=0},
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
        .velocity = {.x=0.0, .y=0.0},
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
        .velocity = {.x=0.0, .y=0.0},
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
        .velocity = {.x=0.0, .y=0.0},
        .life_time = 1.0,
        .updated = 0,
        .update = update_coal
    };
    return p;
}

particle_t new_oil(){
    particle_t p = {
        .id = oil_id,
        .color = {.r=130, .g=130, .b=105, .a=255},
        .velocity = {.x=0.0, .y=0.0},
        .life_time = 1,
        .updated = 0,
        .update = update_oil
    };
    return p;
}

particle_t new_fire(){
    int g = (rand() % (200 - 100 + 1)) + 100;
    // float life_time = ((10.0 - 1.0) * ((float)rand() / RAND_MAX)) + 1.0;
    particle_t p = {
        .id = fire_id,
        .color = {.r=230, .g=g, .b=50, .a=255},
        .velocity = {0.0, 0.0},
        .life_time = 1.0,
        .updated = 0,
        .update = update_fire
    };
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


/*      Update particles        */

/*      UPDATE EMPTY PARTICLE       */
// Do nothing
void update_empty(particle_t *p, int x, int y){
    p_set(*p, get_index(x, y));
}


/*      UPDATE SAND PARTICLE        */
// Try moving bellow else
// try moving to both lower diagonals
#define __sand_max_spread 2.0
#define __sand_max_fall_speed -10.0
void update_sand(particle_t *p, int x, int y){
    int i = get_index(x, y);

    // limit velocities if needed
    if(p->velocity.x > __sand_max_spread) p->velocity.x = __sand_max_spread;
    if(p->velocity.x < - __sand_max_spread) p->velocity.x = - __sand_max_spread;
    if(p->velocity.y > 0.0) p->velocity.y = 0.0;
    if(p->velocity.y < __sand_max_fall_speed) p->velocity.y = __sand_max_fall_speed;

    int j;
    int x_off, y_off, x_coord, y_coord;
    
    x_off = round(p->velocity.x);
    y_off = round(p->velocity.y);

    // Try moving bellow
    x_coord = x + x_off;
    y_coord = y - 1 + y_off;
    
    if(in_bounds(x_coord, y_coord)){
        j = get_index(x_coord, y_coord);
        particle_t temp = simulation->particles[j];
        
        if(temp.id == empty_id){
            p->velocity.x *= 0.8;
            p->velocity.y -= gravity;
            p_set(*p, j);
            p_set(temp, i);
            return;
        }
    }

    // Try moving to the diagonal
    // if x velocity is 0, choose a random direction;
    if(p->velocity.x == 0.0){
        int r = rand() % 2 ? -1 : 1;
        p->velocity.x = r * (float)rand()/RAND_MAX * p->velocity.y;
        if(p->velocity.x > __sand_max_spread) p->velocity.x = __sand_max_spread;
        if(p->velocity.x < - __sand_max_spread) p->velocity.x = - __sand_max_spread;
    }

    int dir = p->velocity.x > 0.0 ? 1 : -1;
    x_off = round(p->velocity.x);


    x_coord = x_off == 0 ? x + dir : x + x_off;
    y_coord = y - 1;
    if(in_bounds(x_coord, y_coord)){
        j = get_index(x_coord, y_coord);
        particle_t temp = simulation->particles[j];

        if(temp.id == empty_id){
            p->velocity.x += dir;
            p->velocity.y += gravity;
            p_set(*p, j);
            p_set(temp, i);
        }
    }

    // Try opossite diagonal
    p->velocity.x *= -0.5;
    x_off = round(p->velocity.x);
    x_coord = x_off == 0 ? x - dir : x + x_off;
    if(in_bounds(x_coord, y_coord)){
        j = get_index(x_coord, y_coord);
        particle_t temp = simulation->particles[j];

        if(temp.id == empty_id){
            p->velocity.x -= dir;
            p->velocity.y += gravity;
            p_set(*p, j);
            p_set(temp, i);
        }
    }

    p->velocity.y += gravity;
    p->velocity.x = 0.0;
    p_set(*p, i);
    return;
}

/*      UPDATE WATER PARTICLE       */
// Try moving bellow else
// try moving to the diagonals like sand.
// Also try to move directly to the side
#define __water_max_spread 8.0
#define __water_max_fall_speed -10.0
#define __water_sink_chance 0.10
void update_water(particle_t *p, int x, int y){
    int i = get_index(x, y);

    // limit velocities if needed
    if(p->velocity.x > __water_max_spread) p->velocity.x = __water_max_spread;
    if(p->velocity.x < - __water_max_spread) p->velocity.x = - __water_max_spread;
    if(p->velocity.y > 0.0) p->velocity.y = 0.0;
    if(p->velocity.y < __water_max_fall_speed) p->velocity.y = __water_max_fall_speed;

    int j;
    int x_off, y_off, x_coord, y_coord;
    
    x_off = round(p->velocity.x);
    y_off = round(p->velocity.y);

    // Try moving bellow
    x_coord = x + x_off;
    y_coord = y - 1 + y_off;
    
    if(in_bounds(x_coord, y_coord)){
        j = get_index(x_coord, y_coord);
        particle_t temp = simulation->particles[j];
        
        if(temp.id == empty_id){
            p->life_time = 1.0;
            p->velocity.x *= 0.8;
            p->velocity.y -= gravity;
            p_set(*p, j);
            p_set(temp, i);
            return;
        }

        if(temp.id == oil_id){
            if(rand() < RAND_MAX * __water_sink_chance){
                p->life_time = 1.0;
                p->velocity.x *= 0.3;
                p->velocity.y -= gravity * 0.5;

                temp.velocity.x = 0.0;
                p_set(*p, j);
                p_set(temp, i);
                return;
            }
        }
    }
    
    // Try moving to the diagonal
    // if x velocity is 0, choose a random direction;
    if(p->velocity.x == 0.0){
        int r = rand() % 2 ? -1 : 1;
        p->velocity.x = r * (float)rand()/RAND_MAX * p->velocity.y;
        if(p->velocity.x > __water_max_spread) p->velocity.x = __water_max_spread;
        if(p->velocity.x < - __water_max_spread) p->velocity.x = - __water_max_spread;
    }

    int dir = p->velocity.x > 0.0 ? 1 : -1;
    x_off = round(p->velocity.x);


    x_coord = x_off == 0 ? x + dir : x + x_off;
    y_coord = y - 1;
    if(in_bounds(x_coord, y_coord)){
        j = get_index(x_coord, y_coord);
        particle_t temp = simulation->particles[j];

        if(temp.id == empty_id){
            p->life_time = 1.0;
            p->velocity.x += dir;
            p->velocity.y += gravity;
            p_set(*p, j);
            p_set(temp, i);
            return;
        }

        if(temp.id == oil_id){
            if(rand() < RAND_MAX * __water_sink_chance){
                p->life_time = 1.0;
                p->velocity.x += dir * 0.5;
                p->velocity.y += gravity * 2;
                
                p_set(*p, j);
                p_set(temp, i);
            }
        }
    }

    // Try opossite diagonal
    float old_velocity = p->velocity.x;
    p->velocity.x *= -0.5;
    x_off = round(p->velocity.x);
    x_coord = x_off == 0 ? x - dir : x + x_off;
    if(in_bounds(x_coord, y_coord)){
        j = get_index(x_coord, y_coord);
        particle_t temp = simulation->particles[j];

        if(temp.id == empty_id){
            p->life_time = 1.0;
            p->velocity.x += -dir;
            p->velocity.y += gravity;
            p_set(*p, j);
            p_set(temp, i);
            return;
        }

        if(temp.id == oil_id){
            if(rand() < RAND_MAX * __water_sink_chance){
                p->life_time = 1.0;
                p->velocity.x += -dir;
                p->velocity.y += gravity * 2;
                
                p_set(*p, j);
                p_set(temp, i);
            }
        }
    }

    // Try moving to the side
    p->life_time -= 0.005;
    p->velocity.x = old_velocity;
    x_off = round(p->velocity.x);
    x_coord = x_off == 0 ? x + dir : x + x_off;
    y_coord = y;
    if(in_bounds(x_coord, y_coord)){
        j = get_index(x_coord, y_coord);
        particle_t temp = simulation->particles[j];

        if(temp.id == empty_id){
            int k = abs(x_coord - x);
            int blocked_path = 0;
            for(int n = 1; n < k; n++){
                if(simulation->particles[get_index(x + n, y_coord)].id != empty_id){
                    blocked_path = 1;
                    break;
                }
            }
            if(!blocked_path){
                if(p->life_time < 0.0){
                    p->velocity.x *= 0.5;
                }else{
                    p->velocity.x += dir;
                }
                p->velocity.y += gravity;
                p_set(*p, j);
                p_set(temp, i);
                return;
            }
        }

        if(temp.id == oil_id){
            if(rand() < RAND_MAX * __water_sink_chance){
                p->life_time = 1.0;
                p->velocity.x += dir * 0.5;
                p->velocity.y += gravity * 2;

                p_set(*p, j);
                p_set(temp, i);
            }
        }
    }

    // Try other side
    p->velocity.x *= -0.5;
    x_off = round(p->velocity.x);
    x_coord = x_off == 0 ? x - dir : x + x_off;
    if(in_bounds(x_coord, y_coord)){
        j = get_index(x_coord, y_coord);
        particle_t temp = simulation->particles[j];

        if(temp.id == empty_id){
            int k = abs(x_coord - x);
            int blocked_path = 0;
            for(int n = 1; n < k; n++){
                if(simulation->particles[get_index(x + n, y_coord)].id != empty_id){
                    blocked_path = 1;
                    break;
                }
            }
            if(!blocked_path){
                if(p->life_time < 0.0){
                    p->velocity.x *= 0.5;
                }else{
                    p->velocity.x -= dir;
                }
                p->velocity.y += gravity;
                p_set(*p, j);
                p_set(temp, i);
                return;
            }
        }

        if(temp.id == oil_id){
            if(rand() < RAND_MAX * __water_sink_chance){
                p->life_time = 1.0;
                p->velocity.x += -dir * 0.5;
                p->velocity.y += gravity * 2;
                
                p_set(*p, j);
                p_set(temp, i);
            }
        }
    }

    p->velocity.y += gravity;
    p->velocity.x = 0.0;
    p_set(*p, i);
    return;    
}

/*      UPDATE COAL PARTICLE        */
// Try moving bellow or to diagonals like sand
// but spreads less
#define __coal_max_spread 0.0
#define __coal_max_fall_speed -10.0
void update_coal(particle_t *p, int x, int y){
    int i = get_index(x, y);

    // limit velocities if needed
    if(p->velocity.x > __coal_max_spread) p->velocity.x = __coal_max_spread;
    if(p->velocity.x < - __coal_max_spread) p->velocity.x = - __coal_max_spread;
    if(p->velocity.y > 0.0) p->velocity.y = 0.0;
    if(p->velocity.y < __coal_max_fall_speed) p->velocity.y = __coal_max_fall_speed;

    int j;
    int x_off, y_off, x_coord, y_coord;
    
    x_off = round(p->velocity.x);
    y_off = round(p->velocity.y);

    // Try moving bellow
    x_coord = x + x_off;
    y_coord = y - 1 + y_off;
    
    if(in_bounds(x_coord, y_coord)){
        j = get_index(x_coord, y_coord);
        particle_t temp = simulation->particles[j];
        
        if(temp.id == empty_id){
            p->velocity.x *= 0.8;
            p->velocity.y -= gravity;
            p_set(*p, j);
            p_set(temp, i);
            return;
        }
    }

    // Try moving to the diagonal
    // if x velocity is 0, choose a random direction;
    if(p->velocity.x == 0.0){
        int r = rand() % 2 ? -1 : 1;
        p->velocity.x = r * (float)rand()/RAND_MAX * p->velocity.y;
        if(p->velocity.x > __sand_max_spread) p->velocity.x = __sand_max_spread;
        if(p->velocity.x < - __sand_max_spread) p->velocity.x = - __sand_max_spread;
    }

    int dir = p->velocity.x > 0.0 ? 1 : -1;
    x_off = round(p->velocity.x);


    x_coord = x_off == 0 ? x + dir : x + x_off;
    y_coord = y - 1;
    if(in_bounds(x_coord, y_coord)){
        j = get_index(x_coord, y_coord);
        particle_t temp = simulation->particles[j];

        if(temp.id == empty_id){
            p->velocity.x += dir;
            p->velocity.y += gravity;
            p_set(*p, j);
            p_set(temp, i);
            return;
        }
    }

    // Try opossite diagonal
    p->velocity.x *= -0.5;
    x_off = round(p->velocity.x);
    x_coord = x_off == 0 ? x - dir : x + x_off;
    if(in_bounds(x_coord, y_coord)){
        j = get_index(x_coord, y_coord);
        particle_t temp = simulation->particles[j];

        if(temp.id == empty_id){
            p->velocity.x -= dir;
            p->velocity.y += gravity;
            p_set(*p, j);
            p_set(temp, i);
            return;
        }
    }

    p->velocity.y += gravity;
    p->velocity.x = 0.0;
    p_set(*p, i);
    return;    
}

/*      UPDATE OIL PARTICLE         */
// Try to move like water
// but spreads less
// hardly mixes with water
#define __oil_max_spread 5.0
#define __oil_max_fall_speed -10.0
#define __oil_sink_chance 0.05
void update_oil(particle_t *p, int x, int y){
    int i = get_index(x, y);

    // limit velocities if needed
    if(p->velocity.x > __oil_max_spread) p->velocity.x = __oil_max_spread;
    if(p->velocity.x < - __oil_max_spread) p->velocity.x = - __oil_max_spread;
    if(p->velocity.y > 0.0) p->velocity.y = 0.0;
    if(p->velocity.y < __oil_max_fall_speed) p->velocity.y = __oil_max_fall_speed;

    int j;
    int x_off, y_off, x_coord, y_coord;
    
    x_off = round(p->velocity.x);
    y_off = round(p->velocity.y);

    // Try moving bellow
    x_coord = x + x_off;
    y_coord = y - 1 + y_off;
    
    if(in_bounds(x_coord, y_coord)){
        j = get_index(x_coord, y_coord);
        particle_t temp = simulation->particles[j];
        
        if(temp.id == empty_id){
            p->life_time = 1.0;
            p->velocity.x *= 0.8;
            p->velocity.y -= gravity;
            p_set(*p, j);
            p_set(temp, i);
            return;
        }

        if(temp.id == water_id){
            if(rand() < RAND_MAX * __oil_sink_chance){
                p->life_time = 1.0;
                p->velocity.x *= 0.3;
                p->velocity.y -= gravity * 0.5;

                p_set(*p, j);
                p_set(temp, i);
                return;
            }
        }
    }
    
    // Try moving to the diagonal
    // if x velocity is 0, choose a random direction;
    if(p->velocity.x == 0.0){
        int r = rand() % 2 ? -1 : 1;
        p->velocity.x = r * (float)rand()/RAND_MAX * p->velocity.y;
        if(p->velocity.x > __water_max_spread) p->velocity.x = __water_max_spread;
        if(p->velocity.x < - __water_max_spread) p->velocity.x = - __water_max_spread;
    }

    int dir = p->velocity.x > 0.0 ? 1 : -1;
    x_off = round(p->velocity.x);


    x_coord = x_off == 0 ? x + dir : x + x_off;
    y_coord = y - 1;
    if(in_bounds(x_coord, y_coord)){
        j = get_index(x_coord, y_coord);
        particle_t temp = simulation->particles[j];

        if(temp.id == empty_id){
            p->life_time = 1.0;
            p->velocity.x += dir * 0.5;
            p->velocity.y += gravity;
            p_set(*p, j);
            p_set(temp, i);
            return;
        }

        if(temp.id == water_id){
            if(rand() < RAND_MAX * __oil_sink_chance){
                p->life_time = 1.0;
                p->velocity.x += dir * 0.25;
                p->velocity.y += gravity * 2;
                
                p_set(*p, j);
                p_set(temp, i);
            }
        }
    }

    // Try opossite diagonal
    float old_velocity = p->velocity.x;
    p->velocity.x *= -0.5;
    x_off = round(p->velocity.x);
    x_coord = x_off == 0 ? x - dir : x + x_off;
    if(in_bounds(x_coord, y_coord)){
        j = get_index(x_coord, y_coord);
        particle_t temp = simulation->particles[j];

        if(temp.id == empty_id){
            p->life_time = 1.0;
            p->velocity.x += -dir * 0.5;
            p->velocity.y += gravity;
            p_set(*p, j);
            p_set(temp, i);
            return;
        }

        if(temp.id == water_id){
            if(rand() < RAND_MAX * __oil_sink_chance){
                p->life_time = 1.0;
                p->velocity.x += -dir * 0.25;
                p->velocity.y += gravity * 0.5;
                
                p_set(*p, j);
                p_set(temp, i);
            }
        }
    }

    // Try moving to the side
    p->life_time -= 0.005;
    p->velocity.x = old_velocity;
    x_off = round(p->velocity.x);
    x_coord = x_off == 0 ? x + dir : x + x_off;
    y_coord = y;
    if(in_bounds(x_coord, y_coord)){
        j = get_index(x_coord, y_coord);
        particle_t temp = simulation->particles[j];

        if(temp.id == empty_id){
            int k = abs(x_coord - x);
            int blocked_path = 0;
            for(int n = 1; n < k; n++){
                if(simulation->particles[get_index(x + n, y_coord)].id != empty_id){
                    blocked_path = 1;
                    break;
                }
            }
            if(!blocked_path){
                if(p->life_time < 0.0){
                    p->velocity.x *= 0.5;
                }else{
                    p->velocity.x += dir * 0.5;
                }
                p->velocity.y += gravity;
                p_set(*p, j);
                p_set(temp, i);
                return;
            }
        }

        if(temp.id == water_id){
            if(rand() < RAND_MAX * __oil_sink_chance){
                p->life_time = 1.0;
                p->velocity.x += dir * 0.25;
                p->velocity.y += gravity * 2;
                
                p_set(*p, j);
                p_set(temp, i);
            }
        }
    }

    // Try other side
    p->velocity.x *= -0.5;
    x_off = round(p->velocity.x);
    x_coord = x_off == 0 ? x - dir : x + x_off;
    if(in_bounds(x_coord, y_coord)){
        j = get_index(x_coord, y_coord);
        particle_t temp = simulation->particles[j];

        if(temp.id == empty_id){
            int k = abs(x_coord - x);
            int blocked_path = 0;
            for(int n = 1; n < k; n++){
                if(simulation->particles[get_index(x + n, y_coord)].id != empty_id){
                    blocked_path = 1;
                    break;
                }
            }
            if(!blocked_path){
                if(p->life_time < 0.0){
                    p->velocity.x *= 0.5;
                }else{
                    p->velocity.x -= dir * 0.5;
                }
                p->velocity.y += gravity;
                p_set(*p, j);
                p_set(temp, i);
                return;
            }
        }

        if(temp.id == water_id){
            if(rand() < RAND_MAX * __oil_sink_chance){
                p->life_time = 1.0;
                p->velocity.x -= dir * 0.25;
                p->velocity.y += gravity * 2;
                
                // temp.velocity.y = 0.0;
                p_set(*p, j);
                p_set(temp, i);
            }
        }
    }

    p->velocity.y += gravity;
    p->velocity.x = 0.0;
    p_set(*p, i);
    return;

    return;
}

/*      UPDATE FIRE PARTICLE        */
// Fire have a short life time
// Spreads by burning flamable materials
// like coal and oil
// Turns into steam on the water
#define __fire_max_fall_speed -2.0
#define __coal_burn_chance 0.01
#define __oil_burn_chance 0.9
void update_fire(particle_t *p, int x, int y){
    int i = get_index(x, y);

    // limit velocities if needed
    if(p->velocity.y > 0.0) p->velocity.y = 0.0;
    if(p->velocity.y < __fire_max_fall_speed) p->velocity.y = __fire_max_fall_speed;

    int x_off, y_off, x_coord, y_coord;
    int j;

    // Try to spread bellow
    if(in_bounds(x, y - 1)){
        j = get_index(x, y - 1);
        particle_t temp = simulation->particles[j];

        if(temp.id == coal_id){
            if(rand() < RAND_MAX * __coal_burn_chance){
                p->life_time = 10.0;
                p_set(new_fire(), j);
                simulation->particles[j].life_time = 10.0;
            }
        }

        if(temp.id == oil_id){
            if(rand() < RAND_MAX * __oil_burn_chance){
                p->life_time = 1.0;
                p_set(new_fire(), j);
            }
        }
    }

    // Try to spread to each side
    if(in_bounds(x + 1, y)){
        j = get_index(x + 1, y);
        particle_t temp = simulation->particles[j];

        if(temp.id == coal_id){
            if(rand() < RAND_MAX * __coal_burn_chance){
                p->life_time = 10.0;
                p_set(new_fire(), j);
                simulation->particles[j].life_time = 10.0;
            }
        }

        if(temp.id == oil_id){
            if(rand() < RAND_MAX * __oil_burn_chance){
                p->life_time = 1.0;
                p_set(new_fire(), j);
            }
        }
    }

    if(in_bounds(x - 1, y)){
        j = get_index(x - 1, y);
        particle_t temp = simulation->particles[j];

        if(temp.id == coal_id){
            if(rand() < RAND_MAX * __coal_burn_chance){
                p->life_time = 10.0;
                p_set(new_fire(), j);
                simulation->particles[j].life_time = 10.0;
            }
        }

        if(temp.id == oil_id){
            if(rand() < RAND_MAX * __oil_burn_chance){
                p->life_time = 1.0;
                p_set(new_fire(), j);
            }
        }
    }
    // Try to spread up
    if(in_bounds(x, y + 1)){
        j = get_index(x, y + 1);
        particle_t temp = simulation->particles[j];

        if(temp.id == coal_id){
            if(rand() < RAND_MAX * __coal_burn_chance){
                p->life_time = 10.0;
                p_set(new_fire(), j);
                simulation->particles[j].life_time = 10.0;
            }
        }

        if(temp.id == oil_id){
            if(rand() < RAND_MAX * __oil_burn_chance){
                p->life_time = 1.0;
                p_set(new_fire(), j);
            }
        }
    }

    // Try to spread on diagonals
    if(in_bounds(x + 1, y - 1)){
        j = get_index(x + 1, y - 1);
        particle_t temp = simulation->particles[j];

        if(temp.id == coal_id){
            if(rand() < RAND_MAX * __coal_burn_chance){
                p->life_time = 10.0;
                p_set(new_fire(), j);
                simulation->particles[j].life_time = 10.0;
            }
        }

        if(temp.id == oil_id){
            if(rand() < RAND_MAX * __oil_burn_chance){
                p->life_time = 1.0;
                p_set(new_fire(), j);
            }
        }
    }

    if(in_bounds(x - 1, y - 1)){
        j = get_index(x - 1, y - 1);
        particle_t temp = simulation->particles[j];

        if(temp.id == coal_id){
            if(rand() < RAND_MAX * __coal_burn_chance){
                p->life_time = 10.0;
                p_set(new_fire(), j);
                simulation->particles[j].life_time = 10.0;
            }
        }

        if(temp.id == oil_id){
            if(rand() < RAND_MAX * __oil_burn_chance){
                p->life_time = 1.0;
                p_set(new_fire(), j);
            }
        }
    }

    if(in_bounds(x + 1, y + 1)){
        j = get_index(x + 1, y + 1);
        particle_t temp = simulation->particles[j];

        if(temp.id == coal_id){
            if(rand() < RAND_MAX * __coal_burn_chance){
                p->life_time = 10.0;
                p_set(new_fire(), j);
                simulation->particles[j].life_time = 10.0;
            }
        }

        if(temp.id == oil_id){
            if(rand() < RAND_MAX * __oil_burn_chance){
                p->life_time = 1.0;
                p_set(new_fire(), j);
            }
        }
    }

    if(in_bounds(x - 1, y + 1)){
        j = get_index(x - 1, y + 1);
        particle_t temp = simulation->particles[j];

        if(temp.id == coal_id){
            if(rand() < RAND_MAX * __coal_burn_chance){
                p->life_time = 10.0;
                p_set(new_fire(), j);
                simulation->particles[j].life_time = 10.0;
            }
        }

        if(temp.id == oil_id){
            if(rand() < RAND_MAX * __oil_burn_chance){
                p->life_time = 1.0;
                p_set(new_fire(), j);
            }
        }
    }

    // try to move bellow
    x_off = round(p->velocity.x);
    y_off = round(p->velocity.y);
    x_coord = x + x_off;
    y_coord = y - 1 + y_off;
    if(in_bounds(x_coord, y_coord)){
        j = get_index(x_coord, y_coord);
        particle_t temp = simulation->particles[j];
        
        if(temp.id == empty_id){
            p->velocity.y -= gravity * 0.25;
            p->life_time -= 0.030;
            if(p->life_time < 0.0){
                *p = new_empty();
            }
            p_set(*p, j);
            p_set(temp, i);
            return;
        }
    }

    p->life_time -= 0.030;
    p->velocity.y += gravity;
    if(p->life_time < 0.0){
        *p = new_empty();
    }
    p_set(*p, i);
    return;
}