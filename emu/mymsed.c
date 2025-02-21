#include <raylib.h>
#include <stdbool.h>
#include <stdlib.h>
// #define _POSIX_C_SOURCE 200808L
#include <time.h>

#define BTH_IO_IMPLEMENTATION
#include "../utils/bth_io.h"

#define BTH_STRING_IMPLEMENTATION
#include "../utils/bth_string.h"

#define CLAY_IMPLEMENTATION
#include "clay/clay.h"
#include "clay/renderer.h"

#define EMUL_LOG(fmt, ...) TraceLog(LOG_WARNING, TextFormat(fmt, __VA_ARGS__));
#include "emul.h"

#define WIN_W 1280
#define WIN_H 960
#define TITLE "My Motorola 68k Debugger"

// #define BG1 (Color){ 33, 33, 38, 255 } // cyan gray idk i'm bad at colors
#define BG (Color){ 20, 24, 30, 255 } // darkish blue
#define FG1 (Color){ 37, 43, 49, 255 } // lighter BG
#define FG2 (Color){ 56, 61, 65, 255 } // gray

#define CLAYCOLOR(c) {c.r, c.g, c.b, c.a}
#define CLAY_RED CLAYCOLOR(RED)
#define CLAY_BLACK CLAYCOLOR(BLACK)
#define CLAY_WHITE CLAYCOLOR(RAYWHITE)
#define CLAY_GRAY CLAYCOLOR(DARKGRAY)
#define BOX_SIZING_GROW() \
    {.width=CLAY_SIZING_GROW({}),.height=CLAY_SIZING_GROW({})}

#define FONT_ID_BODY 0
#define FONT_PATH "/usr/share/fonts/truetype/ProggyClean.ttf"
#define CLAY_CSTR(t, l) CLAY__INIT(Clay_String){.length=(l), .chars=(t)}
#define CLAY_IDS(label, len) Clay__AttachId(Clay__HashString(CLAY__INIT(Clay_String){.length = len, .chars = (label)}, 0, 0))

#define MEMVIEW_ID "memView"

typedef enum {
    MY_TEXTBOX
} My_FocusType;

typedef struct {
    bool focusing;
    Vector2 pos;
    My_FocusType type;
    void *object;
} My_Focus;

typedef struct {
    Clay_Dimensions dimensions;
    Vector2 mousePosition;
    Vector2 scrollDelta;
    size_t frameCount;
    My_Focus focus;
    bool shouldClose;
    bool debugMode;
} My_UiStates;

typedef enum {
    MY_TB_LOCKED,
    MY_TB_VIEWING,
    MY_TB_CAPTURING,
} My_TextBoxState;

typedef struct {
    My_TextBoxState state;
    const char *id;
    char *buffer;
    size_t lineFold;
    size_t lineStart;
    size_t cursor;
    size_t textLen;
    size_t bufferSize;
} My_TextBox;

// typedef struct {
//     My_TextBox memView;
// } My_UiElements;

static My_UiStates uiStates;

static My_TextBox memView = {
    .state = MY_TB_LOCKED,
    .id = MEMVIEW_ID,
    .lineFold = 50,
};

static m68k_cpu CPU;

static Clay_Arena CLAY_MEMORY;

void My_InitUi(void)
{
    SetTraceLogLevel(LOG_WARNING);
    Clay_RaylibInitialize(WIN_W, WIN_H, TITLE, FLAG_WINDOW_RESIZABLE);

    uint64_t clayMinMem = Clay_MinMemorySize();
    CLAY_MEMORY = (Clay_Arena) {
        .memory = calloc(clayMinMem, 1),
        .capacity = clayMinMem,
    };

    uiStates.dimensions.width = GetScreenWidth();
    uiStates.dimensions.height = GetScreenHeight();
    uiStates.focus.focusing = false;
    uiStates.shouldClose = false;
    uiStates.debugMode = false;
    uiStates.frameCount = 0;
    
    Clay_Initialize(CLAY_MEMORY, uiStates.dimensions);

    SetTargetFPS(60);

    Clay_SetMeasureTextFunction(Raylib_MeasureText);

    Raylib_fonts[FONT_ID_BODY] = (Raylib_Font) {
        .font = LoadFontEx(FONT_PATH, 48, 0, 0),
        .fontId = FONT_ID_BODY
    };

    SetWindowMonitor(0);
    SetExitKey(KEY_NULL);

    SetTraceLogLevel(LOG_ALL);
}

