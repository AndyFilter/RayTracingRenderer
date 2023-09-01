cbuffer CameraInfo : register(b0)
{
    float4x4 mx;
    float4 camPos;
    float2 screenSize;
    float3 vp;
    uint frameIdx;
    uint renderFlags;
};

struct Material
{
    float3 baseColor;
    float emission;
    float3 emissionColor;
    float roughness;
    uint matFlags;
}; // size [36]

// Material Flags
const uint MaterialFlags_None = 0;
const uint MaterialFlags_Selected = 0x1;

struct Sphere
{
    float3 pos; // 4 + 3 * 4 = 32
    float radius; // 4
    Material material; // 16 + 16 + 4 = 68
}; // [80]

cbuffer object_data : register(b1)
{
    uint sphereCount;
    Sphere sphere[8];
}

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

struct ProjectionResult
{
    float2 center; // but i'm outputing all the information for debugging and ilustration
    float2 axisA; // purposes
    float2 axisB;
    float a, b, c, d, e, f;
};

// Huge thanks to Inigo Quilez for this function (https://iquilezles.org/articles/sphereproj/)
ProjectionResult projectSphere(in float3 center, in float radius, in float fov)
{
    float4x4 mx_no_aspect = mx;
    mx_no_aspect[0][0] = mx_no_aspect[1][1] = 1;
    
    // transform to camera space	
    float3 o = mul(mx_no_aspect, float4(center.xyz, 1.0)).xyz;
	
    float r2 = radius * radius;
    float z2 = o.z * o.z;
    float l2 = dot(o, o);

	
	// axis
    float2 axa = fov * sqrt(-r2 * (r2 - l2) / ((l2 - z2) * (r2 - z2) * (r2 - z2))) * float2(o.x, o.y);
    float2 axb = fov * sqrt(-r2 * (r2 - l2) / ((l2 - z2) * (r2 - z2) * (r2 - l2))) * float2(-o.y, o.x);
	
	// center
    float2 cen = fov * o.z * o.xy / (z2 - r2);
    
    ProjectionResult result;
    result.center = cen;
    result.axisA = axa;
    result.axisB = axb;
    result.a = r2 - o.y * o.y - z2;
    result.b = r2 - o.x * o.x - z2;
    result.c = 2.0 * o.x * o.y;
    result.d = 2.0 * o.x * o.z * fov;
    result.e = 2.0 * o.y * o.z * fov;
    result.f = (r2 - l2 + z2) * fov * fov;
    return result;
    /* implicit ellipse f(x,y) = a·x² + b·y² + c·x·y + d·x + e·y + f = 0 */
	
}

float sdSegment(float2 a, float2 b, float2 p)
{
    float2 pa = p - a;
    float2 ba = b - a;
    float h = clamp(dot(pa, ba) / dot(ba, ba), 0.0, 1.0);
	
    return length(pa - ba * h);
}

float4 main(PixelInput input) : SV_TARGET
{
    float aspect = vp.x / vp.y;
    float fovFrac = vp.y;
    float4 texColor = tex.Sample(texSampler, input.uv);
    
    // Grid
    //if ((input.uv.x * 28 * aspect) % 1 < 0.06)
    //    texColor += float4(0.3, 0.3, 0.3, 0);
    
    //if (abs((input.uv.y * 14) % 1) < 0.03)
    //    texColor += float4(0.3, 0.3, 0.3, 0);
    
    const float selectedCircleScale = 0.6;

    // Outline round selected spheres
    for (int i = 0; i < sphereCount; i++)
    {
        if ((sphere[i].material.matFlags & 0x1) != 0x1)
            continue;
        
        ProjectionResult proj = projectSphere(sphere[i].pos, sphere[i].radius, 2 / fovFrac);
        
        float dist = sphere[i].pos.z - camPos.z;
        float radius = sphere[i].radius;
        float scale = radius / dist;
        
        float2 p = input.outVec * float3(1, -1, 1);
        
        // axes
        //texColor.xyz = lerp(texColor.xyz, float3(1.0, 1.0, 0.0), (1.0 - smoothstep(0.00, 0.01, sdSegment(proj.center - proj.axisA, proj.center + proj.axisA, p))));
        //texColor.xyz = lerp(texColor.xyz, float3(1.0, 1.0, 0.0), (1.0 - smoothstep(0.00, 0.01, sdSegment(proj.center - proj.axisB, proj.center + proj.axisB, p))));
        
        float impl = proj.a * p.x * p.x + proj.b * p.y * p.y + proj.c * p.x * p.y + proj.d * p.x + proj.e * p.y + proj.f;
        if (sqrt(abs(impl) / scale) < selectedCircleScale)
            texColor.xyz = float3(1, 1, 0.92);
    }
    
    bool correctGamma = (renderFlags & 1u) == 1u;
    return float4(correctGamma ? CorrectGamma(texColor.xyz) : texColor.xyz, 1);
}