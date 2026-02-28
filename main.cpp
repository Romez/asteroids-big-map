#include "include/raylib.h"
#include "include/raymath.h"
#include <iostream>
#include <assert.h>
#include <math.h>
#include <raylib.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <array>
#include <format>

#define MAX_SHOTS 100

#define INIT_SCREEN_WIDTH 1600
#define INIT_SCREEN_HEIGHT 900
#define INFO_TEXT_SIZE 26

#define NET_COLOR GRAY
#define NET_BORDER_COLOR RED

#define MAX_ASTEROIDS_COUNT 10

const float ROTATION_SPEED = PI / 32;
const float MAX_SPEED = 6;

const int NET_GAP = 100;

const int fieldWidth = 2000;
const int fieldHeight = 2000;

Font font;

enum turn {
    LEFT,
    RIGHT,
};

enum move {
    FORWARD,
    BACKWARD,
};

struct Ship {
    Vector2 dir = (Vector2){ 1, 0 };;
    Vector2 pos = (Vector2){ fieldWidth / 2.0f, fieldHeight / 2.0f };
    float speed = 0;
    bool is_engine_working = false;

    void rotate(enum turn t) {
        dir = Vector2Rotate(dir, t == LEFT ? -ROTATION_SPEED : ROTATION_SPEED);
    }

    void move(enum move m) {
        if (m == FORWARD) {
            if (speed < MAX_SPEED) {
                speed += 0.2;
            }
            is_engine_working = true;
        }
        else if (m == BACKWARD) {
            if (speed > -MAX_SPEED) {
                speed -= 0.2;
            }
            is_engine_working = true;
        }
        else {
            assert(false && "Unexpected move type");
        }
    }

    void slowdown() {
        if (speed != 0) {
            Vector2 new_pos = Vector2Add(pos, Vector2Scale(dir, speed));

            if (0 <= new_pos.x && new_pos.x < fieldWidth) {
                pos.x = new_pos.x;
            }

            if (0 <= new_pos.y && new_pos.y < fieldHeight) {
                pos.y = new_pos.y;
            }

            if (speed > 0) {
                if (speed > 0.07) {
                    speed -= 0.07;
                }
                else {
                    speed = 0.0;
                }
            }

            if (speed < 0) {
                if (speed < 0.07) {
                    speed += 0.07;
                }
                else {
                    speed = 0.0;
                }
            }
        }
    }

    std::array<Vector2, 3> getVertices() {
        std::array<Vector2, 3> vs;
        vs[0] = Vector2Scale(dir, 15);

        float l = (3 * PI) / 4;
        vs[1] = Vector2Rotate(vs[0], l);

        float r = (5 * PI) / 4;
        vs[2] = Vector2Rotate(vs[0], r);

        return vs;
    }
};

struct Shot {
    Vector2 pos;
    Vector2 dir;

    bool isShotOnField() {
        return 0 <= pos.x && pos.x <= fieldWidth && 0 <= pos.y && pos.y <= fieldHeight;
    }

    void move() {
        Vector2 shot_speed = Vector2Scale(dir, 10);
        pos = Vector2Add(pos, shot_speed);
    }
};

struct Screen {
    int w;
    int h;
    Vector2 center;
};

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

const float rotationAngle = 0.05;

struct Asteroid {
    Vector2 pos;
    Vector2 dir;
    std::vector<Vector2> vertices;
    Vector2 polyCenter;

    Asteroid() {}

    Asteroid(Vector2 pos, Vector2 dir, std::vector<Vector2> vertices) : pos(pos), dir(dir), vertices(vertices) {
        polyCenter = centerPoint(vertices);
    }

    void rotate() {
        for (size_t i = 0; i < vertices.size(); i++) {
            Vector2 p = vertices[i];
            p = Vector2Subtract(p, polyCenter);
            p = Vector2Rotate(p, rotationAngle);
            p = Vector2Add(p, polyCenter);

            vertices[i] = p;
        }
    }

    void move() {
        pos = Vector2Add(pos, dir);
    }

    bool isOnField(Screen& screen, Ship& ship) {
        if (0 <= pos.x && pos.x <= fieldWidth && 0 <= pos.y && pos.y <= fieldHeight) {
            return true;
        }

        for (Vector2 vertex : vertices) {
            Vector2 p = Vector2Add(pos, vertex);
            if (0 <= p.x && p.x <= fieldWidth && 0 <= p.y && p.y <= fieldHeight) {
                return true;
            }
        }
        return false;
    }
};

