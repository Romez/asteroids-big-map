#pragma once

#include "../include/raylib.h"
#include "../include/raymath.h"

struct Shot {
    Vector2 pos;
    Vector2 dir;
};

Shot buildShot(Vector2 pos, Vector2 dir);

bool isShotOnField(Shot& shot, int fieldWidth, int fieldHeight);

void moveShot(Shot& shot);
