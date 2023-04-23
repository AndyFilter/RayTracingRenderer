struct PixelInput
{
    float3 outVec : POSITION0;
    float3 color : COLOR0;
    float2 uv : TEXCOORD0;
    float4 position : SV_Position;
};

Texture2D tex : Texture : register(t0);
SamplerState texSampler : Sampler : register(s0);

float4 main(PixelInput i) : SV_TARGET
{
    float4 texColor = tex.Sample(texSampler, i.uv);
    
    return texColor;
}