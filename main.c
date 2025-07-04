#include <raylib.h>
#include <raymath.h>

//----------------------------------------------------------------
//game data
//----------------------------------------------------------------

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

bool gameWon = false;
bool quitGame = false;

Vector2 gravitonPosition = (Vector2){-100, -100};
Vector2 atomPosition = (Vector2){-100, -100};
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

//----------------------------------------------------------------
//init functions
//----------------------------------------------------------------

void InitGame() {
    InitWindow(WINDOW_WIDTH, WINDOW_HIGHT, "graviton");

    testingGraviton = LoadTexture("assets/TestingGraviton.png");
    testingAtom = LoadTexture("assets/TestingAtom.png");

    SetTargetFPS(FPS);
}

void InitLevel() {
    char levelData[256];
    char *levelDataPointer = levelData;
    levelDataPointer = LoadFileText("levels/a");
    int i = 0;
    int lineNumber = 1;
    while (levelDataPointer[i] != '\0') {
        if (levelDataPointer[i] == '\n') {
            lineNumber++;
            i++;
        } else if (lineNumber == 1) {
            level.name[i] = levelDataPointer[i];
            i++;
        } else if (lineNumber == 2) {
            level.designer[i] = levelDataPointer[i];
            i++;
        } else if (lineNumber == 3) {
            if (levelDataPointer[i] == '1') {
                level.difficulty = 1;
            } else if (levelDataPointer[i] == '2') {
                level.difficulty = 2;
            } else if (levelDataPointer[i] == '3') {
                level.difficulty = 3;
            } else if (levelDataPointer[i] == '4') {
                level.difficulty = 4;
            }
            i++;
        } else if (lineNumber == 4) {
            level.gravitonStart.x = TextToFloat(TextSubtext(levelDataPointer, i, 4));
            level.gravitonStart.y = TextToFloat(TextSubtext(levelDataPointer, i + 4, 4));
            i += 8;
        } else if (lineNumber == 5) {
            level.atomStart.x = TextToFloat(TextSubtext(levelDataPointer, i, 4));
            level.atomStart.y = TextToFloat(TextSubtext(levelDataPointer, i + 4, 4));
            i += 8;
        } else if (lineNumber == 6) {
            level.finish.x = TextToFloat(TextSubtext(levelDataPointer, i, 4));
            level.finish.y = TextToFloat(TextSubtext(levelDataPointer, i + 4, 4));
            level.finish.width = TextToFloat(TextSubtext(levelDataPointer, i + 8, 4));
            level.finish.height = TextToFloat(TextSubtext(levelDataPointer, i + 12, 4));
            i += 16;
        } else if (lineNumber > 6) {
            level.obstacles[lineNumber - 7].x = TextToFloat(TextSubtext(levelDataPointer, i, 4));
            level.obstacles[lineNumber - 7].y = TextToFloat(TextSubtext(levelDataPointer, i + 4, 4));
            level.obstacles[lineNumber - 7].width = TextToFloat(TextSubtext(levelDataPointer, i + 8, 4));
            level.obstacles[lineNumber - 7].height = TextToFloat(TextSubtext(levelDataPointer, i + 12, 4));
            i += 16;
        }
    }
}

//----------------------------------------------------------------
//misc functions
//----------------------------------------------------------------

void ResetLevel() {
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

//----------------------------------------------------------------
//updating the game functions
//----------------------------------------------------------------

void UpdateAtomTrace() {
    atomTrace[currentAtomTraceSection].start = atomPosition;
    atomTrace[currentAtomTraceSection].end = Vector2Add(Vector2Add(atomPosition, atomSpeed), atomForce);
    atomTrace[currentAtomTraceSection].active = true;
    if (currentAtomTraceSection < FPS - 1) {
        currentAtomTraceSection++;
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

//----------------------------------------------------------------
//updating the screen functions
//----------------------------------------------------------------

void DrawLevel() {
    DrawRectangleLinesEx(level.finish, 4.0, GRAV_BLUE);
    for (int i = 0; i < 16; i++) {
        DrawRectangleLinesEx(level.obstacles[i], 4, GRAV_RED);
    }
    for (int i = 0; i <= FPS; i++) {
         if (atomTrace[i].active == true) {
            DrawLineEx(atomTrace[i].start, atomTrace[i].end, ATOM_TRACE_THICK, GRAV_DGRAY);
         }
    }
    DrawTextureV(testingAtom, Vector2SubtractValue(atomPosition, HALF_SPRITE_WH), GRAV_WHITE);
    DrawTextureV(testingGraviton, Vector2SubtractValue(gravitonPosition, HALF_SPRITE_WH), GRAV_WHITE);
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

void DrawUi() {
    if (gameState == GAME_START) {
        DrawText("Graviton", 64, 64, 128, GRAV_WHITE);
        if (Button((Rectangle){WINDOW_WIDTH - 112, 16, 96, 48}, "Exit")) {
            quitGame = true;
        }
        if (Button((Rectangle){(WINDOW_WIDTH / 2) - 192, (WINDOW_HIGHT / 2) - 64, 384, 128}, "Start")) {
            gameState = GAME_PLAY;
        }
        if (Button((Rectangle){64, WINDOW_HIGHT / 2, 256, 64}, "Levels")) {
            gameState = GAME_LEVELS;
        }
        if (Button((Rectangle){64, (WINDOW_HIGHT / 2) + 128, 256, 64}, "Cosmetics")) {
            gameState = GAME_COSMETICS;
        }
        if (Button((Rectangle){64, (WINDOW_HIGHT / 2) + 256, 256, 64}, "Settings")) {
            gameState = GAME_SETTINGS;
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
    } else if (gameState == GAME_COSMETICS) {
        if (Button((Rectangle){WINDOW_WIDTH - 112, 16, 96, 48}, "Exit")) {
            gameState = GAME_START;
        }
    } else if (gameState == GAME_SETTINGS) {
        if (Button((Rectangle){WINDOW_WIDTH - 112, 16, 96, 48}, "Exit")) {
            gameState = GAME_START;
        }
    } else if (gameState == GAME_PLAY) {
        DrawText(TextFormat("%.2f", timer), WINDOW_WIDTH - 128, 32, 32, GRAV_WHITE);
        DrawText(TextFormat("%d", gravitonMoves), WINDOW_WIDTH - 128, 96, 32, GRAV_WHITE);
    } else if (gameState == GAME_END) {
        DrawText(TextFormat("%.2f", timer), WINDOW_WIDTH - 128, 32, 32, GRAV_WHITE);
        DrawText(TextFormat("%d", gravitonMoves), WINDOW_WIDTH - 128, 96, 32, GRAV_WHITE);
        if (gameWon == true) {
            DrawText("You Won!", 64, 64, 128, GRAV_WHITE);
        }
        if (gameWon == false) {
            DrawText("You Lost!", 64, 64, 128, GRAV_WHITE);
        }
        if (Button((Rectangle){(WINDOW_WIDTH / 2) - 192, (WINDOW_HIGHT / 2) - 64, 384, 128}, "Home")) {
            gameState = GAME_START;
            ResetLevel();
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

//----------------------------------------------------------------
//main function
//----------------------------------------------------------------

int main(void) {

    InitGame();
    InitLevel();
    ResetLevel();

    while (!WindowShouldClose() && (quitGame == false)) {
        Update();
        Draw();
    }

    UnloadTexture(testingGraviton);
    UnloadTexture(testingAtom);

    CloseWindow();

    return 0;
}
