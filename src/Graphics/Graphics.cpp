#include <string>
#include <cassert>
#include <direct.h>
#include <d3dcompiler.h>
#include <chrono>

#include "../GUI/gui.h"
#include "Graphics.h"

#include <d3d.h>
#include <D3DX11tex.h>


using namespace GRAPHICS;

IDXGISwapChain* GRAPHICS::g_pSwapChain = nullptr;
ID3D11Device* GRAPHICS::g_pd3dDevice = nullptr;
ID3D11DeviceContext* GRAPHICS::g_pd3dDeviceContext = nullptr;
std::vector<IDXGIOutput*> GRAPHICS::vOutputs = std::vector<IDXGIOutput*>();
std::vector<IDXGIAdapter*> GRAPHICS::vAdapters = std::vector<IDXGIAdapter*>();
ID3D11Texture2D* GRAPHICS::g_pBackBuffer = nullptr;
D3D11_VIEWPORT* GRAPHICS::g_pViewport = nullptr;
ID3D11RenderTargetView* GRAPHICS::g_mainRenderTargetView = nullptr;
ID3D11RenderTargetView* GRAPHICS::g_rayTracingRTV = nullptr;

ID3D11Buffer* GRAPHICS::g_VertexBuffer = nullptr;
ID3D11Buffer* GRAPHICS::g_ConstantBuffer_Matrix = nullptr;
ID3D11Buffer* GRAPHICS::g_ConstantBuffer_Circles = nullptr;
ID3D10Blob* GRAPHICS::g_VertexShaderCode = nullptr, * GRAPHICS::g_PixelShaderCode = nullptr, * GRAPHICS::g_DisplayTexturePSCode = nullptr;
ID3D11VertexShader* GRAPHICS::g_VertexShader = nullptr;
ID3D11PixelShader* GRAPHICS::g_PixelShader = nullptr;
ID3D11PixelShader* GRAPHICS::g_DisplayTexturePS = nullptr;
ID3D11InputLayout* GRAPHICS::g_InputLayout = nullptr;
ID3D11Texture2D* GRAPHICS::g_TargetTexture = nullptr;
ID3D11Texture2D* GRAPHICS::g_TargetResourceViewTexture = nullptr;
ID3D11Texture2D* GRAPHICS::g_DepthStencilBuffer = nullptr;
ID3D11ShaderResourceView* GRAPHICS::g_RayTracingLastFrameSRV = nullptr;
ID3D11DepthStencilView* GRAPHICS::g_DepthStencilView = nullptr;
ID3D11SamplerState* GRAPHICS::g_BackBufferSamplerState = nullptr;
unsigned int GRAPHICS::frameIdx = 1;
float GRAPHICS::g_Viewport_Width = GUI::windowX - WINDOW_X_MARGIN;
float GRAPHICS::g_Viewport_Height = GUI::windowY - WINDOW_Y_MARGIN;

cb_CameraTransform GRAPHICS::g_cb_CameraTransform_data;
uint64_t GRAPHICS::last_frame_render_time = 0;
Camera* GRAPHICS::g_pMainCamera = nullptr;

// The actual camera used to "render" vertices
Camera* pVertexCamera = nullptr;


#define GET_ACTUAL_CB_SIZE(size) (sizeof(size) + (16 - (sizeof(size) % 16)))


// Forward declarations of helper functions
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

int (*OnGuiFunc)();

const auto format = DXGI_FORMAT_R8G8B8A8_UNORM;//DXGI_FORMAT_R16G16B16A16_FLOAT;
const auto RTV_format = DXGI_FORMAT_R32G32B32A32_FLOAT;//DXGI_FORMAT_R32G32B32A32_FLOAT;
const auto SRV_format = DXGI_FORMAT_R32G32B32A32_FLOAT;//DXGI_FORMAT_R16G16B16A16_FLOAT;

//unsigned long long vtx_Count = 0;

void GRAPHICS::SaveFrameToFile()
{
	HRESULT hr;

	g_pd3dDeviceContext->ResolveSubresource(g_pBackBuffer, 0, g_pBackBuffer, 0, format);
	std::string fileName = "Rendered/Frame";
	fileName += std::to_string(frameIdx);
	fileName += ".png";
	hr = D3DX11SaveTextureToFileA(g_pd3dDeviceContext, g_pBackBuffer, D3DX11_IFF_PNG, fileName.c_str());
	assert(SUCCEEDED(hr));
}

