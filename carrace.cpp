/* ------------------------------------------------------------------
   CAR RACING 3D  -  Computer Graphics (22318) Micro-Project
   Dev-C++ / MinGW version (Windows console)

   The original Turbo C version used conio.h functions (gotoxy,
   textcolor, clrscr, delay, sound ...) that MinGW does not have.
   This version provides the same functions using the Windows
   console API, so the game logic stays identical.

   Controls : LEFT / RIGHT arrow = change lane
              P = pause        X = exit        R = restart (after crash)
   ------------------------------------------------------------------ */
#include <windows.h>
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <time.h>

#define LANES      3
#define LANE_W     10
#define ROAD_X     20                      /* left border column      */
#define ROAD_R     (ROAD_X + LANES*LANE_W) /* right border column     */
#define TOP_ROW    1
#define BOT_ROW    24
#define CAR_W      6
#define CAR_H      4
#define PLAYER_Y   21                      /* top row of player's car */
#define NONE       -100                    /* "no car in this lane"   */
#define BLOCK      219                     /* solid block character   */
#define PANEL_X    55
#define MIN_DELAY  25

/* colours (same numbers as Turbo C) */
enum {
    C_BLACK = 0, C_BLUE = 1, C_LIGHTGRAY = 7, C_LIGHTGREEN = 10,
    C_LIGHTCYAN = 11, C_LIGHTRED = 12, C_YELLOW = 14, C_WHITE = 15
};

/* '#' = solid block, ' ' = empty.  Each car is 6 wide x 4 tall. */
const char *carShape[CAR_H] = {
    " #### ",
    "######",
    " #### ",
    "##  ##"
};

int enemyY[LANES];      /* top row of the enemy car in each lane      */
int player;             /* lane (0..2) of the player's car            */
int score, level, speed;

/* ---------- small console library (replaces conio.h of Turbo C) ---------- */

HANDLE hOut;
WORD   attr = 7;

void setColors(int fg, int bg)
{
    attr = (WORD)(fg | (bg << 4));
    SetConsoleTextAttribute(hOut, attr);
}

void gotoXY(int x, int y)                /* 1-based, like Turbo C */
{
    COORD c;
    c.X = (SHORT)(x - 1);
    c.Y = (SHORT)(y - 1);
    SetConsoleCursorPosition(hOut, c);
}

void putCh(char ch)
{
    DWORD n;
    WriteConsoleA(hOut, &ch, 1, &n, NULL);
}

void putStr(const char *fmt, ...)        /* printf-style output */
{
    char buf[128];
    DWORD n;
    va_list ap;
    va_start(ap, fmt);
    vsprintf(buf, fmt, ap);
    va_end(ap);
    WriteConsoleA(hOut, buf, (DWORD)lstrlenA(buf), &n, NULL);
}

void clearScreen(void)                   /* fills whole screen with current colour */
{
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    DWORD n, size;
    COORD origin;
    origin.X = 0;
    origin.Y = 0;
    GetConsoleScreenBufferInfo(hOut, &csbi);
    size = (DWORD)csbi.dwSize.X * (DWORD)csbi.dwSize.Y;
    FillConsoleOutputCharacterA(hOut, ' ', size, origin, &n);
    FillConsoleOutputAttribute(hOut, attr, size, origin, &n);
    SetConsoleCursorPosition(hOut, origin);
}

void showCursor(BOOL show)
{
    CONSOLE_CURSOR_INFO ci;
    ci.dwSize = 25;
    ci.bVisible = show;
    SetConsoleCursorInfo(hOut, &ci);
}

/* ---------- drawing helpers ---------- */

/* Draw a car at (lane, top row). color < 0 means erase it. */
void drawCar(int lane, int top, int color)
{
    int r, c, row, col;

    setColors(color >= 0 ? color : C_WHITE, C_BLACK);
    col = ROAD_X + lane * LANE_W + 2;

    for (r = 0; r < CAR_H; r++) {
        row = top + r;
        if (row < TOP_ROW || row > BOT_ROW)
            continue;                       /* clip at screen edges   */
        gotoXY(col, row);
        for (c = 0; c < CAR_W; c++) {
            if (color >= 0 && carShape[r][c] == '#')
                putCh((char)BLOCK);
            else
                putCh(' ');
        }
    }
}

/* Black road with double-line borders (drawn once per game). */
void drawRoad(void)
{
    int row, col;

    setColors(C_WHITE, C_BLACK);
    for (row = TOP_ROW; row <= BOT_ROW; row++) {
        for (col = ROAD_X + 1; col < ROAD_R; col++) {
            gotoXY(col, row);
            putCh(' ');
        }
        gotoXY(ROAD_X, row);
        putCh((char)186);
        gotoXY(ROAD_R, row);
        putCh((char)186);
    }
}

/* Dashed lane dividers; 'tick' makes them scroll to fake motion. */
void drawDividers(int tick)
{
    int row, l;

    setColors(C_YELLOW, C_BLACK);
    for (row = TOP_ROW; row <= BOT_ROW; row++) {
        for (l = 1; l < LANES; l++) {
            gotoXY(ROAD_X + l * LANE_W, row);
            if ((row + tick) % 4 < 2)
                putCh((char)179);
            else
                putCh(' ');
        }
    }
}

