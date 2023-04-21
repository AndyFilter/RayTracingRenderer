#pragma once
#include <d3d11.h>
#include <vector>

#include "BufferTypes.h"

#ifndef SRC_GRAPHICS_GRAPHICS_H
#define SRC_GRAPHICS_GRAPHICS_H

namespace GRAPHICS
{
	extern IDXGISwapChain* g_pSwapChain;
	extern ID3D11Device* g_pd3dDevice;
	extern ID3D11DeviceContext* g_pd3dDeviceContext;
	extern std::vector<IDXGIOutput*> vOutputs;
	extern std::vector<IDXGIAdapter*> vAdapters;
	extern ID3D11Texture2D* g_pBackBuffer;
	extern D3D11_VIEWPORT* g_pViewport;
	extern ID3D11RenderTargetView* g_mainRenderTargetView;

	extern ID3D11Buffer* g_VertexBuffer;
	extern ID3D11Buffer* g_ConstantBuffer_Matrix;
	extern ID3D11Buffer* g_ConstantBuffer_Circles;
	extern ID3D10Blob* g_VertexShaderCode, * g_PixelShaderCode;
	extern ID3D11VertexShader* g_VertexShader;
	extern ID3D11PixelShader* g_PixelShader;
	extern ID3D11InputLayout* g_InputLayout;
	extern ID3D11Texture2D* g_TargetTexture;
	extern ID3D11Texture2D* g_DepthStencilBuffer;
	extern ID3D11ShaderResourceView* g_ShaderResourceViewMap;
	extern ID3D11DepthStencilView* g_DepthStencilView;
	extern ID3D11SamplerState* g_BackBufferSamplerState;

	extern cb_VertexShader g_cb_vertexShader_data;

	extern uint64_t last_frame_render_time;

	extern unsigned int frameIdx;
	extern float g_Viewport_Width;
	extern float g_Viewport_Height;

	bool Setup(int (*DrawGuiFunc)(), HWND hwnd);
	void Destroy();
	int RenderFrame();
	void ResetFrame();

	void CreateRenderTarget();
	void CleanupRenderTarget();

	ID3D11ShaderResourceView* SaveFrameToFile();
	void AdvanceFrame();

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

	Vertex cur_vtx_arr[];
}

#endif // !SRC_GRAPHICS_GRAPHICS_H
