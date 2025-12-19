#include "include/raylib.h"
#include "include/raymath.h"
#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

#define MAX_SHOTS 100

#define INIT_SCREEN_WIDTH 1600
#define INIT_SCREEN_HEIGHT 900
#define INFO_TEXT_SIZE 26

#define SIGHT_COLOR RED

const float ROTATION_SPEED = PI / 32;
const float MAX_SPEED = 6;

int fieldWidth = 2000;
int fieldHeight = 2000;

enum turn {
  LEFT,
  RIGHT,
};

enum move {
  FORWARD,
  BACKWARD,
};

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
  int w;
  int h;
  Vector2 center;
} Screen;

int net_gap = 100;

Ship create_ship() {
  Ship ship = {
      .dir = (Vector2){1, 0},
      .field_pos = (Vector2){fieldWidth / 2.0f, fieldHeight / 2.0f},
      .speed = 0.0,
      .is_engine_working = false,
  };
  return ship;
}

void draw_net(Screen *screen, Ship *ship) {
  // Net vertical
  int startX = -fmod(ship->field_pos.x, net_gap);

  int finishX = screen->w;

  for (int i = startX; i < finishX; i += net_gap) {
    int y1 = 0;
    int y2 = screen->h;
    DrawLine(i, y1, i, y2, LIME);
  }

  int startY = -fmod(ship->field_pos.y, net_gap);

  int finishY = screen->h;

  // Net horizontal
  for (int i = startY; i < finishY; i += net_gap) {
    int x1 = 0;
    int x2 = screen->w;

    DrawLine(x1, i, x2, i, LIME);
  }

  // Border lines

  if (ship->field_pos.y < screen->center.y) {
    int y = screen->center.y - ship->field_pos.y;
    DrawLine(0, y, screen->w, y, RED);
  }

  if (fieldHeight - ship->field_pos.y < screen->center.y) {
    int gap = screen->center.y - (fieldHeight - ship->field_pos.y);
    int y = screen->h - gap;
    DrawLine(0, y, screen->w, y, RED);
  }

  if (ship->field_pos.x < screen->center.x) {
    int x = screen->center.x - ship->field_pos.x;
    DrawLine(x, 0, x, screen->h, RED);
  }

  if (fieldWidth - ship->field_pos.x < screen->center.x) {
    int gap = screen->center.x - (fieldWidth - ship->field_pos.x);
    int x = screen->w - gap;
    DrawLine(x, 0, x, screen->h, RED);
  }
}

void draw_ship(Screen *screen, Ship *ship) {
  Vector2 ship_size = Vector2Scale(ship->dir, 15);

  Vector2 v1 = Vector2Add(screen->center, ship_size);

  float l = (3 * PI) / 4;
  Vector2 v2 = Vector2Add(screen->center, Vector2Rotate(ship_size, l));

  float r = (5 * PI) / 4;
  Vector2 v3 = Vector2Add(screen->center, Vector2Rotate(ship_size, r));

  DrawTriangleLines(v1, v2, v3, WHITE);

  if (ship->is_engine_working) {
    for (int i = 0; i < 4; i++) {
      Vector2 ve1 = Vector2Add(v2, Vector2Scale(ship->dir, -5 * i));
      Vector2 ve2 = Vector2Add(v3, Vector2Scale(ship->dir, -5 * i));
      DrawLineV(ve1, ve2, RED);
    }
  }
}

void draw_info(Screen *screen, Ship *ship, AllShots *shots) {
  {
    char buf[26] = {0};
    sprintf(buf, "(%d; %d), %.2lf", (int)ship->field_pos.x,
            (int)ship->field_pos.y, ship->speed);
    DrawText(buf, screen->center.x + 20, screen->center.y, INFO_TEXT_SIZE, LIGHTGRAY);
  }

  {
    char buf[128] = {0};
    snprintf(buf, 128, "shots %d", shots->len);
    DrawText(buf, 10, screen->center.y, INFO_TEXT_SIZE, LIGHTGRAY);
  }
}

Vector2 field_pos_to_screen_pos(Screen *screen, Ship *ship, Vector2 field_pos) {
  float x = screen->center.x - (ship->field_pos.x - field_pos.x);
  float y = screen->center.y - (ship->field_pos.y - field_pos.y);
  return Vector2{x, y};
}

void draw_shots(Screen *screen, Ship *ship, AllShots *shots) {
  for (size_t i = 0; i < MAX_SHOTS; i++) {
    Shot shot = shots->shots[i];
    if (shot.visible) {
      Vector2 shot_point = field_pos_to_screen_pos(screen, ship, shot.pos);
      DrawCircleV(shot_point, 5, RED);
    }
  }
}

Screen init_screen(int w, int h) {
  return (Screen){
	.w = w,
	.h = h,
	.center =
	(Vector2){
	  .x = w / 2.0f,
	  .y = h / 2.0f,
	},
  };
}

void rotate_ship(Ship *ship, enum turn t) {
  ship->dir = Vector2Rotate(ship->dir, t == LEFT ? -ROTATION_SPEED : ROTATION_SPEED);
}

void move_ship(Ship *ship, enum move m) {
  if (m == FORWARD) {
    if (ship->speed < MAX_SPEED) {
      ship->speed += 0.2;
    }
    ship->is_engine_working = true;
  } else if (m == BACKWARD) {
    if (ship->speed > -MAX_SPEED) {
      ship->speed -= 0.2;
    }
    ship->is_engine_working = true;
  } else {
    assert(NULL && "Unexpected move type");
  }
}

