#pragma once

#include "../include/raylib.h"
#include "../include/raymath.h"
#include <vector>

struct Asteroid {
    Vector2 pos;
    Vector2 dir;
    std::vector<Vector2> vertices;
    Vector2 polyCenter;
};

Asteroid buildAsteroid(Vector2 pos, Vector2 dir, std::vector<Vector2> vertices);

void moveAsteroid(Asteroid& asteroid);

void rotateAsteroid(Asteroid &asteroid);

Vector2 centerPoint(std::vector<Vector2> vertices);

std::vector<Asteroid> asteroidToShards(Asteroid& asteroid);
