#include "EditorMainWindow.h"

#include <charconv>
#include <imgui.h>
#include <imgui_internal.h>

#include "../Editor.h"
#include "Scene/SceneManager.h"
#include "Subsystems/Asset/AssetSystem.h"
#include "Subsystems/Rendering/RenderingSystem.h"
#include "Subsystems/UI/UISystem.h"

namespace ho
{
void setupDockLayout(const ImGuiID dockspaceId, const ImVec2& dockspaceSize);

EditorMainWindow::EditorMainWindow(Editor* pOwner)
  : IEngineWindow(StringHandle("EditorMainWindow"))
  , mPendingBakingLightQueue(sMaxPendingBakingLightQueueSize)
  , mhPendingShadowMapResetSceneQueue(sMaxPendingShadowMapResetSceneQueueSize)
  , mpOwner(pOwner)
{
}

EditorMainWindow::~EditorMainWindow() = default;

bool EditorMainWindow::OnInit()
{
    return true;
}

bool EditorMainWindow::OnPreUpdate(float deltaTime)
{
    return true;
}

bool EditorMainWindow::OnUpdate(float deltaTime)
{
    if (mPendingBakingLightQueue.IsEmpty())
    {
        if (!mhPendingShadowMapResetSceneQueue.IsEmpty())
        {
            const ObjectHandle<RScene> hScene = mhPendingShadowMapResetSceneQueue.Front();
            if (hScene.IsValid() && !hScene->IsPendingDestroyed())
            {
                for (const ObjectHandle<RLightComponent> hLightComp : hScene->GetComponents<RLightComponent>())
                {
                    if (hLightComp->IsPendingDestroyed())
                    {
                        continue;
                    }
                    if (hLightComp->GetStaticShadowMap().IsValid())
                    {
                        AssetSystem::GetInstance().DeleteAsset(hLightComp->GetStaticShadowMap());
                    }
                    hLightComp->SetStaticShadowMap(ObjectHandle<RTextureAsset>::sNULL);
                }
            }
            mhPendingShadowMapResetSceneQueue.Pop();
        }
    }
    return true;
}

bool EditorMainWindow::OnPostUpdate(float deltaTime)
{
    return true;
}

bool EditorMainWindow::OnRender()
{
    if (!mPendingBakingLightQueue.IsEmpty())
    {
        const ObjectHandle<RLightComponent> hLight = mPendingBakingLightQueue.Front();
        if (hLight.IsValid() && !hLight->IsPendingDestroyed())
        {
            switch (hLight->GetType())
            {
                case RLightComponent::eLightType::Directional:
                    // TODO: Directional light map baking is not supported currently.
                    break;
                case RLightComponent::eLightType::Point:
                    mpOwner->GetSceneRenderer().BakePointLightStaticShadowMap(hLight.Cast<RPointLightComponent>(),
                                                                              hLight->GetOwner()->GetOwner());
                    break;
                case RLightComponent::eLightType::Spot:
                    mpOwner->GetSceneRenderer().BakeSpotLightStaticShadowMap(hLight.Cast<RSpotLightComponent>(),
                                                                             hLight->GetOwner()->GetOwner());
                    break;
                default:
                    HO_ASSERT(false, "Invalid light type.");
            }
        }
        mPendingBakingLightQueue.Pop();
    }

    return true;
}

void EditorMainWindow::OnShutdown() {}

void EditorMainWindow::DrawUI()
{
    drawMainWindow();
    drawPopups();
}

void EditorMainWindow::drawMainWindow()
{
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

    static const ImGuiWindowFlags sFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
                                           ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoSavedSettings |
                                           ImGuiWindowFlags_NoBringToFrontOnFocus;

    if (ImGui::Begin("##EditorMainWindow", nullptr, sFlags))
    {
        SetFocused(ImGui::IsWindowFocused());

        const auto& titleBarTheme = UISystem::GetInstance().GetTitleBarTheme();

        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(ImGui::GetStyle().ItemSpacing.x, 0.0f));
        drawMainWindowTitleBar(titleBarTheme);
        ImGui::PopStyleVar();

        const ImVec2 clientAreaSize = ImGui::GetContentRegionAvail();

        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 0.0f);

