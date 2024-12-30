#include "include/raylib.h"
#include <math.h>

typedef struct 
{
    float dir;
    Vector2 center;
    Vector2 deltaCenter;
} Ship;

int screenWidth = 1600;
int screenHeight = 900;

int fieldWidth = 2000;
int fieldHeight = 2000;

int net_gap = 100;

void draw_net(Ship ship) {
    // Net vertical    
    int startX = -fmod(ship.deltaCenter.x, net_gap);
    
    int finishX = screenWidth;
  
    for (int i = startX; i < finishX; i += net_gap) {
        int y1 = 0;
        int y2 = screenHeight;        
        DrawLine(i, y1, i, y2, LIME);
    }

    int startY = -fmod(ship.deltaCenter.y, net_gap);

    int finishY = screenHeight;

    // Net horizontal
    for (int i = startY; i < finishY; i += net_gap) {
        int x1 = 0;
        int x2 = screenWidth;

        DrawLine(x1, i, x2, i, LIME);
    }
    //

    // Border lines
    
    if (ship.deltaCenter.y < ship.center.y) {
        int y = ship.center.y - ship.deltaCenter.y;
	DrawLine(0, y, screenWidth, y, RED);
    }

    if (fieldHeight - ship.deltaCenter.y < ship.center.y) {
	int gap = ship.center.y - (fieldHeight - ship.deltaCenter.y);
        int y = screenHeight - gap;
	DrawLine(0, y, screenWidth, y, RED);
    }

    if (ship.deltaCenter.x < ship.center.x) {
	int x = ship.center.x - ship.deltaCenter.x;
	DrawLine(x, 0, x, screenHeight, RED);
    }

    if (fieldWidth - ship.deltaCenter.x < ship.center.x) {
	int gap = ship.center.x - (fieldWidth - ship.deltaCenter.x);
        int x = screenWidth - gap;
	DrawLine(x, 0, x, screenHeight, RED);
    }
}

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);

    InitWindow(screenWidth, screenHeight, "Asteroids");

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    Ship ship = {
        .dir = 0,
	.center = (Vector2) {screenWidth / 2.0, screenHeight / 2.0},
	.deltaCenter = (Vector2) {fieldWidth / 2.0, fieldHeight / 2.0},
    };

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        // TODO: Update your variables here
        //----------------------------------------------------------------------------------

        // screenWidth = GetScreenWidth();
        // screenHeight = GetScreenHeight();

        if (IsKeyPressed(KEY_F11)) {
            ToggleFullscreen();
            screenWidth = GetScreenWidth();
            screenHeight = GetScreenHeight();
        }

        if (IsKeyDown(KEY_LEFT)) {
            ship.dir = fmod(ship.dir + 0.1, 2 * PI);
        }

        if (IsKeyDown(KEY_RIGHT)) {
            ship.dir = fmod(ship.dir - 0.1, 2 * PI);
        }

        if (IsKeyDown(KEY_UP)) {
            ship.deltaCenter.x += cosf(ship.dir) * 5;
            ship.deltaCenter.y -= sinf(ship.dir) * 5;
        }

        if (IsKeyDown(KEY_DOWN)) {
            ship.deltaCenter.x -= cosf(ship.dir) * 5;
            ship.deltaCenter.y += sinf(ship.dir) * 5;
        }

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(DARKGRAY);

            draw_net(ship);

            // Ship
            Vector2 v1 = (Vector2){
                .x = ship.center.x + (cosf(ship.dir) * 15),
                .y = ship.center.y - (sinf(ship.dir) * 15),
            };

            float l = (3 * PI) / 4;
            Vector2 v2 = (Vector2){
                .x = ship.center.x + cos(ship.dir + l) * 15,
                .y = ship.center.y - sin(ship.dir + l) * 15,
            };

            float m = (5 * PI) / 4;
            Vector2 v3 = (Vector2){
                .x = ship.center.x + cos(ship.dir + m) * 15,
                .y = ship.center.y - sin(ship.dir + m) * 15,
            };

            DrawTriangleLines(v1, v2, v3, WHITE);
            // --------------

            // Ship position info
            char position_buf[14];
            sprintf(position_buf, "(%d; %d)", (int)ship.deltaCenter.x, (int)ship.deltaCenter.y);
            DrawText(position_buf, ship.center.x + 20, ship.center.y, 26, LIGHTGRAY);
            //
        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