void drawPanel(void)
{
    setColors(C_LIGHTCYAN, C_BLUE);
    gotoXY(PANEL_X, 4);  putStr("== CAR RACING 3D ==");
    gotoXY(PANEL_X, 7);  putStr("Instructions");
    gotoXY(PANEL_X, 9);  putStr("Left/Right arrows : move");
    gotoXY(PANEL_X, 10); putStr("P : pause");
    gotoXY(PANEL_X, 11); putStr("X : exit");
    gotoXY(PANEL_X, 14); putStr("SCORE  :");
    gotoXY(PANEL_X, 16); putStr("LEVEL  :");
    gotoXY(PANEL_X, 18); putStr("STATUS :");
}

void showStats(void)
{
    setColors(C_WHITE, C_BLUE);
    gotoXY(64, 14); putStr("%-6d", score);
    gotoXY(64, 16); putStr("%-6d", level);
}

void showStatus(const char *msg)
{
    setColors(C_WHITE, C_BLUE);
    gotoXY(64, 18);
    putStr("%-11s", msg);
}

/* ---------- sound effects (Beep = frequency in Hz, time in ms) ---------- */

void crashSound(void)
{
    Beep(60, 110);
    Beep(80, 150);
    Beep(120, 210);
}

void pauseSound(void)
{
    Beep(1500, 210);
    Beep(350, 150);
    Beep(1000, 300);
}

/* ---------- game ---------- */

/* Plays one round. Returns 1 to restart, 0 to quit. */
int play(void)
{
    int i, key, tick = 0, crashed = 0, lane, busy;

    setColors(C_WHITE, C_BLUE);
    clearScreen();
    drawRoad();
    drawPanel();

    score = 0;
    level = 1;
    speed = 80;
    player = 1;
    for (i = 0; i < LANES; i++)
        enemyY[i] = NONE;
    showStats();
    showStatus("Playing...");

    while (!crashed) {

        /* ----- keyboard ----- */
        while (_kbhit()) {
            key = _getch();
            if (key == 0 || key == 224) {           /* extended key  */
                key = _getch();
                if (key == 75 && player > 0) {      /* left arrow    */
                    drawCar(player, PLAYER_Y, -1);
                    player--;
                } else if (key == 77 && player < LANES - 1) { /* right */
                    drawCar(player, PLAYER_Y, -1);
                    player++;
                }
            } else {
                key = tolower(key);
                if (key == 'x')
                    return 0;
                if (key == 'p') {
                    showStatus("Paused");
                    pauseSound();
                    _getch();
                    showStatus("Playing...");
                }
            }
        }

        /* ----- maybe spawn a new enemy car ----- */
        if (rand() % 6 == 0) {
            lane = rand() % LANES;
            busy = 0;                    /* lanes with a car near the top */
            for (i = 0; i < LANES; i++)
                if (enemyY[i] != NONE && enemyY[i] < 8)
                    busy++;
            /* always leave at least one free lane so the road is passable */
            if (enemyY[lane] == NONE && busy < LANES - 1)
                enemyY[lane] = -CAR_H + 1;   /* starts just above screen */
        }

        /* ----- move enemy cars and check collisions ----- */
        for (i = 0; i < LANES; i++) {
            if (enemyY[i] == NONE)
                continue;
            drawCar(i, enemyY[i], -1);               /* erase old     */
            enemyY[i]++;
            if (enemyY[i] > BOT_ROW) {               /* passed player */
                enemyY[i] = NONE;
                score += 10;
                level = score / 100 + 1;
                speed = 80 - (level - 1) * 5;
                if (speed < MIN_DELAY)
                    speed = MIN_DELAY;
                showStats();
            } else {
                drawCar(i, enemyY[i], C_LIGHTRED);
                if (i == player &&
                    enemyY[i] + CAR_H - 1 >= PLAYER_Y &&
                    enemyY[i] <= PLAYER_Y + CAR_H - 1)
                    crashed = 1;
            }
        }

        drawCar(player, PLAYER_Y, C_LIGHTGREEN);
        drawDividers(tick++);
        Sleep(speed);
    }

    /* ----- game over ----- */
    showStatus("Game Over");
    setColors(C_YELLOW, C_BLUE);
    gotoXY(PANEL_X, 21); putStr("R : play again");
    gotoXY(PANEL_X, 22); putStr("X : exit");
    crashSound();

    while (1) {
        key = _getch();
        if (key == 0 || key == 224) {
            _getch();
            continue;
        }
        key = tolower(key);
        if (key == 'r') return 1;
        if (key == 'x') return 0;
    }
}

int main(void)
{
    int again;

    system("mode con: cols=80 lines=25");        /* 80x25 text screen */
    hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleOutputCP(437);                     /* so block/line characters display */
    SetConsoleTitleA("Car Racing 3D");
    srand((unsigned)time(NULL));
    showCursor(FALSE);

    do {
        again = play();
    } while (again);

    showCursor(TRUE);
    setColors(C_LIGHTGRAY, C_BLACK);
    clearScreen();
    return 0;
}