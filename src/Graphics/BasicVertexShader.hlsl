struct ViewParams
{
    float width;
    float height;
    float nearPlaneClip;
    
    float3 getData()
    {
        return float3(width, height, nearPlaneClip);
    }
};

cbuffer cameraInfo : register(b0)
{
    float4x4 mx;
    float4 camPos;
    float2 screenSize;
    ViewParams vp;
    uint frameIdx;
};

struct VertexInput
{
    float3 inPos : POSITION;
    float3 inColor : COLOR;
    float2 uv : TEXCOORD0;
};

struct VertexOutput
{
    float3 outVec : POSITION0;
    float3 color : COLOR0;
    float2 uv : TEXCOORD0;
    float4 position : SV_Position;
};

VertexOutput main(VertexInput vertexInput)
{
    float3 inColor = vertexInput.inColor;
    float3 inPos = vertexInput.inPos;
    float3 outColor = inColor;

    VertexOutput output;
    output.position = mul(float4(inPos, 1.f), mx);
    output.color = outColor;
    output.outVec = inPos;
    output.uv = vertexInput.uv;
    return output;
}