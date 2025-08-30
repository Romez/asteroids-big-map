#include "include/raylib.h"
#include "include/raymath.h"
#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>

#define MAX_SHOTS 5

#define SCREEN_WIDTH 1600
#define SCREEN_HEIGHT 900

const float ROTATION_SPEED = PI / 32;
const float MAX_SPEED = 6;

int fieldWidth = 2000;
int fieldHeight = 2000;

typedef struct {
    Vector2 dir;
    Vector2 field_pos;
    float speed;
    bool is_engine_working;
} Ship;

typedef struct {
    Vector2 pos;
    Vector2 dir;
    bool visible;
} Shot;

typedef struct {
	uint32_t len;
	Shot shots[MAX_SHOTS];
} AllShots;

typedef struct {
    Vector2 pos;
} Health;

const float SCREEN_CENTER_X = SCREEN_WIDTH / 2.0;
const float SCREEN_CENTER_Y = SCREEN_HEIGHT / 2.0;

Vector2 screen_center_v = {
    .x = SCREEN_CENTER_X,
    .y = SCREEN_CENTER_Y
};

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
    Vector2 ship_size = Vector2Scale(ship.dir, 15);

    Vector2 v1 = Vector2Add(screen_center_v, ship_size);

    float l = (3 * PI) / 4;
    Vector2 v2 = Vector2Add(screen_center_v, Vector2Rotate(ship_size, l));

    float r = (5 * PI) / 4;
    Vector2 v3 = Vector2Add(screen_center_v, Vector2Rotate(ship_size, r));

    DrawTriangleLines(v1, v2, v3, WHITE);

    if (ship.is_engine_working) {
	for (int i = 0; i < 4; i++) {
	    Vector2 ve1 = Vector2Add(v2, Vector2Scale(ship.dir, -5 * i));
	    Vector2 ve2 = Vector2Add(v3, Vector2Scale(ship.dir, -5 * i));
	    DrawLineV(ve1, ve2, RED);
	}
    }
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

#define INFO_TEXT_SIZE 26

void draw_info(Ship* ship, AllShots* shots) {
	{
		char buf[26] = {0};
		sprintf(buf, "(%d; %d), %.2lf", (int)ship->field_pos.x, (int)ship->field_pos.y, ship->speed);
		DrawText(buf, SCREEN_CENTER_X + 20, SCREEN_CENTER_Y, INFO_TEXT_SIZE, LIGHTGRAY);
	}
	
	{
		char buf[128] = {0};
		snprintf(buf, 128, "shots %d", shots->len);
		DrawText(buf, 10, SCREEN_CENTER_Y, INFO_TEXT_SIZE, LIGHTGRAY);
	}
}

Ship create_ship() {
	Ship ship = {
		.dir = (Vector2){1, 0},
		.field_pos = (Vector2){(float)fieldWidth / 2.0, (float)fieldHeight / 2.0},
		.speed = 0.0,
		.is_engine_working = false,
    };
	return ship;
}

void draw_shots(Ship* ship, AllShots* shots) {
	for(size_t i = 0; i < MAX_SHOTS; i++) {
		Shot shot = shots->shots[i];
		if (shot.visible) {
			float x = ship->field_pos.x - shot.pos.x;
			float y = ship->field_pos.y - shot.pos.y;

			Vector2 shot_point;
			shot_point.x = SCREEN_CENTER_X - x;
			shot_point.y = SCREEN_CENTER_Y - y;

			DrawCircleV(shot_point, 5, RED);
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

    Ship ship = create_ship();

    // size_t healths_size = 3;
    // Health healths[] = {
	// 	{.pos = (Vector2){400, 400}},
	// 	{.pos = (Vector2){700, 700}},
	// 	{.pos = (Vector2){900, 900}},
    // };

	AllShots shots = {0};

    // Main game loop
    while (!WindowShouldClose()) {
		ship.is_engine_working = false;

		if (IsKeyPressed(KEY_SPACE)) {
			if (shots.len < MAX_SHOTS) {
				for (size_t i = 0; i < MAX_SHOTS; i++) {
					if (!shots.shots[i].visible) {
						Shot shot = {
							.dir = ship.dir,
							.pos = ship.field_pos,
							.visible = true,
						};
						shots.shots[i] = shot;
						shots.len++;
						break;
					}
				}
			}
		}

		if (IsKeyDown(KEY_LEFT)) {
			ship.dir = Vector2Rotate(ship.dir, -ROTATION_SPEED);
		}

		if (IsKeyDown(KEY_RIGHT)) {
			ship.dir = Vector2Rotate(ship.dir, ROTATION_SPEED);
		}

		if (IsKeyDown(KEY_UP)) {
			if (ship.speed < MAX_SPEED) {
				ship.speed += 0.2;
			}
			ship.is_engine_working = true;
		}

		if (IsKeyDown(KEY_DOWN)) {
			if (ship.speed > -MAX_SPEED) {
				ship.speed -= 0.2;
			}
			ship.is_engine_working = true;
		}

		if (ship.speed > 0) {
			Vector2 new_pos = Vector2Add(ship.field_pos, Vector2Scale(ship.dir, ship.speed));

			if (0 <= new_pos.x && new_pos.x < fieldWidth) {
				ship.field_pos.x = new_pos.x;
			}

			if (0 <= new_pos.y && new_pos.y < fieldHeight) {
				ship.field_pos.y = new_pos.y;
			}

			if (ship.speed > 0.07) {
				ship.speed -= 0.07;
			} else {
				ship.speed = 0.0;
			}
		}

		if (ship.speed < 0) {
			ship.field_pos = Vector2Add(ship.field_pos, Vector2Scale(ship.dir, ship.speed));

			if (ship.speed < 0.07) {
				ship.speed += 0.07;
			} else {
				ship.speed = 0.0;
			}
		}

		// Shots
		for (size_t i = 0; i < MAX_SHOTS; i++) {
			Shot shot = shots.shots[i];
			if (shot.visible) {
				if (0 <= shot.pos.x && shot.pos.x <= fieldWidth && 0 <= shot.pos.y && shot.pos.y <= fieldHeight) {
					Vector2 shot_speed = Vector2Scale(shot.dir, 15);
					shot.pos = Vector2Add(shot.pos, shot_speed);
					shots.shots[i] = shot;
				} else {
					shots.shots[i].visible = false;
					shots.len--;
				}
			}
		}

		// Draw
		//----------------------------------------------------------------------------------
		BeginDrawing();
		ClearBackground(DARKGRAY);

		draw_net(ship);
		draw_ship(ship);
		// draw_healths(ship, healths, healths_size);

		draw_shots(&ship, &shots);

		// --------------
		draw_info(&ship, &shots);
		
		EndDrawing();
		//----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow(); // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
