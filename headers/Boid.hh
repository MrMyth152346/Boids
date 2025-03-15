#pragma once

#include "Vector.hh"

class Boid
{
public:
    Vector position;
    Vector velocity;
    Vector stillVelocity;
    Vector nonUnifiedVelocity;
    Vector color;

    void Update(double deltaTime)
    {
        this->position += (nonUnifiedVelocity + (this->stillVelocity + this->velocity).Unit()) * deltaTime;
    }

    Boid(Vector position)
    {
        this->position = position;
    }

    Boid(){}
};