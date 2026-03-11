#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <windows.h>
#include <time.h>

#define ROWS 20
#define COLS 10
#define KEY_UP 72
#define KEY_DOWN 80
#define KEY_LEFT 75
#define KEY_RIGHT 77
#define ARROW_CHAR1 224

#define BASE_FALL_DELAY 500
#define MIN_FALL_DELAY 50
#define SCORE_STEP 500
#define SPEEDUP 20

struct shape {
    int r[4];
    int c[4];
};

struct shape shapes[7] = {
    {{0,0,0,0}, {-1,0,1,2}},      // I
    {{0,0,1,1}, {0,1,0,1}},       // O
    {{0,0,0,1}, {-1,0,1,0}},      // T
    {{0,0,1,1}, {0,1,-1,0}},      // S
    {{0,0,1,1}, {-1,0,0,1}},      // Z
    {{0,0,0,1}, {-1,0,1,-1}},     // J
    {{0,0,0,1}, {-1,0,1,1}}       // L
};

struct piece {
    struct shape s;
    int type;
};

int getFallDelay(int score) {
    int delay = BASE_FALL_DELAY - (score / SCORE_STEP) * SPEEDUP;
    if (delay < MIN_FALL_DELAY) delay = MIN_FALL_DELAY;
    return delay;
}

struct piece getRandomPiece() {
    int idx = rand() % 7;
    struct piece p = { shapes[idx], idx };
    return p;
}

void rotateRightPivot(struct piece* p, int pivotIndex) {
    if (p->type == 1) return;
    int pr = p->s.r[pivotIndex], pc = p->s.c[pivotIndex];
    for (int i = 0; i < 4; i++) {
        int r = p->s.r[i] - pr;
        int c = p->s.c[i] - pc;
        int tmp = r; r = -c; c = tmp;
        p->s.r[i] = r + pr;
        p->s.c[i] = c + pc;
    }
}

int checkCollision(int board[ROWS][COLS], struct shape s, int pr, int pc) {
    for (int i = 0; i < 4; i++) {
        int r = pr + s.r[i], c = pc + s.c[i];
        if (r < 0 || r >= ROWS || c < 0 || c >= COLS || board[r][c]) return 1;
    }
    return 0;
}

int tryRotate(int board[ROWS][COLS], struct piece* p, int pr, int pc) {
    struct piece temp = *p;
    rotateRightPivot(&temp, 1);
    int shifts[] = { 0, -1, 1, -2, 2 };
    for (int i = 0; i < 5; i++) {
        if (!checkCollision(board, temp.s, pr, pc + shifts[i])) {
            *p = temp;
            return pc + shifts[i];
        }
    }
    return pc;
}

void printBoardWithNext(int board[ROWS][COLS], struct piece next[2], struct piece holdPiece, int score) {
    char nextGrids[2][4][4] = { 0 }, holdGrid[4][4] = { 0 };
    char filled = 219, gridCell = 176;

    for (int n = 0; n < 2; n++)
        for (int i = 0; i < 4; i++) {
            int r = next[n].s.r[i] + 1, c = next[n].s.c[i] + 1;
            if (r >= 0 && r < 4 && c >= 0 && c < 4) nextGrids[n][r][c] = 1;
        }
    if (holdPiece.type != -1)
        for (int i = 0; i < 4; i++) {
            int r = holdPiece.s.r[i] + 1, c = holdPiece.s.c[i] + 1;
            if (r >= 0 && r < 4 && c >= 0 && c < 4) holdGrid[r][c] = 1;
        }

    printf("%c", 201); for (int j = 0; j < COLS * 2; j++) printf("%c", 205); printf("%c   Hold\n", 187);
    for (int i = 0; i < ROWS; i++) {
        printf("%c", 186);
        for (int j = 0; j < COLS; j++) printf("%c%c", board[i][j] ? filled : gridCell, board[i][j] ? filled : gridCell);
        printf("%c   ", 186);
        if (i < 4) for (int j = 0; j < 4; j++) printf("%c%c", holdGrid[i][j] ? filled : ' ', holdGrid[i][j] ? filled : ' ');
        else printf("        ");
        printf("   ");
        if (i < 4) for (int j = 0; j < 4; j++) printf("%c%c", nextGrids[0][i][j] ? filled : ' ', nextGrids[0][i][j] ? filled : ' ');
        else if (i >= 4 && i < 8) for (int j = 0; j < 4; j++) printf("%c%c", nextGrids[1][i - 4][j] ? filled : ' ', nextGrids[1][i - 4][j] ? filled : ' ');
        printf("\n");
    }
    printf("%c", 200); for (int j = 0; j < COLS * 2; j++) printf("%c", 205); printf("%c\n", 188);
    printf("Score: %d\n", score);
}

void placeShape(int board[ROWS][COLS], struct shape s, int pr, int pc) {
    for (int i = 0; i < 4; i++) {
        int r = pr + s.r[i], c = pc + s.c[i];
        if (r >= 0 && r < ROWS && c >= 0 && c < COLS) board[r][c] = 1;
    }
}

void lockShape(int board[ROWS][COLS], struct shape s, int pr, int pc) {
    placeShape(board, s, pr, pc);
}

/*void clear_screen() {
    COORD topLeft = { 0,0 };
    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD written;
    CONSOLE_SCREEN_BUFFER_INFO screen;
    GetConsoleScreenBufferInfo(console, &screen);
    FillConsoleOutputCharacterA(console, ' ', screen.dwSize.X * screen.dwSize.Y, topLeft, &written);
    FillConsoleOutputAttribute(console, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE,
        screen.dwSize.X * screen.dwSize.Y, topLeft, &written);
    SetConsoleCursorPosition(console, topLeft);
}*/

