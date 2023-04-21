#pragma once

#include <d3d11.h>
#include <vector>

#define WINDOW_X_MARGIN 16
#define WINDOW_Y_MARGIN 39

namespace GUI
{
	HWND Setup(HINSTANCE instance, int (*OnGuiFunc)());
	int DrawGui() noexcept;
	void Destroy() noexcept;

	const int windowX = 1500, windowY = 1000;
	const int min_WindowX = 600, min_WindowY = 500;

	static UINT windowSize_X = windowX;
	static UINT windowSize_Y = windowY;

	extern bool (*onExitFunc)();

	extern UINT *VSyncFrame;
}

#define TOOLTIP(...)					\
   	if (ImGui::IsItemHovered())			\
		ImGui::SetTooltip(__VA_ARGS__); \