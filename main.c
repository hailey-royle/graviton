#include <raylib.h>
#include <raymath.h>
#include <stdlib.h>
#include <stdio.h>

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
int gravitonMoves = 0;

float timer = 0.0;
char charTimer[16];
char charGravitonMoves[16];

bool gameWon = false;
bool quitGame = false;

Vector2 gravitonPosition = (Vector2){0, 0};
Vector2 atomPosition = (Vector2){0, 0};
Vector2 atomSpeed = (Vector2){0, 0};
Vector2 atomForce = (Vector2){0, 0};

enum GameState {
    GAME_START,
    GAME_LEVELS,
    GAME_COSMETICS,
    GAME_SETTINGS,
    GAME_PLAY,
    GAME_END
};
enum GameState gameState = GAME_START;

struct LevelSelection {
    bool easy;
    bool medium;
    bool hard;
    bool hell;
    bool atempted;
    bool finished;
};
struct LevelSelection levelSelection;

struct AtomTraceSection {
    Vector2 start;
    Vector2 end;
    bool active;
};
struct AtomTraceSection atomTrace[FPS];

struct Level {
    Rectangle obstacles[16];
    char name[64];
    char designer[64];
    Rectangle finish;
    Vector2 gravitonStart;
    Vector2 atomStart;
    int difficulty;
};
struct Level level;

void InitGame() {
    InitWindow(WINDOW_WIDTH, WINDOW_HIGHT, "graviton");

    testingGraviton = LoadTexture("assets/TestingGraviton.png");
    testingAtom = LoadTexture("assets/TestingAtom.png");

    SetTargetFPS(FPS);
}

void InitLevel() {
    FILE *levelFile;
    levelFile = fopen("./levels/a", "r");
    char levelData[64];
    char levelDataTypes[12] = { "!@dofga0123" };
    int obstaclesNumber = 0;
    while (fgets(levelData, 64, levelFile)) {
        char istr[5] = {"0100"};
        if (levelData[0] == levelDataTypes[0]) {
            for (int i = 0; i < 62; i++) {
                level.name[i] = levelData[i + 1];
            }
        } else if (levelData[0] == levelDataTypes[1]) {
            for (int i = 0; i < 62; i++) {
                level.designer[i] = levelData[i + 1];
            }
        } else if (levelData[0] == levelDataTypes[2]) {
            if (levelData[1] == levelDataTypes[7]) {
                level.difficulty = 0;
            } else if (levelData[1] == levelDataTypes[8]) {
                level.difficulty = 1;
            } else if (levelData[2] == levelDataTypes[9]) {
                level.difficulty = 2;
            } else if (levelData[3] == levelDataTypes[10]) {
                level.difficulty = 3;
            }
        } else if (levelData[0] == levelDataTypes[3]) {
            for (int i = 0; i < 4; i++) {
                istr[i] = levelData[i + 1];
            }
            level.obstacles[obstaclesNumber].x = strtof(istr, NULL);
            for (int i = 0; i < 4; i++) {
                istr[i] = levelData[i + 5];
            }
            level.obstacles[obstaclesNumber].y = strtof(istr, NULL);
            for (int i = 0; i < 4; i++) {
                istr[i] = levelData[i + 9];
            }
            level.obstacles[obstaclesNumber].width = strtof(istr, NULL);
            for (int i = 0; i < 4; i++) {
                istr[i] = levelData[i + 13];
            }
            level.obstacles[obstaclesNumber].height = strtof(istr, NULL);
            obstaclesNumber++;
        } else if (levelData[0] == levelDataTypes[4]) {
            for (int i = 0; i < 4; i++) {
                istr[i] = levelData[i + 1];
            }
            level.finish.x = strtof(istr, NULL);
            for (int i = 0; i < 4; i++) {
                istr[i] = levelData[i + 5];
            }
            level.finish.y = strtof(istr, NULL);
            for (int i = 0; i < 4; i++) {
                istr[i] = levelData[i + 9];
            }
            level.finish.width = strtof(istr, NULL);
            for (int i = 0; i < 4; i++) {
                istr[i] = levelData[i + 13];
            }
            level.finish.height = strtof(istr, NULL);
        } else if (levelData[0] == levelDataTypes[5]) {
            for (int i = 0; i < 4; i++) {
                istr[i] = levelData[i + 1];
            }
            level.gravitonStart.x = strtof(istr, NULL);
            for (int i = 0; i < 4; i++) {
                istr[i] = levelData[i + 5];
            }
            level.gravitonStart.y = strtof(istr, NULL);
        } else if (levelData[0] == levelDataTypes[6]) {
            for (int i = 0; i < 4; i++) {
                istr[i] = levelData[i + 1];
            }
            level.atomStart.x = strtof(istr, NULL);
            for (int i = 0; i < 4; i++) {
                istr[i] = levelData[i + 5];
            }
            level.atomStart.y = strtof(istr, NULL);
        }

    }
    fclose(levelFile);

    gravitonPosition = level.gravitonStart;
    atomPosition = level.atomStart;
    atomSpeed = (Vector2){0, 0};
    atomForce = (Vector2){0, 0};
    timer = 0.0;
    gravitonMoves = 0;
    for (int i = 0; i < FPS; i++) {
         atomTrace[i].active = false;
    }
}

