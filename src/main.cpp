#include "GUI/gui.h"
#include "../External/ImGui/imgui.h"
#include "../External/ImGui/imgui_impl_dx11.h"
#include "../External/ImGui/imgui_impl_win32.h"
#include "../External/ImGui/imgui_extensions.h"
#include "Graphics/Graphics.h"

#include <chrono>


/*

------------------ TODO ------------------
////// The order of these is RANDOM //////
+ Move camera to it's own class.
+ Spend an entire day on a single bug that no one would even notice.
- Add UI for rendering frame with selected sample amount and rendering animations.
- Adding objects.
+ UI for modifying added objects and camera parameters.
- Switch from using the deprecated D3DX11SaveTextureToFileA to the new lib
+ UI for ray trace settings.
~ Clean up RenderFrame in GRAPHICS namespace.
+ Fix gamma when writing to file GRAPHICS::SaveFrameToFile() 
+ Fix shader "random" values

*/

bool saveFrames = false;

static std::chrono::steady_clock::time_point last_frame_render_time;
uint64_t last_frame_time = 0;

int OnGui()
{
	ImGui::SetNextWindowSize({ 400, 600 }, ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSizeConstraints({ 400, 600 }, { FLT_MAX, FLT_MAX });
	ImGui::Begin("Scene", 0);

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

		if (ImGui::Button("Reset Image"))
		{
			GRAPHICS::ResetFrame();
		}

		ImGui::TreePop();
	}

	if (ImGui::TreeNodeEx("Ray-Tracing Settings", ImGuiTreeNodeFlags_DefaultOpen))
	{
		static cb_RT_Info local_RT_Info{};
		bool settingsChanged = false;

		ImGui::SliderInt("Rays Per Pixel", reinterpret_cast<int*>(&local_RT_Info.rayCount), 1, 200);
		if (ImGui::IsItemDeactivatedAfterEdit())
			settingsChanged = true;

		ImGui::SliderInt("Max Bounce Count", reinterpret_cast<int*>(&local_RT_Info.rayMaxBounce), 1, 20);
		if (ImGui::IsItemDeactivatedAfterEdit())
			settingsChanged = true;

		if(settingsChanged)
			GRAPHICS::SetRayTracingSettings(local_RT_Info);

		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Object Settings"))
	{
		bool dataChanged = false;
		bool objectDataChanged = false;
		for (int i = 0; i < GRAPHICS::g_pCB_Pixel_ObjectData->sphereCount; i++)
		{
			char name[10];
			sprintf_s(name, "Sphere %i", i+1);

			ImGui::BeginGroup();

			if (ImGui::TreeNodeEx(name, (GRAPHICS::g_pCB_Pixel_ObjectData->circle[i].material.material_Flags & MaterialFlags_Selected) == MaterialFlags_Selected ? ImGuiTreeNodeFlags_Selected : 0))
			{
				//if (ImGui::SliderFloat3("Pos", GRAPHICS::g_pCB_Pixel_ObjectData->circle[i].pos, -20, 20))
				if (ImGui::DragFloat3("Pos", GRAPHICS::g_pCB_Pixel_ObjectData->circle[i].pos, 0.1f, -20, 20)) {}
				if (ImGui::IsItemDeactivatedAfterEdit())
					objectDataChanged = true;

				if (ImGui::DragFloat("Radius", &GRAPHICS::g_pCB_Pixel_ObjectData->circle[i].radius, 0.01f, 0, 10)) {}
				if (ImGui::IsItemDeactivatedAfterEdit())
					objectDataChanged = true;

				ImGui::HeaderTitle("Material");

				if (ImGui::DragFloat3("Color", GRAPHICS::g_pCB_Pixel_ObjectData->circle[i].material.baseColor, 0.005f, 0, 1)) {}
				if (ImGui::IsItemDeactivatedAfterEdit())
					objectDataChanged = true;

				if (ImGui::DragFloat("Roughness", &GRAPHICS::g_pCB_Pixel_ObjectData->circle[i].material.roughness, 0.005f, 0, 1)) {}
				if (ImGui::IsItemDeactivatedAfterEdit())
					objectDataChanged = true;

				if (ImGui::DragFloat3("EColor", GRAPHICS::g_pCB_Pixel_ObjectData->circle[i].material.emissionColor, 0.005f, 0, 1)) {}
				if (ImGui::IsItemDeactivatedAfterEdit())
					objectDataChanged = true;

				if (ImGui::DragFloat("Emission", &GRAPHICS::g_pCB_Pixel_ObjectData->circle[i].material.emission, 0.05f, 0, 50)) {}
				if (ImGui::IsItemDeactivatedAfterEdit())
					objectDataChanged = true;

				ImGui::TreePop();
			}

			ImGui::EndGroup();

			if (ImGui::IsItemHovered()) {
				if ((GRAPHICS::g_pCB_Pixel_ObjectData->circle[i].material.material_Flags & MaterialFlags_Selected) != MaterialFlags_Selected) {
					GRAPHICS::g_pCB_Pixel_ObjectData->circle[i].material.material_Flags |= MaterialFlags_Selected;
					dataChanged = true;
				}
			}
			else if (GRAPHICS::g_pCB_Pixel_ObjectData->circle[i].material.material_Flags & MaterialFlags_Selected) {
				GRAPHICS::g_pCB_Pixel_ObjectData->circle[i].material.material_Flags ^= MaterialFlags_Selected;
				dataChanged = true;
			}
		}
		ImGui::TreePop();

		if (dataChanged || objectDataChanged) {
			GRAPHICS::CommitObjectData();
		}
		if (objectDataChanged)
			GRAPHICS::ResetFrame();
	}

	if (ImGui::TreeNode("Camera Settings"))
	{
		bool dataChanged = false;

		float vFov = GRAPHICS::g_pMainCamera->vFov;

		if (ImGui::DragFloat("Vertical Fov", &GRAPHICS::g_pMainCamera->vFov, 0.1f, 15, 179)) {}
		if (ImGui::IsItemDeactivatedAfterEdit())
			dataChanged = true;

		if (dataChanged) {
			GRAPHICS::g_pMainCamera->Set_vFov(GRAPHICS::g_pMainCamera->vFov);
			GRAPHICS::ResetFrame();
		}

		ImGui::TreePop();
	}

	//ImGui::ShowDemoWindow();

	ImGui::End();

	return 0;
}

int WINAPI WinMain(_In_ HINSTANCE instance, _In_opt_ HINSTANCE, _In_ char* cmdLine, _In_ int showCmd)
{
	AllocConsole();
	AttachConsole(GetCurrentProcessId());
	FILE* conStream;
	freopen_s(&conStream, "CON", "w", stdout);

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
