#include <cstdint>
#include <iostream>
#include <cassert>
#include <cmath>
#include <vector>
#include <format>
#include <raylib.h>
#include <raymath.h>
#include <optional>
#include <algorithm>
#include "./asteroid.h"
#include "./ship.h"
#include "./shot.h"

const int MaxShots = 100;
const int InitScreenWidth = 1600;
const int InitScreenHeight = 900;
const int MaxAsteroidsCount = 10;

const int NetGap = 100;
const Color NetColor = GRAY;
const Color NetBorderColor = RED;

struct Field {
    int w;
    int h;
};

struct Screen {
    int w;
    int h;
    Vector2 center;
    float gap; // Extra margin around the field used for spawning/removing off-screen objects
};

enum class GameScreen {
  TITLE,
  GAME,
};

struct GameState {
    Field field;
    GameScreen gameScreen;
    uint64_t score;
    bool debugDisplay;
    Ship ship;
    std::vector<Shot> shots;
    std::vector<Asteroid> asteroids;
    std::vector<Asteroid> shards;
};

const std::vector<std::vector<Vector2>> asteroidsLibrary = {
    std::vector<Vector2> {
        Vector2{ 51, 78 },
        Vector2{ -15, 99 },
        Vector2{ -20, 0 },
        Vector2{ 0, -40 },
        Vector2{ 82, -48 },
        Vector2{ 126, 12 },
    },
    std::vector<Vector2> {
        Vector2 {-11, -25},
        Vector2 {58, -7},
        Vector2 {72, 69},
        Vector2 {-31, 56},
        Vector2 {-67, 6}
    },
    std::vector<Vector2>{
        Vector2 {-38, -29},
        Vector2 {12, -38},
        Vector2 {18, -90},
        Vector2 {87, -72},
        Vector2 {122, -26},
        Vector2 {96, 5},
        Vector2 {104, 57},
        Vector2 {43, 62},
        Vector2 {18, 44},
        Vector2 {25, 16}
    }
};

bool isAsteroidOnField(const Screen& screen, const Field& field, const Asteroid& asteroid) {
    auto pos = asteroid.pos;
    if (-screen.gap <= pos.x && pos.x <= field.w + screen.gap &&
        -screen.gap <= pos.y && pos.y <= field.h + screen.gap) {
        return true;
    }

    for (const Vector2& vertex : asteroid.vertices) {
        Vector2 p = Vector2Add(pos, vertex);
        if (-screen.gap <= p.x && p.x <= field.w + screen.gap &&
            -screen.gap <= p.y && p.y <= field.h + screen.gap) {
            return true;
        }
    }
    return false;
}

void drawNet(const Screen& screen, const Field& field, const Ship& ship) {
    int startX = -fmod(ship.pos.x, NetGap);

    int finishX = screen.w;

    for (int i = startX; i < finishX; i += NetGap) {
        int y1 = 0;
        int y2 = screen.h;
        DrawLine(i, y1, i, y2, NetColor);
    }

    int startY = -fmod(ship.pos.y, NetGap);

    int finishY = screen.h;

    for (int i = startY; i < finishY; i += NetGap) {
        int x1 = 0;
        int x2 = screen.w;

        DrawLine(x1, i, x2, i, NetColor);
    }

    if (ship.pos.y < screen.center.y) {
        int y = screen.center.y - ship.pos.y;
        DrawLine(0, y, screen.w, y, NetBorderColor);
    }

    if (field.h - ship.pos.y < screen.center.y) {
        int gap = screen.center.y - (field.h - ship.pos.y);
        int y = screen.h - gap;
        DrawLine(0, y, screen.w, y, NetBorderColor);
    }

    if (ship.pos.x < screen.center.x) {
        int x = screen.center.x - ship.pos.x;
        DrawLine(x, 0, x, screen.h, NetBorderColor);
    }

    if (field.w - ship.pos.x < screen.center.x) {
        int gap = screen.center.x - (field.w - ship.pos.x);
        int x = screen.w - gap;
        DrawLine(x, 0, x, screen.h, NetBorderColor);
    }
}

