#include "../../External/ImGui/imgui.h"
#include "../../External/ImGui/imgui_impl_win32.h"
#include "../../External/ImGui/imgui_impl_dx11.h"
#include <tchar.h>
#include <iostream>
#include <direct.h>

#include "gui.h"
#include "../Graphics/Graphics.h"

using namespace GUI;

bool (*GUI::onExitFunc)() = nullptr;

// Data
static ID3D11RenderTargetView* g_mainRenderTargetView = NULL;


LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

float defaultFontSize = 16.0f;

static WNDCLASSEX wc;
static HWND hwnd;

static int (*mainGuiFunc)();

// Setup code, takes a function to run when doing GUI
HWND GUI::Setup(HINSTANCE instance, int (*OnGuiFunc)())
{
	if (OnGuiFunc != NULL)
		mainGuiFunc = OnGuiFunc;
	// Create application window
	//ImGui_ImplWin32_EnableDpiAwareness();
	wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, _T("My Ray Tracing Engine"), NULL };
	//wc.hIcon = LoadIcon(wc.hInstance, MAKEINTRESOURCE(IDI_ICON1));
	//wc.hIconSm = LoadIcon(wc.hInstance, MAKEINTRESOURCE(IDI_ICON1));
	::RegisterClassEx(&wc);

	hwnd = ::CreateWindowW(wc.lpszClassName, _T("Tracing Rays"), (WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SIZEBOX), 200, 100, windowX, windowY, NULL, NULL, wc.hInstance, NULL);

	// Initialize Graphics
	if (!GRAPHICS::Setup(DrawGui, hwnd))
	{
		::UnregisterClass(wc.lpszClassName, wc.hInstance);
		return NULL;
	}


	// Show the window
	::ShowWindow(hwnd, SW_SHOWDEFAULT);
	::UpdateWindow(hwnd);

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

	// Setup Platform/Renderer backends
	ImGui_ImplWin32_Init(hwnd);
	ImGui_ImplDX11_Init(GRAPHICS::g_pd3dDevice, GRAPHICS::g_pd3dDeviceContext);

	// Custom Style
	ImGuiStyle& style = ImGui::GetStyle();
	style.WindowRounding = 6.0f;
	style.ChildRounding = 6.0f;
	style.FrameRounding = 5.0f;
	style.PopupRounding = 4.0f;
	style.GrabRounding = 4.0f;

	IDXGIDevice* pDXGIDevice = nullptr;
	auto hr = GRAPHICS::g_pd3dDevice->QueryInterface(__uuidof(IDXGIDevice), (void**)&pDXGIDevice);

	IDXGIAdapter* pDXGIAdapter = nullptr;
	hr = pDXGIDevice->GetAdapter(&pDXGIAdapter);

	IDXGIFactory* pIDXGIFactory = nullptr;
	pDXGIAdapter->GetParent(__uuidof(IDXGIFactory), (void**)&pIDXGIFactory);

	pIDXGIFactory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_WINDOW_CHANGES | DXGI_MWA_NO_ALT_ENTER);

	return hwnd;
}

