#pragma once

class Vector3
{
public:
	float x = 0, y = 0, z = 0;

	Vector3(float _x, float _y = 0, float _z = 0) : x(_x), y(_y), z(_z) {};

	operator float* () { return &x; };
};

struct Vertex
{
	float Position[3];
	float Color[3];
	float UV[2];
};