#include <raylib.h>
#include <vector>
#include <iostream>

// if the window's edge is solid or just tps to the other edge
bool windowWalls = false;

// game speed
int gameSpeed = 20; // note: higher = slower, lower = faster

// tile size and gap between them
const int tileSize = 12;
const int tileGap = 3;

// size of the apple
const int appleSize = 10;

// couldn't find a proper name so named it like that.
// essentially it says how many tiles away from the window edge to spawn the snake at 
const int windowEdgesTile = 8;

// how many tiles can be in the window
int gameBoundX;
int gameBoundY;

// pos of snake's head
Vector2* snakeHead;
// game over?
bool lost = false;

// move direction
enum MoveDir {
    LEFT,
    RIGHT,
    UP,
    DOWN
};

// game TPS, although I could just limit the game's FPS
int timeCounter = 0;

// current direction the head is heading to. future direction is assigned by input, so that the snake doesn't bug and instantly move in the opposite direction (aka itself).
MoveDir currMovDir = MoveDir::UP;
MoveDir futureMovDir = currMovDir;

// snake itself and food
std::vector<Vector2> snake;
Vector2 food{ (float)GetRandomValue(0, (int)gameBoundX),(float)GetRandomValue(0, (int)gameBoundY) };

// randomize the direction the snake will head to. used when the game starts/restarts
void randomizeMoveDir() {
    int randMovDir = GetRandomValue(0, 3);
    switch (randMovDir)
    {
    case 0:
        currMovDir = MoveDir::LEFT;
        break;
    case 1:
        currMovDir = MoveDir::RIGHT;
        break;
    case 2:
        currMovDir = MoveDir::UP;
        break;
    case 3:
        currMovDir = MoveDir::DOWN;
        break;
    default:
        break;
    }
    futureMovDir = currMovDir;
}

// processes position by move direction. It has 2 purposes, the second one being to create the inital snake body on start/restart
void processMoveDir(Vector2& pos, bool reverse) {
    switch (currMovDir)
    {
    case MoveDir::LEFT:
        pos.x += reverse ? 1 : -1;
        break;
    case MoveDir::RIGHT:
        pos.x += reverse ? -1 : 1;
        break;
    case MoveDir::UP:
        pos.y += reverse ? 1 : -1;
        break;
    case MoveDir::DOWN:
        pos.y += reverse ? -1 : 1;
        break;
    default:
        break;
    }
}

// when the game starts or restarts
void restart() {
    lost = false;
    randomizeMoveDir();
    snake.clear();
    snake.push_back(Vector2{ (float)GetRandomValue(windowEdgesTile, (int)gameBoundX - windowEdgesTile), (float)GetRandomValue(windowEdgesTile, (int)gameBoundY - windowEdgesTile) });
    for (int i = 0; i < 2; i++)
    {
        Vector2 vec2 = snake.front();
        for (int j = 0; j < i + 1; j++) // make the snake's tail look good at the beginning of the game
        {
            processMoveDir(vec2, true);
        }
        snake.push_back(Vector2{ vec2.x, vec2.y });
    }
    food.x = (float)GetRandomValue(0, (int)gameBoundX);
    food.y = (float)GetRandomValue(0, (int)gameBoundY);
    timeCounter = 0;
}

void input() {
    if (IsKeyDown(KEY_RIGHT) && currMovDir != MoveDir::LEFT) {
        futureMovDir = MoveDir::RIGHT;
    }
    if (IsKeyDown(KEY_LEFT) && currMovDir != MoveDir::RIGHT) {
        futureMovDir = MoveDir::LEFT;
    }
    if (IsKeyDown(KEY_UP) && currMovDir != MoveDir::DOWN) {
        futureMovDir = MoveDir::UP;
    }
    if (IsKeyDown(KEY_DOWN) && currMovDir != MoveDir::UP) {
        futureMovDir = MoveDir::DOWN;
    }

    if (IsKeyPressed(KEY_PAGE_UP) && gameSpeed > 1) {
        gameSpeed--;
    }
    if (IsKeyPressed(KEY_PAGE_DOWN) && gameSpeed < 60) {
        gameSpeed++;
    }

    if (IsKeyPressed(KEY_END)) {
        windowWalls = !windowWalls;
    }
}

