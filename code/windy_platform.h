#if !defined(WINDY_PLATFORM_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Davide Stasio $
   $Notice: (C) Copyright 2014 by Davide Stasio. All Rights Reserved. $
   ======================================================================== */
#include "windy_types.h"

// @critical: this should probably be moved, or used in a different way.
// As it is, I think it's getting compiled twice
//#define STB_TRUETYPE_IMPLEMENTATION 1
//#include "stb_truetype.h"

#ifndef MAX_PATH
#define MAX_PATH 100
#endif

#define next_multiple_of_16(x) (16*(((i32)(x / 16))+1))

struct Input_File
{
    char *path;
    u8   *data;
    u64   size;
    u64   write_time;  // @todo: this should be useless in release build
};

#define PLATFORM_READ_FILE(name) Input_File name(char *Path)
typedef PLATFORM_READ_FILE(Platform_Read_File);
#define PLATFORM_RELOAD_CHANGED_FILE(name) b32 name(Input_File *file)
typedef PLATFORM_RELOAD_CHANGED_FILE(Platform_Reload_Changed_File);

struct Platform_Renderer;

#define PLATFORM_LOAD_RENDERER(name) void name(Platform_Renderer *renderer)
typedef PLATFORM_LOAD_RENDERER(Platform_Load_Renderer);

#define PLATFORM_SET_RENDER_TARGETS(name) void name()
typedef PLATFORM_SET_RENDER_TARGETS(Platform_Set_Render_Targets);

#define PLATFORM_CLEAR(name) void name(v3 color)
typedef PLATFORM_CLEAR(Platform_Clear);


struct Platform_Renderer
{
    Platform_Load_Renderer       *load_renderer;
    Platform_Set_Render_Targets  *set_render_targets;
    Platform_Clear               *clear;

    void *platform;
};

struct Input_Keyboard
{
    u32 up;
    u32 down;
    u32 left;
    u32 right;

    u32 w;
    u32 a;
    u32 s;
    u32 d;

    u32 f;
    u32 g;
    u32 x;
    u32 y;
    u32 z;

    u32 space;
    u32 shift;
    u32 ctrl;
    u32 alt;
    u32 esc;

    u32 mouse_middle;
    u32 mouse_left;
    u32 mouse_right;
};

// @note: position is normalized: (0;0) top-left, (1,1)bottom-right
//        delta goes from (-WINDY_WIN32_MOUSE_SENSITIVITY;-WINDY_WIN32_MOUSE_SENSITIVITY) to (+WINDY_WIN32_MOUSE_SENSITIVITY;+WINDY_WIN32_MOUSE_SENSITIVITY)
struct Input_Mouse
{
    union {
        struct { r32 x; r32 y; };
        v2 p;
    };
    union {
        struct { r32 dx; r32 dy; };
        v2 dp;
    };
    i16 wheel;
};

struct Input
{
    Input_Keyboard pressed;
    Input_Keyboard held;

    Input_Mouse mouse;
};

typedef struct Game_Memory
{
    b32 is_initialized;

    u64 storage_size;
    void *storage;

    Platform_Read_File           *read_file;
    Platform_Reload_Changed_File *reload_if_changed;
} game_memory;

#define GAME_UPDATE_AND_RENDER(name) void name(Game_Memory *memory)
typedef GAME_UPDATE_AND_RENDER(Game_Update_And_Render);

#define WINDY_PLATFORM_H
#endif
