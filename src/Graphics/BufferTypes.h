#pragma once

#include <DirectXMath.h>

enum MaterialFlags
{
	MaterialFlags_None = 0,
	MaterialFlags_Selected = 1 << 0,
};

static MaterialFlags operator|(MaterialFlags right, MaterialFlags other) { return (MaterialFlags)((uint64_t)right + (uint64_t)other); };
static MaterialFlags operator^(MaterialFlags right, MaterialFlags other) { return (MaterialFlags)((uint64_t)right ^ (uint64_t)other); };
static MaterialFlags operator&(MaterialFlags right, MaterialFlags other) { return (MaterialFlags)((uint64_t)right & (uint64_t)other); };
static MaterialFlags operator|=(MaterialFlags& right, MaterialFlags other) { return right = right | other; };
static MaterialFlags operator^=(MaterialFlags& right, MaterialFlags other) { return right = right ^ other; };

struct cb_CameraTransform
{
	DirectX::XMMATRIX mx;
	float camPos[4];
	float screenSize[2];
	float padding[2];
	float viewProj[3];
	unsigned int frameIdx;
	unsigned int renderFlags;

	cb_CameraTransform() = default;
};

struct Material
{
	float baseColor[3];
	float emission = 1;
	float emissionColor[3];
	float roughness = 0;
	alignas(16) MaterialFlags material_Flags;

	Material(float r = 1, float g = 1, float b = 1, float _roughness = 1, float er = 0, float eg = 0, float eb = 0, float _emission = 1, MaterialFlags flags = MaterialFlags_None) : emission(_emission),
		material_Flags(flags)
	{
		baseColor[0] = r;
		baseColor[1] = g;
		baseColor[2] = b;

		emissionColor[0] = er;
		emissionColor[1] = eg;
		emissionColor[2] = eb;

		emission = _emission;
		roughness = _roughness;
	}
}; // size [36]

struct SphereEq
{
	float pos[3]; // 8 * 3 = 24
	float radius; // 8
	alignas(16) Material material; // 36

	SphereEq(float r = 1, Material mat = Material(), float x = 0, float y = 0, float z = 0) : radius(r), material(mat)
	{
		//memcpy(pos, _pos.begin(), sizeof(float) * 3);
		pos[0] = x;
		pos[1] = y;
		pos[2] = z;
	}

	//SphereEq() = default;
}; // Size [68] (80)

struct cb_Pixel_ObjectData
{
	uint32_t sphereCount = 0;
private:
	float _padding1[3];
public:
	SphereEq circle[8];

#pragma warning(suppress: 26495) // No need to initialize padding...
	cb_Pixel_ObjectData() = default;
};

struct cb_RT_Info
{
	uint32_t rayCount = 10;
	uint32_t rayMaxBounce = 4;

	cb_RT_Info(const uint32_t rayCount = 10, uint32_t bounce = 4)
		: rayCount(rayCount), rayMaxBounce(bounce)
	{
	}
};