bool Button(const Rectangle rect, const char *text) {
    DrawRectangleRec(rect, GRAV_DGRAY);
    DrawText(text, rect.x + (rect.height / 8), rect.y + (rect.height / 8), rect.height - (rect.height / 4), GRAV_WHITE);
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(GetMousePosition(), rect)) {
        return true;
    } else {
        return false;
    }
}

void ToggleButton(const Rectangle rect, bool *toggle, const char *text) {
    if (Button(rect, text)) {
        if (*toggle == true) {
            *toggle = false;
        } else {
            *toggle = true;
        }
    }
    if (*toggle == true) {
        DrawText("[X]", rect.x + (rect.width - 48), rect.y + 8, (rect.height * 2) / 3, GRAV_WHITE);
    } else {
        DrawText("[ ]", rect.x + (rect.width - 48), rect.y + 8, (rect.height * 2) / 3, GRAV_WHITE);
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
    if (CheckCollisionPointRec(atomPosition, level.finish)) {
        gameState = GAME_END;
        gameWon = true;
    }
    for (int i = 0; i < 16; i++) {
        if (CheckCollisionPointRec(atomPosition, level.obstacles[i])) {
            gameState = GAME_END;
            gameWon = false;
        }
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
        UpdateAtom();
        timer += GetFrameTime();

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            gravitonPosition = GetMousePosition();
            gravitonMoves++;
        }
    }
}

void DrawLevel() {
    //map
    DrawRectangleLinesEx(level.finish, 4.0, GRAV_BLUE);
    for (int i = 0; i < 16; i++) {
        DrawRectangleLinesEx(level.obstacles[i], 4, GRAV_RED);
    }
    //atom trace
    for (int i = 0; i <= FPS; i++) {
         if (atomTrace[i].active == true) {
            DrawLineEx(atomTrace[i].start, atomTrace[i].end, ATOM_TRACE_THICK, GRAV_DGRAY);
         }
    }
    //atom
    DrawTextureV(testingAtom, Vector2SubtractValue(atomPosition, HALF_SPRITE_WH), GRAV_WHITE);
    //graviton
    DrawTextureV(testingGraviton, Vector2SubtractValue(gravitonPosition, HALF_SPRITE_WH), GRAV_WHITE);
}

void DrawTimer() {
    sprintf(charTimer, "%.2f", timer);
    DrawText(charTimer, WINDOW_WIDTH - 128, 32, 32, GRAV_WHITE);
}

void DrawMoves() {
    sprintf(charGravitonMoves, "%d", gravitonMoves);
    DrawText(charGravitonMoves, WINDOW_WIDTH - 128, 96, 32, GRAV_WHITE);
}

void DrawUi() {
    if (gameState == GAME_START) {
        DrawText("Graviton", 64, 64, 128, GRAV_WHITE);

        if (Button((Rectangle){WINDOW_WIDTH - 112, 16, 96, 48}, "Exit")) {
            quitGame = true;
        }
        if (Button((Rectangle){(WINDOW_WIDTH / 2) - 192, (WINDOW_HIGHT / 2) - 64, 384, 128}, "Start")) {
            InitLevel();
            gameState = GAME_PLAY;
        }
        if (Button((Rectangle){64, WINDOW_HIGHT / 2, 256, 64}, "Levels")) {
            gameState = GAME_LEVELS;
        }
        if (Button((Rectangle){64, (WINDOW_HIGHT / 2) + 128, 256, 64}, "Cosmetics")) {
            //cosmetics
        }
        if (Button((Rectangle){64, (WINDOW_HIGHT / 2) + 256, 256, 64}, "Settings")) {
            //settings
        }
    } else if (gameState == GAME_LEVELS) {
        if (Button((Rectangle){WINDOW_WIDTH - 112, 16, 96, 48}, "Exit")) {
            gameState = GAME_START;
        }
        DrawText("Filters", 72, 72, 48, GRAV_WHITE);
        ToggleButton((Rectangle){64, 128, 256, 48}, &levelSelection.easy, "Easy");
        ToggleButton((Rectangle){64, 192, 256, 48}, &levelSelection.medium, "Medium");
        ToggleButton((Rectangle){64, 256, 256, 48}, &levelSelection.hard, "Hard");
        ToggleButton((Rectangle){64, 320, 256, 48}, &levelSelection.hell, "Hell");
        ToggleButton((Rectangle){64, 384, 256, 48}, &levelSelection.atempted, "Atempted");
        ToggleButton((Rectangle){64, 448, 256, 48}, &levelSelection.finished, "Finished");
    } else if (gameState == GAME_PLAY) {
        DrawTimer();
        DrawMoves();
    } else if (gameState == GAME_END) {
        DrawTimer();
        DrawMoves();

        if (gameWon == true) {
            DrawText("You Won!", 64, 64, 128, GRAV_WHITE);
        }
        if (gameWon == false) {
            DrawText("You Lost!", 64, 64, 128, GRAV_WHITE);
        }

        if (Button((Rectangle){(WINDOW_WIDTH / 2) - 192, (WINDOW_HIGHT / 2) - 64, 384, 128}, "Home")) {
            gameState = GAME_START;
        }
    }
}

void Draw() {
    BeginDrawing();
        ClearBackground(GRAV_BLACK);
        DrawLevel();
        DrawUi();
    EndDrawing();
}

int main(void) {

    InitGame();

    InitLevel();

    while (!WindowShouldClose() && (quitGame == false)) {
        Update();
        Draw();
    }

    UnloadTexture(testingGraviton);
    UnloadTexture(testingAtom);

    CloseWindow();

    return 0;
}
