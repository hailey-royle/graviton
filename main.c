#include <raylib.h>
#include <raymath.h>

//----------------------------------------------------------------
//game data
//----------------------------------------------------------------

#define FPS 60
#define SPRITE_SIZE 64
#define GRAVITY 0.5
#define ATOM_TRACE_THICK 4

#define GRAV_BLACK (Color){15, 15, 15, 255}
#define GRAV_WHITE (Color){239, 239, 239, 255}
#define GRAV_DGRAY (Color){63, 63, 63, 255}
#define GRAV_RED (Color){127, 15, 15, 255}
#define GRAV_BLUE (Color){15, 15, 127, 255}

#define MAP_WIDTH 32
#define MAP_HEIGHT 18
#define MAP_ITEM_SIZE 60
#define MAP_ITEM_LINE_WIDTH 4.0

#define UI_SCALE 60
#define TEXT_PADDING_RELITIVE 8

#define DIFFICULTY_TYPES 4
#define PROGRESS_TYPES 3
#define MAX_STRING 32

#define EASY 0
#define MEDIUM 1
#define HARD 2
#define HELL 3
#define NOT_TRIED 0
#define ATTEMPTED 1
#define FINISHED 2

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
    bool difficulty[DIFFICULTY_TYPES];
    bool progress[PROGRESS_TYPES];
};
struct LevelFilters levelFilters;

struct AtomTraceSection {
    Vector2 start;
    Vector2 end;
    bool active;
};
struct AtomTraceSection atomTrace[FPS];

struct Level {
    int map[MAP_WIDTH * MAP_HEIGHT];
    char name[MAX_STRING];
    char designer[MAX_STRING];
    Vector2 gravitonStart;
    Vector2 atomStart;
    int difficulty;
    int progress;
};
struct Level level[16];

//----------------------------------------------------------------
//init functions
//----------------------------------------------------------------

