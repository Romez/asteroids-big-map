#include "include/raylib.h"
#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#define SHOTS_MAX_LEN 5

#define SCREEN_WIDTH 1600
#define SCREEN_HEIGHT 900

int fieldWidth = 2000;
int fieldHeight = 2000;

typedef struct {
    float dir;
    Vector2 field_pos;
    float speed;
    bool is_engine_working;
} Ship;

typedef struct {
    Vector2 pos;
    float dir;
} Shot;

typedef struct {
    Vector2 pos;
} Health;

float SCREEN_CENTER_X = SCREEN_WIDTH / 2.0;
float SCREEN_CENTER_Y = SCREEN_HEIGHT / 2.0;

int net_gap = 100;

void draw_net(Ship ship) {
    // Net vertical
    int startX = -fmod(ship.field_pos.x, net_gap);

    int finishX = SCREEN_WIDTH;

    for (int i = startX; i < finishX; i += net_gap) {
	int y1 = 0;
	int y2 = SCREEN_HEIGHT;
	DrawLine(i, y1, i, y2, LIME);
    }

    int startY = -fmod(ship.field_pos.y, net_gap);

    int finishY = SCREEN_HEIGHT;

    // Net horizontal
    for (int i = startY; i < finishY; i += net_gap) {
	int x1 = 0;
	int x2 = SCREEN_WIDTH;

	DrawLine(x1, i, x2, i, LIME);
    }
    //

    // Border lines

    if (ship.field_pos.y < SCREEN_CENTER_Y) {
	int y = SCREEN_CENTER_Y - ship.field_pos.y;
	DrawLine(0, y, SCREEN_WIDTH, y, RED);
    }

    if (fieldHeight - ship.field_pos.y < SCREEN_CENTER_Y) {
	int gap = SCREEN_CENTER_Y - (fieldHeight - ship.field_pos.y);
	int y = SCREEN_HEIGHT - gap;
	DrawLine(0, y, SCREEN_WIDTH, y, RED);
    }

    if (ship.field_pos.x < SCREEN_CENTER_X) {
	int x = SCREEN_CENTER_X - ship.field_pos.x;
	DrawLine(x, 0, x, SCREEN_HEIGHT, RED);
    }

    if (fieldWidth - ship.field_pos.x < SCREEN_CENTER_X) {
	int gap = SCREEN_CENTER_X - (fieldWidth - ship.field_pos.x);
	int x = SCREEN_WIDTH - gap;
	DrawLine(x, 0, x, SCREEN_HEIGHT, RED);
    }
}

void draw_ship(Ship ship) {
    Vector2 v1 = (Vector2){
	.x = SCREEN_CENTER_X + (cosf(ship.dir) * 15),
	.y = SCREEN_CENTER_Y - (sinf(ship.dir) * 15),
    };

    float l = (3 * PI) / 4;
    Vector2 v2 = (Vector2){
	.x = SCREEN_CENTER_X + cosf(ship.dir + l) * 15,
	.y = SCREEN_CENTER_Y - sinf(ship.dir + l) * 15,
    };

    float r = (5 * PI) / 4;
    Vector2 v3 = (Vector2){
	.x = SCREEN_CENTER_X + cosf(ship.dir + r) * 15,
	.y = SCREEN_CENTER_Y - sinf(ship.dir + r) * 15,
    };

    if (ship.is_engine_working) {
	Vector2 v4 = (Vector2){
	    .x = SCREEN_CENTER_X + cosf(ship.dir + l + (PI / 12)) * 18,
	    .y = SCREEN_CENTER_Y - sinf(ship.dir + l + (PI / 12)) * 18,
	};

	Vector2 v5 = (Vector2){
	    .x = SCREEN_CENTER_X + cosf(ship.dir + r - (PI / 12)) * 18,
	    .y = SCREEN_CENTER_Y - sinf(ship.dir + r - (PI / 12)) * 18,
	};

	Vector2 v6 = (Vector2){
	    .x = SCREEN_CENTER_X + cosf(ship.dir + l + (PI / 6)) * 21,
	    .y = SCREEN_CENTER_Y - sinf(ship.dir + l + (PI / 6)) * 21,
	};

	Vector2 v7 = (Vector2){
	    .x = SCREEN_CENTER_X + cosf(ship.dir + r - (PI / 6)) * 21,
	    .y = SCREEN_CENTER_Y - sinf(ship.dir + r - (PI / 6)) * 21,
	};

	Vector2 v8 = (Vector2){
	    .x = SCREEN_CENTER_X + cosf(ship.dir + l + (PI / 5)) * 26,
	    .y = SCREEN_CENTER_Y - sinf(ship.dir + l + (PI / 5)) * 26,
	};

	Vector2 v9 = (Vector2){
	    .x = SCREEN_CENTER_X + cosf(ship.dir + r - (PI / 5)) * 26,
	    .y = SCREEN_CENTER_Y - sinf(ship.dir + r - (PI / 5)) * 26,
	};

	DrawLineV(v2, v3, RED);
	DrawLineV(v4, v5, RED);
	DrawLineV(v6, v7, RED);
	DrawLineV(v8, v9, RED);
    }

    DrawTriangleLines(v1, v2, v3, WHITE);
}

