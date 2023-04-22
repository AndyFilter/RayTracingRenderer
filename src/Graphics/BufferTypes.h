#pragma once

#include <DirectXMath.h>

struct cb_CameraTransform
{
	DirectX::XMMATRIX mx;
	float camPos[4];
	float screenSize[2];
	float padding[2];
	float viewProj[3];
	unsigned int frameIdx;

	cb_CameraTransform() = default;
};

struct Material
{
	float baseColor[3];
	float emission = 1;
	float emissionColor[3];
	float roughness = 0;

	Material(float r = 1, float g = 1, float b = 1, float _roughness = 1, float er = 0, float eg = 0, float eb = 0, float _emission = 1) : emission(_emission)
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
}; // size [64]

struct SphereEq
{
	float pos[3]; // 8 * 3 = 24
	float radius; // 24 + 8 = 32
	Material material; // 32 + 64 = 96

	SphereEq(float r = 1, Material mat = Material(), float x = 0, float y = 0, float z = 0) : radius(r), material(mat)
	{
		//memcpy(pos, _pos.begin(), sizeof(float) * 3);
		pos[0] = x;
		pos[1] = y;
		pos[2] = z;
	}

	//SphereEq() = default;
}; // Size [96]

struct cb_PixelShader
{
	uint32_t sphereCount = 0;
private:
	float _padding1[3];
public:
	SphereEq circle[8];

#pragma warning(suppress: 26495) // No need to initialize padding...
	cb_PixelShader() = default;
};