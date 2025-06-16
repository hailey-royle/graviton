#include <raylib.h>
#include <raymath.h>
#include <string.h>

#define WINDOW_WIDTH 1920
#define WINDOW_HIGHT 1080
#define FPS 60
#define HALF_SPRITE_WH 32
#define GRAVITY 0.4
#define ATOM_TRACE_THICK 4
#define BUTTONS_NUMBER 16

#define GRAV_BLACK (Color){15, 15, 15, 255}
#define GRAV_WHITE (Color){239, 239, 239, 255}
#define GRAV_DGRAY (Color){63, 63, 63, 255}
#define GRAV_LGRAY (Color){191, 191, 191, 255}
#define GRAV_RED (Color){127, 15, 15, 255}
#define GRAV_BLUE (Color){15, 15, 127, 255}

Texture2D testingGraviton;
Texture2D testingAtom;

int currentAtomTraceSection = 0;

bool gameWon = false;
bool quitGame = false;

Vector2 gravitonPosition = (Vector2){512, (512)};
Vector2 atomPosition = (Vector2){255, 255};
Vector2 atomSpeed = (Vector2){0, 0};
Vector2 atomForce = (Vector2){0, 0};

Rectangle finishBox = (Rectangle){1400, 800, 64, 64};
Rectangle obstacleBox = (Rectangle){(WINDOW_WIDTH / 2), 0, 64, WINDOW_HIGHT};

enum GameState {
    GAME_START,
    GAME_PLAY,
    GAME_END,
    NONE
};
enum GameState gameState = GAME_START;

struct Button {
    char text[16];
    Color rectColor;
    Color textColor;
    Rectangle rect;
    int textOffset;
    enum GameState activeState;
};

struct Button buttons[BUTTONS_NUMBER];

struct AtomTraceSection {
    Vector2 start;
    Vector2 end;
    bool active;
};

struct AtomTraceSection atomTrace[FPS];

void InitGame() {
    for (int i = 0; i < BUTTONS_NUMBER; i++) {
        strcpy(buttons[i].text, "test text");
        buttons[i].rectColor = GRAV_DGRAY;
        buttons[i].textColor = GRAV_WHITE;
        buttons[i].rect = (Rectangle){0, 0, 256, 64};
        buttons[i].textOffset = 8;
        buttons[i].activeState = NONE;
    }

    strcpy(buttons[0].text, "Start");
    buttons[0].rect = (Rectangle){(WINDOW_WIDTH / 2) - 128, (WINDOW_HIGHT / 2) - 64, 256, 128};
    buttons[0].textOffset = 32;
    buttons[0].activeState = GAME_START;

    strcpy(buttons[1].text, "Exit");
    buttons[1].textColor = GRAV_LGRAY;
    buttons[1].rect = (Rectangle){WINDOW_WIDTH - 80, 16, 64, 32};
    buttons[1].activeState = GAME_START;

    strcpy(buttons[2].text, "Levels");
    buttons[2].rect = (Rectangle){64, WINDOW_HIGHT / 2, 256, 64};
    buttons[2].activeState = GAME_START;

    strcpy(buttons[3].text, "Cosmetics");
    buttons[3].rect = (Rectangle){64, (WINDOW_HIGHT / 2) + 128, 256, 64};
    buttons[3].activeState = GAME_START;

    strcpy(buttons[4].text, "Settings");
    buttons[4].rect = (Rectangle){64, (WINDOW_HIGHT / 2) + 256, 256, 64};
    buttons[4].activeState = GAME_START;

    strcpy(buttons[5].text, "Home");
    buttons[5].rect = (Rectangle){(WINDOW_WIDTH / 2) - 128, (WINDOW_HIGHT / 2) - 64, 256, 128};
    buttons[5].textOffset = 32;
    buttons[5].activeState = GAME_END;
}

void ResetGame() {
    gravitonPosition = (Vector2){512, (512)};
    atomPosition = (Vector2){255, 255};
    atomSpeed = (Vector2){0, 0};
    atomForce = (Vector2){0, 0};
    for (int i = 0; i <= FPS; i += 1) {
         atomTrace[i].active = false;
    }
}

void MouseLogic() {
    if (gameState == GAME_PLAY) {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            gravitonPosition = GetMousePosition();
        }
    }
    if (gameState == GAME_START) {
        if (CheckCollisionPointRec(GetMousePosition(), buttons[0].rect)) {
            gameState = GAME_PLAY;
        }
        if (CheckCollisionPointRec(GetMousePosition(), buttons[1].rect)) {
            quitGame = true;
        }
        if (CheckCollisionPointRec(GetMousePosition(), buttons[2].rect)) {
//levels menu
        }
        if (CheckCollisionPointRec(GetMousePosition(), buttons[3].rect)) {
//cosmetics menu
        }
        if (CheckCollisionPointRec(GetMousePosition(), buttons[4].rect)) {
//settings menu
        }
    }
    if (gameState == GAME_END) {
        if (CheckCollisionPointRec(GetMousePosition(), buttons[5].rect)) {
            gameState = GAME_START;
            ResetGame();
        }
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
        gameWon = true;
    }
    if (CheckCollisionPointRec(atomPosition, obstacleBox)) {
        gameState = GAME_END;
        gameWon = false;
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
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        MouseLogic();
    }
    if (gameState == GAME_PLAY) {
        UpdateAtom();
    }
    if (gameState == GAME_END) {
    }
    if (gameState == GAME_START) {
    }
}

void DrawUi(enum GameState state) {
    for (int i = 0; i < BUTTONS_NUMBER; i++) {
        if (buttons[i].activeState == state) {
            DrawRectangleRec(buttons[i].rect, buttons[i].rectColor);
            DrawText(buttons[i].text, (buttons[i].rect.x + buttons[i].textOffset), (buttons[i].rect.y + buttons[i].textOffset), (buttons[i].rect.height - (2 * buttons[i].textOffset)), buttons[i].textColor);
        }
    }
}

void DrawMap() {
    DrawRectangleLinesEx(finishBox, 4.0, GRAV_BLUE);
    DrawRectangleLinesEx(obstacleBox, 4.0, GRAV_RED);
}

void DrawAtomTrace() {
    for (int i = 0; i <= FPS; i += 1) {
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
        }
        if (gameState == GAME_PLAY) {
            ClearBackground(GRAV_BLACK);
            DrawMap();
            DrawAtom();
            DrawGraviton();
        }
        if (gameState == GAME_END) {
            ClearBackground(GRAV_BLACK);
            if (gameWon == true) {
                DrawText("You Won!", 512, 256, 64, GRAV_WHITE);
            }
            if (gameWon == false) {
                DrawText("You Lost!", 512, 256, 64, GRAV_WHITE);
            }
        }
    EndDrawing();
}

int main(void) {
    InitWindow(WINDOW_WIDTH, WINDOW_HIGHT, "graviton");

    testingGraviton = LoadTexture("assets/TestingGraviton.png");
    testingAtom = LoadTexture("assets/TestingAtom.png");

    SetTargetFPS(FPS);

    InitGame();

    while (!WindowShouldClose() && (quitGame == false)) {
        Update();
        Draw();
    }

    UnloadTexture(testingGraviton);
    UnloadTexture(testingAtom);

    CloseWindow();

    return 0;
}
