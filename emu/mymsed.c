#include <raylib.h>
#include <stdlib.h>

#define CLAY_IMPLEMENTATION
#include "clay/clay.h"
#include "clay/renderer.h"

#define WIN_W 640
#define WIN_H 480
#define TITLE "My Motorola 68k Debugger"

// #define BG1 (Color){ 33, 33, 38, 255 } // cyan gray idk i'm bad at colors
#define BG (Color){ 20, 24, 30, 255 } // darkish blue
#define FG1 (Color){ 37, 43, 49, 255 } // lighter BG
#define FG2 (Color){ 56, 61, 65, 255 } // gray

#define CLAYCOLOR(c) {c.r, c.g, c.b, c.a}
#define BOX_SIZING_GROW() \
    {.width=CLAY_SIZING_GROW({}),.height=CLAY_SIZING_GROW({})}

int main(void)
{
    SetTraceLogLevel(LOG_WARNING);
    Clay_RaylibInitialize(WIN_W, WIN_H, TITLE, FLAG_WINDOW_RESIZABLE);

    uint64_t clay_minmem = Clay_MinMemorySize();
    Clay_Arena clay_mem = (Clay_Arena) {
        .memory = malloc(clay_minmem),
        .capacity = clay_minmem,
    };

    Clay_Initialize(clay_mem, (Clay_Dimensions) {
        .width = GetScreenWidth(),
        .height = GetScreenHeight(),
    });

    SetTargetFPS(60);

    SetTraceLogLevel(LOG_TRACE);

    while (!WindowShouldClose())
    {
        Clay_SetLayoutDimensions((Clay_Dimensions){
            .width = GetScreenWidth(),
            .height = GetScreenHeight()
        });

        Clay_BeginLayout();

        CLAY(
            CLAY_RECTANGLE({ .color = CLAYCOLOR(BG) }),
            CLAY_LAYOUT({
                .layoutDirection = CLAY_TOP_TO_BOTTOM,
                .sizing = BOX_SIZING_GROW(),
            })
        ) {
            CLAY(
                CLAY_ID("menu_bar"),
                CLAY_RECTANGLE({
                    .color = CLAYCOLOR(FG1)
                }),
                CLAY_BORDER_OUTSIDE({ .width = 1, .color=CLAYCOLOR(BLACK) }),
                CLAY_LAYOUT({ 
                    .sizing = {
                        .width = CLAY_SIZING_GROW({}),
                        .height = CLAY_SIZING_FIXED(32)
                    }
                })
            ) {
            }
        }

        Clay_RenderCommandArray baked = Clay_EndLayout();

        BeginDrawing();
        Clay_RaylibRender(baked);
        EndDrawing();
    }

    free(clay_mem.memory);
    SetTraceLogLevel(LOG_WARNING);
    CloseWindow();
}