void drawShip(const Screen& screen, const Ship& ship) {
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

void drawInfo(const Font& font, const Ship& ship, size_t shotsSize, size_t asteroidsSize, size_t shardsSize) {
    float leftPadding = 10;
    float topPadding = 10;

    Vector2 textPos{ leftPadding, topPadding };

    {
        std::string buf = std::format("FPS {}", GetFPS());
        DrawTextEx(font, buf.c_str(), textPos, static_cast<float>(font.baseSize), 2, LIGHTGRAY);
        textPos.y += font.baseSize;
    }

    {
        std::string buf = std::format("Ship position ({:d}; {:d})", static_cast<int>(ship.pos.x), static_cast<int>(ship.pos.y));
        DrawTextEx(font, buf.c_str(), textPos, static_cast<float>(font.baseSize), 2, LIGHTGRAY);
        textPos.y += font.baseSize;
    }

    {
        std::string buf = std::format("Ship speed {:0.2f}", ship.speed);
        DrawTextEx(font, buf.c_str(), textPos, static_cast<float>(font.baseSize), 2, LIGHTGRAY);
        textPos.y += font.baseSize;
    }

    {
        std::string buf = std::format("Shots {}", shotsSize);
        DrawTextEx(font, buf.c_str(), textPos, static_cast<float>(font.baseSize), 2, LIGHTGRAY);
        textPos.y += font.baseSize;
    }

    {
        std::string buf = std::format("Asteroids {}", asteroidsSize);
        DrawTextEx(font, buf.c_str(), textPos, static_cast<float>(font.baseSize), 2, LIGHTGRAY);
        textPos.y += font.baseSize;
    }

    {
        std::string buf = std::format("Shards {}", shardsSize);
        DrawTextEx(font, buf.c_str(), textPos, static_cast<float>(font.baseSize), 2, LIGHTGRAY);
        textPos.y += font.baseSize;
    }
}

Vector2 fieldPosToScreenPos(const Screen& screen, const Ship& ship, Vector2 fieldPos) {
    float x = screen.center.x - (ship.pos.x - fieldPos.x);
    float y = screen.center.y - (ship.pos.y - fieldPos.y);
    return Vector2{ x, y };
}

void drawShots(const Screen& screen, const Ship& ship, const std::vector<Shot>& shots) {
    for (const auto& shot : shots) {
        Vector2 shot_point = fieldPosToScreenPos(screen, ship, shot.pos);
        DrawCircleV(shot_point, 5, RED);
    }
}

void drawAsteroid(const Screen& screen, const Ship& ship, const Asteroid& asteroid) {
    for (size_t i = 0, j = asteroid.vertices.size() - 1; i < asteroid.vertices.size(); j = i++) {
        Vector2 p1 = fieldPosToScreenPos(screen, ship, Vector2Add(asteroid.pos, asteroid.vertices[i]));
        Vector2 p2 = fieldPosToScreenPos(screen, ship, Vector2Add(asteroid.pos, asteroid.vertices[j]));
        DrawLineV(p1, p2, WHITE);
    }
}

Screen initScreen(int w, int h) {
    float gap = static_cast<float>(std::max(w, h) / 2);
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

void addShot(std::vector<Shot>& shots, const Ship& ship) {
    if (shots.size() < MaxShots) {
        shots.push_back(buildShot(ship.pos, ship.dir));
    }
}

void removeOutOfBoundsShots(const Field& field, std::vector<Shot>& shots) {
    for (size_t i = 0; i < shots.size();) {
        if (!isShotOnField(shots[i], field.w, field.h)) {
            shots[i] = shots.back();
            shots.pop_back();
        } else {
            i++;
        }
    }
}

void moveShots(std::vector<Shot>& shots) {
    for (auto& shot : shots) {
        moveShot(shot);
    }
}

bool checkPointCollisionWithPoly(Vector2 p, const std::vector<Vector2>& points, Vector2 pos) {
    assert(points.size() >= 3);

    bool inside = false;

    size_t count = points.size();

    for (size_t i = 0, j = count - 1; i < count; j = i++) {
        Vector2 p1 = Vector2Add(points[i], pos);
        Vector2 p2 = Vector2Add(points[j], pos);

        // Check if edge (i,j) crosses a horizontal ray to the right of the point
        bool intersect = ((p1.y > p.y) != (p2.y > p.y)) &&
            (p.x < (p2.x - p1.x) * (p.y - p1.y) / (p2.y - p1.y + 0.000001f) + p1.x);

        if (intersect) {
            inside = !inside;
        }
    }

    return inside;
}

bool checkShipCollisionWithPoly(const Ship& ship, const std::vector<Vector2>& points, Vector2 pos) {
    auto [v1, v2, v3] = getShipVertices(ship);
    v1 = Vector2Add(ship.pos, v1);
    v2 = Vector2Add(ship.pos, v2);
    v3 = Vector2Add(ship.pos, v3);

    return checkPointCollisionWithPoly(v1, points, pos) ||
        checkPointCollisionWithPoly(v2, points, pos) ||
        checkPointCollisionWithPoly(v3, points, pos);
}

Asteroid getRandAsteroid(const Screen& screen, const Field& field) {
    int i = GetRandomValue(0, static_cast<int>(asteroidsLibrary.size()) - 1);

    int randomSide = GetRandomValue(0, 3);

    Vector2 pos;
    Vector2 dir;

    if (randomSide == 0) { // top
        pos = Vector2{ static_cast<float>(GetRandomValue(0, field.w)), -screen.gap };
        dir = Vector2{ static_cast<float>(GetRandomValue(1, 3)), static_cast<float>(GetRandomValue(1, 3)) };
    }
    else if (randomSide == 1) { // right
        pos = Vector2{ field.w + screen.gap, static_cast<float>(GetRandomValue(0, field.h)) };
        dir = Vector2{ static_cast<float>(GetRandomValue(-3, -1)), static_cast<float>(GetRandomValue(-3, 3)) };
    }
    else if (randomSide == 2) { // bottom
        pos = Vector2{ static_cast<float>(GetRandomValue(0, field.w)), field.h + screen.gap};
        dir = Vector2{ static_cast<float>(GetRandomValue(-3, 3)), static_cast<float>(GetRandomValue(-3, -1)) };
    }
    else { // left
        pos = Vector2{ -screen.gap, (float)GetRandomValue(0, field.h) };
        dir = Vector2{ static_cast<float>(GetRandomValue(1, 3)), static_cast<float>(GetRandomValue(-3, 3)) };
    }

    return buildAsteroid(pos, dir, asteroidsLibrary[i]);
}

void drawScore(const Font& font, const Screen& screen, uint64_t score) {
    std::string buf = std::format("Score {}", score);
    Vector2 textSize = MeasureTextEx(font, buf.c_str(), static_cast<float>(font.baseSize), 2);
    Vector2 textPos = {(screen.w - textSize.x) / 2.0f, 10.0f};
    DrawTextEx(font, buf.c_str(), textPos, static_cast<float>(font.baseSize), 2, LIGHTGRAY);
}

void moveAsteroids(std::vector<Asteroid>& asteroids) {
    for (auto& a : asteroids) {
        rotateAsteroid(a);
        moveAsteroid(a);
    }
}

void removeOutOfBoundsAsteroids(const Screen& screen, const Field& field, std::vector<Asteroid>& asteroids) {
    for (size_t i = 0; i < asteroids.size();) {
        if (!isAsteroidOnField(screen, field, asteroids[i])) {
            asteroids[i] = asteroids.back();
            asteroids.pop_back();
        } else {
            i++;
        }
    }
}

std::optional<size_t> findShotCollidedWithAsteroid(const std::vector<Shot>& shots, const Asteroid& asteroid) {
    for (size_t i = 0; i < shots.size(); i++) {
        const Shot& shot = shots[i];

        if (checkPointCollisionWithPoly(shot.pos, asteroid.vertices, asteroid.pos)) {
            return i;
        }
    }

    return std::nullopt;
}

void updateTitleScreen(GameState& gameState) {
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
        gameState.gameScreen = GameScreen::GAME;
    }
}

void renderTitleScreen(const Screen& screen, const Font& font) {
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
}

void updateGame(const Screen& screen, GameState& gameState) {
    gameState.ship.is_engine_working = false;

    if (IsKeyPressed(KEY_SPACE) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        addShot(gameState.shots, gameState.ship);
    }

    if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) {
        rotateShip(gameState.ship, Turn::LEFT);
    }

    if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) {
        rotateShip(gameState.ship, Turn::RIGHT);
    }

    if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W)) {
        moveShip(gameState.ship, Move::FORWARD);
    }

    if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) {
        moveShip(gameState.ship, Move::BACKWARD);
    }

    if (IsKeyPressed(KEY_L)) {
        gameState.debugDisplay = !gameState.debugDisplay;
    }

    slowdownShip(gameState.ship, gameState.field.w, gameState.field.h);

    if (gameState.asteroids.size() < MaxAsteroidsCount) {
        const size_t missingAsteroidsSize = MaxAsteroidsCount - gameState.asteroids.size();
        for (size_t i = 0; i < missingAsteroidsSize; ++i) {
            gameState.asteroids.push_back(getRandAsteroid(screen, gameState.field));
        }
    }

    moveAsteroids(gameState.shards);
    moveAsteroids(gameState.asteroids);
    moveShots(gameState.shots);

    removeOutOfBoundsAsteroids(screen, gameState.field, gameState.shards);
    removeOutOfBoundsAsteroids(screen, gameState.field, gameState.asteroids);
    removeOutOfBoundsShots(gameState.field, gameState.shots);

    for (size_t i = 0; i < gameState.asteroids.size();) {
        Asteroid& asteroid= gameState.asteroids[i];

        if (checkShipCollisionWithPoly(gameState.ship, asteroid.vertices, asteroid.pos)) {
            gameState.asteroids[i] = gameState.asteroids.back();
            gameState.asteroids.pop_back();
        } else {
            i++;
        }
    }

    {
        for (size_t i = 0; i < gameState.asteroids.size();) {
            Asteroid& asteroid = gameState.asteroids[i];

            auto shotIdx = findShotCollidedWithAsteroid(gameState.shots, asteroid);
            if (shotIdx) {
                std::vector<Asteroid> asteroidShards = asteroidToShards(asteroid);
                gameState.shards.insert(gameState.shards.end(),
                                        asteroidShards.begin(),
                                        asteroidShards.end());

                gameState.shots[*shotIdx] = gameState.shots.back();
                gameState.shots.pop_back();

                gameState.asteroids[i] = gameState.asteroids.back();
                gameState.asteroids.pop_back();

                gameState.score++;
            } else {
                i++;
            }
        }
    }
}

