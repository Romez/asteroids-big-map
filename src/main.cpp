#include <iostream>
#include <assert.h>
#include <math.h>
#include <raylib.h>
#include <raymath.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <array>
#include <format>
#include <unordered_map>
#include <unordered_set>
#include <random>
#include "./asteroid.h"
#include "./ship.h"
#include "./shot.h"

#define MAX_SHOTS 100

#define INIT_SCREEN_WIDTH 1600
#define INIT_SCREEN_HEIGHT 900
#define INFO_TEXT_SIZE 26

#define NET_COLOR GRAY
#define NET_BORDER_COLOR RED

#define MAX_ASTEROIDS_COUNT 10

#define MAX(a, b) (a > b ? (a) : (b))

const int NET_GAP = 100;

struct FieldSize {
    int w;
    int h;
};

const FieldSize fieldSize = {2000, 1000};

Font font;

struct Screen {
    int w;
    int h;
    Vector2 center;
    float gap;
};

enum class GameScreen {
  TITLE,
  GAME,
};

std::string genUuid() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static const char* chars = "0123456789abcdef";

    std::string uuid = "xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx";

    for (char& c : uuid) {
        if (c == 'x') {
            c = chars[gen() % 16];
        } else if (c == 'y') {
            c = chars[(gen() % 4) + 8];
        }
    }
    return uuid;
}

bool isAsteroidOnField(const Screen& screen, const Asteroid& asteroid) {
    auto pos = asteroid.pos;
    if (-screen.gap <= pos.x && pos.x <= fieldSize.w + screen.gap &&
        -screen.gap <= pos.y && pos.y <= fieldSize.h + screen.gap) {
        return true;
    }

    for (Vector2 vertex : asteroid.vertices) {
        Vector2 p = Vector2Add(pos, vertex);
        if (-screen.gap <= p.x && p.x <= fieldSize.w + screen.gap &&
            -screen.gap <= p.y && p.y <= fieldSize.h + screen.gap) {
            return true;
        }
    }
    return false;
}

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

    if (fieldSize.h - ship.pos.y < screen.center.y) {
        int gap = screen.center.y - (fieldSize.h - ship.pos.y);
        int y = screen.h - gap;
        DrawLine(0, y, screen.w, y, NET_BORDER_COLOR);
    }

    if (ship.pos.x < screen.center.x) {
        int x = screen.center.x - ship.pos.x;
        DrawLine(x, 0, x, screen.h, NET_BORDER_COLOR);
    }

    if (fieldSize.w - ship.pos.x < screen.center.x) {
        int gap = screen.center.x - (fieldSize.w - ship.pos.x);
        int x = screen.w - gap;
        DrawLine(x, 0, x, screen.h, NET_BORDER_COLOR);
    }
}