//uint32_t advancedSum = 0;

void GRAPHICS::AdvanceFrame()
{
	HRESULT hr;
	//auto start = std::chrono::high_resolution_clock::now();
	frameIdx++;

	// I'm not sure if you can do this without copying because it seams that RTV's texture gets cleared after Present
	if (g_RayTracingLastFrameSRV)
		g_RayTracingLastFrameSRV->Release();
	CD3D11_SHADER_RESOURCE_VIEW_DESC descSRV(D3D11_SRV_DIMENSION_TEXTURE2D, SRV_format);
	g_pd3dDeviceContext->ResolveSubresource(g_TargetResourceViewTexture, 0, g_TargetTexture, 0, SRV_format);
	hr = g_pd3dDevice->CreateShaderResourceView(g_TargetResourceViewTexture, &descSRV, &g_RayTracingLastFrameSRV); // This line causes "memory leak" (-:
	assert(SUCCEEDED(hr));


	return;
	//D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
	//shaderResourceViewDesc.Format = SRV_format;
	//shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	//shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	//shaderResourceViewDesc.Texture2D.MipLevels = 1;

	//hr = g_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)(&g_pBackBuffer));
	////hr = g_pSwapChain->QueryInterface(IID_ID3D11Texture2D, (void**)&g_TargetTexture);
	//if (FAILED(hr))
	//	return;
	//if (g_pBackBuffer)
	//{
	//	if (g_TargetTexture)
	//	{
	//		g_pd3dDeviceContext->ResolveSubresource(g_TargetTexture, 0, g_pBackBuffer, 0, SRV_format);
	//		hr = g_pd3dDevice->CreateShaderResourceView(g_TargetTexture, &shaderResourceViewDesc, &g_RayTracingLastFrameSRV);
	//		assert(SUCCEEDED(hr));
	//	}
	//	g_pBackBuffer->Release();
	//}

	//auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start).count();
	//advancedSum += elapsed;
	//printf("It took %lldus to Advance Frame. Average = %.2f\n", elapsed, (float)advancedSum / (frameIdx-1));
}
//
//void CopyFrame()
//{
//	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
//	shaderResourceViewDesc.Format = SRV_format;
//	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
//	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
//	shaderResourceViewDesc.Texture2D.MipLevels = 1;
//
//	HRESULT hr;
//
//	hr = g_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)(&g_pBackBuffer));
//	if (FAILED(hr))
//		return;
//	if (g_pBackBuffer)
//	{
//		if (g_TargetTexture)
//		{
//			g_pd3dDeviceContext->ResolveSubresource(g_TargetTexture, 0, g_pBackBuffer, 0, SRV_format);
//			hr = g_pd3dDevice->CreateShaderResourceView(g_TargetTexture, &shaderResourceViewDesc, &g_RayTracingLastFrameSRV);
//			assert(SUCCEEDED(hr));
//		}
//		g_pBackBuffer->Release();
//	}
//}