void InitLevel(char *filePath, int levelNumber) {
    char levelDataHold[1024];
    char *levelData= levelDataHold;
    levelData= LoadFileText(filePath);
    int i = 0;
    int iMap = 0;
    int iString = 0;
    int lineNumber = 1;
    while (levelData[i] != '\0') {
        if (levelData[i] == '\n') {
            lineNumber++;
            i++;
            iString = 0;
        } else if (lineNumber == 1) {
            level[levelNumber].name[iString] = levelData[i];
            i++;
            iString++;
        } else if (lineNumber == 2) {
            level[levelNumber].designer[iString] = levelData[i];
            i++;
            iString++;
        } else if (lineNumber == 3) {
            if (levelData[i] == '1') {
                level[levelNumber].difficulty = EASY;
            } else if (levelData[i] == '2') {
                level[levelNumber].difficulty = MEDIUM;
            } else if (levelData[i] == '3') {
                level[levelNumber].difficulty = HARD;
            } else if (levelData[i] == '4') {
                level[levelNumber].difficulty = HELL;
            }
            i++;
        } else if (lineNumber == 4) {
            level[levelNumber].gravitonStart.x = TextToFloat(TextSubtext(levelData, i, 4));
            level[levelNumber].gravitonStart.y = TextToFloat(TextSubtext(levelData, i + 4, 4));
            i += 8;
        } else if (lineNumber == 5) {
            level[levelNumber].atomStart.x = TextToFloat(TextSubtext(levelData, i, 4));
            level[levelNumber].atomStart.y = TextToFloat(TextSubtext(levelData, i + 4, 4));
            i += 8;
        } else if (lineNumber >= 6) {
            level[levelNumber].map[iMap] = levelData[i];
            i++;
            iMap++;
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

    for (int i = 0; i <= DIFFICULTY_TYPES; i++) {
        levelFilters.difficulty[i] = true;
    }
    for (int i = 0; i <= PROGRESS_TYPES; i++) {
        levelFilters.progress[i] = true;
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
    for (int i = 0; i <= FPS; i++) {
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
    for (int i = 0; i < levelFilePathList.count; i++) {
        selectedLevel++;
        selectedLevel = selectedLevel % levelFilePathList.count;
        if (levelFilters.difficulty[level[selectedLevel].difficulty] && levelFilters.progress[level[selectedLevel].progress]) {
            break;
        }
    }
    ResetLevel();
}

void DecreaseSelectedLevel() {
    for (int i = 0; i < levelFilePathList.count; i++) {
        if (selectedLevel == 0) {
            selectedLevel = levelFilePathList.count - 1;
        } else {
            selectedLevel--;
            selectedLevel = selectedLevel % levelFilePathList.count;
        }
        if (levelFilters.difficulty[level[selectedLevel].difficulty] && levelFilters.progress[level[selectedLevel].progress]) {
            break;
        }
    }
    ResetLevel();
}

//----------------------------------------------------------------
//updating the game functions
//----------------------------------------------------------------

void UpdateAtomTrace() {
    atomTrace[currentAtomTraceSection].start = atomPosition;
    atomTrace[currentAtomTraceSection].end = Vector2Add(Vector2Add(atomPosition, atomSpeed), atomForce);
    atomTrace[currentAtomTraceSection].active = true;
    if (currentAtomTraceSection >= FPS) {
        currentAtomTraceSection = 0;
    } else {
        currentAtomTraceSection++;
    }
}

void AtomCollision() {
    for (int i = 0; i <= MAP_WIDTH * MAP_HEIGHT; i++) {
        int mapItemX = (i % MAP_WIDTH) * MAP_ITEM_SIZE;
        int mapItemY = (i / MAP_WIDTH) * MAP_ITEM_SIZE;
        if (level[selectedLevel].map[i] == KEY_ONE) {
            if (CheckCollisionPointRec(atomPosition, (Rectangle){mapItemX, mapItemY, MAP_ITEM_SIZE, MAP_ITEM_SIZE})) {
                gameState = GAME_END;
                gameWon = false;
            }
        } else if (level[selectedLevel].map[i] == KEY_TWO) {
            if (CheckCollisionPointRec(atomPosition, (Rectangle){mapItemX, mapItemY, MAP_ITEM_SIZE, MAP_ITEM_SIZE})) {
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
        for (int i = 0; i <= MAP_WIDTH * MAP_HEIGHT; i++) {
            int mapItemX = (i % MAP_WIDTH) * MAP_ITEM_SIZE;
            int mapItemY = (i / MAP_WIDTH) * MAP_ITEM_SIZE;
            if (level[selectedLevel].map[i] == KEY_ONE) {
                DrawRectangleLinesEx((Rectangle){mapItemX, mapItemY, MAP_ITEM_SIZE, MAP_ITEM_SIZE}, MAP_ITEM_LINE_WIDTH, GRAV_RED);
            } else if (level[selectedLevel].map[i] == KEY_TWO) {
                DrawRectangleLinesEx((Rectangle){mapItemX, mapItemY, MAP_ITEM_SIZE, MAP_ITEM_SIZE}, MAP_ITEM_LINE_WIDTH, GRAV_BLUE);
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
        for (int i = 0; i <= MAP_WIDTH * MAP_HEIGHT; i++) {
            if (level[selectedLevel].map[i] == KEY_ONE) {
                DrawRectangleLinesEx((Rectangle){((i % MAP_WIDTH) * 40) + 320, ((i / MAP_WIDTH) * 40) + 180, 40, 40}, 4.0, GRAV_RED);
            } else if (level[selectedLevel].map[i] == KEY_TWO) {
                DrawRectangleLinesEx((Rectangle){((i % MAP_WIDTH) * 40) + 320, ((i / MAP_WIDTH) * 40) + 180, 40, 40}, 4.0, GRAV_BLUE);
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

bool Button(const Rectangle rect, const char *text, int key) {
    DrawRectangleRec(rect, GRAV_DGRAY);
    int textPadding = rect.height / TEXT_PADDING_RELITIVE;
    int fontSize = rect.height - (textPadding * 2);
    DrawText(text, rect.x + textPadding, rect.y + textPadding, fontSize, GRAV_WHITE);
    if (IsKeyPressed(key) || (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(GetMousePosition(), rect))) {
        return true;
    } else {
        return false;
    }
}

void LevelToggleButton(const Rectangle rect, bool *toggle, const char *text, int key) {
    if (*toggle == true) {
        text = TextFormat("[X]%s", text);
    } else if (*toggle == false) {
        text = TextFormat("[ ]%s", text);
    }
    if (Button(rect, text, key)) {
        ToggleBool(toggle);
        DecreaseSelectedLevel();
        IncreaseSelectedLevel();
    }
}

void DrawUi() {
    if (gameState == GAME_START) {
        DrawLevel(true);
        DrawText("Graviton", 66, 66, 108, GRAV_WHITE);
        if (Button((Rectangle){30 * UI_SCALE, 12, 108, 48}, "Exit", KEY_ESCAPE)) {
            quitGame = true;
        }
        if (Button((Rectangle){13 * UI_SCALE, 15 * UI_SCALE, 6 * UI_SCALE, 2 * UI_SCALE}, "Start", KEY_SPACE)) {
            gameState = GAME_PLAY;
        }
        if (Button((Rectangle){UI_SCALE, 3 * UI_SCALE, 4 * UI_SCALE, (UI_SCALE * 4) / 5}, "Levels", KEY_L)) {
            gameState = GAME_LEVELS;
        }
        if (Button((Rectangle){UI_SCALE, 4 * UI_SCALE, 4 * UI_SCALE, (UI_SCALE * 4) / 5}, "Cosmetics", KEY_C)) {
            gameState = GAME_COSMETICS;
        }
        if (Button((Rectangle){UI_SCALE, 5 * UI_SCALE, 4 * UI_SCALE, (UI_SCALE * 4) / 5}, "Settings", KEY_S)) {
            gameState = GAME_SETTINGS;
        }
        if (Button((Rectangle){UI_SCALE, 6 * UI_SCALE, 4 * UI_SCALE, (UI_SCALE * 4) / 5}, "Information", KEY_I)) {
            gameState = GAME_INFORMATION;
        }
        if (Button((Rectangle){UI_SCALE, 7 * UI_SCALE, 4 * UI_SCALE, (UI_SCALE * 4) / 5}, "Tutorial", KEY_T)) {
            gameState = GAME_TUTORIAL;
        }
    } else if (gameState == GAME_LEVELS) {
        DrawText(level[selectedLevel].name, 66, 66, 108, GRAV_WHITE);
        DrawText(level[selectedLevel].designer, 66, 18, 36, GRAV_WHITE);
        DrawLevel(false);
        if (Button((Rectangle){13 * UI_SCALE, 15 * UI_SCALE, 6 * UI_SCALE, 2 * UI_SCALE}, "Home", KEY_H)) {
            gameState = GAME_START;
        }
        if (Button((Rectangle){11 * UI_SCALE, 15 * UI_SCALE, UI_SCALE, UI_SCALE}, "<-", KEY_J)) {
            DecreaseSelectedLevel();
        }
        if (Button((Rectangle){20 * UI_SCALE, 15 * UI_SCALE, UI_SCALE, UI_SCALE}, "->", KEY_K)) {
            IncreaseSelectedLevel();
        }

        LevelToggleButton((Rectangle){60, 180, 240, 48}, &levelFilters.difficulty[EASY], "Easy", KEY_ONE);
        LevelToggleButton((Rectangle){60, 240, 240, 48}, &levelFilters.difficulty[MEDIUM], "Medium", KEY_TWO);
        LevelToggleButton((Rectangle){60, 300, 240, 48}, &levelFilters.difficulty[HARD], "Hard", KEY_THREE);
        LevelToggleButton((Rectangle){60, 360, 240, 48}, &levelFilters.difficulty[HELL], "Hell", KEY_FOUR);
        LevelToggleButton((Rectangle){60, 420, 240, 48}, &levelFilters.progress[NOT_TRIED], "Not Tried", KEY_FIVE);
        LevelToggleButton((Rectangle){60, 480, 240, 48}, &levelFilters.progress[ATTEMPTED], "Attempted", KEY_SIX);
        LevelToggleButton((Rectangle){60, 540, 240, 48}, &levelFilters.progress[FINISHED], "Finished", KEY_SEVEN);
    } else if (gameState == GAME_COSMETICS) {
        if (Button((Rectangle){13 * UI_SCALE, 15 * UI_SCALE, 6 * UI_SCALE, 2 * UI_SCALE}, "Home", KEY_H)) {
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
        DrawText("5 = filter not tried", 492, 432, 36, GRAV_WHITE);
        DrawText("6 = filter attempeted", 492, 492, 36, GRAV_WHITE);
        DrawText("7 = filter finished", 492, 552, 36, GRAV_WHITE);
        DrawText("j = select left", 492, 612, 36, GRAV_WHITE);
        DrawText("k = select right", 492, 672, 36, GRAV_WHITE);
        if (Button((Rectangle){780, 900, 360, 120}, "Home", KEY_H)) {
            gameState = GAME_START;
        }
    } else if (gameState == GAME_INFORMATION) {
        if (Button((Rectangle){13 * UI_SCALE, 15 * UI_SCALE, 6 * UI_SCALE, 2 * UI_SCALE}, "Home", KEY_H)) {
            gameState = GAME_START;
        }
    } else if (gameState == GAME_TUTORIAL) {
        if (Button((Rectangle){13 * UI_SCALE, 15 * UI_SCALE, 6 * UI_SCALE, 2 * UI_SCALE}, "Home", KEY_H)) {
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
            DrawText("You Won!", 66, 66, 108, GRAV_WHITE);
        }
        if (gameWon == false) {
            DrawText("You Lost!", 66, 66, 108, GRAV_WHITE);
        }
        if (Button((Rectangle){13 * UI_SCALE, 15 * UI_SCALE, 6 * UI_SCALE, 3 * UI_SCALE}, "Home", KEY_H)) {
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