void renderGame(const Screen& screen, const Font& font, const GameState& gameState) {
    BeginDrawing();

    ClearBackground(DARKGRAY);

    drawNet(screen, gameState.field, gameState.ship);
    drawShip(screen, gameState.ship);

    drawShots(screen, gameState.ship, gameState.shots);

    for (const auto& a : gameState.asteroids) {
        drawAsteroid(screen, gameState.ship, a);
    }

    for (const auto& s : gameState.shards) {
        drawAsteroid(screen, gameState.ship, s);
    }

    drawScore(font, screen, gameState.score);

    if (gameState.debugDisplay) {
        drawInfo(font, gameState.ship, gameState.shots.size(), gameState.asteroids.size(), gameState.shards.size());
    }

    EndDrawing();
}

GameState initGameState() {
    Field field{2000, 1000};
    GameState gameState = {
        .field = field,
        .gameScreen = GameScreen::TITLE,
        .score = 0,
        .debugDisplay = false,
        .ship = buildShip(field.w, field.h),
        .shots = {},
        .asteroids = {},
        .shards = {},
    };
    return gameState;
}

int main(void) {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);

    InitWindow(InitScreenWidth, InitScreenHeight, "Asteroids");

    Font font = LoadFontEx("./resources/font.ttf", 32, 0, 0);
    if (font.texture.id == 0) {
        TraceLog(LOG_ERROR, "Failed to load font!");
    }

    SetTargetFPS(60);

    Screen screen = initScreen(GetScreenWidth(), GetScreenHeight());

    GameState gameState = initGameState();

    while (!WindowShouldClose()) {
        if (IsWindowResized()) {
            screen = initScreen(GetScreenWidth(), GetScreenHeight());
        }

        if (gameState.gameScreen == GameScreen::TITLE) {
            updateTitleScreen(gameState);
            renderTitleScreen(screen, font);
        } else if (gameState.gameScreen == GameScreen::GAME) {
            updateGame(screen, gameState);
            renderGame(screen, font, gameState);
        }
    }

    UnloadFont(font);
    CloseWindow();

    std::cout << "Bye!" << std::endl;

    return 0;
}
