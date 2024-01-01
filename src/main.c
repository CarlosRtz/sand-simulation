#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <time.h>
#include "particle.h"

int window_width = 800;
int window_height = 600;

GLFWwindow *window;
GLuint texId;

double cursor_x;
double cursor_y;

double world_x;
double world_y;

float cursor_r = 0.1;
int pressed_left_btn = 0;
int pressed_right_btn = 0;
uint8_t selected_particle = sand_id;

void window_size_callback(GLFWwindow *window, int width, int height);
void cursor_pos_callback(GLFWwindow *window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow *window, int button, int action, int mods);
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
void throw_particles();

int setup_window(){
    if(!glfwInit()) return -1;

    window = glfwCreateWindow(window_width, window_height, "Sand Simulation", NULL, NULL);
    if(!window){
        glfwTerminate();
        return 0;
    }

    glfwMakeContextCurrent(window);

    glfwSetWindowSizeCallback(window, window_size_callback);
    glfwSetCursorPosCallback(window, cursor_pos_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetKeyCallback(window, key_callback);

    return 1;
}

void setupGL(){
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glClearColor(1, 1, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    glGenTextures(1, &texId);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
}

void render(GLFWwindow *window){
    glMatrixMode(GL_MODELVIEW);
    glfwMakeContextCurrent(window);

    glClear(GL_COLOR_BUFFER_BIT);

    glTexImage2D(
        GL_TEXTURE_2D, 
        0,
        GL_RGBA8,
        simulation->width,
        simulation->height, 
        0, 
        GL_RGBA, 
        GL_UNSIGNED_BYTE, 
        simulation->texture_buffer
    );

    glEnable(GL_TEXTURE_2D);
    glBegin(GL_QUADS);
        glTexCoord2f(0, 0);
        glVertex3f(-1, -1, 0);
        glTexCoord2f(1, 0);
        glVertex3f(1, -1, 0);
        glTexCoord2f(1, 1);
        glVertex3f(1, 1, 0);
        glTexCoord2f(0, 1);
        glVertex3f(-1, 1, 0);
    glEnd();

    glDisable(GL_TEXTURE_2D);

    glColor3f(1.0, 0, 0.0);
    glBegin(GL_LINE_LOOP);
    int n = 50;
    for(int i = 0; i < n; i++){
        float theta = 2.0f * 3.1415926f * (float)i / (float)n;

        float x = cursor_r * cosf(theta);
        float y = cursor_r * sinf(theta);

        glVertex2f(x + world_x, y + world_y);
    }
    glEnd();

    glPopMatrix();
    glfwSwapBuffers(window);
}

int main(){
    if(!setup_window()){
        glfwTerminate();
        return -1;
    }

    srand(time(NULL));
    init_simulation(512, 512);
    setupGL();

    double limit_fps = 1.0/60.0;
    double last_time = glfwGetTime();
    double reset_rand_seed = glfwGetTime();

    while(!glfwWindowShouldClose(window)){
        double now = glfwGetTime();
        if(now - last_time > 1.0/60.0){
            if(pressed_left_btn){
                throw_particles();
            }

            update_simulation();
            last_time = glfwGetTime();
        }
     
        if(glfwGetTime() - reset_rand_seed > 300){
            srand(time(NULL));
            reset_rand_seed = glfwGetTime();
        }

        render(window);

        glfwPollEvents();
    }

    destroy_simulation();
    glfwTerminate();    
    return 0;
}


// callbacks
void window_size_callback(GLFWwindow *window, int width, int height){
    window_width = width;
    window_height = height;
    glViewport(0, 0, width, height);
    render(window);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    render(window);
}

void cursor_pos_callback(GLFWwindow *window, double xpos, double ypos){
    cursor_x = xpos;
    cursor_y = ypos;
    
    world_x = 2*(xpos/window_width)-1;
    world_y = -2*(ypos/window_height)+1;
}

void mouse_button_callback(GLFWwindow *window, int button, int action, int mods){
    switch(button){
        case GLFW_MOUSE_BUTTON_LEFT:
            if(action == GLFW_PRESS){
                pressed_left_btn = 1;
                pressed_right_btn = 0;
            }
            if(action == GLFW_RELEASE){
                pressed_left_btn = 0;
            }
        break;

        case GLFW_MOUSE_BUTTON_RIGHT:
            if(action == GLFW_PRESS){
                pressed_left_btn = 0;
                pressed_right_btn = 1;
            }
            if(action == GLFW_RELEASE){
                pressed_right_btn = 0;
            }
        break;
    }
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods){
    switch(key){
        case GLFW_KEY_1:
            if(action == GLFW_PRESS)
                selected_particle = sand_id;
        break;
        case GLFW_KEY_2:
            if(action == GLFW_PRESS)
                selected_particle = water_id;
        break;
        case GLFW_KEY_3:
            if(action == GLFW_PRESS)
                selected_particle = coal_id;
        break;
        case GLFW_KEY_4:
            if(action == GLFW_PRESS)
                selected_particle = oil_id;
        break;
        case GLFW_KEY_5:
            if(action == GLFW_PRESS)
                selected_particle = fire_id;
        break;
        case GLFW_KEY_BACKSPACE:
            clear_particles();
        break;
        case GLFW_KEY_ESCAPE:
            if(action == GLFW_PRESS){
                glfwSetWindowShouldClose(window, GL_TRUE);
            }
        break;
    }
}

void throw_particles(){
    double xpos = world_x + 1;
    double ypos = world_y + 1; 

    int x = round(xpos*simulation->width/2.0);
    int y = round(ypos*simulation->height/2.0);

    int rx = round(cursor_r*simulation->width/2.0);
    int ry = round(cursor_r*simulation->height/2.0);

    for(int k = 0; k < 50; k++){
        int angle = rand() % 360;
        float m = ((float)rand())/((float)RAND_MAX);
        int i = round(sin( 3.1415926 *angle/180.0)*rx*m) + x;
        int j = round(cos( 3.1415926 *angle/180.0)*ry*m) + y;
        
        int index = get_index(i, j);
        switch(selected_particle){
            case sand_id:
                if(in_bounds(i, j) && simulation->particles[index].id == empty_id)
                    simulation->particles[index] = new_sand();
            break;
            case water_id:
                if(in_bounds(i, j) && simulation->particles[index].id == empty_id)
                    simulation->particles[index] = new_water();
            break;
            case coal_id:
                if(in_bounds(i, j) && simulation->particles[index].id == empty_id)
                    simulation->particles[index] = new_coal();
            break;
            case fire_id:
                if(in_bounds(i, j) && (simulation->particles[index].id == empty_id || simulation->particles[index].id == coal_id || simulation->particles[index].id == oil_id))
                    simulation->particles[index] = new_fire();    
            break;
            case oil_id:
                if(in_bounds(i, j) && simulation->particles[index].id == empty_id)
                    simulation->particles[index] = new_oil();
            break;
        }
    }

}