#include <raylib.h>
#include <iostream>
#include <vector>

class sandSpace {
    private:
        Color hueToColor(short);
        short fallPath(short, short, short);

    public:
        sandSpace();
        short numRows, numCols;
        void initialize();
        void draw();
        void update();
        short grid[200][320];
        short velocity[200][320];
        short cellSize;
};

sandSpace::sandSpace() {
    numRows = 200;
    numCols = 320;
    cellSize = 4;
    initialize();
}

void sandSpace::initialize() {
    for (short r = 0; r < numRows; r++) {
        for (short c = 0; c < numCols; c++) {
            grid[r][c] = 0;
            velocity[r][c] = 1;
        }
    }
}

Color sandSpace::hueToColor(short hue) {
    if (hue == 0) return {0,0,0,255};
    else if (hue == 1) return {255,255,255,255};

    hue--;
    float r, g, b;
    float h = static_cast<float>(hue) / 120.0f;
    int sector = static_cast<int>(h);
    float fract = h - sector;

    switch (sector) {
        case 0: r = 1.0f; g = fract; b = 0.0f; break;
        case 1: r = 1.0f - fract; g = 1.0f; b = 0.0f; break;
        case 2: r = 0.0f; g = 1.0f; b = fract; break;
        case 3: r = 0.0f; g = 1.0f - fract; b = 1.0f; break;
        case 4: r = fract; g = 0.0f; b = 1.0f; break;
        case 5: r = 1.0f; g = 0.0f; b = 1.0f - fract; break;
        default: r = 0.0f; g = 0.0f; b = 0.0f; break;
    }

    return Color{ static_cast<unsigned char>(r * 255),
    static_cast<unsigned char>(g * 255), static_cast<unsigned char>(b * 255), 175};
}

void sandSpace::draw() {
    for (int r = 0; r < numRows; r++) {
        for (int c = 0; c < numCols; c++)
            DrawRectangle(c * cellSize, r * cellSize, cellSize, cellSize, hueToColor(grid[r][c]));
    }
}

short sandSpace::fallPath(short f, short r, short c) {
    for (short i = 1; i <= f; i++)
        if (grid[r + i][c] != 0) return i - 1;
    return f;
}

void sandSpace::update() {
    for (short r = numRows - 2; r >= 0; r--) {

        short start, end, step; // alternate loop order to prevent right lean
        if (r % 2) { start = 0; end = numCols; step = 1; }
        else { start = numCols - 1; end = -1; step = -1; }

        for (short c = start; c != end; c += step) {
            if (grid[r][c] <= 1) continue;

            if (grid[r + 1][c] == 0) {
                short fall = r + std::max(1, fallPath(velocity[r][c], r, c) - GetRandomValue(0, 3));
                grid[fall][c] = grid[r][c];
                grid[r][c] = 0;
                velocity[fall][c] = velocity[r][c] + 1;
                velocity[r][c] = 1;
            }

            else if (c < numCols - 1 && c > 0 && grid[r + 1][c + 1] == 0 && grid[r + 1][c - 1] == 0 && grid[r][c - 1] != 1 && grid[r][c + 1] != 1) {
                if (GetRandomValue(0, 1)) grid[r + 1][c - 1] = grid[r][c];
                else grid[r + 1][c + 1] = grid[r][c];
                grid[r][c] = 0;
            }

            else if (c < numCols - 1 && grid[r + 1][c + 1] == 0 && grid[r][c + 1] != 1) {
                grid[r + 1][c + 1] = grid[r][c];
                grid[r][c] = 0;
            }

            else if (c > 0 && grid[r + 1][c - 1] == 0 && grid[r][c - 1] != 1) {
                grid[r + 1][c - 1] = grid[r][c];
                grid[r][c] = 0;
            }
        }
    }
}

void lmbPress(sandSpace *sand, int colorIndex) {
    short xPos = GetMouseX() / sand->cellSize, yPos = GetMouseY() / sand->cellSize;
    static int hue = 0;
    hue = ((hue + 1) % 720) + 1;
    
    int radius = 3;
    for (short xOff = -radius; xOff <= radius; xOff++) {
        for (short yOff = -radius; yOff <= radius; yOff++) {
            short x = xPos + xOff;
            short y = yPos + yOff;
            if (x < 0 || x >= sand->numCols) continue;
            if (y < 0 || y >= sand->numRows) continue;

            if (sand->grid[y][x] != 0) continue;

            if (xOff * xOff + yOff * yOff <= radius * radius)
                sand->grid[y][x] = hue;
        }
    }
}

void rmbPress(sandSpace *sand) {
    static short prevX = -1, prevY = -1;
    short xPos = GetMouseX() / sand->cellSize;
    short yPos = GetMouseY() / sand->cellSize;

    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
        prevX = xPos;
        prevY = yPos;
    }

    if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
        if (prevX != -1 && prevY != -1) {
            short dx = abs(xPos - prevX), dy = abs(yPos - prevY);
            short sx = (prevX < xPos) ? 1 : -1;
            short sy = (prevY < yPos) ? 1 : -1;
            short err = dx - dy;

            while (true) { // bresenham's line algorithm
                if (prevY < sand->numRows && prevX < sand -> numCols && prevY >= 0 && prevX >= 0)
                    sand->grid[prevY][prevX] = 1;
                if (prevX == xPos && prevY == yPos) break;

                short e2 = 2 * err;
                if (e2 > -dy) { err -= dy; prevX += sx; }
                if (e2 < dx) { err += dx; prevY += sy; }
            }
        }

        prevX = xPos;
        prevY = yPos;
    }
}

int main() {
    const short screenWidth = 1280, screenHeight = 800;
    const Color bgColor = {0, 0, 0, 255};

    InitWindow(screenWidth, screenHeight, "Falling Sand Simulator: Left Click; Place Sand, Right Click; Place Barrier, Space; Reset Canvas");
    SetTargetFPS(60);

    sandSpace sand = sandSpace();
    int colorIndex = GetRandomValue(2, 16);
    while (!WindowShouldClose()) {
        BeginDrawing();

        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
            lmbPress(&sand, colorIndex);
        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
            colorIndex = GetRandomValue(2, 16);

        if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
            rmbPress(&sand);

        if (IsKeyPressed(KEY_SPACE))
            sand.initialize();

        sand.update();

        ClearBackground(bgColor);
        sand.draw();
        EndDrawing();
    }

    CloseWindow();
    return 0;
}