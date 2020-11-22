#if !defined(WINDY_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Davide Stasio $
   $Notice: (C) Copyright 2020 by Davide Stasio. All Rights Reserved. $
   ======================================================================== */
#include "windy_platform.h"

#if WINDY_DEBUG
#define Assert(expr) do {if(!(expr)) {*(int *)0 = 0;}} while(0)
#else
#define Assert(expr)
#endif

#define byte_offset(base, offset) ((u8*)(base) + (offset))
inline u16 truncate_to_u16(u32 v) {Assert(v <= 0xFFFF); return (u16)v; };
 
struct Wallet_State
{
};

struct Memory_Pool
{
    memory_index size;
    memory_index used;
    u8 *base;
};

#define push_struct(pool, type) (type *)push_size_((pool), sizeof(type))
#define push_array(pool, length, type) (type *)push_size_((pool), (length)*sizeof(type))
inline void *
push_size_(Memory_Pool *pool, memory_index size)
{
    Assert((pool->used + size) < pool->size);

    void *result = pool->base + pool->used;
    pool->used += size;
    return(result);
}

#define WINDY_H
#endif
