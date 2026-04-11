#pragma once

#include <array>
#include "../include/raylib.h"

enum turn {
    LEFT,
    RIGHT,
};

enum move {
    FORWARD,
    BACKWARD,
};

struct Ship {
    Vector2 dir;
    Vector2 pos;
    float speed;
    bool is_engine_working;
};

Ship buildShip(int fieldWidth, int fieldHeight);

void rotateShip(Ship& ship, enum turn t);

void moveShip(Ship& ship, enum move m);

void slowdownShip(Ship& ship, int fieldWidth, int fieldHeight);

std::array<Vector2, 3> getShipVertices(Ship& ship);