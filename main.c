#include <raylib.h>
#include <raymath.h>

//----------------------------------------------------------------
//game data
//----------------------------------------------------------------

#define FPS 60
#define SPRITE_SIZE 60
#define GRAVITY 0.5
#define ATOM_TRACE_THICK 4

#define GRAV_BLACK (Color){15, 15, 15, 255}
#define GRAV_WHITE (Color){239, 239, 239, 255}
#define GRAV_DGRAY (Color){63, 63, 63, 255}
#define GRAV_RED (Color){127, 15, 15, 255}
#define GRAV_BLUE (Color){15, 15, 127, 255}

#define MAX_LEVELS 16
#define LEVEL_SIZE 648
#define MAX_STRING 32
#define MAP_WIDTH 32
#define MAP_HEIGHT 18
#define MAP_ITEM_SIZE 60
#define MAP_ITEM_LINE 4.0
#define DIFFICULTY_TYPES 4
#define PROGRESS_TYPES 3
#define EASY 0
#define MEDIUM 1
#define HARD 2
#define HELL 3
#define NOT_TRIED 0
#define ATTEMPTED 1
#define FINISHED 2

#define UI_SCALE 60
#define TEXT_PADDING_RELITIVE 8
#define LEVEL_BORDER 4


Texture2D testingGraviton;
Texture2D testingAtom;

FilePathList levelFilePathList;

int selectedLevel = 0;
int currentAtomTraceSection = 0;
int gravitonMoves = 0;
float timer = 0.0;

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
    int difficulty;
    int progress;
};
struct Level level[MAX_LEVELS];

//----------------------------------------------------------------
//init functions
//----------------------------------------------------------------

void InitLevel(char *filePath, int levelNumber) {
    char levelDataHold[LEVEL_SIZE];
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
        } else if (lineNumber >= 4) {
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
    for (int i = 0; i < MAP_WIDTH * MAP_HEIGHT; i++) {
        if (level[selectedLevel].map[i] == 'g') {
            gravitonPosition.x = ((i % MAP_WIDTH) * MAP_ITEM_SIZE) + (SPRITE_SIZE / 2);
            gravitonPosition.y = ((i / MAP_WIDTH) * MAP_ITEM_SIZE) + (SPRITE_SIZE / 2);
        }
    }
    for (int i = 0; i < MAP_WIDTH * MAP_HEIGHT; i++) {
        if (level[selectedLevel].map[i] == 'a') {
            atomPosition.x = ((i % MAP_WIDTH) * MAP_ITEM_SIZE) + (SPRITE_SIZE / 2);
            atomPosition.y = ((i / MAP_WIDTH) * MAP_ITEM_SIZE) + (SPRITE_SIZE / 2);
        }
    }
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
        if (level[selectedLevel].map[i] == 'o') {
            if (CheckCollisionPointRec(atomPosition, (Rectangle){mapItemX, mapItemY, MAP_ITEM_SIZE, MAP_ITEM_SIZE})) {
                gameState = GAME_END;
                gameWon = false;
            }
        } else if (level[selectedLevel].map[i] == 'f') {
            if (CheckCollisionPointRec(atomPosition, (Rectangle){mapItemX, mapItemY, MAP_ITEM_SIZE, MAP_ITEM_SIZE})) {
                gameState = GAME_END;
                gameWon = true;
            }
        }
    }
}

