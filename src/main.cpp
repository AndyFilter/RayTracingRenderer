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
+ Adding objects.
+ UI for modifying added objects and camera parameters.
- Switch from using the deprecated D3DX11SaveTextureToFileA to the new lib
+ UI for ray trace settings.
~ Clean up RenderFrame in GRAPHICS namespace.
+ Fix gamma when writing to file GRAPHICS::SaveFrameToFile() 
+ Fix shader "random" values
- Environment light

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

	bool isFrameDirty = false;

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

			static int i = 0; // Used to slow down the advance speed by 2 (it's the modulo in the line below,	\/)
			if (ImGui::IsItemActive() && ImGui::IsItemHovered() && (curTime - clickedTime) > 150000000 && i++ % 2 == 0) {
				GRAPHICS::AdvanceFrame(saveFrames);
			}
		}

		ImGui::Image(GRAPHICS::g_RayTracingLastFrameSRV, { 200 * GRAPHICS::g_Viewport_Width / GRAPHICS::g_Viewport_Height,200 });

		if (ImGui::Button("Reset Image"))
		{
			isFrameDirty = true;
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

		if (settingsChanged) {
			GRAPHICS::SetRayTracingSettings(local_RT_Info);
			isFrameDirty = true;
		}

		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Object Settings"))
	{
		bool dataChanged = false;
		bool objectDataChanged = false;
		//int deletedIdx = -1;
		for (int i = 0; i < GRAPHICS::g_pCB_Pixel_ObjectData->sphereCount; i++)
		{
			char name[10];
			sprintf_s(name, "Sphere %i", i+1);

			ImGui::BeginGroup();

			ImGui::AlignTextToFramePadding();
			bool treeNodeClicked = ImGui::TreeNodeEx(name, ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap | ((GRAPHICS::g_pCB_Pixel_ObjectData->circle[i].material.material_Flags & MaterialFlags_Selected) == MaterialFlags_Selected ? ImGuiTreeNodeFlags_Selected : 0));
			ImGui::SameLine();
			ImGui::Dummy({ ImGui::GetContentRegionAvail().x - 28, 5 });
			ImGui::SameLine();
			//ImGui::PushID(1375911 + i);
			ImGui::PushID(i);
			if (ImGui::Button("X", {20, 0}))
			{
				if(treeNodeClicked)
					ImGui::TreePop();
				ImGui::EndGroup();
				memcpy(GRAPHICS::g_pCB_Pixel_ObjectData->circle + i, GRAPHICS::g_pCB_Pixel_ObjectData->circle + i + 1, (GRAPHICS::g_pCB_Pixel_ObjectData->sphereCount - i - 1) * sizeof(SphereEq));
				GRAPHICS::g_pCB_Pixel_ObjectData->sphereCount--;
				i--;
				objectDataChanged = true;
				ImGui::PopID();
				continue;
			}
			ImGui::PopID();

			if (treeNodeClicked)
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

				ImGui::Separator();

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

		if (ImGui::Button("+", { -1,0 }))
		{
			if (GRAPHICS::g_pCB_Pixel_ObjectData->sphereCount < MAX_SPHERE_COUNT)
			{
				SphereEq* newSphere = &GRAPHICS::g_pCB_Pixel_ObjectData->circle[GRAPHICS::g_pCB_Pixel_ObjectData->sphereCount++];
				newSphere->pos[2] = 10;
				objectDataChanged = true;
			}
				
		}

		if (dataChanged || objectDataChanged) {
			GRAPHICS::CommitObjectData();
		}
		if (objectDataChanged)
			isFrameDirty = true;

		ImGui::TreePop();
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
			isFrameDirty = true;
		}

		ImGui::TreePop();
	}

	if (isFrameDirty)
		GRAPHICS::ResetFrame();

	//ImGui::ShowDemoWindow();

	ImGui::End();

	return 0;
}

int WINAPI WinMain(_In_ HINSTANCE instance, _In_opt_ HINSTANCE, _In_ char* cmdLine, _In_ int showCmd)
{
#ifdef _DEBUG
	AllocConsole();
	AttachConsole(GetCurrentProcessId());
	FILE* conStream;
	freopen_s(&conStream, "CON", "w", stdout);
#endif

	// Gui sets-up GRAPHICS
	HWND hwnd = GUI::Setup(instance, OnGui);

	ImGui::GetIO().IniFilename = nullptr;

	// Try to create directory for rendered frames
	CreateDirectory(L"Rendered", NULL);

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