void drawNet(Screen& screen, Ship& ship) {
    // Net vertical
    int startX = -fmod(ship.pos.x, NET_GAP);

    int finishX = screen.w;

    for (int i = startX; i < finishX; i += NET_GAP) {
        int y1 = 0;
        int y2 = screen.h;
        DrawLine(i, y1, i, y2, NET_COLOR);
    }

    int startY = -fmod(ship.pos.y, NET_GAP);

    int finishY = screen.h;

    // Net horizontal
    for (int i = startY; i < finishY; i += NET_GAP) {
        int x1 = 0;
        int x2 = screen.w;

        DrawLine(x1, i, x2, i, NET_COLOR);
    }

    // Border lines

    if (ship.pos.y < screen.center.y) {
        int y = screen.center.y - ship.pos.y;
        DrawLine(0, y, screen.w, y, NET_BORDER_COLOR);
    }

    if (fieldHeight - ship.pos.y < screen.center.y) {
        int gap = screen.center.y - (fieldHeight - ship.pos.y);
        int y = screen.h - gap;
        DrawLine(0, y, screen.w, y, NET_BORDER_COLOR);
    }

    if (ship.pos.x < screen.center.x) {
        int x = screen.center.x - ship.pos.x;
        DrawLine(x, 0, x, screen.h, NET_BORDER_COLOR);
    }

    if (fieldWidth - ship.pos.x < screen.center.x) {
        int gap = screen.center.x - (fieldWidth - ship.pos.x);
        int x = screen.w - gap;
        DrawLine(x, 0, x, screen.h, NET_BORDER_COLOR);
    }
}

void drawShip(Screen& screen, Ship& ship) {
    auto [v1, v2, v3] = ship.getVertices();

    v1 = Vector2Add(screen.center, v1);
    v2 = Vector2Add(screen.center, v2);
    v3 = Vector2Add(screen.center, v3);

    DrawTriangleLines(v1, v2, v3, WHITE);

    if (ship.is_engine_working) {
        for (int i = 0; i < 4; i++) {
            Vector2 ve1 = Vector2Add(v2, Vector2Scale(ship.dir, -5 * i));
            Vector2 ve2 = Vector2Add(v3, Vector2Scale(ship.dir, -5 * i));
            DrawLineV(ve1, ve2, RED);
        }
    }
}

void drawInfo(Screen& screen, Ship& ship, std::vector<Shot>& shots, std::vector<Asteroid>& asteroids) {
    // Ship position on the field
    float leftPadding = 10;
    float topPadding = 10;

    Vector2 textPos{ leftPadding, topPadding };

    {
        std::string buf = std::format("FPS {:d}", GetFPS());
        DrawTextEx(font, buf.c_str(), textPos, (float)font.baseSize, 2, LIGHTGRAY);
        textPos.y += font.baseSize;
    }

    {
        std::string buf = std::format("Ship position ({:d}; {:d})", (int)ship.pos.x, (int)ship.pos.y);
        DrawTextEx(font, buf.c_str(), textPos, (float)font.baseSize, 2, LIGHTGRAY);
        textPos.y += font.baseSize;
    }

    {
        std::string buf = std::format("Ship speed {:0.2f}", ship.speed);
        DrawTextEx(font, buf.c_str(), textPos, (float)font.baseSize, 2, LIGHTGRAY);
        textPos.y += font.baseSize;
    }

    // Shots on the field
    {
        std::string buf = std::format("Shots {}", shots.size());
        DrawTextEx(font, buf.c_str(), textPos, (float)font.baseSize, 2, LIGHTGRAY);
        textPos.y += font.baseSize;
    }

    /*
    {
        for (Shot& shot : shots) {
            std::string buf = std::format("({:0.2f}; {:0.2f})", shot.pos.x, shot.pos.y);
            DrawTextEx(font, buf.c_str(), textPos, (float)font.baseSize, 2, LIGHTGRAY);
            textPos.y += font.baseSize;
        }
    }
    */

    // Asteroids on the field
    {
        std::string buf = std::format("Asteroids {}", asteroids.size());
        DrawTextEx(font, buf.c_str(), textPos, (float)font.baseSize, 2, LIGHTGRAY);
        textPos.y += font.baseSize;
    }

    /*
    {
        for (Asteroid& a : asteroids) {
            std::string buf = std::format("({:0.2f}; {:0.2f})", a.pos.x, a.pos.y);
            DrawTextEx(font, buf.c_str(), textPos, (float)font.baseSize, 2, LIGHTGRAY);
            textPos.y += font.baseSize;
        }
    }
    */
}

