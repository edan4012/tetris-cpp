#include "raylib.h"
#include <cstdlib>

#define SQUARE_SIZE 20
#define GRID_W 12
#define GRID_H 20

#define FALL_SPEED 30
#define FAST_DROP_SPEED 3

enum class Cell { EMPTY, MOVING, FULL, BLOCK };

class Piece {
public:
    Cell shape[4][4];

    void Clear() {
        for (int i = 0;i < 4;i++)
            for (int j = 0;j < 4;j++)
                shape[i][j] = Cell::EMPTY;
    }

    void Randomize() {
        Clear();
        int r = GetRandomValue(0, 6);

        switch (r) {
        case 0: shape[1][1] = shape[2][1] = shape[1][2] = shape[2][2] = Cell::MOVING; break;
        case 1: shape[1][0] = shape[1][1] = shape[1][2] = shape[2][2] = Cell::MOVING; break;
        case 2: shape[1][2] = shape[2][0] = shape[2][1] = shape[2][2] = Cell::MOVING; break;
        case 3: shape[0][1] = shape[1][1] = shape[2][1] = shape[3][1] = Cell::MOVING; break;
        case 4: shape[1][0] = shape[1][1] = shape[1][2] = shape[2][1] = Cell::MOVING; break;
        case 5: shape[1][1] = shape[2][1] = shape[2][2] = shape[3][2] = Cell::MOVING; break;
        case 6: shape[1][2] = shape[2][2] = shape[2][1] = shape[3][1] = Cell::MOVING; break;
        }
    }

    void Rotate() {
        Cell temp[4][4];

        for (int i = 0;i < 4;i++)
            for (int j = 0;j < 4;j++)
                temp[i][j] = shape[3 - j][i];

        for (int i = 0;i < 4;i++)
            for (int j = 0;j < 4;j++)
                shape[i][j] = temp[i][j];
    }
};

class Grid {
public:
    Cell grid[GRID_W][GRID_H];

    void Init() {
        for (int i = 0;i < GRID_W;i++) {
            for (int j = 0;j < GRID_H;j++) {
                if (j == GRID_H - 1 || i == 0 || i == GRID_W - 1)
                    grid[i][j] = Cell::BLOCK;
                else
                    grid[i][j] = Cell::EMPTY;
            }
        }
    }

    void Draw(int ox, int oy) {
        for (int j = 0;j < GRID_H;j++) {
            for (int i = 0;i < GRID_W;i++) {
                int x = ox + i * SQUARE_SIZE;
                int y = oy + j * SQUARE_SIZE;

                switch (grid[i][j]) {
                case Cell::EMPTY:
                    DrawRectangleLines(x, y, SQUARE_SIZE, SQUARE_SIZE, LIGHTGRAY); break;
                case Cell::FULL:
                    DrawRectangle(x, y, SQUARE_SIZE, SQUARE_SIZE, GRAY); break;
                case Cell::BLOCK:
                    DrawRectangle(x, y, SQUARE_SIZE, SQUARE_SIZE, LIGHTGRAY); break;
                default: break;
                }
            }
        }
    }
};

class Game {
private:
    Grid grid;
    Piece current, next;

    int posX, posY;
    int fallCounter;

    bool gameOver;

public:
    Game() { Init(); }

    void Init() {
        grid.Init();
        current.Randomize();
        next.Randomize();

        posX = GRID_W / 2 - 2;
        posY = 0;

        fallCounter = 0;
        gameOver = false;
    }

    bool Collision(Piece& p, int x, int y) {
        for (int i = 0;i < 4;i++) {
            for (int j = 0;j < 4;j++) {
                if (p.shape[i][j] == Cell::MOVING) {
                    if (grid.grid[x + i][y + j] == Cell::FULL ||
                        grid.grid[x + i][y + j] == Cell::BLOCK)
                        return true;
                }
            }
        }
        return false;
    }

    void Merge() {
        for (int i = 0;i < 4;i++)
            for (int j = 0;j < 4;j++)
                if (current.shape[i][j] == Cell::MOVING)
                    grid.grid[posX + i][posY + j] = Cell::FULL;
    }

    void ClearLines() {
        for (int j = GRID_H - 2;j >= 0;j--) {
            bool full = true;
            for (int i = 1;i < GRID_W - 1;i++) {
                if (grid.grid[i][j] != Cell::FULL) {
                    full = false;
                    break;
                }
            }

            if (full) {
                for (int y = j;y > 0;y--) {
                    for (int i = 1;i < GRID_W - 1;i++)
                        grid.grid[i][y] = grid.grid[i][y - 1];
                }
                j++;
            }
        }
    }

    void Spawn() {
        current = next;
        next.Randomize();
        posX = GRID_W / 2 - 2;
        posY = 0;

        if (Collision(current, posX, posY))
            gameOver = true;
    }

    void Update() {
        if (gameOver) {
            if (IsKeyPressed(KEY_ENTER)) Init();
            return;
        }

        // 좌우 이동
        if (IsKeyPressed(KEY_LEFT) && !Collision(current, posX - 1, posY)) posX--;
        if (IsKeyPressed(KEY_RIGHT) && !Collision(current, posX + 1, posY)) posX++;

        // 회전
        if (IsKeyPressed(KEY_UP)) {
            Piece temp = current;
            temp.Rotate();
            if (!Collision(temp, posX, posY))
                current = temp;
        }

        // 빠른 낙하
        int speed = IsKeyDown(KEY_DOWN) ? FAST_DROP_SPEED : FALL_SPEED;

        fallCounter++;
        if (fallCounter >= speed) {
            if (!Collision(current, posX, posY + 1)) {
                posY++;
            }
            else {
                Merge();
                ClearLines();
                Spawn();
            }
            fallCounter = 0;
        }
    }

    void Draw() {
        BeginDrawing();
        ClearBackground(RAYWHITE);

        int ox = 200, oy = 40;

        grid.Draw(ox, oy);

        // 현재 블록
        for (int i = 0;i < 4;i++) {
            for (int j = 0;j < 4;j++) {
                if (current.shape[i][j] == Cell::MOVING) {
                    DrawRectangle(
                        ox + (posX + i) * SQUARE_SIZE,
                        oy + (posY + j) * SQUARE_SIZE,
                        SQUARE_SIZE, SQUARE_SIZE, DARKGRAY);
                }
            }
        }

        // 다음 블록 표시
        DrawText("NEXT", 500, 40, 20, GRAY);

        for (int i = 0;i < 4;i++) {
            for (int j = 0;j < 4;j++) {
                if (next.shape[i][j] == Cell::MOVING) {
                    DrawRectangle(
                        500 + i * SQUARE_SIZE,
                        80 + j * SQUARE_SIZE,
                        SQUARE_SIZE, SQUARE_SIZE, GRAY);
                }
                else {
                    DrawRectangleLines(
                        500 + i * SQUARE_SIZE,
                        80 + j * SQUARE_SIZE,
                        SQUARE_SIZE, SQUARE_SIZE,
                        LIGHTGRAY);
                }
            }
        }

        if (gameOver)
            DrawText("GAME OVER (ENTER)", 220, 200, 20, RED);

        EndDrawing();
    }
};

int main() {
    InitWindow(800, 450, "Tetris C++ Class Complete");
    SetTargetFPS(60);

    Game game;

    while (!WindowShouldClose()) {
        game.Update();
        game.Draw();
    }

    CloseWindow();
    return 0;
}