void drawShip(Screen& screen, Ship& ship) {
    auto [v1, v2, v3] = getShipVertices(ship);

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

void drawInfo(Screen& screen, Ship& ship, std::unordered_map<std::string, Shot>& shots, std::unordered_map<std::string, Asteroid>& asteroids) {
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

void drawShots(Screen& screen, Ship& ship, std::unordered_map<std::string, Shot>& shots) {
    for (auto [_, shot] : shots) {
    //for (size_t i = 0; i < shots.size(); i++) {
        // Shot shot = shots[i];

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
    float gap = static_cast<float>(MAX(w, h) / 2);
    Vector2 center = {
        .x = w / 2.0f,
        .y = h / 2.0f,
    };
    return {
        .w = w,
        .h = h,
        .center = center,
        .gap = gap,
    };
}

void addShot(std::unordered_map<std::string, Shot>& shots, Ship& ship) {
    if (shots.size() < MAX_SHOTS) {
        shots[genUuid()] = buildShot(ship.pos, ship.dir);
    }
}

void moveShots(std::unordered_map<std::string, Shot>& shots) {
    std::vector<std::string> toRemove;

    for (auto& [shotId, shot] : shots) {
        if (isShotOnField(shot, fieldSize.w, fieldSize.h)) {
            moveShot(shot);
        }
        else {
            toRemove.push_back(shotId);
        }
    }

    for (auto shotId : toRemove) {
        shots.erase(shotId);
    }
}

bool CheckPointCollisionWithPoly(Vector2 p, std::vector<Vector2>& points) {
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

bool checkShipCollisionWithPoly(Ship& ship, std::vector<Vector2>& points) {
    auto [v1, v2, v3] = getShipVertices(ship);
    v1 = Vector2Add(ship.pos, v1);
    v2 = Vector2Add(ship.pos, v2);
    v3 = Vector2Add(ship.pos, v3);

    return CheckPointCollisionWithPoly(v1, points) ||
           CheckPointCollisionWithPoly(v2, points) ||
           CheckPointCollisionWithPoly(v3, points);
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

Asteroid getRandAsteroid(Screen& screen) {
    int i = GetRandomValue(0, asteroidsLibarary.size() - 1);

    int radomSide = GetRandomValue(0, 3);

    Vector2 pos;
    Vector2 dir;

    if (radomSide == 0) { // top
        pos = Vector2{ (float)GetRandomValue(0, fieldSize.w), -screen.gap };
        dir = Vector2{ (float)GetRandomValue(1, 3), (float)GetRandomValue(1, 3) };
    }
    else if (radomSide == 1) { // right
        pos = Vector2{ fieldSize.w + screen.gap, (float)GetRandomValue(0, fieldSize.h) };
        dir = Vector2{ (float)GetRandomValue(-3, -1), (float)GetRandomValue(-3, 3) };
    }
    else if (radomSide == 2) { // bottom
        pos = Vector2{ (float)GetRandomValue(0, fieldSize.w), fieldSize.h + screen.gap};
        dir = Vector2{ (float)GetRandomValue(-3, 3), (float)GetRandomValue(-3, -1) };
    }
    else { // left
        pos = Vector2{ -screen.gap, (float)GetRandomValue(0, fieldSize.h) };
        dir = Vector2{ (float)GetRandomValue(1, 3), (float)GetRandomValue(-3, 3) };
    }

    return buildAsteroid(pos, dir, asteroidsLibarary[i]);
}

void drawScore(Screen& screen, uint64_t score) {
    Vector2 textPos = { screen.w / 2.0f, 10 };
    std::string buf = std::format("Score {:d}", score);
    DrawTextEx(font, buf.c_str(), textPos, (float)font.baseSize, 2, LIGHTGRAY);
}

void moveAsteroids(std::unordered_map<std::string, Asteroid>& asteroids) {
    for (auto& [_, a] : asteroids) {
        rotateAsteroid(a);
        moveAsteroid(a);
    }
}

void removeAsteroidsFromField(const Screen& screen, std::unordered_map<std::string, Asteroid>& asteroids) {
    std::vector<std::string> toRemove;
    for (auto& [id, a] : asteroids) {
        if (!isAsteroidOnField(screen, a)) {
            toRemove.push_back(id);
        }
    }

    for (auto id : toRemove) {
        asteroids.erase(id);
    }
}

int main(void) {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);

    InitWindow(INIT_SCREEN_WIDTH, INIT_SCREEN_HEIGHT, "Asteroids");

    font = LoadFontEx("./resources/font.ttf", 32, 0, 0);
    if (font.texture.id == 0) {
        TraceLog(LOG_ERROR, "Failed to load font!");
    }

    SetTargetFPS(60);

    bool debugDisplay = false;

    GameScreen gameScreen = GameScreen::TITLE;

    Screen screen = initScreen(GetScreenWidth(), GetScreenHeight());

    uint64_t score = 0;

    Ship ship = buildShip(fieldSize.w, fieldSize.h);

    std::unordered_map<std::string, Shot> shots;
    std::unordered_map<std::string, Asteroid> asteroids;
    std::unordered_map<std::string, Asteroid> shards;

    while (!WindowShouldClose()) {
        if (IsWindowResized()) {
            screen = initScreen(GetScreenWidth(), GetScreenHeight());
        }

        if (gameScreen == GameScreen::TITLE) {
            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
                gameScreen = GameScreen::GAME;
            }

            BeginDrawing();

            ClearBackground(DARKGRAY);

            std::string text = "Press Enter/Space to start";
            Vector2 textSize = MeasureTextEx(font, text.c_str(), font.baseSize, 2);

            Vector2 textPos = {
                .x = ((float)screen.w / 2) - (textSize.x / 2),
                .y = ((float)screen.h / 2) - (textSize.y / 2),
            };

            DrawTextEx(font, text.c_str(), textPos, (float)font.baseSize, 2, LIGHTGRAY);

            EndDrawing();
        } else if (gameScreen == GameScreen::GAME) {
            ship.is_engine_working = false;

            if (IsKeyPressed(KEY_SPACE) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                addShot(shots, ship);
            }

            if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) {
                rotateShip(ship, LEFT);
            }

            if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) {
                rotateShip(ship, RIGHT);
            }

            if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W)) {
                moveShip(ship, FORWARD);
            }

            if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) {
                moveShip(ship, BACKWARD);
            }

            if (IsKeyPressed(KEY_L)) {
                debugDisplay = !debugDisplay;
            }

            slowdownShip(ship, fieldSize.w, fieldSize.h);

            // Add asteroids

            if (asteroids.size() < MAX_ASTEROIDS_COUNT) {
                for (size_t i = 0; i < MAX_ASTEROIDS_COUNT - asteroids.size(); i++) {
                    asteroids[genUuid()] = getRandAsteroid(screen);
                }
            }

            removeAsteroidsFromField(screen, shards);
            moveAsteroids(shards);

            removeAsteroidsFromField(screen, asteroids);
            moveAsteroids(asteroids);

            {
                std::vector<std::string> asteroidsToRemove;
                std::vector<std::string> shotsToRemove;

                for (auto [asteroidId, asteroid] : asteroids) {
                    std::vector<Vector2> asteroidVerticesFieldPos;

                    for (Vector2 v : asteroid.vertices) {
                        Vector2 p = Vector2Add(asteroid.pos, v);
                        asteroidVerticesFieldPos.push_back(p);
                    }

                    // check asteroid and ship collision

                    if (checkShipCollisionWithPoly(ship, asteroidVerticesFieldPos)) {
                        asteroidsToRemove.push_back(asteroidId);
                        continue;
                    }

                    for (auto& [shotId, shot] : shots) {
                        if (CheckPointCollisionWithPoly(shot.pos, asteroidVerticesFieldPos)) {
                            score++;

                            std::vector<Asteroid> asteroidShards = asteroidToShards(asteroid);

                            for (auto shard : asteroidShards) {
                                shards[genUuid()] = shard;
                            }

                            asteroidsToRemove.push_back(asteroidId);
                            shotsToRemove.push_back(shotId);
                        }
                    }
                }

                for (auto asteroidId : asteroidsToRemove) {
                    asteroids.erase(asteroidId);
                }

                for (auto shotId : shotsToRemove) {
                    shots.erase(shotId);
                }
            }

            moveShots(shots);

            BeginDrawing();

            ClearBackground(DARKGRAY);

            drawNet(screen, ship);
            drawShip(screen, ship);

            drawShots(screen, ship, shots);

            for (auto [_, asteroid] : asteroids) {
                drawAsteroid(screen, ship, asteroid);
            }

            for (auto [_, shard] : shards) {
                drawAsteroid(screen, ship, shard);
            }

            drawScore(screen, score);

            if (debugDisplay) {
                drawInfo(screen, ship, shots, asteroids);
            }

            EndDrawing();
        }
    }

    CloseWindow();

    std::cout << "Buy!" << std::endl;

    return 0;
}
