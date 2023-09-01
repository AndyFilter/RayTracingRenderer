#pragma once

#include <DirectXMath.h>

using namespace DirectX;

class Camera
{
public:
	float vFov = 90; // Vertical Field Of View In Degrees
	float aspectRatio = 1; // Width / Height
	float nearZ = 0.1f;
	float farZ = 100.f;
	XMVECTOR pos{};
	XMVECTOR lookAtPos{};

	// Fov angle in degrees
	Camera(XMVECTOR position, XMVECTOR lookAtPosition, float _aspectRatio, float vFOV = 90, float _nearZ = 0.1f, float _farZ = 100.f) :
		pos(position), lookAtPos(lookAtPosition), aspectRatio(_aspectRatio), vFov(vFOV), nearZ(_nearZ), farZ(_farZ), cam_up({0,1,0,0})
	{
		vFov_rad = vFov * XM_PI / 180.f;
		projectionMatrix = XMMatrixPerspectiveFovLH(vFov_rad, aspectRatio, nearZ, farZ);
		viewMatrix = XMMatrixLookAtLH(pos, lookAtPos, { 0,1,0,0 } );

		local2worldMatrix = worldMatrix * viewMatrix * projectionMatrix;
		local2worldMatrix = XMMatrixTranspose(local2worldMatrix);
	};

	Camera() = default;

	void Set_vFov(float newVFov)
	{
		vFov = newVFov;
		vFov_rad = vFov * XM_PI / 180.f;
		UpdateProjMatrix();
	}

	void Set_AspectRatio(float newAspect)
	{
		aspectRatio = newAspect;
		UpdateProjMatrix();
	}

	void UpdateTransformMatrix()
	{
		projectionMatrix = XMMatrixPerspectiveFovLH(vFov_rad, aspectRatio, nearZ, farZ);
		viewMatrix = XMMatrixLookAtLH(pos, lookAtPos, cam_up);

		local2worldMatrix = worldMatrix * viewMatrix * projectionMatrix;
		local2worldMatrix = XMMatrixTranspose(local2worldMatrix);
	}

	XMMATRIX GetTransformationMatrix() {
		return local2worldMatrix;
	}

private:
	const XMMATRIX worldMatrix = DirectX::XMMatrixIdentity();
	XMMATRIX viewMatrix{};
	XMMATRIX projectionMatrix{};

	const XMVECTOR cam_up;

	XMMATRIX local2worldMatrix{};

	float vFov_rad = XM_PIDIV2;

	void UpdateProjMatrix()
	{
		projectionMatrix = XMMatrixPerspectiveFovLH(vFov_rad, aspectRatio, nearZ, farZ);

		local2worldMatrix = worldMatrix * viewMatrix * projectionMatrix;
		local2worldMatrix = XMMatrixTranspose(local2worldMatrix);
	}
};