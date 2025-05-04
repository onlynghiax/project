#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <cmath>
using namespace std;

const int WINDOW_WIDTH = 400;
const int WINDOW_HEIGHT = 450; 
const int GRID_SIZE = 4;
const int CELL_SIZE = WINDOW_WIDTH / GRID_SIZE;
const int ANIMATION_SPEED = 10; 

SDL_Window *window = nullptr;
SDL_Renderer *renderer = nullptr;
TTF_Font *font = nullptr;

vector<vector<int>> grid(GRID_SIZE, vector<int>(GRID_SIZE, 0));
vector<vector<pair<int, int>>> animationGrid(GRID_SIZE, vector<pair<int, int>>(GRID_SIZE, {0, 0}));

bool gameOver = false;
bool gameWon = false;
bool gameStarted = false;
int moveCount = 0;
int score = 0;

// Khởi tạo SDL và TTF
void initialize()
{
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();
    window = SDL_CreateWindow("2048", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    font = TTF_OpenFont("C:/Users/doant/OneDrive/Documents/coding/LTNC/sdl2/project/arial.ttf", 24);
    if (!font)
    {
        cerr << "Failed to load font! Make sure 'arial.ttf' is in the same directory as the executable.\n";
        exit(1);
    }
}

// Đóng SDL và TTF
void close()
{
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
}

// Hiển thị chữ
void drawText(const char *text, int x, int y, SDL_Color color)
{
    SDL_Surface *surface = TTF_RenderText_Solid(font, text, color);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect rect = {x, y, surface->w, surface->h};
    SDL_RenderCopy(renderer, texture, nullptr, &rect);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

// Hiển thị màn hình bắt đầu
void drawStartScreen()
{
    SDL_SetRenderDrawColor(renderer, 187, 173, 160, 255);
    SDL_RenderClear(renderer);

    // Hiển thị tiêu đề "2048"
    SDL_Color titleColor = {119, 110, 101, 255};
    drawText("2048", WINDOW_WIDTH / 2 - 30, WINDOW_HEIGHT / 2 - 100, titleColor);

    // Hiển thị nút "Start Game"
    SDL_Rect startButton = {WINDOW_WIDTH / 2 - 75, WINDOW_HEIGHT / 2 - 25, 150, 50};
    SDL_SetRenderDrawColor(renderer, 255, 235, 205, 1 ); // Chọn màu be cho nút "Start Game"
    SDL_RenderFillRect(renderer, &startButton);
    SDL_Color buttonTextColor = {119, 110, 101, 255}; // Chọn màu xanh lá cho chữ "Start Game"
    drawText("Start Game", WINDOW_WIDTH / 2 - 60, WINDOW_HEIGHT / 2 - 15, buttonTextColor);

    SDL_RenderPresent(renderer);
}

// Khởi tạo ô 4x4 với 2 ô 1x1 chứa số ngẫu nhiên
void drawGrid()
{
    SDL_SetRenderDrawColor(renderer, 187, 173, 160, 255);
    SDL_RenderClear(renderer);

    for (int i = 0; i < GRID_SIZE; ++i)
    {
        for (int j = 0; j < GRID_SIZE; ++j)
        {
            SDL_Rect cellRect = {j * CELL_SIZE + 5, i * CELL_SIZE + 55, CELL_SIZE - 10, CELL_SIZE - 10}; // Điều chỉnh vị trí y cho bộ đếm di chuyển và điểm số

           // Đặt màu dựa trên việc ô có số hay không
            if (grid[i][j] == 0)
            {
                SDL_SetRenderDrawColor(renderer, 205, 193, 180, 255); // Màu sáng cho ô trống
            }
            else
            {
                SDL_SetRenderDrawColor(renderer, 238, 228, 218, 255); // Màu tối hơn cho ô có số
            }
            SDL_RenderFillRect(renderer, &cellRect);

            if (grid[i][j] != 0)
            {
                SDL_Color textColor = {119, 110, 101, 255};
                char buffer[10];
                snprintf(buffer, sizeof(buffer), "%d", grid[i][j]);
                int textWidth, textHeight;
                TTF_SizeText(font, buffer, &textWidth, &textHeight);
                int x = j * CELL_SIZE + (CELL_SIZE - textWidth) / 2 + animationGrid[i][j].first;
                int y = i * CELL_SIZE + (CELL_SIZE - textHeight) / 2 + animationGrid[i][j].second + 55; // Điều chỉnh vị trí y cho bộ đếm di chuyển và điểm số
                drawText(buffer, x, y, textColor);
            }
        }
    }

    // Hiển thị bộ đếm di chuyển
    char moveBuffer[20];
    snprintf(moveBuffer, sizeof(moveBuffer), "Moves: %d", moveCount);
    SDL_Color textColor = {119, 110, 101, 255};
    drawText(moveBuffer, 10, 15, textColor);

    // Hiển thị điểm số
    char scoreBuffer[20];
    snprintf(scoreBuffer, sizeof(scoreBuffer), "Score: %d", score);
    drawText(scoreBuffer, WINDOW_WIDTH - 150, 15, textColor);

    if (gameOver)
    {
        // Hiển thị lớp phủ đen bán trong suốt
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 128); // Màu đen với độ mờ 50%
        SDL_Rect overlay = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
        SDL_RenderFillRect(renderer, &overlay);

        // Hiển thị chữ "You Lose!"
        SDL_Color loseColor = {255, 0, 0, 255};
        drawText("You Lose!", WINDOW_WIDTH / 2 - 50, WINDOW_HEIGHT / 2 - 20, loseColor);
    }

    if (gameWon)
    {
        // Hiển thị lớp phủ đen bán trong suốt
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 128); // Màu đen với độ mờ 50%
        SDL_Rect overlay = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
        SDL_RenderFillRect(renderer, &overlay);

        // Hiển thị chữ "You Win!"
        SDL_Color winColor = {0, 255, 0, 255};
        drawText("You Win!", WINDOW_WIDTH / 2 - 50, WINDOW_HEIGHT / 2 - 20, winColor);
    }

    SDL_RenderPresent(renderer);
}

