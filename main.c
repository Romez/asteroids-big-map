#include "raylib.h"
#include <math.h>

typedef struct 
{
    float dir;
    Vector2 center;
    Vector2 deltaCenter;
} Ship;

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    int screenWidth = 1600;
    int screenHeight = 900;

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);

    InitWindow(screenWidth, screenHeight, "Asteroids");

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    int fieldWidth = 10000;
    int fieldHeight = 10000;

    Ship ship = {
        .dir = 0,
        .center = (Vector2) {screenWidth / 2, screenHeight / 2},
        .deltaCenter = (Vector2) {0, 0},
    };

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        // TODO: Update your variables here
        //----------------------------------------------------------------------------------

        screenWidth = GetScreenWidth();
        screenHeight = GetScreenHeight();

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

            // Net
            for (int i = -fmod(ship.deltaCenter.x, 100); i < screenWidth; i += 100) {
                DrawLine(i, 0, i, screenHeight, LIME);
            }

            for (int i = -fmod(ship.deltaCenter.y, 100); i < screenHeight; i += 100) {
                DrawLine(0, i, screenWidth, i, LIME);
            }
            //

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