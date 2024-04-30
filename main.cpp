#include <windows.h>
#include <iostream>
#include <math.h>
#include <conio.h>
#define PI 3.14159265359

HANDLE hOut;
int BUFFER_WIDTH;
int BUFFER_HEIGHT;
char* TXTBUFFER;



bool setConsoleFontSize(int width, int height) {
    CONSOLE_FONT_INFOEX fontInfo;
    fontInfo.cbSize = sizeof(fontInfo);
    if (!GetCurrentConsoleFontEx(hOut, FALSE, &fontInfo)) {

        return false;
    }
    fontInfo.dwFontSize.X = width;
    fontInfo.dwFontSize.Y = height;
    if (!SetCurrentConsoleFontEx(hOut, FALSE, &fontInfo)) {

        return false;
    }
    return true;
}

BOOL WINAPI ConsoleCtrlHandler(DWORD dwCtrlType) {
    switch (dwCtrlType) {
        case CTRL_CLOSE_EVENT:
            delete[] TXTBUFFER;
            return TRUE; 
        default:
            return FALSE; 
    }
}

void initializeWindow(const char* p, int width, int height, int fontwidth, int fontheight) {
    SetConsoleTitle(TEXT(p));
    hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    setConsoleFontSize(fontwidth,fontheight);
    if (hOut == INVALID_HANDLE_VALUE) { return; }
    DWORD dwMode = 0;
    if (!GetConsoleMode(hOut, &dwMode)) { return; }
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    if (!SetConsoleMode(hOut, dwMode)) { return; }
    HWND console = GetConsoleWindow();
    RECT r;
    GetWindowRect(console, &r);
    MoveWindow(console, r.left, r.top, width, height, TRUE);
    CONSOLE_SCREEN_BUFFER_INFO scrBufferInfo;
    GetConsoleScreenBufferInfo(hOut, &scrBufferInfo);
    short winWidth = scrBufferInfo.srWindow.Right - scrBufferInfo.srWindow.Left + 1;
    short winHeight = scrBufferInfo.srWindow.Bottom - scrBufferInfo.srWindow.Top + 1;
    short scrBufferWidth = scrBufferInfo.dwSize.X;
    short scrBufferHeight = scrBufferInfo.dwSize.Y;
    COORD newSize;
    newSize.X = scrBufferWidth;
    newSize.Y = winHeight;
    int Status = SetConsoleScreenBufferSize(hOut, newSize);
    GetConsoleScreenBufferInfo(hOut, &scrBufferInfo);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hOut, &cursorInfo);
    cursorInfo.bVisible = false;
    SetConsoleCursorInfo(hOut, &cursorInfo);
    BUFFER_WIDTH = scrBufferInfo.srWindow.Right - scrBufferInfo.srWindow.Left + 1;
    BUFFER_HEIGHT = scrBufferInfo.srWindow.Bottom - scrBufferInfo.srWindow.Top + 1;
    TXTBUFFER = new char[BUFFER_WIDTH * BUFFER_HEIGHT];
    SetConsoleCtrlHandler(ConsoleCtrlHandler, TRUE);
}




void plot(int x, int y, char p){
    int index = y*BUFFER_WIDTH+x;
    if(index>0 && index<BUFFER_WIDTH*BUFFER_HEIGHT){TXTBUFFER[index]=p;}
}

void vline(int x, int y, char p, int height){
    for(int h=0;h<height;h++){
        plot(x,y+h,p);
    }
}

void line(int x1, int y1, int x2, int y2, char p) {
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = x1 < x2 ? 1 : -1;
    int sy = y1 < y2 ? 1 : -1;
    int err = dx - dy;

    while (x1 != x2 || y1 != y2) {
        plot(x1, y1, p);
        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x1 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y1 += sy;
        }
    }
    plot(x2, y2, p); // plot the end point
}

void rect(int x, int y, int width, int height, char p){
    for(int w=0;w<width;w++){
        for(int h=0;h<height;h++){
            plot(x+w,y+h,p);
        }
    }
}

void clear() {
    std::fill(TXTBUFFER, TXTBUFFER + BUFFER_WIDTH * BUFFER_HEIGHT, ' ');

}

void render() {
    COORD C;C.X=0;C.Y=0;
    SetConsoleCursorPosition(hOut, C); 
    DWORD written;
    WriteConsole(hOut, TXTBUFFER, BUFFER_WIDTH * BUFFER_HEIGHT, &written, NULL); 
}



