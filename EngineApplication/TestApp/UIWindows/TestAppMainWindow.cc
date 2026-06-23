#include "TestAppMainWindow.h"

#include <imgui.h>

#include "Subsystems/Rendering/IRenderingSystem.h"

namespace ho
{
TestAppMainWindow::TestAppMainWindow(StringHandle hMainFrameBufferName)
  : IUIWindow(StringHandle("TestAppMainWindow"))
  , mhMainFrameBufferName(hMainFrameBufferName)
{
}

void TestAppMainWindow::onGUI()
{
    static ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
                                    ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus;

    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    bool bIsOpen = true;
    if (ImGui::Begin("Main Render Target", &bIsOpen, flags))
    {
        ImTextureID textureID = (ImTextureID)(intptr_t)IRenderingSystem::GetInstance().GetRenderTargetNativeHandle(
            mhMainFrameBufferName, IRenderingSystem::eRenderTargetType::Color);

        ImVec2 uv0(0.0f, 1.0f);
        ImVec2 uv1(1.0f, 0.0f);

        ImGui::Image(textureID, viewport->Size, uv0, uv1);
    }
    ImGui::End();

    ImGui::PopStyleVar();
}

} // namespace ho