bool GRAPHICS::Setup(int (*DrawGuiFunc)(), HWND hwnd)
{
	// Initialize Direct3D
	if (!CreateDeviceD3D(hwnd))
	{
		CleanupDeviceD3D();
		return false;
	}

	OnGuiFunc = DrawGuiFunc;

	g_pViewport = new D3D11_VIEWPORT();
	g_pViewport->TopLeftX = 0;
	g_pViewport->TopLeftY = 0;
	g_pViewport->Width = (FLOAT)g_Viewport_Width;
	g_pViewport->Height = (FLOAT)g_Viewport_Height;
	g_pViewport->MinDepth = 0.f;
	g_pViewport->MaxDepth = 1.f;
	g_pd3dDeviceContext->RSSetViewports(1, g_pViewport);
	
	
	D3D11_SAMPLER_DESC samplerDesc;
	ZeroMemory(&samplerDesc, sizeof(samplerDesc));
	samplerDesc.Filter = D3D11_FILTER_MINIMUM_MIN_MAG_MIP_LINEAR;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	HRESULT hr = g_pd3dDevice->CreateSamplerState(&samplerDesc, &g_BackBufferSamplerState);
	assert(SUCCEEDED(hr));


	// Compile and create vertex shader

	std::string path = "";
	char pBuf[1024];

#pragma warning(suppress: 6031)
	_getcwd(pBuf, 1024);
	path = pBuf;
	path += "\\";
	std::wstring wpath = std::wstring(path.begin(), path.end());

	std::wstring vertFragPath = wpath + L"src\\Graphics\\BasicVertexShader.hlsl";

	ID3D10Blob* errorBlob = 0;

	hr = D3DCompileFromFile(vertFragPath.c_str(), nullptr, nullptr, "main",
		"vs_5_0", D3DCOMPILE_SKIP_OPTIMIZATION, 0, &g_VertexShaderCode,
		&errorBlob);

	assert(SUCCEEDED(hr));

	hr = g_pd3dDevice->CreateVertexShader(g_VertexShaderCode->GetBufferPointer(), g_VertexShaderCode->GetBufferSize(), 0, &g_VertexShader);
	assert(SUCCEEDED(hr));

	if (errorBlob) errorBlob->Release();


	// Compile and create pixel shader
	std::wstring pixelFragPath = wpath + L"src\\Graphics\\BasicPixelShader.hlsl";

	errorBlob = 0;

	hr = D3DCompileFromFile(pixelFragPath.c_str(), nullptr, nullptr, "main",
		"ps_5_0", 0, 0, &g_PixelShaderCode,
		&errorBlob);
	assert(SUCCEEDED(hr));

	hr = g_pd3dDevice->CreatePixelShader(g_PixelShaderCode->GetBufferPointer(), g_PixelShaderCode->GetBufferSize(), 0, &g_PixelShader);
	assert(SUCCEEDED(hr));

	if (errorBlob) errorBlob->Release();

	// Compile and create another pixel shader (I should move that to some separate function...)
	pixelFragPath = wpath + L"src\\Graphics\\DisplayTexturePS.hlsl";

	errorBlob = 0;

	hr = D3DCompileFromFile(pixelFragPath.c_str(), nullptr, nullptr, "main",
		"ps_5_0", 0, 0, &g_DisplayTexturePSCode,
		&errorBlob);
	assert(SUCCEEDED(hr));

	hr = g_pd3dDevice->CreatePixelShader(g_DisplayTexturePSCode->GetBufferPointer(), g_DisplayTexturePSCode->GetBufferSize(), 0, &g_DisplayTexturePS);
	assert(SUCCEEDED(hr));

	if (errorBlob) errorBlob->Release();

	// Initialize Constant Buffer
	D3D11_BUFFER_DESC desc;
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.MiscFlags = 0;
	desc.ByteWidth = static_cast<UINT>(GET_ACTUAL_CB_SIZE(cb_CameraTransform));
	desc.StructureByteStride = 0;

	hr = g_pd3dDevice->CreateBuffer(&desc, 0, &g_ConstantBuffer_Matrix);
	assert(SUCCEEDED(hr));

	desc.ByteWidth = static_cast<UINT>(GET_ACTUAL_CB_SIZE(cb_PixelShader));

	hr = g_pd3dDevice->CreateBuffer(&desc, 0, &g_ConstantBuffer_Circles);
	assert(SUCCEEDED(hr));

	// Create input layout

	D3D11_INPUT_ELEMENT_DESC inputDesc[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,   0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",    0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	hr = g_pd3dDevice->CreateInputLayout(inputDesc, _countof(inputDesc), g_VertexShaderCode->GetBufferPointer(), g_VertexShaderCode->GetBufferSize(), &g_InputLayout);
	assert(SUCCEEDED(hr));

	cb_PixelShader cb_pixelData;
	cb_pixelData.circle[0] = SphereEq(0.5f, { 0.4f, 0.8f, 0.6f, 0.4f }, -1.f, 0.05f, 6.f );
	cb_pixelData.circle[1] = SphereEq(8.0f, { 0.7f, 0.4f, 0.9f}, -0.8f, 8.5f, 7.f );
	cb_pixelData.circle[2] = SphereEq(0.5f, { 0, 0, 0, 1, 1, 1, 1.f, 4.f }, 0.12f, -1.15f, 6.0f );
	cb_pixelData.circle[3] = SphereEq(0.4f, { 0.9f, 0.2f, 0.2f }, 1.f, 0.3f, 7.f );
	cb_pixelData.sphereCount = 4;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	g_pd3dDeviceContext->Map(g_ConstantBuffer_Circles, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	CopyMemory(mappedResource.pData, &cb_pixelData, sizeof(cb_PixelShader));
	g_pd3dDeviceContext->Unmap(g_ConstantBuffer_Circles, 0);
	g_pd3dDeviceContext->PSSetConstantBuffers(1, 1, &g_ConstantBuffer_Circles);

	g_pMainCamera = new Camera({ 0,0,-1,0 }, { 0,0,0,0 }, (g_pViewport->Width + WINDOW_X_MARGIN) / (g_pViewport->Height + WINDOW_Y_MARGIN), 60.f);
	pVertexCamera = new Camera({ 0,0,-1,0 }, { 0,0,0,0 }, (g_pViewport->Width + WINDOW_X_MARGIN) / (g_pViewport->Height + WINDOW_Y_MARGIN), 90.f);

	return true;
}

void GRAPHICS::Destroy()
{
	CleanupDeviceD3D();
}

int GRAPHICS::RenderFrame()
{
	auto frame_start_time = std::chrono::steady_clock::now();
	float clearColor[4] = { 0.1f, 0.1f, 0.1f, 1.f };

	//g_pd3dDeviceContext->ClearDepthStencilView(g_DepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1u, 1u);
	g_pd3dDeviceContext->ClearRenderTargetView(g_rayTracingRTV, clearColor);


	g_pd3dDeviceContext->OMSetRenderTargets(1, &g_rayTracingRTV, 0);

	float aspectRatio = (g_pViewport->Width + WINDOW_X_MARGIN) / (g_pViewport->Height + WINDOW_Y_MARGIN);

	float vert_unit_x = aspectRatio;// tanf(fovRadians * 0.5f)* aspectRatio;
	float vert_unit_y = 1;// tanf(fovRadians * 0.5f);

	Vertex myVertices[] = {
	{  -vert_unit_x,  vert_unit_y, 0.f, {1,0,0}, {0.f, 0.f} },
	{  vert_unit_x, -vert_unit_y, 0.f, {0,1,0}, {1.0f, 1.f} },
	{ -vert_unit_x, -vert_unit_y, 0.f, {0,0,0}, { 0.f, 1.f} },
	{  -vert_unit_x,  vert_unit_y, 0.f, {1,0,0}, {0.0f, 0.f} },
	{  vert_unit_x, vert_unit_y, 0.f, {0,1,0}, {1.0f, 0.f} },
	{ vert_unit_x, -vert_unit_y, 0.f, {0,0,0}, {1.f, 1.f} },
	};

	D3D11_SUBRESOURCE_DATA bufData;
	ZeroMemory(&bufData, sizeof(bufData));
	bufData.pSysMem = myVertices;

	if (g_VertexBuffer)
		g_VertexBuffer->Release();

	D3D11_BUFFER_DESC bufDesc;
	ZeroMemory(&bufDesc, sizeof(bufDesc));
	bufDesc.ByteWidth = sizeof(Vertex) * _countof(myVertices);
	bufDesc.Usage = D3D11_USAGE_DEFAULT;
	bufDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	HRESULT hr = g_pd3dDevice->CreateBuffer(&bufDesc, &bufData, &g_VertexBuffer);
	assert(SUCCEEDED(hr));

	// Update Constant Buffer Data
	ZeroMemory(&g_cb_CameraTransform_data, sizeof(cb_CameraTransform));
	CopyMemory(g_cb_CameraTransform_data.camPos, &pVertexCamera->pos, sizeof(float) * 4);

	g_cb_CameraTransform_data.frameIdx = frameIdx+1;

	g_cb_CameraTransform_data.mx = pVertexCamera->GetTransformationMatrix();

	float fovFrac = g_pMainCamera->vFov / 90.f;
	g_cb_CameraTransform_data.viewProj[0] = g_pMainCamera->aspectRatio * fovFrac;
	g_cb_CameraTransform_data.viewProj[1] = fovFrac;
	g_cb_CameraTransform_data.viewProj[2] = 1;

	g_cb_CameraTransform_data.screenSize[0] = g_pViewport->Width;
	g_cb_CameraTransform_data.screenSize[1] = g_pViewport->Height;

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	hr = g_pd3dDeviceContext->Map(g_ConstantBuffer_Matrix, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	assert(SUCCEEDED(hr));
	CopyMemory(mappedResource.pData, &g_cb_CameraTransform_data, sizeof(cb_CameraTransform));
	g_pd3dDeviceContext->Unmap(g_ConstantBuffer_Matrix, 0);
	g_pd3dDeviceContext->VSSetConstantBuffers(0, 1, &g_ConstantBuffer_Matrix);
	g_pd3dDeviceContext->PSSetConstantBuffers(0, 1, &g_ConstantBuffer_Matrix);


	UINT vbStride = sizeof(Vertex), vbOffset = 0;
	g_pd3dDeviceContext->IASetVertexBuffers(0, 1, &g_VertexBuffer, &vbStride, &vbOffset);
	g_pd3dDeviceContext->IASetInputLayout(g_InputLayout);
	g_pd3dDeviceContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	g_pd3dDeviceContext->VSSetShader(g_VertexShader, 0, 0);
	g_pd3dDeviceContext->PSSetShader(g_PixelShader, 0, 0);
	g_pd3dDeviceContext->PSSetShaderResources(0, 1, &g_RayTracingLastFrameSRV);
	g_pd3dDeviceContext->PSSetSamplers(0, 1, &g_BackBufferSamplerState);

	g_pd3dDeviceContext->Draw(6, 0);

	//frameIdx++;

	if(frameIdx == 0)
		AdvanceFrame();

	//SaveFrameToFile();

	float clearColor2[4] = { 0.4f, 0.3f, 0.5f, 1.f };
	g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clearColor2);
	g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, NULL);

	g_pd3dDeviceContext->VSSetShader(NULL, 0, 0);
	g_pd3dDeviceContext->PSSetShader(g_DisplayTexturePS, 0, 0);

	// Reuse (almost) all the data setup on the first Draw call.
	g_pd3dDeviceContext->Draw(6, 0);

	if (int GuiRetVal = OnGuiFunc())
		return GuiRetVal;

	last_frame_render_time = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - frame_start_time).count();

	auto presentRes = g_pSwapChain->Present(2, 0);
	if (presentRes == DXGI_STATUS_OCCLUDED)
		Sleep(10);

	//p_CopySRV->Release();

	return 0;
}


