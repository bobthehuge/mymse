#pragma once

#include <raylib.h>
#include <stdint.h>
#include "clay.h"

#define CLAY_RECTANGLE_TO_RAYLIB_RECTANGLE(rectangle) (Rectangle) { .x = rectangle.x, .y = rectangle.y, .width = rectangle.width, .height = rectangle.height }
#define CLAY_COLOR_TO_RAYLIB_COLOR(color) (Color) { .r = (unsigned char)roundf(color.r), .g = (unsigned char)roundf(color.g), .b = (unsigned char)roundf(color.b), .a = (unsigned char)roundf(color.a) }

typedef struct
{
    uint32_t fontId;
    Font font;
} Raylib_Font;
 
typedef enum
{
    CUSTOM_LAYOUT_ELEMENT_TYPE_3D_MODEL
} CustomLayoutElementType;

typedef struct
{
    Model model;
    float scale;
    Vector3 position;
    Matrix rotation;
} CustomLayoutElement_3DModel;

typedef struct
{
    CustomLayoutElementType type;
    union {
        CustomLayoutElement_3DModel model;
    };
} CustomLayoutElement;

Ray GetScreenToWorldPointWithZDistance(
    Vector2 position, Camera camera, int screenWidth, 
    int screenHeight, float zDistance
);

extern Raylib_Font Raylib_fonts[10];
extern Camera Raylib_camera;

// static inline Clay_Dimensions Raylib_MeasureText(
Clay_Dimensions Raylib_MeasureText(
    Clay_String *text, Clay_TextElementConfig *config
);

void Clay_RaylibInitialize(
    int width, int height, const char *title, unsigned int flags
);

void Clay_RaylibRender(Clay_RenderCommandArray renderCommands);
