#include "GUI/gui.h"
#include "../External/ImGui/imgui.h"
#include "../External/ImGui/imgui_impl_dx11.h"
#include "../External/ImGui/imgui_impl_win32.h"
#include "Graphics/Graphics.h"

#include <chrono>


/*

------------------ TODO ------------------
////// The order of these is RANDOM //////
+ Move camera to it's own class.
+ Spend an entire day on a single bug that no one would even notice.
- Add UI for rendering frame with selected sample amount and rendering animations.
- Adding objects.
- UI for modifying added objects and camera parameters.
- Switch from using the deprecated D3DX11SaveTextureToFileA to the new lib
+ UI for ray trace settings.
~ Clean up RenderFrame in GRAPHICS namespace.
+ Fix gamma when writing to file GRAPHICS::SaveFrameToFile() 

*/

bool saveFrames = false;

static std::chrono::steady_clock::time_point last_frame_render_time;
uint64_t last_frame_time = 0;

int OnGui()
{
	ImGui::SetNextWindowSize({ 400, 600 }, ImGuiCond_FirstUseEver);
	ImGui::Begin("Scene", 0, ImGuiWindowFlags_NoResize);

	ImGui::Text("Frame time: %lluus", GRAPHICS::last_frame_render_time);

	ImGui::Text("FPS: %f", ImGui::GetIO().Framerate);

	//ImGui::Text("View Matrix:");
	//{
	//	for (int dx = 0; dx < 4; dx++) {
	//			ImGui::Text("%.3f %.3f %.3f %.3f", GRAPHICS::g_cb_CameraTransform_data.mx.r[dx].m128_f32[0], GRAPHICS::g_cb_CameraTransform_data.mx.r[dx].m128_f32[1], GRAPHICS::g_cb_CameraTransform_data.mx.r[dx].m128_f32[2], GRAPHICS::g_cb_CameraTransform_data.mx.r[dx].m128_f32[3]);
	//	}
	//}


	if (ImGui::TreeNodeEx("Rendering", ImGuiTreeNodeFlags_DefaultOpen))
	{
		//static ID3D11ShaderResourceView* tex;
		ImGui::Text("Frame: %i", GRAPHICS::frameIdx);

		ImGui::Checkbox("Save to file", &saveFrames);
		ImGui::SameLine();
		ImGui::Checkbox("Correct Gamma", &GRAPHICS::g_UseCorrectedGamma);

		if (ImGui::Button("Save one Frame"))
		{
			GRAPHICS::SaveFrameToFile(true);
		}
		ImGui::SameLine();
		{
			auto curTime = last_frame_render_time.time_since_epoch().count();
			static uint64_t clickedTime;
			if(ImGui::Button("Advance Frame"))
				GRAPHICS::AdvanceFrame(saveFrames);
			if (ImGui::IsItemActivated()) {
				clickedTime = curTime;
			}

			if (ImGui::IsItemDeactivated())
			{
				clickedTime = (uint64_t)-1;
			}

			if (ImGui::IsItemActive() && ImGui::IsItemHovered() && (curTime - clickedTime) > 150000000) {
				GRAPHICS::AdvanceFrame(saveFrames);
			}
		}

		ImGui::Image(GRAPHICS::g_RayTracingLastFrameSRV, { 200 * GRAPHICS::g_Viewport_Width / GRAPHICS::g_Viewport_Height,200 });

		ImGui::TreePop();
	}

	if (ImGui::TreeNodeEx("Ray-Tracing Settings", ImGuiTreeNodeFlags_DefaultOpen))
	{
		static cb_RT_Info local_RT_Info{10};

		ImGui::SliderInt("Rays Per Pixel", reinterpret_cast<int*>(&local_RT_Info.rayCount), 1, 200);

		ImGui::SliderInt("Max Bounce Amount", reinterpret_cast<int*>(&local_RT_Info.rayMaxBounce), 1, 20);

		if(ImGui::Button("Save Settings"))
			GRAPHICS::SetRayTracingSettings(local_RT_Info);

		ImGui::TreePop();
	}

	//ImGui::ShowDemoWindow();

	ImGui::End();

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