void logic() {
    timeCounter++;
    snakeHead = &snake.front();

    if (!lost) {
        input();

        if (((timeCounter / gameSpeed) % 2) == 1)
        {
            currMovDir = futureMovDir;
            std::copy_backward(snake.begin(), snake.end() - 1, snake.end());
            processMoveDir(*snakeHead, false);

            timeCounter = 0;
        }

        for (Vector2& snakePos : snake) {
            if ((&snakePos) == snakeHead) continue;
            if (CheckCollisionRecs({ snakeHead->x, snakeHead->y, 1, 1 }, { snakePos.x, snakePos.y, 1, 1 })) {
                lost = true;
                timeCounter = 0;
            }
        }

        if (!windowWalls) {
            if (snakeHead->x < 0) {
                snakeHead->x = static_cast<float>(gameBoundX);
            }
            if (snakeHead->x > gameBoundX) {
                snakeHead->x = 0;
            }
            if (snakeHead->y < 0) {
                snakeHead->y = static_cast<float>(gameBoundY);
            }
            if (snakeHead->y > gameBoundY) {
                snakeHead->y = 0;
            }
        }
        else {
            if (snakeHead->x < 0 || snakeHead->x > gameBoundX || snakeHead->y < 0 || snakeHead->y > gameBoundY) {
                lost = true;
                timeCounter = 0;
            }
        }

        // if the "windowWalls" code above was below this one, snakeHead would have undefined behaviour
        // thanks to breakpoints I didn't wait 5 hours
        if (CheckCollisionRecs(Rectangle{ snakeHead->x, snakeHead->y, 1, 1 }, Rectangle{ food.x, food.y, 1, 1 })) {
            food.x = (float)GetRandomValue(0, (int)gameBoundX);
            food.y = (float)GetRandomValue(0, (int)gameBoundY);
            Vector2 vec2{ snake.back() };
            snake.push_back(vec2);
        }
    }
    else {
        if (((timeCounter / 180) % 2) == 1)
        {
            restart();
        }
    }
}

void draw() {
    BeginDrawing();
    ClearBackground(BLACK);

    DrawRectangle(int(food.x * tileSize + food.x * tileGap + static_cast<float>(tileGap) / 2 + static_cast<float>((tileSize - appleSize)) / 2), int(food.y * tileSize + food.y * tileGap + static_cast<float>(tileGap) / 2 + static_cast<float>((tileSize - appleSize)) / 2), appleSize, appleSize, RED);

    for (Vector2& snakePos : snake) {
        DrawRectangle(int(snakePos.x * tileSize + snakePos.x * tileGap + static_cast<float>(tileGap) / 2), int(snakePos.y * tileSize + snakePos.y * tileGap + static_cast<float>(tileGap) / 2), tileSize, tileSize, WHITE);
    }

    if (lost) {
        Vector2 textSize = MeasureTextEx(GetFontDefault(), "GAME OVER", 26, 1);
        DrawText("GAME OVER", int(static_cast<float>(GetScreenWidth()) / 2 - textSize.x / 2), int(static_cast<float>(GetScreenHeight()) / 2 - textSize.y / 2), 26, RAYWHITE);
    }

    if (windowWalls) {
        DrawRectangleLinesEx(Rectangle{ 0, 0, static_cast<float>(GetScreenWidth()), static_cast<float>(GetScreenHeight()) }, 1, WHITE);
    }

    DrawFPS(10, 10);
    EndDrawing();
}

int main()
{
    InitWindow(800, 600, "Snake Game | Arrows - move | END key - edge collision | PG DOWN/UP - speed");
    SetTargetFPS(60);

    gameBoundX = (int)((GetScreenWidth() - (tileSize + tileGap)) / (tileSize + tileGap));
    gameBoundY = (int)((GetScreenHeight() - (tileSize + tileGap)) / (tileSize + tileGap));

    restart();
    while (!WindowShouldClose()) {
        logic();
        draw();
    }

    //delete snakeHead;
    // shakeHead = pointer to Vector element
    CloseWindow();

    return 0;
}