// Tạo thêm ô chứa số ngẫu nhiên
void addRandomTile()
{
   vector<pair<int, int>> emptyCells;
    for (int i = 0; i < GRID_SIZE; ++i)
    {
        for (int j = 0; j < GRID_SIZE; ++j)
        {
            if (grid[i][j] == 0)
            {
                emptyCells.push_back({i, j});
            }
        }
    }

    if (!emptyCells.empty())
    {
        int index = rand() % emptyCells.size();
        int value = (rand() % 10) < 9 ? 2 : 4;
        grid[emptyCells[index].first][emptyCells[index].second] = value;
    }
}

// Kiểm tra xem có thể di chuyển ô hay không
bool canMove()
{
    for (int i = 0; i < GRID_SIZE; ++i)
    {
        for (int j = 0; j < GRID_SIZE; ++j)
        {
            if (grid[i][j] == 0)
            {
                return true;
            }
            if (i < GRID_SIZE - 1 && grid[i][j] == grid[i + 1][j])
            {
                return true;
            }
            if (j < GRID_SIZE - 1 && grid[i][j] == grid[i][j + 1])
            {
                return true;
            }
        }
    }
    return false;
}

// Di chuyển ô
void moveTiles(int dx, int dy)
{
    bool moved = false;
    vector<vector<int>> newGrid = grid;
    vector<vector<pair<int, int>>> newAnimationGrid = animationGrid;

    if (dx != 0)
    {
        for (int i = 0; i < GRID_SIZE; ++i)
        {
            for (int j = (dx == 1 ? GRID_SIZE - 2 : 1); (dx == 1 ? j >= 0 : j < GRID_SIZE); j -= dx)
            {
                if (newGrid[i][j] != 0)
                {
                    int x = j + dx;
                    while (x >= 0 && x < GRID_SIZE && newGrid[i][x] == 0)
                    {
                        x += dx;
                    }
                    if (x >= 0 && x < GRID_SIZE && newGrid[i][x] == newGrid[i][j])
                    {
                        newGrid[i][x] *= 2;
                        score += newGrid[i][x]; // Cập nhật điểm 
                        newGrid[i][j] = 0;
                        newAnimationGrid[i][j] = {(x - j) * CELL_SIZE, 0};
                        moved = true;
                    }
                    else
                    {
                        x -= dx;
                        if (x != j)
                        {
                            newGrid[i][x] = newGrid[i][j];
                            newGrid[i][j] = 0;
                            newAnimationGrid[i][j] = {(x - j) * CELL_SIZE, 0};
                            moved = true;
                        }
                    }
                }
            }
        }
    }
    else if (dy != 0)
    {
        for (int j = 0; j < GRID_SIZE; ++j)
        {
            for (int i = (dy == 1 ? GRID_SIZE - 2 : 1); (dy == 1 ? i >= 0 : i < GRID_SIZE); i -= dy)
            {
                if (newGrid[i][j] != 0)
                {
                    int y = i + dy;
                    while (y >= 0 && y < GRID_SIZE && newGrid[y][j] == 0)
                    {
                        y += dy;
                    }
                    if (y >= 0 && y < GRID_SIZE && newGrid[y][j] == newGrid[i][j])
                    {
                        newGrid[y][j] *= 2;
                        score += newGrid[y][j]; // Câp nhật điểm
                        newGrid[i][j] = 0;
                        newAnimationGrid[i][j] = {0, (y - i) * CELL_SIZE};
                        moved = true;
                    }
                    else
                    {
                        y -= dy;
                        if (y != i)
                        {
                            newGrid[y][j] = newGrid[i][j];
                            newGrid[i][j] = 0;
                            newAnimationGrid[i][j] = {0, (y - i) * CELL_SIZE};
                            moved = true;
                        }
                    }
                }
            }
        }
    }

    if (moved)
    {
        grid = newGrid;
        animationGrid = newAnimationGrid;
        addRandomTile();
        moveCount++;

        // Kiểm tra xem người chơi đã thắng chưa
        for (int i = 0; i < GRID_SIZE; ++i)
        {
            for (int j = 0; j < GRID_SIZE; ++j)
            {
                if (grid[i][j] == 2048)
                {
                    gameWon = true;
                    return;
                }
            }
        }

        if (!canMove())
        {
            gameOver = true;
        }
    }
}

