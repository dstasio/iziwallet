struct VS_OUTPUT
{
    float4 proj_pos  : SV_POSITION;
    float2 txc       : TEXCOORD0;
    float3 normal    : NORMAL;
    float3 world_pos : POSITION;
};

#if VERTEX_HLSL // ---------------------------------------------------
//#pragma pack_matrix(row_major)

struct VS_INPUT
{
    float3 pos    : POSITION;
    float3 normal : NORMAL;
    float2 txc    : TEXCOORD;
};

cbuffer Matrices: register(b0)
{
    float4x4 model;
    float4x4 camera;
    float4x4 projection;
}

VS_OUTPUT
main(VS_INPUT input)
{
    VS_OUTPUT output;
    output.world_pos = (float3)mul(model, float4(input.pos, 1.f));
    float4x4 screen_space = mul(projection, camera);
    output.proj_pos = mul(screen_space, float4(output.world_pos, 1.f));
    //output.txc = input.txc;
    output.txc = input.pos.xy;
    output.normal = input.normal;

    return(output);
}

#elif PIXEL_HLSL // --------------------------------------------------
#include "hlsl_defines.h"

SamplerState TextureSamplerState;

struct PS_OUTPUT
{
    float4 color : SV_TARGET;
};

cbuffer Lights: register(b0)
{
    float3 color;
    float3 lightpos;
    float3 eye;
}

cbuffer Settings: register(b1)
{
    unsigned int flags;
    float3 solid_color;         // used if FLAG_SOLIDCOLOR is set
}

Texture2D SampleTexture;


float4
light(float3 color, float3 dir, float3 eyedir, float3 normal)
{
    float ambient = 0.2f;
    float diffuse = max(dot(dir, normal), 0.f);
    float specular = pow(max(dot(reflect(-dir, normal), eyedir), 0.0f), 512);
    return float4(color*(ambient+diffuse+specular*0.2f), 1.f);
}

PS_OUTPUT
main(VS_OUTPUT input)
{
    PS_OUTPUT output; 
    float3 normal = normalize(input.normal);
    float3 eyedir = normalize(eye - input.world_pos);
    float3 lightdir = normalize(lightpos - input.world_pos);
    output.color = float4(1.f, 1.f, 1.f, 1.f);
    if (!(flags & PHONG_FLAG_UNLIT))
        output.color = light(color, lightdir, eyedir, normal);
    if (flags & PHONG_FLAG_SOLIDCOLOR)
        output.color *= float4(solid_color, 1.f);
    else
        output.color *= SampleTexture.Sample(TextureSamplerState, input.txc);
    return(output);
}

#endif
