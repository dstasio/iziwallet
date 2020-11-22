#include "hlsl_defines.h"
struct VS_OUTPUT
{
    float4 pos  : SV_POSITION;
    float2 txc  : TEXCOORD;
    float4 color: COLOR0;
};

cbuffer DebugSettings: register(b2)
{
    uint   debug_type;
    float4 debug_color;
    float3 debug_pos[4];
}

#if VERTEX_HLSL // ---------------------------------------------------

struct VS_INPUT
{
    float2 pos    : POSITION;
    float2 txc    : TEXCOORD;
};

cbuffer Matrices: register(b0)
{
    float4x4 model;
    float4x4 camera;
    float4x4 projection;
}

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;
    if (debug_type == DEBUG_LINE)
    {
        float3 pos = float3(0.f, 0.f, 0.f);
        if (input.txc.x == 0.f)
        {
            pos = debug_pos[0];
        }
        else if (input.txc.x == 1.f)
        {
            pos = debug_pos[1];
        }

        float4x4 screen_space = mul(projection, camera);
        output.pos = mul(screen_space, float4(pos, 1.f));
    }

    output.txc = input.txc;
    output.color = debug_color;
    return output;
}

#elif PIXEL_HLSL // --------------------------------------------------

float4 main(VS_OUTPUT input) : SV_TARGET
{
    float4 output = float4(debug_color.xyzw);
    return output;
}

#endif
