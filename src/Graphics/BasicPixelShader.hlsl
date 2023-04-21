struct Material
{
    float3 baseColor;
    float emission;
    float3 emissionColor;
    float roughness;
}; // size [64]

struct Sphere
{
    float3 pos; // 8 + 3 * 8 = 32
    float radius; // 8
    Material material; // 32 + 32 = 96
};

cbuffer object_data : register(b1)
{
    uint sphereCount;
    Sphere sphere[8];
}

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

cbuffer CameraInfo : register(b0)
{
    float4x4 mx;
    float4 camPos;
    float2 screenSize;
    ViewParams vp;
    uint frameIdx;
};


Texture2D backTex : Texture : register(t0);
SamplerState texSampler : Sampler : register(s0);

struct PixelInput
{
    float3 position : POSITION0;
    float3 color : COLOR0;
    float2 uv : TEXCOORD0;
    float4 screenSpace : SV_Position;
};

struct PixelOutput
{
    float4 attachment0 : SV_Target0;
};

struct Ray
{
    float3 origin;
    float3 dir;
};

struct HitInfo
{
    bool wasHit;
    float dist;
    float3 worldPoint;
    float3 normal;
    Material mat;
};

uint NextRandom(inout uint state)
{
    state = state * 747796405u + 2891336453u;
    uint word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
    return (word >> 22u) ^ word;
}

float RandomValue(inout uint state)
{
    return NextRandom(state) / 4294967295.0; // 2^32 - 1
}

// Random value in normal distribution (with mean=0 and sd=1)
float RandomValueNormalDistribution(inout uint state)
{
	// Thanks to https://stackoverflow.com/a/6178290
    float theta = 2 * 3.1415926 * RandomValue(state);
    float rho = sqrt(-2 * log(RandomValue(state)));
    return rho * cos(theta);
}

float2 RandomPointInCircle(inout uint rngState)
{
    float angle = RandomValue(rngState) * 2 * 3.1415926;
    float2 pointOnCircle = float2(cos(angle), sin(angle));
    return pointOnCircle * sqrt(RandomValue(rngState));
}

float3 RandomDirection(inout uint state)
{
	// Thanks to https://math.stackexchange.com/a/1585996
    float x = RandomValueNormalDistribution(state);
    float y = RandomValueNormalDistribution(state);
    float z = RandomValueNormalDistribution(state);
    return normalize(float3(x, y, z));
}

HitInfo RaySphereCollision(Ray ray, float radius, float3 sphereCenter)
{
    HitInfo hit = (HitInfo) 0;
    hit.wasHit = false;
    
    float3 sphereDir = ray.origin - sphereCenter; // relative location
    
    float a = dot(ray.dir, ray.dir); // (ray.dir)^2 as a scalar
    float b = 2 * dot(sphereDir, ray.dir);
    float c = dot(sphereDir, sphereDir) - pow(radius, 2);
    
    float delta = b * b - (4 * a * c);
    
    if (delta >= 0)
    {
        //float sqrtDelta = sqrt(delta);
        
        float dist = (-b - sqrt(delta)) / (2 * a);
        //float dist2 = (-b - sqrtDelta)/(2*a);
        
        if (dist >= 0)
        {
            hit.dist = dist;
            hit.wasHit = true;
            hit.worldPoint = ray.origin + (ray.dir * dist);
            hit.normal = normalize(hit.worldPoint - sphereCenter);
        }
    }

    
    return hit;
}

HitInfo TraceRay(Ray ray)
{
    HitInfo closestHit = (HitInfo) 0;
    closestHit.wasHit = false;
    
    for (int i = 0; i < sphereCount; i++)
    {
        HitInfo curHit = RaySphereCollision(ray, sphere[i].radius, sphere[i].pos);

        if (!closestHit.wasHit || curHit.wasHit && (curHit.dist < closestHit.dist))
        {
            closestHit = curHit;
            closestHit.mat = sphere[i].material;
        }
    }
    
    return closestHit;
}

PixelOutput main(PixelInput pixelInput)
{
    PixelOutput output;
    
    float2 uv = pixelInput.uv;
    uint2 pixelCoord = uv * screenSize;
    uint pixelIndex = pixelCoord.y * screenSize.x + pixelCoord.x;
    
    uint rngState = pixelIndex + frameIdx * 75391;
    
    output.attachment0 = float4(0, 0, 0, 1);
    //output.attachment0.xy = uv.xy;
    
    float3 localViewPoint = float3(uv - 0.5f, 1) * vp.getData();
    float3 pointView = mul(mx, float4(localViewPoint, 1));
    
    float2 rd = float2((uv - 0.5f) * screenSize.xy) / screenSize.y;
    
    float3 outColor = 1;
    float3 outLight = 0;
    
    float3 TotalIncomingLight = 0;
    
    uint MAX_RAY_COUNT = 10;
    Ray ray;
    
    for (int rayIdx = 0; rayIdx < MAX_RAY_COUNT; rayIdx++)
    {
        ray.origin.xyz = camPos.xyz;
        ray.dir = normalize(localViewPoint);
        
        for (int i = 0; i <= 4; i++)
        {
            HitInfo hit = TraceRay(ray);

            if (!hit.wasHit)
                break;
            
            ray.origin = hit.worldPoint;

            float3 diffDir = normalize(hit.normal + RandomDirection(rngState));
            diffDir *= sign(dot(hit.normal, diffDir));
           
            float3 reflectedDir = reflect(ray.dir, hit.normal);
            
            ray.dir = lerp(reflectedDir, diffDir, hit.mat.roughness);
            
            
            float3 emittedLight = hit.mat.emissionColor * hit.mat.emission;
            outLight += emittedLight * outColor;
            outColor *= hit.mat.baseColor;
        }
        TotalIncomingLight += outLight;

    }
    float4 lastFrameColor = backTex.Sample(texSampler, pixelInput.uv);

    output.attachment0.xyz = TotalIncomingLight / MAX_RAY_COUNT;
    
    // Average all frames
    float weight = 1.0 / (frameIdx);
    float4 accumulatedCol = saturate(lastFrameColor * (1 - weight) + output.attachment0 * weight);
    
    output.attachment0 = accumulatedCol;
    
    return output;
}