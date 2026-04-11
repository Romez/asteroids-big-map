#include <assert.h>
#include "./ship.h"
#include "../include/raymath.h"

const float ROTATION_SPEED = PI / 32;

const float MAX_SPEED = 6;

Ship buildShip(int fieldWidth, int fieldHeight) {
    return Ship {
        .dir = {1, 0},
        .pos = { fieldWidth / 2.0f, fieldHeight / 2.0f },
        .speed = 0,
        .is_engine_working = false,
    };
}

void rotateShip(Ship& ship, enum turn t) {
    ship.dir = Vector2Rotate(ship.dir, t == LEFT ? -ROTATION_SPEED : ROTATION_SPEED);
}

void moveShip(Ship& ship, enum move m) {
    if (m == FORWARD) {
        if (ship.speed < MAX_SPEED) {
            ship.speed += 0.2;
        }
        ship.is_engine_working = true;
    }
    else if (m == BACKWARD) {
        if (ship.speed > -MAX_SPEED) {
            ship.speed -= 0.2;
        }
        ship.is_engine_working = true;
    }
    else {
        assert(false && "Unexpected move type");
    }
}

void slowdownShip(Ship& ship, int fieldWidth, int fieldHeight) {
    if (ship.speed != 0) {
        Vector2 new_pos = Vector2Add(ship.pos, Vector2Scale(ship.dir, ship.speed));

        if (0 <= new_pos.x && new_pos.x < fieldWidth) {
            ship.pos.x = new_pos.x;
        }

        if (0 <= new_pos.y && new_pos.y < fieldHeight) {
            ship.pos.y = new_pos.y;
        }

        if (ship.speed > 0) {
            if (ship.speed > 0.07) {
                ship.speed -= 0.07;
            }
            else {
                ship.speed = 0.0;
            }
        }

        if (ship.speed < 0) {
            if (ship.speed < 0.07) {
                ship.speed += 0.07;
            }
            else {
                ship.speed = 0.0;
            }
        }
    }
}

std::array<Vector2, 3> getShipVertices(Ship& ship) {
    std::array<Vector2, 3> vs;
    vs[0] = Vector2Scale(ship.dir, 15);

    float l = (3 * PI) / 4;
    vs[1] = Vector2Rotate(vs[0], l);

    float r = (5 * PI) / 4;
    vs[2] = Vector2Rotate(vs[0], r);

    return vs;
}