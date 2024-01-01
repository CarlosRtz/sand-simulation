# sand-simulation
A simple sand simulation game, inspired by Noita, to learn more about Cellular Automata and C language.

# Compiling
**Note:** Currently uses **GLFW**
- To compile using gcc:
    - Run `gcc ./src/main.c ./src/particle.c -o sand-sim -lglfw -lGL -lm`
    - The executable will be placed in the current directory
    - Run `./sand-sim`

# Controls
- `1` Select sand particle
- `2` Select water particle
- `3` Select coal particle
- `4` Select oil particle
- `5` Select fire particle
- `backspace` clear all particles
- `esc` Quit application
- Left mouse button throw particles into the simulation
