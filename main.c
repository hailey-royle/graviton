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

struct LevelFilters {
    bool difficulty[4];
    bool progress[3];
};
struct LevelFilters levelFilters;

struct AtomTraceSection {
    Vector2 start;
    Vector2 end;
    bool active;
};
struct AtomTraceSection atomTrace[FPS];

struct Level {
    int map[576];
    char name[64];
    char designer[64];
    Vector2 gravitonStart;
    Vector2 atomStart;
    int difficulty;
    int progress;
};
struct Level level[16];

//----------------------------------------------------------------
//init functions
//----------------------------------------------------------------

void InitLevel(char *filePath, int funi) {
    char levelData[1024];
    char *levelDataPointer = levelData;
    levelDataPointer = LoadFileText(filePath);
    int i = 0;
    int j = 0;
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
                level[funi].difficulty = 0;
            } else if (levelDataPointer[i] == '2') {
                level[funi].difficulty = 1;
            } else if (levelDataPointer[i] == '3') {
                level[funi].difficulty = 2;
            } else if (levelDataPointer[i] == '4') {
                level[funi].difficulty = 3;
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
        } else if (lineNumber >= 6) {
            level[funi].map[j] = levelDataPointer[i];
            i++;
            j++;
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

    levelFilters.difficulty[0] = true;
    levelFilters.difficulty[1] = true;
    levelFilters.difficulty[2] = true;
    levelFilters.difficulty[3] = true;
    levelFilters.progress[0] = true;
    levelFilters.progress[1] = true;
    levelFilters.progress[2] = true;
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

void ToggleBool(bool *toggle) {
    if (*toggle == true) {
        *toggle = false;
    } else {
        *toggle = true;
    }
}

void IncreaseSelectedLevel() {
    do {
        selectedLevel++;
        selectedLevel = selectedLevel % levelFilePathList.count;
    } while (!levelFilters.difficulty[level[selectedLevel].difficulty] || !levelFilters.progress[level[selectedLevel].progress]);
    ResetLevel();
}

void DecreaseSelectedLevel() {
    do {
        selectedLevel--;
        selectedLevel = selectedLevel % levelFilePathList.count;
    } while (!levelFilters.difficulty[level[selectedLevel].difficulty] || !levelFilters.progress[level[selectedLevel].progress]);
    ResetLevel();
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
    for (int i = 0; i <= 576; i++) {
        if (level[selectedLevel].map[i] == 49) {
            if (CheckCollisionPointRec(atomPosition, (Rectangle){(i % 32) * 60, (i / 32) * 60, 60, 60})) {
                gameState = GAME_END;
                gameWon = false;
            }
        } else if (level[selectedLevel].map[i] == 50) {
            if (CheckCollisionPointRec(atomPosition, (Rectangle){(i % 32) * 60, (i / 32) * 60, 60, 60})) {
                gameState = GAME_END;
                gameWon = true;
            }
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
    if (gameState != GAME_PLAY) {
        if (IsKeyPressed(KEY_H)) {
            gameState = GAME_START;
            ResetLevel();
        } else if (IsKeyPressed(KEY_L)) {
            gameState = GAME_LEVELS;
            ResetLevel();
        } else if (IsKeyPressed(KEY_C)) {
            gameState = GAME_COSMETICS;
            ResetLevel();
        } else if (IsKeyPressed(KEY_S)) {
            gameState = GAME_SETTINGS;
            ResetLevel();
        } else if (IsKeyPressed(KEY_I)) {
            gameState = GAME_INFORMATION;
            ResetLevel();
        } else if (IsKeyPressed(KEY_T)) {
            gameState = GAME_TUTORIAL;
            ResetLevel();
        } else if (IsKeyPressed(KEY_SPACE)) {
            gameState = GAME_PLAY;
            ResetLevel();
        }
    }
    if (gameState == GAME_LEVELS) {
        if (IsKeyPressed(KEY_ONE)) {
            ToggleBool(&levelFilters.difficulty[0]);
        } else if (IsKeyPressed(KEY_TWO)) {
            ToggleBool(&levelFilters.difficulty[1]);
        } else if (IsKeyPressed(KEY_THREE)) {
            ToggleBool(&levelFilters.difficulty[2]);
        } else if (IsKeyPressed(KEY_FOUR)) {
            ToggleBool(&levelFilters.difficulty[3]);
        } else if (IsKeyPressed(KEY_FIVE)) {
            ToggleBool(&levelFilters.progress[0]);
        } else if (IsKeyPressed(KEY_SIX)) {
            ToggleBool(&levelFilters.progress[1]);
        } else if (IsKeyPressed(KEY_SEVEN)) {
            ToggleBool(&levelFilters.progress[2]);
        } else if (IsKeyPressed(KEY_J)) {
            DecreaseSelectedLevel();
        } else if (IsKeyPressed(KEY_K)) {
            IncreaseSelectedLevel();
        }
    } else if (gameState == GAME_PLAY) {
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
        for (int i = 0; i <= 576; i++) {
            if (level[selectedLevel].map[i] == 49) {
                DrawRectangleLinesEx((Rectangle){(i % 32) * 60, (i / 32) * 60, 60, 60}, 4.0, GRAV_RED);
            } else if (level[selectedLevel].map[i] == 50) {
                DrawRectangleLinesEx((Rectangle){(i % 32) * 60, (i / 32) * 60, 60, 60}, 4.0, GRAV_BLUE);
            }
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
        for (int i = 0; i <= 576; i++) {
            if (level[selectedLevel].map[i] == 49) {
                DrawRectangleLinesEx((Rectangle){((i % 32) * 40) + 320, ((i / 32) * 40) + 180, 40, 40}, 4.0, GRAV_RED);
            } else if (level[selectedLevel].map[i] == 50) {
                DrawRectangleLinesEx((Rectangle){((i % 32) * 40) + 320, ((i / 32) * 40) + 180, 40, 40}, 4.0, GRAV_BLUE);
            }
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
        ToggleBool(toggle);
    }
}

void DrawUi() {
    if (gameState == GAME_START) {
        DrawLevel(true);
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
            DecreaseSelectedLevel();
        }
        if (Button((Rectangle){1200, 900, 60, 60}, "->")) {
            IncreaseSelectedLevel();
        }
        ToggleButton((Rectangle){60, 180, 240, 48}, &levelFilters.difficulty[0], "Easy");
        ToggleButton((Rectangle){60, 240, 240, 48}, &levelFilters.difficulty[1], "Medium");
        ToggleButton((Rectangle){60, 300, 240, 48}, &levelFilters.difficulty[2], "Hard");
        ToggleButton((Rectangle){60, 360, 240, 48}, &levelFilters.difficulty[3], "Hell");
        ToggleButton((Rectangle){60, 420, 240, 48}, &levelFilters.progress[0], "Not Started");
        ToggleButton((Rectangle){60, 480, 240, 48}, &levelFilters.progress[1], "Started");
        ToggleButton((Rectangle){60, 540, 240, 48}, &levelFilters.progress[2], "Finished");
    } else if (gameState == GAME_COSMETICS) {
        if (Button((Rectangle){780, 900, 360, 120}, "Home")) {
            gameState = GAME_START;
        }
    } else if (gameState == GAME_SETTINGS) {
        DrawText("hotkeys", 60, 120, 48, GRAV_WHITE);
        DrawText("space = start level", 66, 192, 36, GRAV_WHITE);
        DrawText("h = home", 66, 252, 36, GRAV_WHITE);
        DrawText("l = levels", 66, 312, 36, GRAV_WHITE);
        DrawText("c = cosmetics", 66, 372, 36, GRAV_WHITE);
        DrawText("s = settings", 66, 432, 36, GRAV_WHITE);
        DrawText("i = information", 66, 492, 36, GRAV_WHITE);
        DrawText("t = tutorial", 66, 552, 36, GRAV_WHITE);
        DrawText("level menu hotkeys", 480, 120, 48, GRAV_WHITE);
        DrawText("1 = filter easy", 492, 192, 36, GRAV_WHITE);
        DrawText("2 = filter medium", 492, 252, 36, GRAV_WHITE);
        DrawText("3 = filter hard", 492, 312, 36, GRAV_WHITE);
        DrawText("4 = filter hell", 492, 372, 36, GRAV_WHITE);
        DrawText("5 = filter not started", 492, 432, 36, GRAV_WHITE);
        DrawText("6 = filter started", 492, 492, 36, GRAV_WHITE);
        DrawText("7 = filter finished", 492, 552, 36, GRAV_WHITE);
        DrawText("j = select left", 492, 612, 36, GRAV_WHITE);
        DrawText("k = select right", 492, 672, 36, GRAV_WHITE);
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
        DrawLevel(true);
        DrawText(TextFormat("%.2f", timer), 1792, 32, 32, GRAV_WHITE);
        DrawText(TextFormat("%d", gravitonMoves), 1792, 96, 32, GRAV_WHITE);
    } else if (gameState == GAME_END) {
        DrawLevel(true);
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
