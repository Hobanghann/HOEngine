#include "EditorInspectorWindow.h"

#include <IconsMaterialDesignIcons.h>
#include <imgui.h>
#include <imgui_internal.h>

#include "../Editor.h"
#include "../Utils/EditorDisplayUtils.h"
#include "../Utils/PropertyDrawer.h"
#include "Platforms/IPlatformApplication.h"
#include "Scene/Components/Components.h"
#include "Scene/GameObjects/GameObject.h"
#include "Scene/Scene.h"

namespace ho
{
EditorInspectorWindow::EditorInspectorWindow(Editor* pOwner)
  : IEngineWindow(StringHandle("EditorInspectorWindow"))
  , mpOwner(pOwner)
{
}

EditorInspectorWindow::~EditorInspectorWindow() = default;

bool EditorInspectorWindow::OnInit()
{
    return true;
}

bool EditorInspectorWindow::OnPreUpdate(float deltaTime)
{
    return true;
}

bool EditorInspectorWindow::OnUpdate(float deltaTime)
{
    return true;
}

bool EditorInspectorWindow::OnPostUpdate(float deltaTime)
{
    return true;
}

bool EditorInspectorWindow::OnRender()
{
    return true;
}

void EditorInspectorWindow::OnShutdown() {}

void EditorInspectorWindow::DrawUI()
{
    ImGui::SetNextWindowSize(
        {static_cast<float>(IPlatformApplication::GetInstance().GetMainWindow()->GetClientWidth()) * 0.25f,
         static_cast<float>(IPlatformApplication::GetInstance().GetMainWindow()->GetClientHeight())},
        ImGuiCond_FirstUseEver);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(2.0f, 2.0f));

    if (ImGui::Begin("Inspector", &mbOpen, 0))
    {
        SetFocused(ImGui::IsWindowFocused());

        const ObjectHandle<RGameObject> hSelectedObj = mpOwner->GetSelectedGameObject();

        if (hSelectedObj.IsValid())
        {
            drawTransformComponent(hSelectedObj->GetComponent<RTransformComponent>());

            for (auto& hComp : hSelectedObj->GetComponents())
            {
                if (hComp.IsNULL() ||
                    hComp->GetClass() == ReflectionSystem::GetInstance().GetClassByType<RTransformComponent>())
                {
                    continue;
                }

                drawComponent(hComp);
            }

            const char* buttonText = "Add Component";

            const float buttonWidth = ImGui::CalcTextSize(buttonText).x + ImGui::GetStyle().FramePadding.x * 2.0f;

            const float offsetX = (ImGui::GetContentRegionAvail().x - buttonWidth) * 0.5f;

            if (offsetX > 0.0f)
            {
                ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offsetX);
            }

            if (ImGui::Button(buttonText))
            {
                mbOpenAddComponentPopup = true;
            }
            drawAddComponentPopup();
        }
    }
    ImGui::End();

    ImGui::PopStyleVar();
}

