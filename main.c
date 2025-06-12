#include <raylib.h>

#define WINDOW_WIDTH 1920
#define WINDOW_HIGHT 1080

#define GRAV_BLACK (Color){15, 15, 15, 255}

int main(void)
{
    InitWindow(WINDOW_WIDTH, WINDOW_HIGHT, "graviton");

    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        BeginDrawing();
            ClearBackground(GRAV_BLACK);
        EndDrawing();
    }
    CloseWindow();

    return 0;
}
