struct VS_OUTPUT
{
    float4 Pos : SV_POSITION;
    float2 TXC : TEXCOORD;
};

#if VERTEX_HLSL

struct VS_INPUT
{
    float3 Pos : POSITION;
    float2 TXC : TEXCOORD;
};

VS_OUTPUT
main(VS_INPUT Input)
{
    VS_OUTPUT Output;
    Output.Pos = float4(Input.Pos, 1.f);
    Output.TXC = Input.TXC;
    return(Output);
}

#elif PIXEL_HLSL

cbuffer Constants: register(b0)
{
    uint Switch;
}

SamplerState TextureSamplerState
{
    Filter = MIN_MAG_MIP_LINEAR;
};

Texture2D SampleTexture;

float4
main(VS_OUTPUT Input) : SV_TARGET
{
    float4 Color = SampleTexture.Sample(TextureSamplerState, Input.TXC);
    if(Switch)
    {
        Color = SampleTexture.Sample(TextureSamplerState,
                                     float2(Input.TXC.x, 1.f -
                                            Input.TXC.y));
    }

    return(Color);
}

#endif
