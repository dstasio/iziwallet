#if !defined(WINDY_MATH_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Davide Stasio $
   $Notice: (C) Copyright 2020 by Davide Stasio. All Rights Reserved. $
   ======================================================================== */
#include "windy_intrinsics.h"

#define PI 3.141592653589793238462643383279f
#define DegToRad (PI/180.f)
#define RadToDeg (180.f/PI)
#define COL_X 0
#define COL_Y 1
#define COL_Z 2
#define COL_W 3

//
// Scalar ------------------------------------------------------------
//
inline r32 Square(r32 x)
{
    r32 result = x*x;
    return result;
}

//
// Vector 2 ----------------------------------------------------------
//
inline v2 make_v2(r32 x, r32 y) { return {x, y}; }

v2 operator*(v2 a, r32 b)
{
    v2 result = {a.x*b, a.y*b};
    return result;
}

v2 &operator*=(v2 &a, r32 b)
{
    a = a * b;
    return a;
}

//
// Vector 3 ----------------------------------------------------------
// 

inline v3 make_v3(r32 x, r32 y, r32 z) { return {  x,   y,   z}; }
inline v3 make_v3(v4  a)               { return {a.x, a.y, a.z}; }
inline v3 make_v3(v2  a)               { return {a.x, a.y, 0.f}; }
inline v3 make_v3(r32 a)               { return {  a,   a,   a}; }

v3 operator-(v3 a)
{
    v3 result = {-a.x, -a.y, -a.z};
    return result;
}

v3 operator*(v3 a, r32 b)
{
    v3 result = {a.x*b, a.y*b, a.z*b};
    return result;
}
v3 operator*(r32 a, v3 b)  {return b * a;}

v3 operator/(v3 a, r32 b)
{
    v3 result = {a.x/b, a.y/b, a.z/b};
    return result;
}

v3 operator+(v3 a, v3 b)
{
    v3 result = {a.x+b.x, a.y+b.y, a.z+b.z};
    return result;
}

v3 operator-(v3 a, v3 b)
{
    v3 result = {a.x-b.x, a.y-b.y, a.z-b.z};
    return result;
}
v3 operator-(v3 a, r32 b)
{
    v3 result = {a.x-b, a.y-b, a.z-b};
    return result;
}

v3 &operator*=(v3 &a, r32 b)
{
    a = a * b;
    return a;
}

v3 &operator/=(v3 &a, r32 b)
{
    a = a / b;
    return a;
}

v3 &operator+=(v3 &a, v3 b)
{
    a = a + b;
    return a;
}

v3 &operator-=(v3 &a, v3 b)
{
    a = a - b;
    return a;
}

inline r32
Dot(v3 a, v3 b)
{
    r32 result = a.x*b.x + a.y*b.y + a.z*b.z;
    return result;
}

inline v3
Cross(v3 a, v3 b)
{
    v3 result = {
        a.y*b.z - a.z*b.y,
        a.z*b.x - a.x*b.z,
        a.x*b.y - a.y*b.x
    };
    return result;
}

inline r32
Length_Sq(v3 a)
{
    r32 result = Dot(a, a);
    return result;
}

inline r32
Length(v3 a)
{
    r32 result = Length_Sq(a);
    if(result != 1)
    {
        result = Sqrt(result);
    }
    return result;
}

inline v3
Normalize(v3 a)
{
    v3 result = a / Length(a);
    return result;
}

inline v3
Lerp(v3 a, v3 b, r32 t)
{
    v3 result = a*t + b*(1.f-t);
    return result;
}

//
// Vector 4 ----------------------------------------------------------
// 
inline v4 make_v4(r32 x, r32 y, r32 z, r32 w) { return {  x,   y,   z,   w}; }
inline v4 make_v4(v3  a)                      { return {a.x, a.y, a.z, 1.f}; }
inline v4 make_v4(r32 a)                      { return {  a,   a,   a,   a}; }

inline r32
Dot(v4 a, v4 b)
{
    r32 result = a.x*b.x + a.y*b.y + a.z*b.z + a.w*b.w;
    return result;
}