static inline void My_ClayUpdate(void)
{
    Clay_SetLayoutDimensions(uiStates.dimensions);

    Clay_SetPointerState(
        uiStates.mousePosition, 
        IsMouseButtonDown(MOUSE_LEFT_BUTTON)
    );

    Clay_UpdateScrollContainers(
        true,
        uiStates.scrollDelta,
        GetFrameTime()
    );
}

// calloc's `textBox` internal buffer to desired `size`
static inline void My_TextBoxInit(My_TextBox *textBox, size_t size)
{
    textBox->buffer = calloc(size, 1);
    textBox->cursor = 0;
    textBox->lineStart = 0;
    textBox->textLen = 0;
    textBox->bufferSize = size;
    textBox->state = MY_TB_VIEWING;
}

void My_TextboxPointerHandler(
    Clay_ElementId id,
    Clay_PointerData pointerData,
    intptr_t userData)
{
    My_TextBox *textBox = (My_TextBox *)userData;

    if (textBox->state == MY_TB_LOCKED)
    {
        TraceLog(LOG_TRACE, "target is LOCKED");
        return;
    }
    
    if (pointerData.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME &&
        textBox->state == MY_TB_VIEWING)
    {
        SetMouseCursor(MOUSE_CURSOR_IBEAM);

        textBox->state = MY_TB_CAPTURING;
        TraceLog(LOG_TRACE, "target is now CAPTURING");

        uiStates.focus = (My_Focus){
            .focusing = true,
            .pos = 0,
            .type = MY_TEXTBOX,
            .object = textBox
        };

        TraceLog(LOG_TRACE, "target is FOCUSED");
    }
}

static inline void __TextBoxDeleteChar(My_TextBox *textBox)
{
    textBox->cursor--;

    if (textBox->buffer[textBox->cursor] == '\n')
    {
        char *bol = findrchr(
            textBox->buffer + textBox->cursor - 1,
            textBox->buffer,
            '\n'
        );

        textBox->lineStart = bol == textBox->buffer ? 
            0 : (bol - textBox->buffer + 1);
    }

    textBox->buffer[textBox->cursor] = '\0';
    textBox->textLen--;
}

void My_TextBoxCaptureInput(My_TextBox *textBox)
{
    if (textBox->state != MY_TB_CAPTURING)
        return;

    static int key;
    static int chr;

    static int c1;

    int nkey = GetKeyPressed();

    if (key == KEY_NULL || IsKeyUp(key) || nkey != KEY_NULL)
    {
        key = nkey;
        chr = GetCharPressed();
        c1 = 12 * GetFPS() / 60; // 12 is kinda good when at 60FPS
    }
    else
    {

        if (--c1)
            return;

        c1 = 2 * GetFPS() / 60; // same story here
    }

    if (key == KEY_ESCAPE)
    {
        TraceLog(LOG_TRACE, "target is now VIEWING");
        SetMouseCursor(MOUSE_CURSOR_DEFAULT);
        textBox->state = MY_TB_VIEWING;
        uiStates.focus.focusing = false;
        TraceLog(LOG_TRACE, "target is no longer FOCUSED");

        return;
    }

    if (IsKeyDown(KEY_F1))
    {
        textBox->buffer[textBox->textLen] = '\0';
        TraceLog(
            LOG_TRACE,
            TextFormat("{\n%s\n}", textBox->buffer)
        );
    }
    else if ((chr >= ' ' && chr <= '~') || key == KEY_ENTER)
    {
        if (textBox->cursor >= textBox->bufferSize - 1)
        {
            TraceLog(
                LOG_WARNING,
                TextFormat("%s: prevented action would overflow\n")
            );
            return;
        }
        
        if (textBox->cursor - textBox->lineStart 
            == textBox->lineFold || key == KEY_ENTER)
        {
            textBox->buffer[textBox->cursor++] = '\n';
            textBox->lineStart = textBox->cursor;
            textBox->textLen++;
        }

        if (key != KEY_ENTER)
        {
            textBox->buffer[textBox->cursor++] = chr;
            textBox->textLen++;
        }
    }
    else if (key == KEY_BACKSPACE && textBox->textLen > 0)
    {
        __TextBoxDeleteChar(textBox);

        if (textBox->textLen > 0 
            && textBox->cursor % textBox->lineFold == 0)
            __TextBoxDeleteChar(textBox);
    }
    else if (key == KEY_LEFT && textBox->cursor > 0)
    {
        textBox->cursor--;
    }
    else if (key == KEY_RIGHT && textBox->cursor < textBox->textLen)
    {
        textBox->cursor++;
    }
}