void EditorInspectorWindow::drawAddComponentPopup()
{
    if (mbOpenAddComponentPopup)
    {
        const ImVec2 buttonMin = ImGui::GetItemRectMin();
        const ImVec2 buttonSize = ImGui::GetItemRectSize();

        ImGui::SetNextWindowPos({buttonMin.x, buttonMin.y + buttonSize.y});

        ImGui::OpenPopup("##InspectorAddComponentPopup");
        mbOpenAddComponentPopup = false;
    }

    const ObjectHandle<RGameObject> hSelectedObj = mpOwner->GetSelectedGameObject();
    HO_ASSERT(hSelectedObj.IsValid(), "This function must be called with valid handle. Check caller.");

    ImGui::SetNextWindowSize(ImVec2(200.0f, 0.0f), ImGuiCond_Appearing);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4.0f, 8.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(ImGui::GetStyle().ItemSpacing.x, 6.0f));
    if (ImGui::BeginPopup("##InspectorAddComponentPopup", ImGuiWindowFlags_NoDecoration))
    {
        if (ImGui::BeginMenu("Mesh"))
        {
            if (ImGui::MenuItem(ICON_MDI_CUBE_OUTLINE " Static Mesh"))
            {
                if (!hSelectedObj->HasComponent<RStaticMeshComponent>())
                {
                    const ObjectHandle<RStaticMeshComponent> hNewComp =
                        hSelectedObj->GetOwner()->CreateComponent<RStaticMeshComponent>();
                    hSelectedObj->AttachComponent(hNewComp);
                }
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Light"))
        {
            if (ImGui::MenuItem(ICON_MDI_SUN_WIRELESS_OUTLINE " Directional Light"))
            {
                if (!hSelectedObj->HasComponent<RDirectionalLightComponent>())
                {
                    const ObjectHandle<RDirectionalLightComponent> hNewComp =
                        hSelectedObj->GetOwner()->CreateComponent<RDirectionalLightComponent>();
                    hSelectedObj->AttachComponent(hNewComp);
                }
                ImGui::CloseCurrentPopup();
            }
            if (ImGui::MenuItem(ICON_MDI_LIGHTBULB_ON_OUTLINE " Point Light"))
            {
                if (!hSelectedObj->HasComponent<RPointLightComponent>())
                {
                    const ObjectHandle<RPointLightComponent> hNewComp =
                        hSelectedObj->GetOwner()->CreateComponent<RPointLightComponent>();
                    hSelectedObj->AttachComponent(hNewComp);
                }
                ImGui::CloseCurrentPopup();
            }
            if (ImGui::MenuItem(ICON_MDI_FLASHLIGHT " Spot Light"))
            {
                if (!hSelectedObj->HasComponent<RSpotLightComponent>())
                {
                    const ObjectHandle<RSpotLightComponent> hNewComp =
                        hSelectedObj->GetOwner()->CreateComponent<RSpotLightComponent>();
                    hSelectedObj->AttachComponent(hNewComp);
                }
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Environment"))
        {
            if (ImGui::MenuItem(ICON_MDI_WEATHER_PARTLY_CLOUDY " Sky Box"))
            {
                if (!hSelectedObj->HasComponent<RSkyBoxComponent>())
                {
                    const ObjectHandle<RSkyBoxComponent> hNewComp =
                        hSelectedObj->GetOwner()->CreateComponent<RSkyBoxComponent>();
                    hSelectedObj->AttachComponent(hNewComp);
                    if (hSelectedObj->GetOwner()->GetMainSkyBox().IsNULL())
                    {
                        hSelectedObj->GetOwner()->SetMainSkyBox(hNewComp);
                    }
                }
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Camera"))
        {
            if (ImGui::MenuItem(ICON_MDI_CAMERA " Camera"))
            {
                if (!hSelectedObj->HasComponent<RCameraComponent>())
                {
                    const ObjectHandle<RCameraComponent> hNewComp =
                        hSelectedObj->GetOwner()->CreateComponent<RCameraComponent>();
                    hSelectedObj->AttachComponent(hNewComp);
                }
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndMenu();
        }

        ImGui::EndPopup();
    }

    ImGui::PopStyleVar(2);
}

void EditorInspectorWindow::drawComponent(ObjectHandle<RComponent> hComp)
{
    HO_ASSERT(hComp.IsValid(), "Component is null.");

    const ObjectHandle<RGameObject> hSelectedObj = mpOwner->GetSelectedGameObject();
    HO_ASSERT(hSelectedObj.IsValid(), "This function must be called with valid handle. Check caller.");

    bool bIsOpen = true;
    ImGui::PushID(hComp.Get());
    char buf[100] = {};
    snprintf(buf, 100, "%s %s", getComponentDisplayIcon(hComp), *hComp->GetClass().GetName());
    if (ImGui::CollapsingHeader(buf, &bIsOpen, ImGuiTreeNodeFlags_DefaultOpen))
    {
        if (ImGui::BeginChild("Info", ImVec2(0, 0), ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeY, 0))
        {
            PropertyDrawer::DrawClassProperties(hComp.Get());
        }
        ImGui::EndChild();
    }
    ImGui::PopID();

    if (!bIsOpen)
    {
        hSelectedObj->GetOwner()->DestroyComponent(hComp);
    }
}

void EditorInspectorWindow::drawTransformComponent(ObjectHandle<RTransformComponent> hTransformComp)
{
    HO_ASSERT(hTransformComp.IsValid(), "Component is null.");

    if (ImGui::CollapsingHeader(ICON_MDI_AXIS_ARROW " Transform", ImGuiTreeNodeFlags_DefaultOpen))
    {
        if (ImGui::BeginChild(
                "TransformComponentInfo", ImVec2(0, 0.0f), ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeY, 0))
        {
            if (ImGui::BeginTable(
                    "##TransformComponentTable", 2, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_PadOuterX))
            {
                bool bTransformChanged = false;

                ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed);
                ImGui::TableSetupColumn("Widget", ImGuiTableColumnFlags_WidthStretch);

                Vector3 scale = hTransformComp->GetScale();
                Vector3 eularAngle = hTransformComp->GetRotation().ToEuler() * math::RAD1_IN_DEG;
                Vector3 pos = hTransformComp->GetPosition();

                auto drawVector3Control = [&](const char* id,
                                              float* values,
                                              float speed,
                                              float minVal,
                                              float maxVal,
                                              ImGuiSliderFlags flags = 0)
                {
                    ImGui::PushID(id);

                    const float availWidth = ImGui::GetContentRegionAvail().x;
                    const float spacing = ImGui::GetStyle().ItemSpacing.x;
                    const float textWidth = ImGui::CalcTextSize("X").x;

                    float itemWidth = (availWidth - (textWidth * 3.0f) - (spacing * 5.0f)) / 3.0f;
                    if (itemWidth < 1.0f)
                    {
                        itemWidth = 1.0f;
                    }

                    ImGui::AlignTextToFramePadding();
                    ImGui::TextDisabled("X");
                    ImGui::SameLine();
                    ImGui::SetNextItemWidth(itemWidth);
                    if (ImGui::DragFloat("##X", &values[0], speed, minVal, maxVal, "%.3f", flags))
                    {
                        bTransformChanged = true;
                    }

                    ImGui::SameLine();

                    ImGui::AlignTextToFramePadding();
                    ImGui::TextDisabled("Y");
                    ImGui::SameLine();
                    ImGui::SetNextItemWidth(itemWidth);
                    if (ImGui::DragFloat("##Y", &values[1], speed, minVal, maxVal, "%.3f", flags))
                    {
                        bTransformChanged = true;
                    }

                    ImGui::SameLine();

                    ImGui::AlignTextToFramePadding();
                    ImGui::TextDisabled("Z");
                    ImGui::SameLine();
                    ImGui::SetNextItemWidth(itemWidth);
                    if (ImGui::DragFloat("##Z", &values[2], speed, minVal, maxVal, "%.3f", flags))
                    {
                        bTransformChanged = true;
                    }

                    ImGui::PopID();
                };

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::AlignTextToFramePadding();
                ImGui::TextUnformatted("Scale");

                ImGui::TableNextColumn();
                drawVector3Control("Scale", scale.Data, 0.05f, 0.1f, FLT_MAX, ImGuiSliderFlags_None);

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::AlignTextToFramePadding();
                ImGui::TextUnformatted("Rotation");

                ImGui::TableNextColumn();
                drawVector3Control("Rotation", eularAngle.Data, 0.05f, -180.0f, 180.0f, ImGuiSliderFlags_WrapAround);

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::AlignTextToFramePadding();
                ImGui::TextUnformatted("Position");

                ImGui::TableNextColumn();
                drawVector3Control("Position", pos.Data, 0.05f, 0.0f, 0.0f, ImGuiSliderFlags_None);

                ImGui::EndTable();

                if (bTransformChanged)
                {
                    eularAngle *= math::DEG1_IN_RAD;
                    hTransformComp->SetScale(scale);
                    hTransformComp->SetRotation(Quaternion::FromEuler(eularAngle.X, eularAngle.Y, eularAngle.Z));
                    hTransformComp->SetPosition(pos);
                }
            }
        }
        ImGui::EndChild();
    }
}

} // namespace ho