void slowdown_ship(Ship *ship) {
  if (ship->speed != 0) {
    Vector2 new_pos = Vector2Add(ship->field_pos, Vector2Scale(ship->dir, ship->speed));

    if (0 <= new_pos.x && new_pos.x < fieldWidth) {
      ship->field_pos.x = new_pos.x;
    }

    if (0 <= new_pos.y && new_pos.y < fieldHeight) {
      ship->field_pos.y = new_pos.y;
    }

    if (ship->speed > 0) {
      if (ship->speed > 0.07) {
        ship->speed -= 0.07;
      } else {
        ship->speed = 0.0;
      }
    }

    if (ship->speed < 0) {
      if (ship->speed < 0.07) {
        ship->speed += 0.07;
      } else {
        ship->speed = 0.0;
      }
    }
  }
}

void add_shot(Screen *screen, AllShots *shots, Ship *ship, Vector2 sight) {
  if (shots->len < MAX_SHOTS) {
    for (size_t i = 0; i < MAX_SHOTS; i++) {
      if (!shots->shots[i].visible) {
        Vector2 dir = (Vector2){
            .x = sight.x - screen->center.x,
            .y = sight.y - screen->center.y,
        };
        Shot shot = {
            .pos = ship->field_pos,
            .dir = Vector2Scale(Vector2Normalize(dir), 1),
            .visible = true,
        };
        shots->shots[i] = shot;
        shots->len++;
        break;
      }
    }
  }
}

bool is_shot_on_field(Shot *shot) {
  float x = shot->pos.x;
  float y = shot->pos.y;
  return 0 <= x && x <= fieldWidth && 0 <= y && y <= fieldHeight;
}

void move_shots(AllShots *shots) {
  for (size_t i = 0; i < MAX_SHOTS; i++) {
    Shot shot = shots->shots[i];
    if (shot.visible) {
      if (is_shot_on_field(&shot)) {
        Vector2 shot_speed = Vector2Scale(shot.dir, 15);
        shot.pos = Vector2Add(shot.pos, shot_speed);
        shots->shots[i] = shot;
      } else {
        shots->shots[i].visible = false;
        shots->len--;
      }
    }
  }
}

void draw_sight(Vector2 sight) {
  int gap = 10;

  DrawCircleV(sight, 2, SIGHT_COLOR);
  DrawCircleLinesV(sight, 30, SIGHT_COLOR);

  DrawRectangle(sight.x, sight.y - 40, 2, 30, SIGHT_COLOR);  // top
  DrawRectangle(sight.x + gap, sight.y, 30, 2, SIGHT_COLOR); // right
  DrawRectangle(sight.x, sight.y + gap, 2, 30, SIGHT_COLOR); // bottom
  DrawRectangle(sight.x - 40, sight.y, 30, 2, SIGHT_COLOR);  // left
}

typedef struct {
  Vector2 pos;
  Vector2 dir;
  std::vector<Vector2> points;
} Asteroid;

// std::vector<Asteroid> asteroids;

int main(void) {
  // Initialization
  //--------------------------------------------------------------------------------------
  SetConfigFlags(FLAG_WINDOW_RESIZABLE);

  InitWindow(INIT_SCREEN_WIDTH, INIT_SCREEN_HEIGHT, "Asteroids");

  SetTargetFPS(60);

  DisableCursor();
  //--------------------------------------------------------------------------------------

  Ship ship = create_ship();

  AllShots shots = {0};

  Screen screen = init_screen(GetScreenWidth(), GetScreenHeight());

  Vector2 sight = {0, 0};

  Asteroid ast = (Asteroid){
    .pos = Vector2{fieldWidth / 2.f, fieldHeight / 2.f},
    .dir = Vector2{1, 0},
    .points = std::vector<Vector2> {
      Vector2{1, 0}, Vector2{1, 1}, Vector2{-1, 0}, Vector2{-1, -1},
    },
  };

  // Main game loop
  while (!WindowShouldClose()) {
    if (IsWindowResized()) {
      screen = init_screen(GetScreenWidth(), GetScreenHeight());
    }

    sight = GetMousePosition();

    ship.is_engine_working = false;

    if (IsKeyPressed(KEY_SPACE) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
      add_shot(&screen, &shots, &ship, sight);
    }

    if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) {
      rotate_ship(&ship, LEFT);
    }

    if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) {
      rotate_ship(&ship, RIGHT);
    }

    if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W)) {
      move_ship(&ship, FORWARD);
    }

    if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) {
      move_ship(&ship, BACKWARD);
    }

    slowdown_ship(&ship);

    move_shots(&shots);

    // Draw
    //----------------------------------------------------------------------------------
    BeginDrawing();
    ClearBackground(DARKGRAY);

    draw_net(&screen, &ship);
    draw_ship(&screen, &ship);

    draw_shots(&screen, &ship, &shots);

	// DrawSplineLinear(ast.points.data(), ast.points.size(), 1, LIME);
    
    for (size_t i = 1; i < ast.points.size(); i++) {
      Vector2 p = Vector2Add(ast.pos, Vector2Scale(ast.points[i], 10.0));      
	  DrawCircleV(field_pos_to_screen_pos(&screen, &ship, p), 2, LIME);    
    }
    
    draw_sight(sight);

    // --------------
    draw_info(&screen, &ship, &shots);

    EndDrawing();
    //----------------------------------------------------------------------------------
  }

  // De-Initialization
  //--------------------------------------------------------------------------------------
  CloseWindow(); // Close window and OpenGL context
  //--------------------------------------------------------------------------------------

  return 0;
}
