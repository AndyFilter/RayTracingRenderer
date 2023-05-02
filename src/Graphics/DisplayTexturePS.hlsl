cbuffer CameraInfo : register(b0)
{
    float4x4 mx;
    float4 camPos;
    float2 screenSize;
    float3 vp;
    uint frameIdx;
    uint renderFlags;
};

struct PixelInput
{
    float3 outVec : POSITION0;
    float3 color : COLOR0;
    float2 uv : TEXCOORD0;
    float4 position : SV_Position;
};

Texture2D tex : Texture : register(t0);
SamplerState texSampler : Sampler : register(s0);

float3 CorrectGamma(float3 color)
{
    float3 outColor = 0;
    
    if(color.x + color.y + color.z < 0.0005)
    {
        outColor = color / 12.f;
    }
    else
    {
        outColor = pow((color + 0.055) / 1.055f, 2.4f);
    }
    
    return outColor;
}

float4 main(PixelInput i) : SV_TARGET
{
    float4 texColor = tex.Sample(texSampler, i.uv);
    
    bool correctGamma = renderFlags & 1u;
    return float4(correctGamma ? CorrectGamma(texColor.xyz) : texColor.xyz, 1);
}