void UpdateAtom() {
    atomForce = Vector2Scale(Vector2Subtract(gravitonPosition, atomPosition), 
            (GRAVITY / Vector2Distance(atomPosition, gravitonPosition)));
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

void DrawLevel(int localUiScale) {
    int xOffset = ((MAP_WIDTH * UI_SCALE) - (MAP_WIDTH * localUiScale)) / 2;
    int yOffset = ((MAP_HEIGHT * UI_SCALE) - (MAP_HEIGHT * localUiScale)) / 2;
    if (localUiScale != UI_SCALE) {
        DrawRectangleLinesEx(
            (Rectangle){
            xOffset - LEVEL_BORDER, 
            yOffset - LEVEL_BORDER, 
            (localUiScale * MAP_WIDTH) + (LEVEL_BORDER * 2), 
            (localUiScale * MAP_HEIGHT) + (LEVEL_BORDER * 2)}, 
            LEVEL_BORDER, GRAV_DGRAY);
    }

    int mapItemLineScaled = (MAP_ITEM_LINE * localUiScale) / UI_SCALE;
    for (int i = 0; i <= MAP_WIDTH * MAP_HEIGHT; i++) {
        int mapItemX = ((i % MAP_WIDTH) * localUiScale) + xOffset;
        int mapItemY = ((i / MAP_WIDTH) * localUiScale) + yOffset;
        if (level[selectedLevel].map[i] == 'o') {
            DrawRectangleLinesEx((Rectangle){mapItemX, mapItemY, localUiScale, localUiScale}, mapItemLineScaled, GRAV_RED);
        } else if (level[selectedLevel].map[i] == 'f') {
            DrawRectangleLinesEx((Rectangle){mapItemX, mapItemY, localUiScale, localUiScale}, mapItemLineScaled, GRAV_BLUE);
        }
    }

    if (localUiScale == UI_SCALE) {
        for (int i = 0; i <= FPS; i++) {
            if (atomTrace[i].active == true) {
                DrawLineEx(atomTrace[i].start, atomTrace[i].end, ATOM_TRACE_THICK, GRAV_DGRAY);
            }
        }
    }

    DrawTexturePro(testingAtom, (Rectangle){0, 0, SPRITE_SIZE, SPRITE_SIZE},
        (Rectangle){
        (((atomPosition.x - (SPRITE_SIZE / 2)) * localUiScale) / UI_SCALE) + xOffset, 
        (((atomPosition.y - (SPRITE_SIZE / 2)) * localUiScale) / UI_SCALE) + yOffset, 
        (SPRITE_SIZE * localUiScale) / UI_SCALE, 
        (SPRITE_SIZE * localUiScale) / UI_SCALE}, 
        (Vector2){0, 0}, 0.0, WHITE);

    DrawTexturePro(testingGraviton, (Rectangle){0, 0, SPRITE_SIZE, SPRITE_SIZE}, 
        (Rectangle){
        (((gravitonPosition.x - (SPRITE_SIZE / 2)) * localUiScale) / UI_SCALE) + xOffset, 
        (((gravitonPosition.y - (SPRITE_SIZE / 2)) * localUiScale) / UI_SCALE) + yOffset, 
        (SPRITE_SIZE * localUiScale) / UI_SCALE, 
        (SPRITE_SIZE * localUiScale) / UI_SCALE}, 
        (Vector2){0, 0}, 0.0, WHITE);
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

void Ui() {
    int textPaddingLarge = (2 * UI_SCALE) / TEXT_PADDING_RELITIVE;
    int textPaddingSmall = UI_SCALE / TEXT_PADDING_RELITIVE;
    int fontSizeLarge = (2 * UI_SCALE) - (textPaddingLarge * 2);
    int fontSizeSmall = UI_SCALE - (textPaddingSmall * 2);
    int fontSizeExtraSmall = UI_SCALE - (textPaddingLarge * 2);

    if (gameState == GAME_START) {
        DrawLevel(UI_SCALE);

        DrawText("Graviton", UI_SCALE + textPaddingLarge, UI_SCALE + textPaddingLarge, fontSizeLarge, GRAV_WHITE);

        if (Button((Rectangle){30 * UI_SCALE, UI_SCALE / 5, (UI_SCALE * 9) / 5, (UI_SCALE * 4) / 5}, "Exit", KEY_ESCAPE)) {
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
        DrawLevel((UI_SCALE * 2) / 3);

        DrawText(level[selectedLevel].name, UI_SCALE + textPaddingLarge, UI_SCALE + textPaddingLarge, fontSizeLarge, GRAV_WHITE);
        DrawText(level[selectedLevel].designer, UI_SCALE + textPaddingLarge, textPaddingLarge, fontSizeExtraSmall, GRAV_WHITE);

        if (Button((Rectangle){13 * UI_SCALE, 15 * UI_SCALE, 6 * UI_SCALE, 2 * UI_SCALE}, "Home", KEY_H)) {
            gameState = GAME_START;
        }
        if (Button((Rectangle){11 * UI_SCALE, 15 * UI_SCALE, UI_SCALE, UI_SCALE}, "<-", KEY_J)) {
            DecreaseSelectedLevel();
        }
        if (Button((Rectangle){20 * UI_SCALE, 15 * UI_SCALE, UI_SCALE, UI_SCALE}, "->", KEY_K)) {
            IncreaseSelectedLevel();
        }

        LevelToggleButton((Rectangle){UI_SCALE, 3 * UI_SCALE, 4 * UI_SCALE, (UI_SCALE * 4) / 5}, 
                &levelFilters.difficulty[EASY], "Easy", KEY_ONE);

        LevelToggleButton((Rectangle){UI_SCALE, 4 * UI_SCALE, 4 * UI_SCALE, (UI_SCALE * 4) / 5}, 
                &levelFilters.difficulty[MEDIUM], "Medium", KEY_TWO);

        LevelToggleButton((Rectangle){UI_SCALE, 5 * UI_SCALE, 4 * UI_SCALE, (UI_SCALE * 4) / 5}, 
                &levelFilters.difficulty[HARD], "Hard", KEY_THREE);

        LevelToggleButton((Rectangle){UI_SCALE, 6 * UI_SCALE, 4 * UI_SCALE, (UI_SCALE * 4) / 5}, 
                &levelFilters.difficulty[HELL], "Hell", KEY_FOUR);

        LevelToggleButton((Rectangle){UI_SCALE, 7 * UI_SCALE, 4 * UI_SCALE, (UI_SCALE * 4) / 5}, 
                &levelFilters.progress[NOT_TRIED], "Not Tried", KEY_FIVE);

        LevelToggleButton((Rectangle){UI_SCALE, 8 * UI_SCALE, 4 * UI_SCALE, (UI_SCALE * 4) / 5}, 
                &levelFilters.progress[ATTEMPTED], "Attempted", KEY_SIX);

        LevelToggleButton((Rectangle){UI_SCALE, 9 * UI_SCALE, 4 * UI_SCALE, (UI_SCALE * 4) / 5}, 
                &levelFilters.progress[FINISHED], "Finished", KEY_SEVEN);

    } else if (gameState == GAME_COSMETICS) {
        if (Button((Rectangle){13 * UI_SCALE, 15 * UI_SCALE, 6 * UI_SCALE, 2 * UI_SCALE}, "Home", KEY_H)) {
            gameState = GAME_START;
        }
    } else if (gameState == GAME_SETTINGS) {
        DrawText("hotkeys", UI_SCALE + textPaddingSmall, (2 * UI_SCALE) + textPaddingSmall, fontSizeSmall, GRAV_WHITE);
        DrawText("space = Start Level", UI_SCALE + textPaddingSmall, (3 * UI_SCALE) + textPaddingSmall, fontSizeExtraSmall, GRAV_WHITE);
        DrawText("h = Home", UI_SCALE + textPaddingSmall, (4 * UI_SCALE) + textPaddingSmall, fontSizeExtraSmall, GRAV_WHITE);
        DrawText("l = Levles", UI_SCALE + textPaddingSmall, (5 * UI_SCALE) + textPaddingSmall, fontSizeExtraSmall, GRAV_WHITE);
        DrawText("c = Cosmetics", UI_SCALE + textPaddingSmall, (6 * UI_SCALE) + textPaddingSmall, fontSizeExtraSmall, GRAV_WHITE);
        DrawText("s = Settings", UI_SCALE + textPaddingSmall, (7 * UI_SCALE) + textPaddingSmall, fontSizeExtraSmall, GRAV_WHITE);
        DrawText("i = Information", UI_SCALE + textPaddingSmall, (8 * UI_SCALE) + textPaddingSmall, fontSizeExtraSmall, GRAV_WHITE);
        DrawText("t = Tutorial", UI_SCALE + textPaddingSmall, (9 * UI_SCALE) + textPaddingSmall, fontSizeExtraSmall, GRAV_WHITE);

        DrawText("1 = Filter Easy", 
                (8 * UI_SCALE) + textPaddingSmall, (3 * UI_SCALE) + textPaddingSmall, fontSizeExtraSmall, GRAV_WHITE);
        DrawText("2 = Filter Meduim", 
                (8 * UI_SCALE) + textPaddingSmall, (4 * UI_SCALE) + textPaddingSmall, fontSizeExtraSmall, GRAV_WHITE);
        DrawText("3 = Filter Hard", 
                (8 * UI_SCALE) + textPaddingSmall, (5 * UI_SCALE) + textPaddingSmall, fontSizeExtraSmall, GRAV_WHITE);
        DrawText("4 = FIlter Hell", 
                (8 * UI_SCALE) + textPaddingSmall, (6 * UI_SCALE) + textPaddingSmall, fontSizeExtraSmall, GRAV_WHITE);
        DrawText("5 = Filter Not Tried", 
                (8 * UI_SCALE) + textPaddingSmall, (7 * UI_SCALE) + textPaddingSmall, fontSizeExtraSmall, GRAV_WHITE);
        DrawText("6 = Filter Attempted", 
                (8 * UI_SCALE) + textPaddingSmall, (8 * UI_SCALE) + textPaddingSmall, fontSizeExtraSmall, GRAV_WHITE);
        DrawText("7 = Filter Finished", 
                (8 * UI_SCALE) + textPaddingSmall, (9 * UI_SCALE) + textPaddingSmall, fontSizeExtraSmall, GRAV_WHITE);
        DrawText("j = Select Left", 
                (8 * UI_SCALE) + textPaddingSmall, (10 * UI_SCALE) + textPaddingSmall, fontSizeExtraSmall, GRAV_WHITE);
        DrawText("k = Select Right", 
                (8 * UI_SCALE) + textPaddingSmall, (11 * UI_SCALE) + textPaddingSmall, fontSizeExtraSmall, GRAV_WHITE);
        
        if (Button((Rectangle){13 * UI_SCALE, 15 * UI_SCALE, 6 * UI_SCALE, 2 * UI_SCALE}, "Home", KEY_H)) {
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
        DrawLevel(UI_SCALE);

        DrawText(TextFormat("%.2f", timer), 1792, 32, 32, GRAV_WHITE);
        DrawText(TextFormat("%d", gravitonMoves), 1792, 96, 32, GRAV_WHITE);
    } else if (gameState == GAME_END) {
        DrawLevel(UI_SCALE);

        DrawText(TextFormat("%.2f", timer), 1792, 32, 32, GRAV_WHITE);
        DrawText(TextFormat("%d", gravitonMoves), 1792, 96, 32, GRAV_WHITE);

        if (gameWon == true) {
            DrawText("You Won!", UI_SCALE + textPaddingLarge, UI_SCALE + textPaddingLarge, fontSizeLarge, GRAV_WHITE);
        }
        if (gameWon == false) {
            DrawText("You Lost!", UI_SCALE + textPaddingLarge, UI_SCALE + textPaddingLarge, fontSizeLarge, GRAV_WHITE);
        }

        if (Button((Rectangle){13 * UI_SCALE, 15 * UI_SCALE, 6 * UI_SCALE, 2 * UI_SCALE}, "Home", KEY_H)) {
            gameState = GAME_START;
            ResetLevel();
        }
    }
}

void Draw() {
    BeginDrawing();
        ClearBackground(GRAV_BLACK);
        Ui();
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