// Cập nhật hoạt ảnh (gộp 2 ô)
void updateAnimation()
{
    bool animating = false;
    for (int i = 0; i < GRID_SIZE; ++i)
    {
        for (int j = 0; j < GRID_SIZE; ++j)
        {
            if (animationGrid[i][j].first != 0)
            {
                int dx = (animationGrid[i][j].first > 0) ? ANIMATION_SPEED : -ANIMATION_SPEED;
                animationGrid[i][j].first -= dx;
                if (abs(animationGrid[i][j].first) < ANIMATION_SPEED)
                {
                    animationGrid[i][j].first = 0;
                }
                animating = true;
            }
            if (animationGrid[i][j].second != 0)
            {
                int dy = (animationGrid[i][j].second > 0) ? ANIMATION_SPEED : -ANIMATION_SPEED;
                animationGrid[i][j].second -= dy;
                if (abs(animationGrid[i][j].second) < ANIMATION_SPEED)
                {
                    animationGrid[i][j].second = 0;
                }
                animating = true;
            }
        }
    }

    if (animating)
    {
        drawGrid();
    }
}

int main(int argc, char *argv[])
{
    srand(time(0));
    initialize();

    bool running = true;
    SDL_Event event;

    while (running)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                running = false;
            }
            else if (event.type == SDL_MOUSEBUTTONDOWN && !gameStarted)
            {
                int x, y;
                SDL_GetMouseState(&x, &y);
                if (x >= WINDOW_WIDTH / 2 - 75 && x <= WINDOW_WIDTH / 2 + 75 && y >= WINDOW_HEIGHT / 2 - 25 && y <= WINDOW_HEIGHT / 2 + 25)
                {
                    // Khi người chơi ấn nút "Start Game"
                    gameStarted = true;
                    gameOver = false;
                    gameWon = false;
                    moveCount = 0;
                    score = 0;
                    grid = vector<vector<int>>(GRID_SIZE, vector<int>(GRID_SIZE, 0));
                    addRandomTile();
                    addRandomTile();
                }
            }
            else if (event.type == SDL_KEYDOWN && gameStarted && !gameOver && !gameWon)
            {
                switch (event.key.keysym.sym)
                {
                case SDLK_UP:
                    moveTiles(0, -1);
                    break;
                case SDLK_DOWN:
                    moveTiles(0, 1);
                    break;
                case SDLK_LEFT:
                    moveTiles(-1, 0);
                    break;
                case SDLK_RIGHT:
                    moveTiles(1, 0);
                    break;
                }
            }
        }

        if (gameStarted)
        {
            updateAnimation();
            drawGrid();
        }
        else
        {
            drawStartScreen();
        }
    }

    close();
    return 0;
}