//
// Matrix 4 ----------------------------------------------------------
// Column Major
// @todo: maybe add m4t (matrix4 transposed)?

inline m4
Identity_m4()
{
    m4 matrix = {
        1.f, 0.f, 0.f, 0.f,
        0.f, 1.f, 0.f, 0.f,
        0.f, 0.f, 1.f, 0.f,
        0.f, 0.f, 0.f, 1.f
    };
    return matrix;
}

inline m4
Transpose(m4 a)
{
    m4 matrix = {
        a.m[0][0], a.m[1][0], a.m[2][0], a.m[3][0],
        a.m[0][1], a.m[1][1], a.m[2][1], a.m[3][1],
        a.m[0][2], a.m[1][2], a.m[2][2], a.m[3][2],
        a.m[0][3], a.m[1][3], a.m[2][3], a.m[3][3]
    };
    return matrix;
}

#if 0
inline m4
Invert(m4 A)
{
    m4 I = Identity_m4();

    i32 pivot_index = 0;
//    for (i32 col = 0; col < 4; ++col)
    while (1)
    {
        u32 col_index = 0;
        while (A.col[col_index] == 0)
        {
            // @todo
        }

        col_index += 1;
    }

    return I;
}
#endif

v4 operator*(m4 a, v4 b)
{
    v4 result = {};
    a = Transpose(a);  // column-major to row-major

    result.x = Dot(a.col[0], b);
    result.y = Dot(a.col[1], b);
    result.z = Dot(a.col[2], b);
    result.w = Dot(a.col[3], b);
    return result;
}

v3 operator*(m4 a, v3 b)
{
    v3 result = make_v3(a*make_v4(b));
    return result;
}

m4 operator*(m4 a, m4 b)
{
    m4 matrix = {};
    a = Transpose(a);

    for(u32 row_index = 0;
        row_index < 4;
        ++row_index)
    {
        for(u32 col_index = 0;
            col_index < 4;
            ++col_index)
        {
            matrix.m[col_index][row_index] = Dot(a.col[row_index], b.col[col_index]);
        }
    }
    return matrix;
}

m4 &operator*=(m4 &a, m4 b)
{
    a = a*b;
    return a;
}

inline m4
Flip_m4(i32 first_comp, i32 second_comp, i32 third_comp, i32 fourth_comp)
{
    Assert((first_comp >= 0) && (first_comp < 4));
    Assert((second_comp >= 0) && (second_comp < 4));
    Assert((third_comp >= 0) && (third_comp < 4));
    Assert((fourth_comp >= 0) && (fourth_comp < 4));
    m4 matrix = {};
    matrix.m[0][first_comp] = 1.f;
    matrix.m[1][second_comp] = 1.f;
    matrix.m[2][third_comp] = 1.f;
    matrix.m[3][fourth_comp] = 1.f;

    return matrix;
}

inline m4
Scale_m4(r32 sx, r32 sy, r32 sz)
{
    m4 matrix = {
         sx, 0.f, 0.f, 0.f,
        0.f,  sy, 0.f, 0.f,
        0.f, 0.f,  sz, 0.f,
        0.f, 0.f, 0.f, 1.f
    };
    return matrix;
}
inline m4 Scale_m4(v3  s) { return Scale_m4(s.x, s.y, s.z); }
inline m4 Scale_m4(v2  s) { return Scale_m4(s.x, s.y, 1.f); }
inline m4 Scale_m4(r32 s) { return Scale_m4(  s,   s,   s); }

inline m4
Translation_m4(r32 tx, r32 ty, r32 tz)
{
    m4 matrix = {
        1.f, 0.f, 0.f, 0.f,
        0.f, 1.f, 0.f, 0.f,
        0.f, 0.f, 1.f, 0.f,
         tx,  ty,  tz, 1.f
    };
    return matrix;
}
inline m4 Translation_m4(v3  t) { return Translation_m4(t.x, t.y, t.z); }
inline m4 Translation_m4(r32 t) { return Translation_m4(  t,   t,   t); }

