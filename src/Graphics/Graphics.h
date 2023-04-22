#pragma once
#include <d3d11.h>
#include <vector>

#include "BufferTypes.h"
#include "Camera.h"
#include "Types.h"

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

	extern cb_CameraTransform g_cb_CameraTransform_data;

	extern uint64_t last_frame_render_time;

	extern unsigned int frameIdx;
	extern float g_Viewport_Width;
	extern float g_Viewport_Height;

	// "Virtual" camera used for ray tracing
	extern Camera* g_pMainCamera;

	bool Setup(int (*DrawGuiFunc)(), HWND hwnd);
	void Destroy();
	int RenderFrame();
	void ResetFrame();

	void CreateRenderTarget();
	void CleanupRenderTarget();

	ID3D11ShaderResourceView* SaveFrameToFile();
	void AdvanceFrame();

	Vertex cur_vtx_arr[];
}

#endif // !SRC_GRAPHICS_GRAPHICS_H