static inline void My_InputHandler(void)
{
    if (uiStates.focus.focusing)
    {
        My_TextBoxCaptureInput(uiStates.focus.object);
    }
    else
    {
        int key = GetKeyPressed();

        uiStates.shouldClose = WindowShouldClose() || key == KEY_ESCAPE;
        uiStates.debugMode ^= IsKeyPressed(KEY_F2);
    }
}

static inline void My_UpdateUiStates(void)
{
    uiStates.dimensions.width = GetScreenWidth();
    uiStates.dimensions.height = GetScreenHeight();
    uiStates.mousePosition = GetMousePosition();
    uiStates.scrollDelta = GetMouseWheelMoveV();

    My_InputHandler();
}

int main(void)
{
    My_InitUi();
    My_TextBoxInit(&memView, M68K_MEM * 2);
    
    char *text = NULL;
    size_t textLen = readfn(&text, 0, "../samples/sample2.srec");
    size_t linesCount = 0;
    char **lines = getnlines(text, &linesCount);

    uint8_t *cpuMem = calloc(M68K_MEM, 1);
    CPU.mem = cpuMem;

    int flashed = m68k_flash(&CPU, lines, linesCount);

    fdumpf(memView.buffer, (char *)cpuMem, 100);
    memView.textLen = 200;

    while (!uiStates.shouldClose)
    {
        My_UpdateUiStates();
        My_ClayUpdate();
        
        Clay_SetDebugModeEnabled(uiStates.debugMode);

        Clay_BeginLayout();
        CLAY(
            CLAY_ID("root"),
            CLAY_RECTANGLE({ .color = CLAYCOLOR(BG) }),
            CLAY_LAYOUT({
                .layoutDirection = CLAY_TOP_TO_BOTTOM,
                .sizing = BOX_SIZING_GROW(),
            })
        ) {
            CLAY(
                CLAY_ID("menuBar"),
                CLAY_RECTANGLE({
                    .color = CLAYCOLOR(FG1)
                }),
                CLAY_BORDER({.bottom={ .width = 1, .color=CLAY_GRAY }}),
                CLAY_LAYOUT({ 
                    .sizing = {
                        .width = CLAY_SIZING_GROW({}),
                        .height = CLAY_SIZING_FIXED(31)
                    }
                })
            ) {}
            CLAY(
                CLAY_ID(MEMVIEW_ID),
                // CLAY_BORDER_OUTSIDE({ .width = 1, .color=CLAYCOLOR(RED) }),
                CLAY_LAYOUT({
                    .padding = { 8, 8 },
                    .sizing = { 
                        .width = CLAY_SIZING_FIXED(GetScreenWidth() / 2),
                        .height = CLAY_SIZING_GROW({})
                    } 
                }),
                CLAY_BORDER({ .right = { .width = 1, .color = CLAY_GRAY } }),
                Clay_OnHover(My_TextboxPointerHandler, (intptr_t)&memView)
            ) {
                CLAY(
                    CLAY_TEXT(
                        CLAY_CSTR(memView.buffer, memView.textLen),
                        CLAY_TEXT_CONFIG({
                            .fontId = FONT_ID_BODY,
                            .fontSize = 24,
                            .textColor = CLAYCOLOR(RAYWHITE),
                            .wrapMode = CLAY_TEXT_WRAP_NEWLINES,
                            .letterSpacing = 0
                        })
                    )
                ) {}
            }
        }
        Clay_RenderCommandArray layout = Clay_EndLayout();

        Vector2 textSize = MeasureTextEx(
            Raylib_fonts[FONT_ID_BODY].font,
            "0123456789ABCDEF",
            24, 0
        );

        textSize.x /= 16;

        BeginDrawing();

        Clay_RaylibRender(layout);

        DrawRectangle(
            8 + (memView.cursor - memView.lineStart) * textSize.x,
            31 + 8 + (memView.cursor / memView.lineFold) * textSize.y,
            2,
            15,
            RED
        );

        EndDrawing();
        uiStates.frameCount++;
    }

    free(CLAY_MEMORY.memory);
    free(text);
    free(lines);
    free(cpuMem);
    free(memView.buffer);
    SetTraceLogLevel(LOG_WARNING);
    CloseWindow();
}
