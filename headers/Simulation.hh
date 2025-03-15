#pragma once

#include <SDL2/SDL.h>
#include <vector>
#include <random>
#include <algorithm>
#include <thread>
#include <mutex>

#include "Boid.hh"

class Simulation
{
private:
    bool simulationRunning = true;
    float simulationSpeed = 1.0f;

    Vector windowSize = Vector(800.0f,700.0f);
    Vector cellSize = Vector(100.0f,100.0f);

    const double seperationFactor = 30.0;
    const double coheranceFactor  = 1.0;
    const double alignmentFactor  = 0.1;
    const uint64_t quantity       = 500;

    bool mouseClicked = false;
    int mousePositionX = 0;
    int mousePositionY = 0;

public:
    Vector FindCell(Boid boid)
    {
        //SDL_assert(&boid != nullptr);

        for (size_t x = 0; x < windowSize.X / cellSize.X; x++)
        {
            for (size_t y = 0; y < windowSize.Y / cellSize.Y; y++)
            {
                bool insideX = boid.position.X >= x * cellSize.X && boid.position.X <= x * cellSize.X + cellSize.X;
                bool insideY = boid.position.Y >= y * cellSize.Y && boid.position.Y <= y * cellSize.Y + cellSize.Y;

                if (insideX && insideY)
                    return Vector(x, y);
            }
        }

        return Vector(0,0);
    }

    void FindValues(Boid *boid, Boid *otherBoid, Vector *coherence, Vector *seperation, Vector *alignment)
    {
        //SDL_assert(boid != NULL && otherBoid != NULL && coherence != NULL && seperation != NULL && alignment != NULL);
        

        double distance = (boid->position - otherBoid->position).Magnitude();


        Vector newSeperation = Vector(0,0);
        Vector newCoherance = Vector(0,0);
        Vector newAlignment = Vector(0,0);

        if (distance <= 15.0)
            boid->color = (boid->color - boid->color.Unit() + otherBoid->color.Unit());
        
        if (distance > 1)
        {

            if (distance <= 100.0)
            {
                newSeperation += (boid->position - otherBoid->position).Unit() * this->seperationFactor / (distance / 2.0);
                newCoherance += (otherBoid->position - boid->position).Unit() * this->coheranceFactor;
                
                if (distance <= 10.0)
                {
                    newAlignment += (boid->velocity - otherBoid->velocity).Unit() * this->alignmentFactor;
                }
            }
        }

        coherence->X += newCoherance.X;
        coherence->Y += newCoherance.Y;

        seperation->X += newSeperation.X;
        seperation->Y += newSeperation.Y;

        alignment->X += newAlignment.X;
        alignment->Y += newAlignment.Y;
    }

