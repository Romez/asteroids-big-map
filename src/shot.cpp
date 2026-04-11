#include "./shot.h"

bool isShotOnField(Shot& shot, int fieldWidth, int fieldHeight) {
    return 0 <= shot.pos.x && shot.pos.x <= fieldWidth && 0 <= shot.pos.y && shot.pos.y <= fieldHeight;
}

void moveShot(Shot& shot) {
    Vector2 shot_speed = Vector2Scale(shot.dir, 10);
    shot.pos = Vector2Add(shot.pos, shot_speed);
}

Shot buildShot(Vector2 pos, Vector2 dir) {
    return {
        .pos = pos,
        .dir = Vector2Scale(Vector2Normalize(dir), 1),
    };
}