void resetCursor() {
    COORD topLeft = { 0,0 };
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), topLeft);
}

void copyBoard(int src[ROWS][COLS], int dst[ROWS][COLS]) {
    for (int i = 0; i < ROWS; i++)
        for (int j = 0; j < COLS; j++)
            dst[i][j] = src[i][j];
}

int clearFullRows(int board[ROWS][COLS], int* score) {
    int rowsCleared = 0;
    for (int i = 0; i < ROWS; i++) {
        int full = 1;
        for (int j = 0; j < COLS; j++)
            if (board[i][j] == 0) full = 0;
        if (full) {
            rowsCleared++;
            for (int k = i; k > 0; k--)
                for (int j = 0; j < COLS; j++)
                    board[k][j] = board[k - 1][j];
            for (int j = 0; j < COLS; j++) board[0][j] = 0;
            i--;
        }
    }
    if (rowsCleared > 0) {
        int points[] = { 0,100,300,500,800 };
        if (rowsCleared > 4) rowsCleared = 4;
        *score += points[rowsCleared];
    }
    return rowsCleared;
}

void flashAndClearRows(int board[ROWS][COLS], int* score, struct piece next[2], struct piece* holdPiece) {
    int rows[4], count = 0;
    for (int i = 0; i < ROWS; i++) {
        int full = 1;
        for (int j = 0; j < COLS; j++) if (!board[i][j]) full = 0;
        if (full) rows[count++] = i;
    }
    if (count == 0) return;

    int temp[ROWS][COLS];
    for (int k = 0; k < 4; k++) {
        for (int i = 0; i < count; i++) {
            int r = rows[i];
            for (int j = 0; j < COLS; j++)
                board[r][j] = (k % 2 == 0) ? 0 : 1;
        }
        copyBoard(board, temp);
        resetCursor();
        //clear_screen();
        placeShape(temp, next[0].s, 0, 0); // just pass current board
        printBoardWithNext(temp, next, *holdPiece, *score);
        Sleep(100);
    }

    clearFullRows(board, score);
}

void holdSwap(struct piece* current, struct piece* holdPiece, struct piece next[2], int* holdUsed, int* pr, int* pc, int board[ROWS][COLS]) {
    if (*holdUsed && holdPiece->type == -1) return;
    if (holdPiece->type == -1) {
        *holdPiece = *current;
        *current = next[0];
        next[0] = next[1];
        next[1] = getRandomPiece();
        *holdUsed = 1;
    }
    else {
        struct piece temp = *current;
        *current = *holdPiece;
        *holdPiece = temp;
    }
    *pr = 0;
    *pc = COLS / 2;
    int shifts[] = { 0,-1,1,-2,2 };
    for (int i = 0; i < 5; i++) {
        if (!checkCollision(board, (*current).s, *pr, *pc + shifts[i])) {
            *pc += shifts[i];
            break;
        }
    }
}



int main() {
    SetConsoleOutputCP(437);
    SetConsoleCP(437);
    srand(time(NULL));

    int board[ROWS][COLS] = { 0 }, temp[ROWS][COLS], score = 0;
    int ch1, ch2, posRow = 0, posCol = COLS / 2;
    DWORD lastFall = GetTickCount();
    struct piece current = getRandomPiece(), next[2] = { getRandomPiece(),getRandomPiece() };
    struct piece holdPiece = { .type = -1 }; int holdUsed = 0;

    while (1) {
        if (_kbhit()) {
            ch1 = _getch();
            if (ch1 == 27) break;
            if (ch1 == ' ') { // hold
                holdSwap(&current, &holdPiece, next, &holdUsed, &posRow, &posCol, board);
                continue;
            }
            if (ch1 == ARROW_CHAR1 || ch1 == 0) {
                ch2 = _getch();
                switch (ch2) {
                case KEY_UP: posCol = tryRotate(board, &current, posRow, posCol); break;
                case KEY_DOWN: if (!checkCollision(board, current.s, posRow + 1, posCol)) posRow++; break;
                case KEY_LEFT: if (!checkCollision(board, current.s, posRow, posCol - 1)) posCol--; break;
                case KEY_RIGHT: if (!checkCollision(board, current.s, posRow, posCol + 1)) posCol++; break;
                }
            }
        }

        if (GetTickCount() - lastFall > getFallDelay(score)) {
            if (!checkCollision(board, current.s, posRow + 1, posCol)) posRow++;
            else {
                lockShape(board, current.s, posRow, posCol);
                flashAndClearRows(board, &score, next, &holdPiece);
                holdUsed = 0;
                posRow = 0; posCol = COLS / 2;
                current = next[0]; next[0] = next[1]; next[1] = getRandomPiece();
                if (checkCollision(board, current.s, posRow, posCol)) {
                    resetCursor();
                    //clear_screen();
                    copyBoard(board, temp);
                    placeShape(temp, current.s, posRow, posCol);
                    printBoardWithNext(temp, next, holdPiece, score);
                    printf("\n*** GAME OVER ***\nFinal Score: %d\n", score);
                    Sleep(3000); break;
                }
            }
            lastFall = GetTickCount();
        }

        copyBoard(board, temp);
        placeShape(temp, current.s, posRow, posCol);
        resetCursor();
        //clear_screen();
        printBoardWithNext(temp, next, holdPiece, score);
        Sleep(30);
    }
    return 0;
}