    void Run()
    {
        SDL_Window *window = SDL_CreateWindow("Boids", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, windowSize.X, windowSize.Y, SDL_WINDOW_SHOWN);

        SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        SDL_Event event;

        std::vector<std::vector<std::vector<Boid*>>> boids;

        std::random_device randomNumberGenerator;
        std::uniform_real_distribution<float> floatDistribution(-windowSize.Magnitude() / 4, windowSize.Magnitude() / 4);
        std::uniform_real_distribution<float> floatDistributionNegative(-windowSize.Magnitude(), windowSize.Magnitude());
        std::uniform_real_distribution<float> colorDistribution(0, 255);

        for (int x = 0; x < windowSize.X / cellSize.X; x++)
        {
            std::vector<std::vector<Boid*>> vectorX;
            boids.push_back(vectorX);

            for (int y = 0; y < windowSize.Y / cellSize.Y; y++)
            {
                std::vector<Boid*> vectorY;

                boids[x].push_back(vectorY);
            }
        }

        for (uint64_t i = 0; i < quantity; i++)
        {

            Boid *boid = new Boid(windowSize / 2.0 + Vector(floatDistribution(randomNumberGenerator),floatDistribution(randomNumberGenerator)));

            boid->position = Vector(SDL_clamp(boid->position.X, 0.0f, windowSize.X), SDL_clamp(boid->position.Y, 0.0f, windowSize.X));
            boid->velocity = Vector(floatDistributionNegative(randomNumberGenerator),floatDistributionNegative(randomNumberGenerator)).Unit();
            boid->color = Vector(colorDistribution(randomNumberGenerator), colorDistribution(randomNumberGenerator));

            Vector cell = FindCell(*boid);

            boids[cell.X][cell.Y].push_back(boid);
        }

        uint64_t last = 0, now = SDL_GetPerformanceCounter();

        double deltaTime;

        std::vector<std::thread*> threads;

        std::mutex mutex;

        while (this->simulationRunning)
        {
            last = now;
            now  = SDL_GetPerformanceCounter();
            deltaTime = (double)(now - last) * 50 / (double)SDL_GetPerformanceFrequency();

            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);

            for (std::vector<std::vector<Boid*>> boidsX : boids)
            {
                std::thread *thread = new std::thread([&, boidsX] {
                    
                    for (std::vector<Boid*> boidsY : boidsX)
                    {
                        for (Boid* boid : boidsY)
                        {
                            if (boid != nullptr)
                            {
                                Vector coherence  = Vector(0.0f, 0.0f);
                                Vector seperation = Vector(0.0f, 0.0f);
                                Vector alignment  = Vector(0.0f, 0.0f);
                                
                                Vector boidsCell = FindCell(*boid);

                                
                                for (float x = 0.0f; x < windowSize.X / cellSize.X; x++)
                                    for (float y = 0.0f; y < windowSize.Y / cellSize.Y; y++)
                                        if (!boids[x][y].empty() && (Vector(x, y) - boidsCell).Magnitude() <= 2.0)
                                            for (Boid *otherBoid : boids[x][y])
                                                if (otherBoid != boid && otherBoid != nullptr)
                                                    FindValues(boid, otherBoid, &coherence, &seperation, &alignment);
        //(Vector(floatDistributionNegative(randomNumberGenerator), floatDistributionNegative(randomNumberGenerator)).Unit() / 64)


                                mutex.lock();
                                boid->velocity = boid->velocity.Unit() + (seperation + coherence) / 3.0 * deltaTime * 10.0;
                                boid->stillVelocity = boid->stillVelocity.Unit() + alignment * deltaTime;
                                if (mouseClicked == true)
                                    boid->nonUnifiedVelocity = (boid->position - Vector(mousePositionX, mousePositionY)).Unit() * -100.0 / ((boid->position - Vector(mousePositionX, mousePositionY)).Magnitude() * 2.0);
                                boid->Update(deltaTime);
                                
                                if (boid->position.X > windowSize.X)
                                    boid->position.X = 1.0;
                                if (boid->position.X < 0.0)
                                    boid->position.X = windowSize.X - 1.0;
                                if (boid->position.Y > windowSize.Y)
                                    boid->position.Y = 1.0;
                                if (boid->position.Y < 0.0)
                                    boid->position.Y = windowSize.Y - 1.0;
                                
                                boids[boidsCell.X][boidsCell.Y].erase(std::find(boids[boidsCell.X][boidsCell.Y].begin(), boids[boidsCell.X][boidsCell.Y].end(), boid));
                                //boid->position = Vector(SDL_clamp(boid->position.X, 0.0f, windowSize.X), SDL_clamp(boid->position.Y, 0.0f, windowSize.X));
                                boidsCell = FindCell(*boid);
                                boids[boidsCell.X][boidsCell.Y].push_back(boid);

                                SDL_Vertex vertex_1 = {{boid->position.X, boid->position.Y + 2.5f}, {(unsigned char)boid->color.X, 0, (unsigned char)boid->color.Y, 255}, {1, 1}};
                                SDL_Vertex vertex_2 = {{boid->position.X - 2.5f, boid->position.Y - 2.5f}, {(unsigned char)boid->color.X, 0, (unsigned char)boid->color.Y, 255}, {1, 1}};
                                SDL_Vertex vertex_3 = {{boid->position.X + 2.5f, boid->position.Y - 2.5f}, {(unsigned char)boid->color.X, 0, (unsigned char)boid->color.Y, 255}, {1, 1}};                        

                                SDL_Vertex vertices[] = {
                                    vertex_1,
                                    vertex_2,
                                    vertex_3
                                };
                                SDL_RenderGeometry(renderer, nullptr, vertices, 3, nullptr, 0);

                                mutex.unlock();

                            }
                            
                        }
                    }
                });

                threads.push_back(thread);
            }

            
            for (int64_t i = threads.size() - 1; i >= 0; i--)
            {
                std::thread *thread = threads[i];

                if (thread != nullptr && thread->joinable() == true)
                {
                    thread->join();
                    threads.pop_back();
                }
            }


            SDL_RenderPresent(renderer);


            SDL_PollEvent(&event);

            SDL_GetMouseState(&mousePositionX, &mousePositionY);
            
            switch (event.type)
            {
            case SDL_KEYDOWN:
                if (event.key.keysym.sym == SDLK_h)
                    if (simulationSpeed > 0.0f)
                        simulationSpeed -= 0.1f;
                    else
                        simulationSpeed = 1.0f;
                break;

            case SDL_MOUSEBUTTONDOWN:
                mouseClicked = true;
                break;

            case SDL_MOUSEBUTTONUP:
                mouseClicked = false;
                break;

            case SDL_QUIT:
                this->simulationRunning = false;
                break;
            
            default:
                break;
            }
        }
    }

    void Quit()
    {
        this->simulationRunning = false;
    }
};