bool CreateDeviceD3D(HWND hWnd)
{
	// Setup swap chain
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 1;
	sd.BufferDesc.Width = 0;
	sd.BufferDesc.Height = 0;
	sd.BufferDesc.Format = format;
	sd.BufferDesc.RefreshRate.Numerator = 0; // 60
	sd.BufferDesc.RefreshRate.Denominator = 0; // 1
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	UINT createDeviceFlags = 0;// D3D11_CREATE_DEVICE_DEBUG;
	//createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
	D3D_FEATURE_LEVEL featureLevel;
	const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };

	if (D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext) != S_OK)
		return false;

	GRAPHICS::CreateRenderTarget();
	return true;
}

void CleanupDeviceD3D()
{
	GRAPHICS::CleanupRenderTarget();
	g_pSwapChain->SetFullscreenState(FALSE, NULL);
	if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = NULL; }
	if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = NULL; }
	if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = NULL; }
	if (g_pBackBuffer) { g_pBackBuffer->Release(); g_pBackBuffer = NULL; };

	if (g_InputLayout) g_InputLayout->Release();
	if (g_PixelShader) g_PixelShader->Release();
	if (g_VertexShader) g_VertexShader->Release();
	if (g_PixelShaderCode) g_PixelShaderCode->Release();
	if (g_VertexShaderCode) g_VertexShaderCode->Release();
	if (g_VertexBuffer) g_VertexBuffer->Release();
}

