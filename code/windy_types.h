#if !defined(WINDY_TYPES_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Davide Stasio $
   $Notice: (C) Copyright 2020 by Davide Stasio. All Rights Reserved. $
   ======================================================================== */
#include <stdint.h>

#define global static
#define internal static
#define local_persist static

#define Kilobytes(k) (1024LL*(k))
#define Megabytes(m) (1024LL*Kilobytes(m))
#define Gigabytes(g) (1024LL*Megabytes(g))
#define Terabytes(t) (1024LL*Gigabytes(t))

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t  i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef i32 b32;

typedef size_t memory_index;

typedef float  r32;
typedef double r64;

union v2i
{
    struct { i32 x, y; };
    i32    row[2];
};

union v2
{
    struct { r32 x, y; };
    r32    row[2];
};

union v3
{
    struct { r32 x, y, z; };
    r32    row[3];
    v2     xy;

    operator bool()
    {
        return x||y||z;
    }

};

union v4
{
    struct { r32 x, y, z, w; };
    struct { v3 xyz; r32 _w; };
    r32    row[4];
};

union m4
{
    r32 m[4][4];
    v4  col [4];
};

#define WINDY_TYPES_H
#endif
