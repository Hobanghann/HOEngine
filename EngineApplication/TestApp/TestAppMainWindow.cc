#include "TestAppMainWindow.h"

#include <imgui.h>

#include "Subsystems/UI/UISystem.h"

namespace ho
{
TestAppMainWindow::TestAppMainWindow()
  : IEngineWindow(StringHandle("TestAppMainWindow"))
{
}

void TestAppMainWindow::DrawUI()
{
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

    static const ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
                                          ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoSavedSettings |
                                          ImGuiWindowFlags_NoBringToFrontOnFocus;

    if (ImGui::Begin("##TestAppMainWindowID", nullptr, flags))
    {
        const auto& titleBarTheme = UISystem::GetInstance().GetTitleBarTheme();

        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(ImGui::GetStyle().ItemSpacing.x, 0.0f));
        drawMainWindowTitleBar(titleBarTheme);
        ImGui::PopStyleVar();

        ImVec2 clientAreaSize = ImGui::GetContentRegionAvail();

        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 0.0f);

        static const ImGuiWindowFlags childFlags =
            ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings;

        if (ImGui::BeginChild("##ClientArea", clientAreaSize, 0, childFlags))
        {
            // Draw docking space
            ImGuiID dockspaceId = ImGui::GetID("TestAppWindowDockSpace");
            ImVec2 remainingSpace = ImGui::GetContentRegionAvail();
            ImGui::DockSpace(dockspaceId, remainingSpace, ImGuiDockNodeFlags_None);
        }
        ImGui::EndChild();

        ImGui::PopStyleVar();
    }
    ImGui::End();

    ImGui::PopStyleVar(2);
}

} // namespace ho