void GRAPHICS::ResetFrame()
{
	if(g_RayTracingLastFrameSRV)
		g_RayTracingLastFrameSRV->Release();
	g_RayTracingLastFrameSRV = nullptr;

	if (g_TargetTexture)
		g_TargetTexture->Release();
	g_TargetTexture = NULL;

	if (g_TargetResourceViewTexture)
		g_TargetResourceViewTexture->Release();
	g_TargetResourceViewTexture = NULL;

	if (g_rayTracingRTV)
		g_rayTracingRTV->Release();
	g_rayTracingRTV = NULL;


	D3D11_TEXTURE2D_DESC texDesc;
	ZeroMemory(&texDesc, sizeof(texDesc));
	texDesc.ArraySize = 1;
	texDesc.Format = RTV_format;
	texDesc.Width = (UINT)g_Viewport_Width;
	texDesc.Height = (UINT)g_Viewport_Height;
	texDesc.MipLevels = 1;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	texDesc.CPUAccessFlags = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;

	HRESULT hr = g_pd3dDevice->CreateTexture2D(&texDesc, nullptr, &g_TargetTexture);
	assert(SUCCEEDED(hr));

	hr = g_pd3dDevice->CreateTexture2D(&texDesc, nullptr, &g_TargetResourceViewTexture);
	assert(SUCCEEDED(hr));

	CD3D11_RENDER_TARGET_VIEW_DESC rtDesc(D3D11_RTV_DIMENSION_TEXTURE2D, RTV_format);
	hr = g_pd3dDevice->CreateRenderTargetView(g_TargetTexture, &rtDesc, &g_rayTracingRTV);
	assert(SUCCEEDED(hr));

	frameIdx = 0;
}