Vector2 fieldPosToScreenPos(Screen& screen, Ship& ship, Vector2 field_pos) {
    float x = screen.center.x - (ship.pos.x - field_pos.x);
    float y = screen.center.y - (ship.pos.y - field_pos.y);
    return Vector2{ x, y };
}

void drawShots(Screen& screen, Ship& ship, std::vector<Shot>& shots) {
    for (size_t i = 0; i < shots.size(); i++) {
        Shot shot = shots[i];

        Vector2 shot_point = fieldPosToScreenPos(screen, ship, shot.pos);
        DrawCircleV(shot_point, 5, RED);
    }
}

void drawAsteroid(Screen& screen, Ship& ship, Asteroid& asteroid) {
    for (size_t i = 0, j = asteroid.vertices.size() - 1; i < asteroid.vertices.size(); j = i++) {
        Vector2 p1 = fieldPosToScreenPos(screen, ship, Vector2Add(asteroid.pos, asteroid.vertices[i]));
        Vector2 p2 = fieldPosToScreenPos(screen, ship, Vector2Add(asteroid.pos, asteroid.vertices[j]));
        DrawLineV(p1, p2, WHITE);
    }
}

Screen initScreen(int w, int h) {
    return (Screen) {
        .w = w,
            .h = h,
            .center = (Vector2){
                .x = w / 2.0f,
                .y = h / 2.0f,
        },
    };
}

void addShot(Screen& screen, std::vector<Shot>& shots, Ship& ship) {
    if (shots.size() < MAX_SHOTS) {
        Shot shot = {
            .pos = ship.pos,
            .dir = Vector2Scale(Vector2Normalize(ship.dir), 1),
        };

        shots.push_back(shot);
    }
}

void moveShots(std::vector<Shot>& shots) {
    std::vector<size_t> to_remove;

    for (size_t i = 0; i < shots.size(); i++) {
        Shot& shot = shots[i];

        if (shot.isShotOnField()) {
            shot.move();
        }
        else {
            to_remove.push_back(i);
        }
    }

    for (size_t i = to_remove.size(); i-- > 0; ) {
        shots.erase(shots.begin() + to_remove[i]);
    }
}

bool CheckAsteroidCollision(Vector2 p, std::vector<Vector2>& points) {
    assert(points.size() > 1);

    bool inside = false;

    size_t count = points.size();

    for (size_t i = 0, j = count - 1; i < count; j = i++) {
        Vector2 p1 = points[i];
        Vector2 p2 = points[j];

        // Check if edge (i,j) crosses a horizontal ray to the right of the point
        bool intersect = ((p1.y > p.y) != (p2.y > p.y)) &&
            (p.x < (p2.x - p1.x) * (p.y - p1.y) / (p2.y - p1.y + 0.000001f) + p1.x);

        if (intersect) {
            inside = !inside;
        }
    }

    return inside;
}

std::vector<std::vector<Vector2>> asteroidsLibarary = {
    std::vector<Vector2> {
        Vector2{ 51, 78 },
        Vector2{ -15, 99 },
        Vector2{ -20, 0 },
        Vector2{ 0, -40 },
        Vector2{ 82, -48 },
        Vector2{ 126, 12 },
    }
};

Asteroid getRandAsteroid() {
    int i = GetRandomValue(0, asteroidsLibarary.size() - 1);

    int side = GetRandomValue(0, 3);

    Vector2 pos;
    Vector2 dir;
    Asteroid asteroid;

    if (side == 0) { // top
        pos = Vector2{(float)GetRandomValue(0, fieldWidth), 0 };
        dir = Vector2{(float)GetRandomValue(1, 3), (float)GetRandomValue(1, 3)};
    } else if (side == 1) { // right
        pos = Vector2{0, (float)GetRandomValue(0, fieldHeight)};
        dir = Vector2{(float)GetRandomValue(1, 3), (float)GetRandomValue(-3, 3)};
    } else if (side == 2) { // bottom
        pos = Vector2{ (float)GetRandomValue(0, fieldWidth), fieldHeight };
        dir = Vector2{ (float)GetRandomValue(-3, 3), (float)GetRandomValue(-3, -1)};
    } else { // left
        pos = Vector2{0, (float)GetRandomValue(0, fieldHeight)};
        dir = Vector2{(float)GetRandomValue(1, 3), (float)GetRandomValue(-3, 3)};
    }

    return Asteroid(pos, dir, asteroidsLibarary[i]);
}