void draw_healths(Ship ship, Health *healths, size_t healths_size) {
    for (int i = 0; i < healths_size; i++) {
	Health h = healths[i];
	float x = ship.field_pos.x - h.pos.x;
	float y = ship.field_pos.y - h.pos.y;
	float radius = 26.0;
	if (fabs(x) - radius < SCREEN_CENTER_X &&
	    fabs(y) - radius < SCREEN_CENTER_Y) {
	    DrawCircle(SCREEN_CENTER_X - x, SCREEN_CENTER_Y - y, radius, RED);
	}
    }
}

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void) {
    // Initialization
    //--------------------------------------------------------------------------------------
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Asteroids");

    SetTargetFPS(60); // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    Ship ship = {
	.dir = 0,
	.field_pos = (Vector2){(float)fieldWidth / 2.0, (float)fieldHeight / 2.0},
	.speed = 0.0,
	.is_engine_working = false,
    };

    size_t healths_size = 3;
    Health healths[] = {
	{.pos = (Vector2){400, 400}},
	{.pos = (Vector2){700, 700}},
	{.pos = (Vector2){900, 900}},
    };

    size_t shots_size = 0;
    Shot shots[SHOTS_MAX_LEN];

    // Main game loop
    while (!WindowShouldClose()) {
	if (IsKeyPressed(KEY_SPACE)) {
	    if (shots_size < SHOTS_MAX_LEN) {
		Shot shot;
		shot.dir = ship.dir;
		shot.pos = ship.field_pos;

		shots[shots_size++] = shot;
	    }
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

	// shots
	for(size_t i = 0; i < shots_size; i++) {
	    Shot shot = shots[i];

	    shot.pos.x += cosf(shot.dir) * 2;
	    shot.pos.y -= sinf(shot.dir) * 2;

	    if (0 <= shot.pos.x && shot.pos.x <= fieldWidth &&
		0 <= shot.pos.y && shot.pos.y <= fieldHeight) {
		shots[i] = shot;
	    } else {
		memcpy(&shots[i], &shots[i+1], sizeof(Shot) * (shots_size - i - 1));
		shots_size -= 1;
	    }
	}

	// Draw
	//----------------------------------------------------------------------------------
	BeginDrawing();
	ClearBackground(DARKGRAY);

	draw_net(ship);
	draw_ship(ship);
	draw_healths(ship, healths, healths_size);

	for(size_t i = 0; i < shots_size; i++) {
	    Shot shot = shots[i];

            float x = ship.field_pos.x - shot.pos.x;
	    float y = ship.field_pos.y - shot.pos.y;

            if (fabs(x) < SCREEN_CENTER_X && fabs(y) < SCREEN_CENTER_Y) {
		float x2 = x + cosf(shot.dir) * 20;
		float y2 = y - sinf(shot.dir) * 20;

		DrawLine(SCREEN_CENTER_X - x, SCREEN_CENTER_Y - y,
			 SCREEN_CENTER_X - x2, SCREEN_CENTER_Y - y2, BLUE);
	    }
	}

	// --------------

	// Ship position info
	char position_buf[26];
	sprintf(position_buf, "(%d; %d), %.2f, %.2f", (int)ship.field_pos.x,
		(int)ship.field_pos.y, ship.speed, ship.dir);
	DrawText(position_buf, SCREEN_CENTER_X + 20, SCREEN_CENTER_Y, 26,
		 LIGHTGRAY);
	//
	EndDrawing();
	//----------------------------------------------------------------------------------

	ship.is_engine_working = false;
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow(); // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
