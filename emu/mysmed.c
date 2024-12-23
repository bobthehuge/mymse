#include <raylib.h>

#define WIN_W 640
#define WIN_H 480
#define TITLE "My Motorola 68k Debugger"

int main(void)
{
    SetTraceLogLevel(LOG_WARNING);

    InitWindow(WIN_W, WIN_H, TITLE);
    SetTargetFPS(60);

    SetTraceLogLevel(LOG_TRACE);

    while(!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(RED);
        EndDrawing();
    }
    
    SetTraceLogLevel(LOG_WARNING);
    CloseWindow();
}
