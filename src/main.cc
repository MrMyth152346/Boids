#include <iostream>

#include "../headers/Simulation.hh"

int main(int argc, char* argv[])
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
        std::cout << "Failed initiliazing SDL" << std::endl;

    Simulation simulation;

    simulation.Run();

    return 0;
}