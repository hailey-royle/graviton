#include <raylib.h>
#include <raymath.h>
#include <string.h>

#define WINDOW_WIDTH 1920
#define WINDOW_HIGHT 1080
#define FPS 60
#define HALF_SPRITE_WH 32
#define GRAVITY 0.4
#define ATOM_TRACE_THICK 4

#define GRAV_BLACK (Color){15, 15, 15, 255}
#define GRAV_WHITE (Color){239, 239, 239, 255}
#define GRAV_DGRAY (Color){63, 63, 63, 255}
#define GRAV_RED (Color){127, 15, 15, 255}
#define GRAV_BLUE (Color){15, 15, 127, 255}

Texture2D testingGraviton;
Texture2D testingAtom;

int currentAtomTraceSection = 0;

Vector2 gravitonPosition = (Vector2){512, (512)};
Vector2 atomPosition = (Vector2){255, 255};
Vector2 atomSpeed = (Vector2){0, 0};
Vector2 atomForce = (Vector2){0, 0};

Rectangle finishBox = (Rectangle){1400, 800, 64, 64};
Rectangle obstacleBox = (Rectangle){(WINDOW_WIDTH / 2), 0, 64, WINDOW_HIGHT};
Rectangle startButtonRectangle = (Rectangle){((WINDOW_WIDTH / 2) - 128), ((WINDOW_HIGHT / 2) - 64), 256, 128};

enum GameState {
    GAME_START,
    GAME_PLAY,
    GAME_WON,
    GAME_LOST,
    NONE
};
enum GameState gameState = GAME_START;

struct Button {
    char text[16];
    Color rectColor;
    Color textColor;
    Vector2 position;
    Vector2 size;
    enum GameState activeState;
};

struct Button buttons[16];

void InitGame() {
    for (int i = 0; i < 16; i++) {
        strcpy(buttons[i].text, "test text");
        buttons[i].rectColor = GRAV_DGRAY;
        buttons[i].textColor = GRAV_WHITE;
        buttons[i].position = (Vector2){0, 0};
        buttons[i].size = (Vector2){128, 64};
        buttons[i].activeState = NONE;
    }
    buttons[0].activeState = GAME_START;
}

struct AtomTraceSection {
    Vector2 start;
    Vector2 end;
    bool active;
};

struct AtomTraceSection atomTrace[FPS];

void ResetGame() {
    gravitonPosition = (Vector2){512, (512)};
    atomPosition = (Vector2){255, 255};
    atomSpeed = (Vector2){0, 0};
    atomForce = (Vector2){0, 0};
    int i = 0;
    for (i = 0; i <= FPS; i += 1) {
         atomTrace[i].active = false;
    }
}

void StartButtonInput() {
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        if (CheckCollisionPointRec(GetMousePosition(), startButtonRectangle)) {
            ResetGame();
            gameState = GAME_PLAY;
        }
    }
}

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
        gameState = GAME_WON;
    }
    if (CheckCollisionPointRec(atomPosition, obstacleBox)) {
        gameState = GAME_LOST;
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
    if (gameState == GAME_PLAY) {
        UpdateGraviton();
        UpdateAtom();
    }
    if (gameState == GAME_LOST || gameState == GAME_WON) {
        StartButtonInput();
    }
    if (gameState == GAME_START) {
        StartButtonInput();
    }
}

void DrawStartButton() {
    DrawRectangleRec(startButtonRectangle, GRAV_DGRAY);
    DrawText("Start!", ((WINDOW_WIDTH / 2) - 96), ((WINDOW_HIGHT / 2) - 32), 64, GRAV_WHITE);
}

void DrawUi(enum GameState state) {
    int i = 0;
    for (i = 0; i < 16; i++) {
        if (buttons[i].activeState == state) {
            DrawRectangleV(buttons[i].position, buttons[i].size, buttons[i].rectColor);
            DrawText(buttons[i].text, (buttons[i].position.x + 8), (buttons[i].position.y + 8), (buttons[i].size.y - 16), buttons[i].textColor);
        }
    }
}


void DrawMap() {
    DrawRectangleLinesEx(finishBox, 4.0, GRAV_BLUE);
    DrawRectangleLinesEx(obstacleBox, 4.0, GRAV_RED);
}

void DrawAtomTrace() {
    int i = 0;
    for (i = 0; i <= FPS; i += 1) {
         if (atomTrace[i].active == true) {
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
        DrawUi(gameState);
        if (gameState == GAME_START) {
            ClearBackground(GRAV_BLACK);
            DrawText("Graviton", 64, 64, 64, GRAV_WHITE);
            DrawStartButton();
        }
        if (gameState == GAME_PLAY) {
            ClearBackground(GRAV_BLACK);
            DrawMap();
            DrawAtom();
            DrawGraviton();
        }
        if (gameState == GAME_WON) {
            ClearBackground(GRAV_BLACK);
            DrawText("You Won!", 512, 256, 64, GRAV_WHITE);
            DrawStartButton();
        }
        if (gameState == GAME_LOST) {
            ClearBackground(GRAV_BLACK);
            DrawText("You Lost!", 512, 256, 64, GRAV_WHITE);
            DrawStartButton();
        }
    EndDrawing();
}

int main(void) {
    InitWindow(WINDOW_WIDTH, WINDOW_HIGHT, "graviton");

    testingGraviton = LoadTexture("assets/TestingGraviton.png");
    testingAtom = LoadTexture("assets/TestingAtom.png");

    SetTargetFPS(FPS);

    InitGame();

    while (!WindowShouldClose()) {
        Update();
        Draw();
    }

    UnloadTexture(testingGraviton);
    UnloadTexture(testingAtom);

    CloseWindow();

    return 0;
}
