struct VS_OUTPUT
{
    float4 pos : SV_POSITION;
    float2 txc : TEXCOORD;
};

#if VERTEX_HLSL // ---------------------------------------------------

struct VS_INPUT
{
    float2 pos    : POSITION;
    float2 txc    : TEXCOORD;
};

cbuffer Matrices: register(b0)
{
    float4x4 model;
}

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;
    output.pos = mul(model, float4(input.pos.xy, 0.f, 1.f));
    output.txc = input.txc;

    return output;
}

#elif PIXEL_HLSL // --------------------------------------------------

SamplerState texture_sampler_state;

Texture2D font;

float4 main(VS_OUTPUT input) : SV_TARGET
{
    float4 color = float4(0.2f, 0.7f, 0.5f, 0.7f);
    float4 output = color*font.Sample(texture_sampler_state, input.txc);
    return output;
}

#endif