void drawScore(Screen& screen, uint64_t score) {
    Vector2 textPos = {screen.w / 2.0f, 10};
    std::string buf = std::format("Score {:d}", score);
    DrawTextEx(font, buf.c_str(), textPos, (float)font.baseSize, 2, LIGHTGRAY);
}

int main(void) {
    // Initialization
    //--------------------------------------------------------------------------------------
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);

    InitWindow(INIT_SCREEN_WIDTH, INIT_SCREEN_HEIGHT, "Asteroids");

    font = LoadFontEx("./resources/font.ttf", 32, 0, 0);
    if (font.texture.id == 0) {
        TraceLog(LOG_ERROR, "Failed to load font!");
    }

    SetTargetFPS(60);

    //--------------------------------------------------------------------------------------

    bool debugDisplay = false;

    uint64_t score = 0;

    Ship ship;

    std::vector<Shot> shots;

    Screen screen = initScreen(GetScreenWidth(), GetScreenHeight());

    std::vector<Asteroid> asteroids;

    while (!WindowShouldClose()) {
        if (IsWindowResized()) {
            screen = initScreen(GetScreenWidth(), GetScreenHeight());
        }

        ship.is_engine_working = false;

        if (IsKeyPressed(KEY_SPACE) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            addShot(screen, shots, ship);
        }

        if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) {
            ship.rotate(LEFT);
        }

        if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) {
            ship.rotate(RIGHT);
        }

        if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W)) {
            ship.move(FORWARD);
        }

        if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) {
            ship.move(BACKWARD);
        }

        if (IsKeyPressed(KEY_L)) {
            debugDisplay = !debugDisplay;
        }

        ship.slowdown();

        moveShots(shots);

        for (size_t i = 0; i < MAX_ASTEROIDS_COUNT - asteroids.size(); i++) {
            asteroids.push_back(getRandAsteroid());
        }

        {
            std::vector<size_t> asteroids_to_remove;
            std::vector<size_t> shots_to_remove;

            for (size_t i = 0; i < asteroids.size(); i++) {
                Asteroid& asteroid = asteroids[i];

                asteroid.rotate();
                asteroid.move();

                // is on field?
                if (!asteroid.isOnField(screen, ship)) {
                    asteroids_to_remove.push_back(i);
                    continue;
                }

                std::vector<Vector2> asteroidVertices;
                for (Vector2 v : asteroid.vertices) {
                    Vector2 p = Vector2Add(asteroid.pos, v);
                    asteroidVertices.push_back(p);
                }

                // check ship collision

                auto [v1, v2, v3] = ship.getVertices();
                v1 = Vector2Add(ship.pos, v1);
                v2 = Vector2Add(ship.pos, v2);
                v3 = Vector2Add(ship.pos, v3);

                if (CheckAsteroidCollision(v1, asteroidVertices) ||
                    CheckAsteroidCollision(v2, asteroidVertices) ||
                    CheckAsteroidCollision(v3, asteroidVertices)) {
                    asteroids_to_remove.push_back(i);
                    continue;
                }

                for (size_t j = 0; j < shots.size(); j++) {
                    Shot& shot = shots[j];
                    if (CheckAsteroidCollision(shot.pos, asteroidVertices)) {
                        shots_to_remove.push_back(j);
                        asteroids_to_remove.push_back(i);
                        score++;
                    }
                }
            }

            for (size_t i = asteroids_to_remove.size(); i-- > 0; ) {
                asteroids.erase(asteroids.begin() + asteroids_to_remove[i]);
            }

            for (size_t i = shots_to_remove.size(); i-- > 0; ) {
                shots.erase(shots.begin() + shots_to_remove[i]);
            }
        }

        // Draw ---------------------------------------------------------------------------
        BeginDrawing();

        ClearBackground(DARKGRAY);

        drawNet(screen, ship);
        drawShip(screen, ship);

        drawShots(screen, ship, shots);

        for (Asteroid& asteroid : asteroids) {
            drawAsteroid(screen, ship, asteroid);
        }

        drawScore(screen, score);

        if (debugDisplay) {
            drawInfo(screen, ship, shots, asteroids);
        }

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow(); // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    std::cout << "Buy!" << std::endl;

    return 0;
}
