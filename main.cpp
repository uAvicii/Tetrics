#include <windows.h>
#include <time.h>

const int BLOCK_SIZE = 20;
const int WIDTH = 20;
const int HEIGHT = 20;
const int WINDOW_WIDTH = WIDTH * BLOCK_SIZE;
const int WINDOW_HEIGHT = HEIGHT * BLOCK_SIZE;

enum { I, O, T, S, Z, J, L };

struct Block {
    int x, y;
};

struct Tetromino {
    int shape[4][4];
    int size;
    COLORREF color;
};

Tetromino tetrominoes[] = {
    {{ {0, 0, 0, 0}, {1, 1, 1, 1}, {0, 0, 0, 0}, {0, 0, 0, 0} }, 4, RGB(0, 255, 255)}, // I
    {{ {1, 1}, {1, 1} }, 2, RGB(255, 255, 0)}, // O
    {{ {0, 1, 0}, {1, 1, 1}, {0, 0, 0} }, 3, RGB(128, 0, 128)}, // T
    {{ {0, 1, 1}, {1, 1, 0}, {0, 0, 0} }, 3, RGB(0, 255, 0)}, // S
    {{ {1, 1, 0}, {0, 1, 1}, {0, 0, 0} }, 3, RGB(255, 0, 0)}, // Z
    {{ {1, 0, 0}, {1, 1, 1}, {0, 0, 0} }, 3, RGB(0, 0, 255)}, // J
    {{ {0, 0, 1}, {1, 1, 1}, {0, 0, 0} }, 3, RGB(255, 165, 0)} // L
};

HINSTANCE hInst;
HWND hWnd;
Block currentPos;
Tetromino currentTetromino;
COLORREF colors[HEIGHT][WIDTH];
bool gameOver = false;

void InitGame() {
    srand((unsigned int)time(NULL));
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            colors[y][x] = RGB(0, 0, 0);
        }
    }
    currentPos = { WIDTH / 2, 0 };
    currentTetromino = tetrominoes[rand() % 7];
}

void DrawBlock(HDC hdc, int x, int y, COLORREF color) {
    RECT rect;
    rect.left = x * BLOCK_SIZE;
    rect.top = y * BLOCK_SIZE;
    rect.right = rect.left + BLOCK_SIZE;
    rect.bottom = rect.top + BLOCK_SIZE;
    HBRUSH brush = CreateSolidBrush(color);
    FillRect(hdc, &rect, brush);
    DeleteObject(brush);
}

void DrawTetromino(HDC hdc, Tetromino& tetromino, int posX, int posY) {
    for (int y = 0; y < tetromino.size; y++) {
        for (int x = 0; x < tetromino.size; x++) {
            if (tetromino.shape[y][x]) {
                DrawBlock(hdc, posX + x, posY + y, tetromino.color);
            }
        }
    }
}

bool IsCollision(int newX, int newY, Tetromino& tetromino) {
    for (int y = 0; y < tetromino.size; y++) {
        for (int x = 0; x < tetromino.size; x++) {
            if (tetromino.shape[y][x]) {
                int checkX = newX + x;
                int checkY = newY + y;
                if (checkX < 0 || checkX >= WIDTH || checkY >= HEIGHT || (checkY >= 0 && colors[checkY][checkX] != RGB(0, 0, 0))) {
                    return true;
                }
            }
        }
    }
    return false;
}

void MoveBlock(int dx, int dy) {
    int newX = currentPos.x + dx;
    int newY = currentPos.y + dy;
    if (!IsCollision(newX, newY, currentTetromino)) {
        currentPos.x = newX;
        currentPos.y = newY;
    }
}

void RotateTetromino(Tetromino& tetromino) {
    Tetromino rotated = tetromino;
    for (int y = 0; y < tetromino.size; y++) {
        for (int x = 0; x < tetromino.size; x++) {
            rotated.shape[x][tetromino.size - y - 1] = tetromino.shape[y][x];
        }
    }
    if (!IsCollision(currentPos.x, currentPos.y, rotated)) {
        currentTetromino = rotated;
    }
}

void DropBlock() {
    if (IsCollision(currentPos.x, currentPos.y + 1, currentTetromino)) {
        for (int y = 0; y < currentTetromino.size; y++) {
            for (int x = 0; x < currentTetromino.size; x++) {
                if (currentTetromino.shape[y][x]) {
                    colors[currentPos.y + y][currentPos.x + x] = currentTetromino.color;
                }
            }
        }
        // 检查是否有行被填满
        for (int y = 0; y < HEIGHT; y++) {
            bool lineFilled = true;
            for (int x = 0; x < WIDTH; x++) {
                if (colors[y][x] == RGB(0, 0, 0)) {
                    lineFilled = false;
                    break;
                }
            }
            if (lineFilled) {
                // 清除满行
                for (int k = y; k > 0; k--) {
                    for (int x = 0; x < WIDTH; x++) {
                        colors[k][x] = colors[k - 1][x];
                    }
                }
                for (int x = 0; x < WIDTH; x++) {
                    colors[0][x] = RGB(0, 0, 0);
                }
                y--; // 检查新填满的行
            }
        }
        currentPos = { WIDTH / 2, 0 };
        currentTetromino = tetrominoes[rand() % 7];
        if (IsCollision(currentPos.x, currentPos.y, currentTetromino)) {
            gameOver = true;
        }
    }
    else {
        currentPos.y++;
    }
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_CREATE:
        SetTimer(hWnd, 1, 500, NULL);
        InitGame();
        break;
    case WM_TIMER:
        if (!gameOver) {
            DropBlock();
            InvalidateRect(hWnd, NULL, TRUE);
        }
        break;
    case WM_KEYDOWN:
        if (!gameOver) {
            switch (wParam) {
            case VK_LEFT:
                MoveBlock(-1, 0);
                break;
            case VK_RIGHT:
                MoveBlock(1, 0);
                break;
            case VK_DOWN:
                DropBlock();
                break;
            case VK_UP:
                RotateTetromino(currentTetromino);
                break;
            }
            InvalidateRect(hWnd, NULL, TRUE);
        }
        break;
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        for (int y = 0; y < HEIGHT; y++) {
            for (int x = 0; x < WIDTH; x++) {
                DrawBlock(hdc, x, y, colors[y][x]);
            }
        }
        if (!gameOver) {
            DrawTetromino(hdc, currentTetromino, currentPos.x, currentPos.y);
        }
        EndPaint(hWnd, &ps);
    }
                 break;
    case WM_DESTROY:
        KillTimer(hWnd, 1);
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow) {
    hInst = hInstance;
    WNDCLASSEXW wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = L"Tetris";
    wcex.hIconSm = LoadIcon(wcex.hInstance, IDI_APPLICATION);

    RegisterClassExW(&wcex);

    hWnd = CreateWindowW(L"Tetris", L"Tetris", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, WINDOW_WIDTH + 16, WINDOW_HEIGHT + 39, NULL, NULL, hInstance, NULL);

    if (!hWnd) {
        return FALSE;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}
