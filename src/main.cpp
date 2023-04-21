#include "GUI/gui.h"
#include "../External/ImGui/imgui.h"
#include "../External/ImGui/imgui_impl_dx11.h"
#include "../External/ImGui/imgui_impl_win32.h"
#include "Graphics/Graphics.h"

#include <chrono>


/*

------------------ TODO ------------------
////// The order of these is RANDOM //////
- Move camera to it's own class.
- Add UI for rendering frame with selected sample amount and rendering animations.
- Adding objects.
- UI for modifying added objects and camera parameters.
- Switch from using the deprecated D3DX11SaveTextureToFileA to the new lib
- UI for ray trace settings.
- Clean up RenderFrame in GRAPHICS namespace.

*/


static std::chrono::steady_clock::time_point last_frame_render_time;
uint64_t last_frame_time = 0;

int OnGui()
{
	ImGui::Text("Frame time: %lluus", GRAPHICS::last_frame_render_time);

	ImGui::Text("FPS: %f", ImGui::GetIO().Framerate);

	//ImGui::Text("View Matrix:");
	//{
	//	for (int dx = 0; dx < 4; dx++) {
	//			ImGui::Text("%.3f %.3f %.3f %.3f", GRAPHICS::g_cb_vertexShader_data.mx.r[dx].m128_f32[0], GRAPHICS::g_cb_vertexShader_data.mx.r[dx].m128_f32[1], GRAPHICS::g_cb_vertexShader_data.mx.r[dx].m128_f32[2], GRAPHICS::g_cb_vertexShader_data.mx.r[dx].m128_f32[3]);
	//	}
	//}


	if (ImGui::TreeNodeEx("Rendering", ImGuiTreeNodeFlags_DefaultOpen))
	{
		//static ID3D11ShaderResourceView* tex;

		if (ImGui::Button("Render Frame"))
		{
			GRAPHICS::SaveFrameToFile();
		}
		ImGui::SameLine();
		if (ImGui::Button("Advance Frame"))
		{
			GRAPHICS::AdvanceFrame();
		}

		ImGui::Image(GRAPHICS::g_ShaderResourceViewMap, { 200 * GRAPHICS::g_Viewport_Width / GRAPHICS::g_Viewport_Height,200 });

		ImGui::TreePop();
	}

	//ImGui::ShowDemoWindow();

	return 0;
}

int WINAPI WinMain(_In_ HINSTANCE instance, _In_opt_ HINSTANCE, _In_ char* cmdLine, _In_ int showCmd)
{
	//AllocConsole();
	//AttachConsole(GetCurrentProcessId());
	//FILE* conStream;
	//freopen_s(&conStream, "CON", "w", stdout);

	// Gui sets-up GRAPHICS
	HWND hwnd = GUI::Setup(instance, OnGui);

	while (true)
	{
		last_frame_render_time = std::chrono::steady_clock::now();
		if (GRAPHICS::RenderFrame())
			break;
		last_frame_time = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - last_frame_render_time).count();
	}

	GUI::Destroy();

	exit(0);
}
