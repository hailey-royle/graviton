#include <raylib.h>

#define WINDOW_WIDTH 1920
#define WINDOW_HIGHT 1080
#define GRAVITON_WH 64

#define GRAV_BLACK (Color){15, 15, 15, 255}
#define GRAV_WHITE (Color){239, 239, 239, 255}

Texture2D _testingGraviton;

Vector2 gravitonPosition = (Vector2){255, 255};

void UpdateGraviton() {
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        gravitonPosition = GetMousePosition();
        gravitonPosition.x -= (GRAVITON_WH/2);
        gravitonPosition.y -= (GRAVITON_WH/2);
    }
}

void Update(){
    UpdateGraviton();
}

void DrawGraviton() {
    DrawTextureV(_testingGraviton, gravitonPosition, GRAV_WHITE);
}

void Draw() {
    BeginDrawing();
        ClearBackground(GRAV_BLACK);
        DrawGraviton();
    EndDrawing();
}

int main(void)
{
    InitWindow(WINDOW_WIDTH, WINDOW_HIGHT, "graviton");

    _testingGraviton = LoadTexture("assets/TestingGraviton.png");

    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        Update();
        Draw();
    }
    CloseWindow();

    return 0;
}