        static const ImGuiWindowFlags sChildFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
                                                    ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_MenuBar;

        if (ImGui::BeginChild("##ClientArea", clientAreaSize, 0, sChildFlags))
        {
            drawMenuBar();

            // Draw docking space
            const ImGuiID dockspaceId = ImGui::GetID("##MainWindowDockSpace");
            const ImVec2 dockspaceSize = ImGui::GetContentRegionAvail();

            setupDockLayout(dockspaceId, dockspaceSize);

            ImGui::DockSpace(dockspaceId, dockspaceSize, ImGuiDockNodeFlags_None);
        }
        ImGui::EndChild();

        ImGui::PopStyleVar();
    }
    ImGui::End();

    ImGui::PopStyleVar(2);
}

void EditorMainWindow::drawMenuBar()
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.0f, 6.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, ImGui::GetStyle().WindowBorderSize);
    if (ImGui::BeginMenuBar())
    {
        drawFileMenu();
        drawProjectMenu();
        drawBuildMenu();
        drawWindowMenu();
        drawSettingsMenu();

        ImGui::EndMenuBar();
    }
    ImGui::PopStyleVar(2);
}

void EditorMainWindow::drawFileMenu()
{
    if (ImGui::BeginMenu("File"))
    {
        if (ImGui::MenuItem("New Scene", nullptr))
        {
            mbOpenNewScenePopup = true;
        }
        if (ImGui::MenuItem("Open Scene", nullptr))
        {
        }
        if (ImGui::MenuItem("Save Scene", nullptr))
        {
        }
        ImGui::EndMenu();
    }
}

void EditorMainWindow::drawProjectMenu()
{
    if (ImGui::BeginMenu("Project"))
    {
        if (ImGui::MenuItem("New Project", nullptr))
        {
        }
        if (ImGui::MenuItem("Open Project", nullptr))
        {
        }
        if (ImGui::MenuItem("Save Project", nullptr))
        {
        }
        ImGui::EndMenu();
    }
}

void EditorMainWindow::drawBuildMenu()
{
    if (ImGui::BeginMenu("Build"))
    {
        if (ImGui::MenuItem("Bake Shadow Maps", nullptr))
        {
            const ObjectHandle<RScene> hCurrScene = mpOwner->GetCurrentScene();
            if (hCurrScene.IsValid())
            {
                for (const ObjectHandle<RLightComponent> hLightComp : hCurrScene->GetComponents<RLightComponent>())
                {
                    if (!hLightComp->IsCastingShadow() || hLightComp->IsPendingDestroyed())
                    {
                        continue;
                    }
                    mPendingBakingLightQueue.Push(hLightComp);
                }
            }
        }
        if (ImGui::MenuItem("Reset Shadow Maps", nullptr))
        {
            const ObjectHandle<RScene> hCurrScene = mpOwner->GetCurrentScene();
            if (hCurrScene.IsValid())
            {
                mhPendingShadowMapResetSceneQueue.Push(hCurrScene);
            }
        }
        ImGui::EndMenu();
    }
}

