#include <raylib.h>
#include <raymath.h>

//----------------------------------------------------------------
//game data
//----------------------------------------------------------------

#define FPS 60
#define SPRITE_SIZE 64
#define GRAVITY 0.5
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

FilePathList levelFilePathList;

int selectedLevel = 0;
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
    GAME_INFORMATION,
    GAME_TUTORIAL,
    GAME_PLAY,
    GAME_END
};
enum GameState gameState = GAME_START;

struct LevelSelection {
    bool easy;
    bool medium;
    bool hard;
    bool hell;
    bool attempted;
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
struct Level level[16];

//----------------------------------------------------------------
//init functions
//----------------------------------------------------------------

void InitLevel(char *filePath, int funi) {
    char levelData[256];
    char *levelDataPointer = levelData;
    levelDataPointer = LoadFileText(filePath);
    int i = 0;
    int lineNumber = 1;
    while (levelDataPointer[i] != '\0') {
        if (levelDataPointer[i] == '\n') {
            lineNumber++;
            i++;
        } else if (lineNumber == 1) {
            level[funi].name[i] = levelDataPointer[i];
            i++;
        } else if (lineNumber == 2) {
            level[funi].designer[i] = levelDataPointer[i];
            i++;
        } else if (lineNumber == 3) {
            if (levelDataPointer[i] == '1') {
                level[funi].difficulty = 1;
            } else if (levelDataPointer[i] == '2') {
                level[funi].difficulty = 2;
            } else if (levelDataPointer[i] == '3') {
                level[funi].difficulty = 3;
            } else if (levelDataPointer[i] == '4') {
                level[funi].difficulty = 4;
            }
            i++;
        } else if (lineNumber == 4) {
            level[funi].gravitonStart.x = TextToFloat(TextSubtext(levelDataPointer, i, 4));
            level[funi].gravitonStart.y = TextToFloat(TextSubtext(levelDataPointer, i + 4, 4));
            i += 8;
        } else if (lineNumber == 5) {
            level[funi].atomStart.x = TextToFloat(TextSubtext(levelDataPointer, i, 4));
            level[funi].atomStart.y = TextToFloat(TextSubtext(levelDataPointer, i + 4, 4));
            i += 8;
        } else if (lineNumber == 6) {
            level[funi].finish.x = TextToFloat(TextSubtext(levelDataPointer, i, 4));
            level[funi].finish.y = TextToFloat(TextSubtext(levelDataPointer, i + 4, 4));
            level[funi].finish.width = TextToFloat(TextSubtext(levelDataPointer, i + 8, 4));
            level[funi].finish.height = TextToFloat(TextSubtext(levelDataPointer, i + 12, 4));
            i += 16;
        } else if (lineNumber > 6) {
            level[funi].obstacles[lineNumber - 7].x = TextToFloat(TextSubtext(levelDataPointer, i, 4));
            level[funi].obstacles[lineNumber - 7].y = TextToFloat(TextSubtext(levelDataPointer, i + 4, 4));
            level[funi].obstacles[lineNumber - 7].width = TextToFloat(TextSubtext(levelDataPointer, i + 8, 4));
            level[funi].obstacles[lineNumber - 7].height = TextToFloat(TextSubtext(levelDataPointer, i + 12, 4));
            i += 16;
        }
    }
}

void InitGame() {
    InitWindow(0, 0, "graviton");

    testingGraviton = LoadTexture("assets/TestingGraviton.png");
    testingAtom = LoadTexture("assets/TestingAtom.png");

    SetTargetFPS(FPS);

    levelFilePathList = LoadDirectoryFiles("levels");
    for (int i = 0; i < levelFilePathList.count; i++) {
        InitLevel(levelFilePathList.paths[i], i);
    }
}

//----------------------------------------------------------------
//misc functions
//----------------------------------------------------------------

void ResetLevel() {
    gravitonPosition = level[selectedLevel].gravitonStart;
    atomPosition = level[selectedLevel].atomStart;
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
    if (CheckCollisionPointRec(atomPosition, level[selectedLevel].finish)) {
        gameState = GAME_END;
        gameWon = true;
    }
    for (int i = 0; i < 16; i++) {
        if (CheckCollisionPointRec(atomPosition, level[selectedLevel].obstacles[i])) {
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

void DrawLevel(bool fullscreen) {
    if (fullscreen == true) { 
        DrawRectangleLinesEx(level[selectedLevel].finish, 4.0, GRAV_BLUE);
        for (int i = 0; i < 16; i++) {
            DrawRectangleLinesEx(level[selectedLevel].obstacles[i], 4.0, GRAV_RED);
        }
        for (int i = 0; i <= FPS; i++) {
            if (atomTrace[i].active == true) {
                DrawLineEx(atomTrace[i].start, atomTrace[i].end, ATOM_TRACE_THICK, GRAV_DGRAY);
            }
        }
        DrawTexturePro(testingAtom, (Rectangle){0, 0, 63, 63}, (Rectangle){atomPosition.x - 32, atomPosition.y - 32, 64, 64}, (Vector2){0, 0}, 0.0, WHITE);
        DrawTexturePro(testingGraviton, (Rectangle){0, 0, 63, 63}, (Rectangle){gravitonPosition.x - 32, gravitonPosition.y - 32, 64, 64}, (Vector2){0, 0}, 0.0, WHITE);
    } else {
        DrawRectangle(320, 180, 1280, 720, GRAV_BLACK);
        DrawRectangleLinesEx((Rectangle){316, 176, 1288, 728}, 4.0, GRAV_DGRAY);
        DrawRectangleLinesEx((Rectangle){
                ((level[selectedLevel].finish.x * 2) / 3) + 320, 
                ((level[selectedLevel].finish.y * 2) / 3) + 180, 
                ((level[selectedLevel].finish.width * 2) / 3), 
                ((level[selectedLevel].finish.height * 2) / 3)
                }, 2.0, GRAV_BLUE);
        for (int i = 0; i < 16; i++) {
            DrawRectangleLinesEx((Rectangle){
                ((level[selectedLevel].obstacles[i].x * 2) / 3) + 320,
                ((level[selectedLevel].obstacles[i].y * 2) / 3) + 180,
                ((level[selectedLevel].obstacles[i].width * 2) / 3),
                ((level[selectedLevel].obstacles[i].height * 2) / 3)
                }, 2.0, GRAV_RED);
        }
        DrawTexturePro(testingAtom, (Rectangle){0, 0, 63, 63},
            (Rectangle){(((atomPosition.x - 16) * 2) / 3) + 320, (((atomPosition.y - 16) * 2) / 3) + 180, 32, 32},
            (Vector2){0, 0}, 0.0, WHITE);
        DrawTexturePro(testingGraviton, (Rectangle){0, 0, 63, 63},
            (Rectangle){(((gravitonPosition.x - 16) * 2) / 3) + 320, (((gravitonPosition.y - 16) * 2) / 3) + 180, 32, 32},
            (Vector2){0, 0}, 0.0, WHITE);
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
    if (*toggle == true) {
        text = TextFormat("[X]%s", text);
    } else if (*toggle == false) {
        text = TextFormat("[ ]%s", text);
    }
    if (Button(rect, text)) {
        if (*toggle == true) {
            *toggle = false;
        } else {
            *toggle = true;
        }
    }
}

void DrawUi() {
    if (gameState == GAME_START) {
        DrawText("Graviton", 60, 60, 120, GRAV_WHITE);
        if (Button((Rectangle){1800, 12, 108, 48}, "Exit")) {
            quitGame = true;
        }
        if (Button((Rectangle){780, 900, 360, 120}, "Start")) {
            gameState = GAME_PLAY;
        }
        if (Button((Rectangle){60, 180, 240, 48}, "Levels")) {
            gameState = GAME_LEVELS;
        }
        if (Button((Rectangle){60, 240, 240, 48}, "Cosmetics")) {
            gameState = GAME_COSMETICS;
        }
        if (Button((Rectangle){60, 300, 240, 48}, "Settings")) {
            gameState = GAME_SETTINGS;
        }
        if (Button((Rectangle){60, 360, 240, 48}, "Information")) {
            gameState = GAME_INFORMATION;
        }
        if (Button((Rectangle){60, 420, 240, 48}, "Tutorial")) {
            gameState = GAME_TUTORIAL;
        }
    } else if (gameState == GAME_LEVELS) {
        DrawLevel(false);
        if (Button((Rectangle){780, 900, 360, 120}, "Home")) {
            gameState = GAME_START;
        }
        if (Button((Rectangle){660, 900, 60, 60}, "<-")) {
            if (selectedLevel > 0) {
                selectedLevel--;
                ResetLevel();
            }
        }
        if (Button((Rectangle){1200, 900, 60, 60}, "->")) {
            if (selectedLevel < levelFilePathList.count - 1) {
                selectedLevel++;
                ResetLevel();
            }
        }
        ToggleButton((Rectangle){60, 180, 240, 48}, &levelSelection.easy, "Easy");
        ToggleButton((Rectangle){60, 240, 240, 48}, &levelSelection.medium, "Medium");
        ToggleButton((Rectangle){60, 300, 240, 48}, &levelSelection.hard, "Hard");
        ToggleButton((Rectangle){60, 360, 240, 48}, &levelSelection.hell, "Hell");
        ToggleButton((Rectangle){60, 420, 240, 48}, &levelSelection.attempted, "Attempted");
        ToggleButton((Rectangle){60, 480, 240, 48}, &levelSelection.finished, "Finished");
    } else if (gameState == GAME_COSMETICS) {
        if (Button((Rectangle){780, 900, 360, 120}, "Home")) {
            gameState = GAME_START;
        }
    } else if (gameState == GAME_SETTINGS) {
        if (Button((Rectangle){780, 900, 360, 120}, "Home")) {
            gameState = GAME_START;
        }
    } else if (gameState == GAME_INFORMATION) {
        if (Button((Rectangle){780, 900, 360, 120}, "Home")) {
            gameState = GAME_START;
        }
    } else if (gameState == GAME_TUTORIAL) {
        if (Button((Rectangle){780, 900, 360, 120}, "Home")) {
            gameState = GAME_START;
        }
    } else if (gameState == GAME_PLAY) {
        DrawText(TextFormat("%.2f", timer), 1792, 32, 32, GRAV_WHITE);
        DrawText(TextFormat("%d", gravitonMoves), 1792, 96, 32, GRAV_WHITE);
    } else if (gameState == GAME_END) {
        DrawText(TextFormat("%.2f", timer), 1792, 32, 32, GRAV_WHITE);
        DrawText(TextFormat("%d", gravitonMoves), 1792, 96, 32, GRAV_WHITE);
        if (gameWon == true) {
            DrawText("You Won!", 60, 60, 120, GRAV_WHITE);
        }
        if (gameWon == false) {
            DrawText("You Lost!", 60, 60, 120, GRAV_WHITE);
        }
        if (Button((Rectangle){780, 900, 360, 120}, "Home")) {
            gameState = GAME_START;
            ResetLevel();
        }
    }
}

void Draw() {
    BeginDrawing();
        ClearBackground(GRAV_BLACK);
        DrawLevel(true);
        DrawUi();
    EndDrawing();
}

//----------------------------------------------------------------
//main function
//----------------------------------------------------------------

int main(void) {

    InitGame();
    ResetLevel();

    while (!WindowShouldClose() && (quitGame == false)) {
        Update();
        Draw();
    }

    UnloadTexture(testingGraviton);
    UnloadTexture(testingAtom);
    UnloadDirectoryFiles(levelFilePathList);

    CloseWindow();

    return 0;
}