int GUI::DrawGui() noexcept
{
	static bool showMainWindow = true;
	static bool done = false;

	MSG msg;
	while (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
	{
		::TranslateMessage(&msg);
		::DispatchMessage(&msg);
		if (msg.message == WM_QUIT)
		    done = true;
	}
	if (done)
		return 1;


	// Start the Dear ImGui frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	if (int GuiRetVal = mainGuiFunc())
		return GuiRetVal;
	//Assemble Together Draw Data
	ImGui::Render();
	//Render Draw Data
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	return 0;
}

void GUI::Destroy() noexcept
{
	// Cleanup
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	GRAPHICS::Destroy();
	::DestroyWindow(hwnd);
	::UnregisterClass(wc.lpszClassName, wc.hInstance);
}

void WindowResize()
{
	using namespace GRAPHICS;

	if (g_pd3dDevice != NULL)
	{
		GRAPHICS::CleanupRenderTarget();
		g_pSwapChain->ResizeBuffers(1, 0, 0, DXGI_FORMAT_UNKNOWN, 0);
		GRAPHICS::g_Viewport_Width = (FLOAT)windowSize_X;
		GRAPHICS::g_Viewport_Height = (FLOAT)windowSize_Y;
		g_pViewport->Width = (FLOAT)windowSize_X;
		g_pViewport->Height = (FLOAT)windowSize_Y;
		GRAPHICS::CreateRenderTarget();
		g_pd3dDeviceContext->RSSetViewports(1, g_pViewport);
	}

	GRAPHICS::WindowResized();
}

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
		return true;

	static bool restore_clicked = false;
	
	enum WindowStatus
	{
		WND_NONE = 0,
		WND_SIZE = 1,
		WND_MOVE = 2,
		WND_IS_MINIMIZED = 4,
		WND_IS_MAXIMIZED = 8,
	};

	static uint8_t windowLastChangeState = WND_NONE;

	//printf("Window State: %u\n", windowLastChangeState);

	switch (msg)
	{
	case WM_SIZE:
		windowLastChangeState |= WND_SIZE; // 0b00000001
		windowSize_X = (UINT)LOWORD(lParam);
		windowSize_Y = (UINT)HIWORD(lParam);
		//printf("Requested res: (%u, %u)\n", windowSize_X, windowSize_Y);
		if (wParam == SIZE_MINIMIZED) {
			windowLastChangeState |= WND_IS_MINIMIZED;
			//windowLastChangeState &= ~WND_IS_MAXIMIZED;
		}
		if (wParam == SIZE_MAXIMIZED) {
			windowLastChangeState |= WND_IS_MAXIMIZED;
			//windowLastChangeState &= ~WND_IS_MINIMIZED;
		}

		if (((wParam == SIZE_MAXIMIZED || wParam == SIZE_MAXSHOW || restore_clicked) && !(windowLastChangeState & WND_IS_MINIMIZED)) || (windowLastChangeState & WND_IS_MAXIMIZED && wParam == SIZE_RESTORED)) {
			WindowResize();
			restore_clicked = false;
			windowLastChangeState &= ~(WND_SIZE | WND_MOVE);
		}

		if (wParam == SIZE_RESTORED)
			windowLastChangeState &= ~(WND_IS_MINIMIZED | WND_IS_MAXIMIZED);
		break;
	case WM_EXITSIZEMOVE:
		if (windowLastChangeState & WND_SIZE && !(windowLastChangeState & WND_MOVE)) {
			WindowResize();
		}
		windowLastChangeState &= ~(WND_SIZE | WND_MOVE);
		return 0;
	case WM_MOVING:
		windowLastChangeState |= WND_MOVE; // 0b00000010
		break;
	case WM_GETMINMAXINFO:
	{
		LPMINMAXINFO lpMMI = (LPMINMAXINFO)lParam;
		lpMMI->ptMinTrackSize.x = min_WindowX;
		lpMMI->ptMinTrackSize.y = min_WindowY;
		break;
	}
	case WM_SYSKEYDOWN:
		//if (wParam == VK_RETURN && (lParam & 0x60000000) == 0x20000000) {
		//	windowSize_X = 3440;
		//	windowSize_Y = 1440;
		//	WindowResize();
		//	GRAPHICS::g_pSwapChain->SetFullscreenState(TRUE, nullptr);
		//	restore_clicked = false;
		//	windowLastChangeState &= ~(WND_SIZE | WND_MOVE);
		//	windowLastChangeState |= WND_IS_MAXIMIZED;
		//}
		break;
	case WM_SYSCOMMAND:
		if (wParam == SC_CLOSE)
		{
			if (onExitFunc)
			{
				if (onExitFunc())
					break;
				else
					return 0;
			}
			else
				break;
		}
		if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
			return 0;
		if (wParam == SC_RESTORE){
			restore_clicked = true;
		}
		break;
	case WM_DESTROY:
		if (onExitFunc)
		{
			if (onExitFunc())
			{
				::PostQuitMessage(0);
				return 0;
			}
		}
		else
		{
			::PostQuitMessage(0);
			return 0;
		}
	}


	return ::DefWindowProc(hWnd, msg, wParam, lParam);
}
