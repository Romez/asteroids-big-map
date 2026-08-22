#include "./asteroid.h"
#include <vector>
#include <deque>
#include <algorithm>
#include <random>

const float rotationAngle = 0.05;

std::mt19937 rng(std::random_device{}());

Vector2 centerPoint(std::vector<Vector2> vertices) {
    float x = 0;
    float y = 0;
    float a = 0;

    size_t n = vertices.size();

    for (size_t i = 0; i < n; i++) {
        float x1 = vertices[i].x;
        float y1 = vertices[i].y;

        float x2 = vertices[(i + 1) % n].x;
        float y2 = vertices[(i + 1) % n].y;

        a += x1 * y2 - x2 * y1;

        float cross = (x1 * y2 - x2 * y1);
        x += (x1 + x2) * cross;
        y += (y1 + y2) * cross;
    }

    x /= (3 * a);
    y /= (3 * a);

    return Vector2{ x, y };
}

Asteroid buildAsteroid(Vector2 pos, Vector2 dir, std::vector<Vector2> vertices) {
    return Asteroid {
        .pos = pos,
        .dir = dir,
        .vertices = vertices,
        .polyCenter = centerPoint(vertices),
    };
}

void rotateAsteroid(Asteroid& asteroid) {
    for (size_t i = 0; i < asteroid.vertices.size(); i++) {
        Vector2 p = asteroid.vertices[i];
        p = Vector2Subtract(p, asteroid.polyCenter);
        p = Vector2Rotate(p, rotationAngle);
        p = Vector2Add(p, asteroid.polyCenter);

        asteroid.vertices[i] = p;
    }
}

void moveAsteroid(Asteroid& asteroid) {
    asteroid.pos = Vector2Add(asteroid.pos, asteroid.dir);
}

std::vector<Asteroid> asteroidToShards(Asteroid& asteroid) {
    std::vector<Asteroid> shards = {};

    size_t v_size = asteroid.vertices.size();
    for (size_t j = 0; j < v_size; j++) {
        Vector2 v1 = asteroid.vertices[j];
        Vector2 v2 = asteroid.vertices[(j + 1) % v_size];
        Vector2 v3 = asteroid.polyCenter;

        Vector2 dir = Vector2Scale(Vector2Normalize(v1), 3);

        Vector2 pos = Vector2Add(asteroid.pos, dir);

        Asteroid shard = buildAsteroid(pos, dir, {v1, v2, v3});

        shards.push_back(shard);
    }

    std::shuffle(shards.begin(), shards.end(), rng);

    return shards;
}
