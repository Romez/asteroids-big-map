#include "include/raylib.h"
#include <math.h>
#include <stdio.h>

typedef struct {
    float dir;
    Vector2 screen_pos;
    Vector2 field_pos;
    float speed;
    bool is_engine_working;
} Ship;

int screenWidth = 1600;
int screenHeight = 900;

int fieldWidth = 2000;
int fieldHeight = 2000;

int net_gap = 100;

void draw_net(Ship ship) {
    // Net vertical
    int startX = -fmod(ship.field_pos.x, net_gap);

    int finishX = screenWidth;

    for (int i = startX; i < finishX; i += net_gap) {
        int y1 = 0;
        int y2 = screenHeight;
        DrawLine(i, y1, i, y2, LIME);
    }

    int startY = -fmod(ship.field_pos.y, net_gap);

    int finishY = screenHeight;

    // Net horizontal
    for (int i = startY; i < finishY; i += net_gap) {
        int x1 = 0;
        int x2 = screenWidth;

        DrawLine(x1, i, x2, i, LIME);
    }
    //

    // Border lines

    if (ship.field_pos.y < ship.screen_pos.y) {
        int y = ship.screen_pos.y - ship.field_pos.y;
	DrawLine(0, y, screenWidth, y, RED);
    }

    if (fieldHeight - ship.field_pos.y < ship.screen_pos.y) {
	int gap = ship.screen_pos.y - (fieldHeight - ship.field_pos.y);
        int y = screenHeight - gap;
	DrawLine(0, y, screenWidth, y, RED);
    }

    if (ship.field_pos.x < ship.screen_pos.x) {
	int x = ship.screen_pos.x - ship.field_pos.x;
	DrawLine(x, 0, x, screenHeight, RED);
    }

    if (fieldWidth - ship.field_pos.x < ship.screen_pos.x) {
	int gap = ship.screen_pos.x - (fieldWidth - ship.field_pos.x);
        int x = screenWidth - gap;
	DrawLine(x, 0, x, screenHeight, RED);
    }
}

void draw_ship(Ship ship) {
    Vector2 v1 = (Vector2){
	.x = ship.screen_pos.x + (cosf(ship.dir) * 15),
	.y = ship.screen_pos.y - (sinf(ship.dir) * 15),
    };

    float l = (3 * PI) / 4;
    Vector2 v2 = (Vector2){
	.x = ship.screen_pos.x + cosf(ship.dir + l) * 15,
	.y = ship.screen_pos.y - sinf(ship.dir + l) * 15,
    };

    float r = (5 * PI) / 4;
    Vector2 v3 = (Vector2){
	.x = ship.screen_pos.x + cosf(ship.dir + r) * 15,
	.y = ship.screen_pos.y - sinf(ship.dir + r) * 15,
    };

    if (ship.is_engine_working) {
	Vector2 v4 = (Vector2) {
	    .x = ship.screen_pos.x + cosf(ship.dir + l + (PI / 12)) * 18,
	    .y = ship.screen_pos.y - sinf(ship.dir + l + (PI / 12)) * 18,
	};

	Vector2 v5 = (Vector2) {
	    .x = ship.screen_pos.x + cosf(ship.dir + r - (PI / 12)) * 18,
	    .y = ship.screen_pos.y - sinf(ship.dir + r - (PI / 12)) * 18,
	};

	Vector2 v6 = (Vector2) {
	    .x = ship.screen_pos.x + cosf(ship.dir + l + (PI / 6)) * 21,
	    .y = ship.screen_pos.y - sinf(ship.dir + l + (PI / 6)) * 21,
	};

	Vector2 v7 = (Vector2) {
	    .x = ship.screen_pos.x + cosf(ship.dir + r - (PI / 6)) * 21,
	    .y = ship.screen_pos.y - sinf(ship.dir + r - (PI / 6)) * 21,
	};

	Vector2 v8 = (Vector2) {
	    .x = ship.screen_pos.x + cosf(ship.dir + l + (PI / 5)) * 26,
	    .y = ship.screen_pos.y - sinf(ship.dir + l + (PI / 5)) * 26,
	};

	Vector2 v9 = (Vector2) {
	    .x = ship.screen_pos.x + cosf(ship.dir + r - (PI / 5)) * 26,
	    .y = ship.screen_pos.y - sinf(ship.dir + r - (PI / 5)) * 26,
	};

	DrawLineV(v2, v3, RED);
	DrawLineV(v4, v5, RED);
	DrawLineV(v6, v7, RED);
	DrawLineV(v8, v9, RED);
    }

    DrawTriangleLines(v1, v2, v3, WHITE);
}

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void) {
    // Initialization
    //--------------------------------------------------------------------------------------
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);

    InitWindow(screenWidth, screenHeight, "Asteroids");

    SetTargetFPS(60); // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    Ship ship = {
        .dir = 0,
	.screen_pos = (Vector2) {screenWidth / 2.0, screenHeight / 2.0},
	.field_pos = (Vector2) {fieldWidth / 2.0, fieldHeight / 2.0},
	.speed = 0.0,
	.is_engine_working = false,
    };

    // Main game loop
    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_F11)) {
            ToggleFullscreen();
            screenWidth = GetScreenWidth();
            screenHeight = GetScreenHeight();
        }

        if (IsKeyDown(KEY_LEFT)) {
            ship.dir = fmod(ship.dir + 0.1, 2 * PI);
        }

        if (IsKeyDown(KEY_RIGHT)) {
	    if (ship.dir > 0.1) {
		ship.dir = fmod(ship.dir - 0.1, 2 * PI);
	    } else {
		ship.dir = 2 * PI;
	    }
        }

        if (IsKeyDown(KEY_UP)) {
	    if (ship.speed < 6) {
		ship.speed += 0.2;
	    }
	    ship.is_engine_working = true;
        }

        if (IsKeyDown(KEY_DOWN)) {
	    if (ship.speed > -6) {
		ship.speed -= 0.2;
	    }
	    ship.is_engine_working = true;
        }

	if (ship.speed > 0.0) {
	    float x = ship.field_pos.x + cosf(ship.dir) * ship.speed;
	    float y = ship.field_pos.y - sinf(ship.dir) * ship.speed;

	    if (0 <= x && x <= fieldWidth) {
		ship.field_pos.x = x;
	    }

	    if (0 <= y && y <= fieldHeight) {
		ship.field_pos.y = y;
	    }

	    if (ship.speed > 0.07) {
		ship.speed -= 0.07;
	    } else {
		ship.speed = 0.0;
	    }
	}

	if (ship.speed < 0.0) {
	    float x = ship.field_pos.x + cosf(ship.dir) * ship.speed;
            float y = ship.field_pos.y - sinf(ship.dir) * ship.speed;

	    if (0 <= x && x <= fieldWidth) {
		ship.field_pos.x = x;
	    }

	    if (0 <= y && y <= fieldHeight) {
		ship.field_pos.y = y;
	    }

	    if (ship.speed < 0.07) {
		ship.speed += 0.07;
	    } else {
		ship.speed = 0.0;
	    }
	}

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();
	ClearBackground(DARKGRAY);

	draw_net(ship);
	draw_ship(ship);

	// --------------

	// Ship position info
	char position_buf[26];
	sprintf(position_buf, "(%d; %d), %.2f, %.2f", (int)ship.field_pos.x, (int)ship.field_pos.y, ship.speed, ship.dir);
	DrawText(position_buf, ship.screen_pos.x + 20, ship.screen_pos.y, 26, LIGHTGRAY);
	//
        EndDrawing();
        //----------------------------------------------------------------------------------

	ship.is_engine_working = false;
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
