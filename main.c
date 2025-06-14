#include <raylib.h>
#include <raymath.h>

#define WINDOW_WIDTH 1920
#define WINDOW_HIGHT 1080
#define FPS 60
#define HALF_SPRITE_WH 32
#define GRAVITY 0.4
#define ATOM_TRACE_THICK 4

#define GRAV_BLACK (Color){15, 15, 15, 255}
#define GRAV_WHITE (Color){239, 239, 239, 255}
#define GRAV_DGRAY (Color){63, 63, 63, 255}

Texture2D testingGraviton;
Texture2D testingAtom;

int currentAtomTraceSection = 0;

Vector2 gravitonPosition = (Vector2){(WINDOW_WIDTH / 2), (WINDOW_HIGHT / 2)};
Vector2 atomPosition = (Vector2){255, 255};
Vector2 atomSpeed = (Vector2){0, 0};
Vector2 atomForce = (Vector2){0, 0};

Rectangle finishBox = (Rectangle){200, 800, 64, 64};

enum GameState {
    GAME_PLAY,
    GAME_END
};
enum GameState gameState = GAME_PLAY;


struct AtomTraceSection {
    Vector2 start;
    Vector2 end;
    bool active;
};

struct AtomTraceSection atomTrace[FPS];

void UpdateGraviton() {
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        gravitonPosition = GetMousePosition();
    }
}

void UpdateAtomTrace() {
    atomTrace[currentAtomTraceSection].start = atomPosition;
    atomTrace[currentAtomTraceSection].end = Vector2Add(Vector2Add(atomPosition, atomSpeed), atomForce);
    atomTrace[currentAtomTraceSection].active = true;
    if (currentAtomTraceSection < FPS) {
        ++currentAtomTraceSection;
    } else {
        currentAtomTraceSection = 0;
    }
}

void AtomCollision() {
    if (CheckCollisionPointRec(atomPosition, finishBox)) {
        gameState = GAME_END;
    }
}

void UpdateAtom() {
    atomForce = Vector2Scale(Vector2Subtract(gravitonPosition, atomPosition), (GRAVITY / Vector2Distance(atomPosition, gravitonPosition)));
    atomSpeed = Vector2Add(atomSpeed, atomForce);
    atomPosition = Vector2Add(Vector2Add(atomPosition, atomSpeed), atomForce);
    UpdateAtomTrace();
    AtomCollision();
}

void Update() {
    UpdateGraviton();
    UpdateAtom();
}

void DrawMap() {
    DrawRectangleLinesEx(finishBox, 4.0, RED);
}

void DrawAtomTrace() {
    int i = 0;
    for (i = 0; i <= FPS; i += 1) {
         if ((atomTrace[i].active = true)) {
            DrawLineEx(atomTrace[i].start, atomTrace[i].end, ATOM_TRACE_THICK, GRAV_DGRAY);
         }
    }
}

void DrawAtom() {
    DrawAtomTrace();
    DrawTextureV(testingAtom, Vector2SubtractValue(atomPosition, HALF_SPRITE_WH), GRAV_WHITE);
}

void DrawGraviton() {
    DrawTextureV(testingGraviton, Vector2SubtractValue(gravitonPosition, HALF_SPRITE_WH), GRAV_WHITE);
}

void Draw() {
    BeginDrawing();
        if ((gameState == GAME_PLAY)) {
            ClearBackground(GRAV_BLACK);
            DrawMap();
            DrawAtom();
            DrawGraviton();
        }
        if ((gameState == GAME_END)) {
            ClearBackground(GRAV_BLACK);
            DrawText("You Win", 512, 256, 64, GRAV_WHITE);
        }
    EndDrawing();
}

int main(void) {
    InitWindow(WINDOW_WIDTH, WINDOW_HIGHT, "graviton");

    testingGraviton = LoadTexture("assets/TestingGraviton.png");
    testingAtom = LoadTexture("assets/TestingAtom.png");

    SetTargetFPS(FPS);

    while (!WindowShouldClose()) {
        Update();
        Draw();
    }

    UnloadTexture(testingGraviton);
    UnloadTexture(testingAtom);

    CloseWindow();

    return 0;
}