inline m4
Pitch_m4(r32 delta) // Around X axis
{
    r32 sin = Sin(delta);
    r32 cos = Cos(delta);
    m4 matrix = {
        1.f,  0.f, 0.f, 0.f,
        0.f,  cos, sin, 0.f,
        0.f, -sin, cos, 0.f,
        0.f,  0.f, 0.f, 1.f
    };
    return matrix;
}

inline m4
Roll_m4(r32 delta) // Around Y axis
{
    r32 sin = Sin(delta);
    r32 cos = Cos(delta);
    m4 matrix = {
         cos, 0.f, sin, 0.f,
         0.f, 1.f, 0.f, 0.f,
        -sin, 0.f, cos, 0.f,
         0.f, 0.f, 0.f, 1.f
    };
    return matrix;
}

inline m4
Yaw_m4(r32 delta) // Around Z axis
{
    r32 sin = Sin(delta);
    r32 cos = Cos(delta);
    m4 matrix = {
         cos, sin, 0.f, 0.f,
        -sin, cos, 0.f, 0.f,
         0.f, 0.f, 1.f, 0.f,
         0.f, 0.f, 0.f, 1.f
    };
    return matrix;
}

/*
 * Euler angles: (pitch, roll, yaw) in that order
*/
inline m4
Rotation_m4(v3 r)
{
    m4 matrix = Pitch_m4(r.x) * Roll_m4(r.y) * Yaw_m4(r.z);
    return matrix;
}

inline m4
Camera_m4(v3 pos, v3 target, v3 up)
{
    v3 n = Normalize(target - pos);
    v3 v = Normalize(Cross(n, up));
    v3 u = Normalize(Cross(v, n));
    r32 p_v = -Dot(pos, v);
    r32 p_u = -Dot(pos, u);
    r32 p_n = -Dot(pos, n);

    m4 matrix = {
        v.x, u.x, n.x, 0.f,
        v.y, u.y, n.y, 0.f,
        v.z, u.z, n.z, 0.f,
        p_v, p_u, p_n, 1.f
    };

    return matrix;
}

inline m4
Perspective_m4(r32 fov, r32 ar, r32 n, r32 f)
{
    r32 cot = 1.f/Tan(fov/2.f);
    m4 matrix = {
        cot/ar, 0.f,       0.f, 0.f,
        0.f,    cot,       0.f, 0.f,
        0.f,    0.f,  -f/(n-f), 1.f,
        0.f,    0.f, n*f/(n-f), 0.f
    };
    return matrix;
}

inline m4
Ortho_m4(r32 size, r32 ar, r32 n, r32 f)
{
    r32 inv = 1.f/size;
    // @todo: test this matrix
    m4 matrix = {
        inv/ar, 0.f,               0.f, 0.f,
        0.f,    inv,               0.f, 0.f,
        0.f,    0.f, 1.f/(f-n), 0.f,
        0.f,    0.f, (-n)/(f-n), 1.f
    };
    return matrix;
}

inline m4
Transform_m4(v3 pos, v3 euler, v3 scale)
{
    //m4 matrix = Translation_m4(pos) * Rotation_m4(euler) * Scale_m4(scale);
    m4 matrix = Scale_m4(scale);
    matrix.m[3][0] = pos.x;
    matrix.m[3][1] = pos.y;
    matrix.m[3][2] = pos.z;
    return matrix;
}

inline m4
WorldToLocal_m4(v3 u, v3 v, v3 n, v3 origin = {})
{
    m4 matrix = {
             u.x, v.x, n.x, 0.f,
             u.y, v.y, n.y, 0.f,
             u.z, v.z, n.z, 0.f,
             0.f, 0.f, 0.f, 1.f
    };
    matrix *= Translation_m4(-origin);
    return matrix;
}

inline m4
NoTranslation_m4(m4 a)
{
    m4 result = a;
    result.col[3] = make_v4(0.f, 0.f, 0.f, 1.f);
    return result;
}

#define WINDY_MATH_H
#endif