void GRAPHICS::CreateRenderTarget()
{
	g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&g_pBackBuffer));
	if (!g_pBackBuffer)
		ExitProcess(1);

	HRESULT hr = g_pd3dDevice->CreateRenderTargetView(g_pBackBuffer, NULL, &g_mainRenderTargetView);
	assert(SUCCEEDED(hr));
	g_pBackBuffer->Release();

	ResetFrame();

	if (g_pViewport) {
		float aspectRatio = (g_pViewport->Width + WINDOW_X_MARGIN) / (g_pViewport->Height + WINDOW_Y_MARGIN);
		pVertexCamera->aspectRatio = aspectRatio;
		g_pMainCamera->aspectRatio = aspectRatio;

		pVertexCamera->UpdateTransformMatrix();
		g_pMainCamera->UpdateTransformMatrix();
	}
	
	//D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
	//shaderResourceViewDesc.Format = SRV_format;
	//shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	//shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	//shaderResourceViewDesc.Texture2D.MipLevels = 1;

	//g_pd3dDevice->CreateShaderResourceView(g_pBackBuffer, &shaderResourceViewDesc, &g_RayTracingLastFrameSRV);
}

void GRAPHICS::CleanupRenderTarget()
{
	if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = NULL; }
	if (g_rayTracingRTV) { g_rayTracingRTV->Release(); g_rayTracingRTV = NULL; }
}


// --------- Rendering ---------

//void AddTriangle(Vector3 p1, Vector3 p2, Vector3 p3, float Color[3])
//{
//	Vertex myVertices[] = {
//		{  p1.x, p1.y, p1.z, {1,0,0}, {0.5f, 0.f}},
//		{  p2.x, p2.y, p2.z, {0,1,0}, {1.0f, 1.f} },
//		{  p3.x, p3.y, p3.z, {0,0,0}, {0.f, 1.f} },
//	};
//	
//	D3D11_BUFFER_DESC bufDesc;
//	ZeroMemory(&bufDesc, sizeof(bufDesc));
//	bufDesc.ByteWidth = sizeof(Vertex) * _countof(myVertices);
//	bufDesc.Usage = D3D11_USAGE_DEFAULT;
//	bufDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
//
//	D3D11_SUBRESOURCE_DATA bufData;
//	ZeroMemory(&bufData, sizeof(bufData));
//	bufData.pSysMem = myVertices;
//
//	HRESULT hr = g_pd3dDevice->CreateBuffer(&bufDesc, &bufData, &g_VertexBuffer);
//	assert(SUCCEEDED(hr));
//	vtx_Count += 3;
//}
