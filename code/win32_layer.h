#if !defined(WIN32_LAYER_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Davide Stasio $
   $Notice: (C) Copyright 2014 by Davide Stasio. All Rights Reserved. $
   ======================================================================== */
#include "windy_types.h"

#if WINDY_DEBUG
#define Assert(expr) do {if(!(expr)) {*(int *)0 = 0;}} while(0)
#define AssertKeep(expr) Assert(expr)
#else
#define Assert(expr)
#define AssertKeep(expr)  {(expr);}
#endif

#define byte_offset(base, offset) ((u8*)(base) + (offset))
inline u16 truncate_to_u16(u32 v) {Assert(v <= 0xFFFF); return (u16)v; };
#define next_multiple_of_16(x) (16*(((i32)(x / 16))+1))

#ifndef MAX_PATH
#define MAX_PATH 100
#endif

struct Input_File
{
    char *path;
    u8   *data;
    u64   size;
    u64   write_time;  // @todo: this should be useless in release build
};

struct Platform_Renderer
{
    void *platform;
};

typedef struct Game_Memory
{
    b32 is_initialized;

    u64 storage_size;
    void *storage;
} game_memory;
 
struct Memory_Pool
{
    memory_index size;
    memory_index used;
    u8 *base;
};

#define push_struct(type) (type *)push_size_((&global_memory), sizeof(type))
#define push_array(length, type) (type *)push_size_((&global_memory), (length)*sizeof(type))
inline void *
push_size_(Memory_Pool *pool, memory_index size)
{
    Assert((pool->used + size) < pool->size);

    void *result = pool->base + pool->used;
    pool->used += size;
    return(result);
}

#define WIN32_LAYER_H
#endif
