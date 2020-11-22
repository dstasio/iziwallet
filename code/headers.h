#if !defined(HEADERS_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Davide Stasio $
   $Notice: (C) Copyright 2020 by Davide Stasio. All Rights Reserved. $
   ======================================================================== */

#pragma pack(push, 1)
struct Bitmap_Header
{
    u16 Signature;
    u32 FileSize;
    u32 Reserved;
    u32 DataOffset;
    u32 InfoHeaderSize;
    u32 Width;
    u32 Height;
    u16 Planes;
    u16 BitsPerPixel;
    u32 Compression;
    u32 ImageSize;
    u32 XPixelsPerMeter;
    u32 YPixelsPerMeter;
    u32 ColorsUsed;
    u32 ImportantColors;
};

#define WEXP_VERTEX_SIZE 8*sizeof(r32)
// indices are u16
struct Wexp_Header
{
    u16 signature;
    u16 vert_offset;
    u32 indices_offset;
    u32 eof_offset;
};
#pragma pack(pop)

#define HEADERS_H
#endif
