#if !defined(WINDY_PLATFORM_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Davide Stasio $
   $Notice: (C) Copyright 2014 by Davide Stasio. All Rights Reserved. $
   ======================================================================== */
#include "windy_types.h"
#include "headers.h"

// @critical: this should probably be moved, or used in a different way.
// As it is, I think it's getting compiled twice
#define STB_TRUETYPE_IMPLEMENTATION 1
#include "stb_truetype.h"

#ifndef MAX_PATH
#define MAX_PATH 100
#endif

#define next_multiple_of_16(x) (16*(((i32)(x / 16))+1))

#include "hlsl_defines.h"
// @todo: check correct alignment
#pragma pack(push, 16)
struct Platform_Phong_Settings
{
    u32 flags;
    v3  color;
};
#pragma pack(pop)

#pragma pack(push, 4)
struct Platform_Debug_Shader_Settings
{
    u32 type;
    u32 _pad;
    u32 _pad2;
    u32 _pad3;
    v4  color;
    v4  positions[4];
};
#pragma pack(pop)

// @todo @cleanup: TEMPORARY HACK
struct Platform_Light_Buffer
{
    v3 color;
    r32 pad0_; // 16 bytes
    v3 p;      // 28 bytes
    r32 pad1_;
    v3 eye;    // 40 bytes
};

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
struct Platform_Shader
{
    void *vertex;
    void *pixel;

    Input_File vertex_file;
    Input_File  pixel_file;
};

struct Platform_Mesh_Buffers
{
    void *vert;
    void *index;
    void *platform;

    u16 index_count;
    u8  vert_stride;

    Wexp_Header *wexp;

    Platform_Phong_Settings settings;
};

// @todo: add guards for when 'handle == 0' (texture not initalized)
struct Platform_Texture
{
    u32   width;
    u32   height;
    u32   size;
    void *bytes;

    void *handle; // @note: (ID3D11Texture2D*)
    void *platform; // @note: optional (ID3D11ShaderResourceView*)
};

#define first_nonwhite_char '!'
#define last_nonwhite_char  '~'
#define n_supported_characters (1+last_nonwhite_char-first_nonwhite_char)
struct Platform_Font
{
    stbtt_fontinfo info;
    r32 height, scale;
    Platform_Texture chars[n_supported_characters];
};

#define PLATFORM_LOAD_RENDERER(name) void name(Platform_Renderer *renderer)
typedef PLATFORM_LOAD_RENDERER(Platform_Load_Renderer);

#define PLATFORM_RELOAD_SHADER(func) void func(Platform_Shader *shader, char *name)
typedef PLATFORM_RELOAD_SHADER(Platform_Reload_Shader);

#define PLATFORM_INIT_TEXTURE(name) void name(Platform_Texture *texture)
typedef PLATFORM_INIT_TEXTURE(Platform_Init_Texture);

// "shader" is needed for d3d11, can be null otherwise
#define PLATFORM_LOAD_WEXP(name) void name(Platform_Mesh_Buffers *buffers, Platform_Shader *shader)
typedef PLATFORM_LOAD_WEXP(Platform_Load_Wexp);

#define PLATFORM_INIT_SQUARE_MESH(name) void name(Platform_Shader *shader)
typedef PLATFORM_INIT_SQUARE_MESH(Platform_Init_Square_Mesh);


#define CLEAR_NONE    0
#define CLEAR_COLOR   0b001
#define CLEAR_DEPTH   0b010
#define CLEAR_STENCIL 0b100
#define CLEAR_ALL     CLEAR_COLOR|CLEAR_DEPTH|CLEAR_STENCIL
#define PLATFORM_CLEAR(name) void name(u32 what, v3 color, r32 depth, u8 stencil)
typedef PLATFORM_CLEAR(Platform_Clear);

#define PLATFORM_SET_ACTIVE_MESH(name) void name(Platform_Mesh_Buffers *buffers)
typedef PLATFORM_SET_ACTIVE_MESH(Platform_Set_Active_Mesh);

#define PLATFORM_SET_ACTIVE_TEXTURE(name) void name(Platform_Texture *texture)
typedef PLATFORM_SET_ACTIVE_TEXTURE(Platform_Set_Active_Texture);

#define PLATFORM_SET_ACTIVE_SHADER(name) void name(Platform_Shader *shader)
typedef PLATFORM_SET_ACTIVE_SHADER(Platform_Set_Active_Shader);

#define PLATFORM_SET_RENDER_TARGETS(name) void name()
typedef PLATFORM_SET_RENDER_TARGETS(Platform_Set_Render_Targets);

#define PLATFORM_SET_DEPTH_STENCIL(name) void name(b32 depth_enable, b32 stencil_enable, u32 stencil_ref_value)
typedef PLATFORM_SET_DEPTH_STENCIL(Platform_Set_Depth_Stencil);


// (0,0) = Top-Left; (global_width,global_height) = Bottom-Right
// @todo: test sub-pixel placement with AA.
#define PLATFORM_DRAW_RECT(name) void name(Platform_Shader *shader, v2 size, v2 pos)
typedef PLATFORM_DRAW_RECT(Platform_Draw_Rect);

#define PLATFORM_DRAW_TEXT(name) void name(Platform_Shader *shader, Platform_Font *font, char *text, v2 pivot)
typedef PLATFORM_DRAW_TEXT(Platform_Draw_Text);

#define PLATFORM_DRAW_MESH(name) void name(Platform_Mesh_Buffers *mesh, m4 *model_transform, Platform_Shader *shader, \
                                           m4 *in_camera, m4 *in_screen, Platform_Light_Buffer *light, v3 *eye, bool wireframe_overlay)
typedef PLATFORM_DRAW_MESH(Platform_Draw_Mesh);

#define PLATFORM_DRAW_LINE(name) void name(v3 a, v3 b, v4 color, bool on_top, m4 *camera_transform, m4 *screen_transform)
typedef PLATFORM_DRAW_LINE(Platform_Draw_Line);


struct Platform_Renderer
{
    Platform_Load_Renderer    *load_renderer;
    Platform_Reload_Shader    *reload_shader;
    Platform_Init_Texture     *init_texture;
    Platform_Load_Wexp        *load_wexp;
    Platform_Init_Square_Mesh *init_square_mesh;

    Platform_Clear              *clear;
    Platform_Set_Active_Mesh    *set_active_mesh;
    Platform_Set_Active_Texture *set_active_texture;
    Platform_Set_Active_Shader  *set_active_shader;
    Platform_Set_Render_Targets  *set_render_targets;
    Platform_Set_Depth_Stencil   *set_depth_stencil;

    Platform_Draw_Rect *draw_rect;
    Platform_Draw_Text *draw_text;
    Platform_Draw_Mesh *draw_mesh;
    Platform_Draw_Line *draw_line;

    Platform_Mesh_Buffers  square;
    Platform_Shader        debug_shader;

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

enum Gamemode
{
    GAMEMODE_GAME,
    GAMEMODE_EDITOR,
    GAMEMODE_MENU
};

#define GAME_UPDATE_AND_RENDER(name) void name(Input *input, r32 dtime, Platform_Renderer *renderer, Game_Memory *memory, Gamemode *gamemode, u32 width, u32 height)
typedef GAME_UPDATE_AND_RENDER(Game_Update_And_Render);

#define WINDY_PLATFORM_H
#endif