void EditorMainWindow::drawWindowMenu()
{
    if (ImGui::BeginMenu("Window"))
    {
        if (ImGui::MenuItem("Scene Hierarchy", nullptr, mpOwner->mpWindows[1]->IsOpen()))
        {
            mpOwner->mpWindows[1]->SetOpen(!mpOwner->mpWindows[1]->IsOpen());
        }
        if (ImGui::MenuItem("Scene View", nullptr, mpOwner->mpWindows[2]->IsOpen()))
        {
            mpOwner->mpWindows[2]->SetOpen(!mpOwner->mpWindows[2]->IsOpen());
        }
        if (ImGui::MenuItem("Camera Preview", nullptr, mpOwner->mpWindows[3]->IsOpen()))
        {
            mpOwner->mpWindows[3]->SetOpen(!mpOwner->mpWindows[3]->IsOpen());
        }
        if (ImGui::MenuItem("Inspector", nullptr, mpOwner->mpWindows[4]->IsOpen()))
        {
            mpOwner->mpWindows[4]->SetOpen(!mpOwner->mpWindows[4]->IsOpen());
        }
        if (ImGui::MenuItem("Asset Browser", nullptr, mpOwner->mpWindows[5]->IsOpen()))
        {
            mpOwner->mpWindows[5]->SetOpen(!mpOwner->mpWindows[5]->IsOpen());
        }
        if (ImGui::MenuItem("Console", nullptr, mpOwner->mpWindows[6]->IsOpen()))
        {
            mpOwner->mpWindows[6]->SetOpen(!mpOwner->mpWindows[6]->IsOpen());
        }
        if (ImGui::BeginMenu("Debug"))
        {
            if (ImGui::MenuItem("ImGui Metrics", nullptr, mbOpenImGuiMetricsWindow))
            {
                mbOpenImGuiMetricsWindow = !mbOpenImGuiMetricsWindow;
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenu();
    }
}

void EditorMainWindow::drawSettingsMenu()
{
    if (ImGui::BeginMenu("Settings"))
    {
        if (ImGui::MenuItem("VSync", nullptr, RenderingSystem::GetInstance().IsVSyncEnabled()))
        {
            RenderingSystem::GetInstance().SetVSync(!RenderingSystem::GetInstance().IsVSyncEnabled());
        }
        ImGui::EndMenu();
    }
}

void EditorMainWindow::drawPopups()
{
    drawNewScenePopup();

    if (mbOpenImGuiMetricsWindow)
    {
        ImGui::ShowMetricsWindow(&mbOpenImGuiMetricsWindow);
    }
}

void EditorMainWindow::drawNewScenePopup()
{
    if (mbOpenNewScenePopup)
    {
        ImGui::SetNextWindowSize(ImVec2(400.0f, 0.0f), ImGuiCond_Appearing);
        ImGui::OpenPopup("New Scene##MainWindowNewScenePopup");
        mbOpenNewScenePopup = false;
    }

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4.0f, 8.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(ImGui::GetStyle().ItemSpacing.x, 10.0f));
    if (ImGui::BeginPopupModal("New Scene##MainWindowNewScenePopup",
                               nullptr,
                               ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse))
    {
        static char sNameBuf[256] = "";

        ImGui::AlignTextToFramePadding();
        ImGui::Text("Scene Name:");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(-1.0f);
        ImGui::InputText("##SceneNameInputText", sNameBuf, HO_ARRAY_SIZE(sNameBuf));

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        const float windowWidth = ImGui::GetContentRegionAvail().x;
        const float buttonWidth = 80.0f;
        const float allButtonWidth = buttonWidth + buttonWidth + ImGui::GetStyle().ItemSpacing.x;

        const float beginX = (windowWidth - allButtonWidth) * 0.5f;

        if (beginX > 0.0f)
        {
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + beginX);
        }

        const bool bIsInputEmpty = (strlen(sNameBuf) == 0);

        if (bIsInputEmpty)
        {
            ImGui::BeginDisabled(true);
        }

        if (ImGui::Button("Create", ImVec2(buttonWidth, 0)) || (!bIsInputEmpty && ImGui::IsKeyPressed(ImGuiKey_Enter)))
        {
            const std::string newSceneNameStr = getUniqueSceneName(std::string(sNameBuf));
            const ObjectHandle<RScene> hNewScene =
                SceneManager::GetInstance().CreateScene(StringHandle(newSceneNameStr));
            HO_ASSERT(hNewScene.IsValid(), "Failed to create scene");
            mpOwner->SetCurrentScene(hNewScene);
            sNameBuf[0] = '\0';
            ImGui::CloseCurrentPopup();
        }

        if (bIsInputEmpty)
        {
            ImGui::EndDisabled();
        }

        ImGui::SameLine();

        if (ImGui::Button("Cancel", ImVec2(buttonWidth, 0)) || ImGui::IsKeyPressed(ImGuiKey_Escape))
        {
            sNameBuf[0] = '\0';
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
    ImGui::PopStyleVar(2);
}

void setupDockLayout(const ImGuiID dockspaceId, const ImVec2& dockspaceSize)
{
    if (ImGui::DockBuilderGetNode(dockspaceId) != nullptr)
    {
        return;
    }

    ImGui::DockBuilderAddNode(dockspaceId, ImGuiDockNodeFlags_DockSpace);
    ImGui::DockBuilderSetNodeSize(dockspaceId, dockspaceSize);

    ImGuiID topId = dockspaceId;
    ImGuiID bottomId = 0;

    ImGui::DockBuilderSplitNode(topId, ImGuiDir_Down, 0.25f, &bottomId, &topId);

    ImGuiID hierarchyId = 0;
    ImGuiID topRemainingId = 0;

    ImGui::DockBuilderSplitNode(topId, ImGuiDir_Left, 0.20f, &hierarchyId, &topRemainingId);

    ImGuiID inspectorId = 0;
    ImGuiID sceneViewId = 0;

    ImGui::DockBuilderSplitNode(topRemainingId, ImGuiDir_Right, 0.25f, &inspectorId, &sceneViewId);

    ImGuiID assetBrowserId = 0;
    ImGuiID consoleId = 0;

    ImGui::DockBuilderSplitNode(bottomId, ImGuiDir_Right, 0.35f, &consoleId, &assetBrowserId);

    // Dock windows.
    ImGui::DockBuilderDockWindow("Scene Hierarchy", hierarchyId);
    ImGui::DockBuilderDockWindow("Scene View", sceneViewId);
    ImGui::DockBuilderDockWindow("Camera Preview", sceneViewId);
    ImGui::DockBuilderDockWindow("Inspector", inspectorId);
    ImGui::DockBuilderDockWindow("Asset Browser", assetBrowserId);
    ImGui::DockBuilderDockWindow("Console", consoleId);

    ImGui::DockBuilderFinish(dockspaceId);
}

// NOTE: Supposed duplicated name handled to 'Name (n)'
std::string EditorMainWindow::getUniqueSceneName(const std::string& baseNameStr) const
{
    static const int32_t sMaxDupCount = 2048;
    static std::vector<bool> sbIsNumUsed(sMaxDupCount);

    std::fill(sbIsNumUsed.begin(), sbIsNumUsed.end(), false);
    int32_t maxNum = -1;
    for (const auto& hScene : SceneManager::GetInstance().GetScenes())
    {
        std::string sceneNameStr(*hScene->GetName());
        if (baseNameStr.size() > sceneNameStr.size())
        {
            continue;
        }

        const std::string::size_type idx = sceneNameStr.find(baseNameStr);
        if (idx != 0)
        {
            continue;
        }

        int32_t num = -1;
        if (baseNameStr.size() == sceneNameStr.size())
        {
            num = 0;
        }
        else if (sceneNameStr.find(" (", idx) == baseNameStr.size() && sceneNameStr[sceneNameStr.size() - 1] == ')')
        {
            const char* first = sceneNameStr.c_str() + baseNameStr.size() + 2;
            const char* last = sceneNameStr.c_str() + sceneNameStr.size() - 1;

            int value = 0;
            auto result = std::from_chars(first, last, value);

            if (result.ec == std::errc{} && result.ptr == last)
            {
                num = value;
            }
        }

        HO_ASSERT(num < sMaxDupCount, "Duplicated object count exceeds maximum limit.");
        if (num >= 0)
        {
            sbIsNumUsed[num] = true;
            if (maxNum < num)
            {
                maxNum = num;
            }
        }
    }

    int32_t targetNum = -1;
    for (int32_t i = 0; i <= maxNum; ++i)
    {
        if (!sbIsNumUsed[i])
        {
            targetNum = i;
            break;
        }
    }

    if (targetNum == -1)
    {
        targetNum = maxNum + 1;
    }

    if (targetNum == 0)
    {
        return baseNameStr;
    }

    return std::string(baseNameStr).append(" (").append(std::to_string(targetNum)).append(")");
}
} // namespace ho