int xdim = 12;
int ydim = 15;
int map[15][12] = {
    {3,2,1,1,1,1,1,1,1,1,2,3},
    {2,0,0,0,0,0,0,0,0,0,0,2},
    {1,0,0,0,0,1,2,0,0,0,0,1},
    {1,0,0,3,0,0,0,0,3,0,0,1},
    {1,0,0,2,0,0,0,0,2,0,0,1},
    {1,0,0,1,0,0,0,0,1,0,0,1},
    {1,0,0,1,0,0,0,0,1,0,0,1},
    {1,0,0,1,0,0,0,0,1,0,0,1},
    {1,0,0,1,0,0,0,0,1,0,0,1},
    {1,0,0,1,0,0,0,0,1,0,0,1},
    {1,0,0,2,0,0,0,0,2,0,0,1},
    {1,0,0,3,0,0,0,0,3,0,0,1},
    {1,0,0,0,0,1,2,0,0,0,0,1},
    {2,0,0,0,0,0,0,0,0,0,0,2},
    {3,2,1,1,1,1,1,1,1,1,2,3}
};

float playerx;
float playery;
float fov;
float angInc;
float angSpeed;
float playerangle;
float playerspeed;
int cw;
int ch;
float distanceToProj;

void movePlayer(int key) {
    float dist = 1;
    switch (key) {
        case 72:
            playerx += cos(playerangle) * dist;
            playery += sin(playerangle) * dist;
            break;
        case 80:
            playerx -= cos(playerangle) * dist;
            playery -= sin(playerangle) * dist;
            break;
        case 75:
            playerangle -= angSpeed;
            if (playerangle < 0)
                playerangle += 2 * PI;
            break;
        case 77:
            playerangle += angSpeed;
            if (playerangle >= 2 * PI)
                playerangle -= 2 * PI;
            break;
    }
}

void checkInput() {
    if (_kbhit()) {
        int key = _getch();
        movePlayer(key);
    }
}

struct Raycast{
    float x;
    float y;
    char p;
};

char texture(int n){
    switch (n) {
        case 1: 
            return '#';
            break;
        case 2: 
            return '*';
            break;
        case 3: 
            return '-'; 
            break;
        default:
            return ' ';
            break;
    }
}

Raycast raycast(float angle) {
    float px = playerx;
    float py = playery;
    float dx = 0.5*cos(angle);
    float dy = 0.5*sin(angle);
    while(true){
        if(map[(int)(py/ch)][(int)(px/cw)] != 0){
            break;
        }
        px+=dx;
        py+=dy;
    }
    return Raycast{px,py,texture(map[(int)(py/ch)][(int)(px/cw)])};
}



void drawmap(float scale = 0.3) {
    int cw = BUFFER_WIDTH / xdim;
    int ch = BUFFER_HEIGHT / ydim;
    float anglestart = playerangle - fov / 2;
    for (int i = 0; i < BUFFER_WIDTH; i++) {
        float ang = anglestart + angInc * i;
        auto [x2, y2, p] = raycast(ang);
        line(playerx * scale, playery * scale, x2 * scale, y2 * scale, '@');
        float distance = sqrt(pow(x2 - playerx, 2) + pow(y2 - playery, 2)) * cos(ang - playerangle) ;
        float projectedheight = (ch / distance) * distanceToProj;
        vline(i, BUFFER_HEIGHT / 2 - projectedheight / 2, p, projectedheight);
    }
    for (int x = 0; x < xdim; x++) {
        for (int y = 0; y < ydim; y++) {
            if (map[y][x] !=0 ) {
                rect(x * cw * scale, y * ch * scale, cw * scale, ch * scale, texture(map[y][x]));
            }
        }
    }
}



int main() {

    initializeWindow("RAYCASTER", 800, 600,3,3);
    playerx = BUFFER_WIDTH/2;
    playery = BUFFER_HEIGHT/2;
    fov = PI/3;
    angInc = fov/BUFFER_WIDTH;
    playerangle = PI;
    playerspeed = 2;
    angSpeed = 0.2;
    cw = BUFFER_WIDTH/xdim;
    ch = BUFFER_HEIGHT/ydim;
    distanceToProj = (BUFFER_WIDTH/2)/tan(fov/2);
    while(true) { 
        checkInput();
        clear();
        drawmap();
        render();
    }
    
}
