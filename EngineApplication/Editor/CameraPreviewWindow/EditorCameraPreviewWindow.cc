#include "EditorCameraPreviewWindow.h"

#include <IconsMaterialDesignIcons.h>
#include <imgui.h>

#include "../Editor.h"
#include "Platforms/IPlatformApplication.h"
#include "Scene/Scene.h"
#include "Subsystems/Rendering/RenderingSystem.h"

namespace ho
{
struct SceneViewArea
{
    ImVec2 Position;
    ImVec2 ScreenPosition;
    ImVec2 Size;
};

EditorCameraPreviewWindow::EditorCameraPreviewWindow(Editor* pOwner)
  : IEngineWindow(StringHandle("EditorCameraPreviewWindow"))
  , mhFrameBufferName(StringHandle("CameraPreviewFrameBuffer"))
  , mpOwner(pOwner)
{
}

EditorCameraPreviewWindow::~EditorCameraPreviewWindow() {}

bool EditorCameraPreviewWindow::OnInit()
{
    // Create FrameBuffer
    FrameBufferDesc frameBufferDesc;
    frameBufferDesc.hName = mhFrameBufferName;
    frameBufferDesc.Width = mFrameBufferWidth;
    frameBufferDesc.Height = mFrameBufferHeight;
    frameBufferDesc.ColorTargetFormats[0] = eTextureFormat::R8G8B8A8_UNORM;
    frameBufferDesc.ColorTargetFormats[1] = eTextureFormat::R16G16B16A16_FLOAT;
    frameBufferDesc.DepthTargetFormat = eTextureFormat::D24_UNORM_S8_UINT;
    if (!RenderingSystem::GetInstance().CreateFramebuffer(frameBufferDesc))
    {
        return false;
    }
    return true;
}

bool EditorCameraPreviewWindow::OnPreUpdate(float deltaTime)
{
    return true;
}

bool EditorCameraPreviewWindow::OnUpdate(float deltaTime)
{
    if (mbNeedResizeFrameBuffer)
    {
        if (!RenderingSystem::GetInstance().DestroyFramebuffer(mhFrameBufferName))
        {
            return false;
        }
        FrameBufferDesc frameBufferDesc;
        frameBufferDesc.hName = mhFrameBufferName;
        frameBufferDesc.Width = mFrameBufferWidth;
        frameBufferDesc.Height = mFrameBufferHeight;
        frameBufferDesc.ColorTargetFormats[0] = eTextureFormat::R8G8B8A8_UNORM;
        frameBufferDesc.ColorTargetFormats[1] = eTextureFormat::R16G16B16A16_FLOAT;
        frameBufferDesc.DepthTargetFormat = eTextureFormat::D24_UNORM_S8_UINT;
        if (!RenderingSystem::GetInstance().CreateFramebuffer(frameBufferDesc))
        {
            return false;
        }

        mbNeedResizeFrameBuffer = false;
    }

    return true;
}

bool EditorCameraPreviewWindow::OnPostUpdate(float deltaTime)
{
    return true;
}

bool EditorCameraPreviewWindow::OnRender()
{
    if (!mbOpen)
    {
        return true;
    }

    if (mhCameraComp.IsValid() && !mhCameraComp->IsDestroyed())
    {
        const ObjectHandle<const RTransformComponent> hCameraTransformComp =
            mhCameraComp->GetOwner()->GetComponent<RTransformComponent>();
        const Matrix4x4 viewMat = hCameraTransformComp->GetWorldTransform().Orthonormalized().InverseFast().Matrix;
        mpOwner->GetSceneRenderer().RenderForwardWithExternalCamera(
            mRenderingMode,
            mRenderingFlags,
            mhCameraComp->GetOwner()->GetOwner(),
            mhFrameBufferName,
            hCameraTransformComp->GetWorldTransform().GetPosition(),
            viewMat,
            mhCameraComp->GetProjection());
    }

    return true;
}

void EditorCameraPreviewWindow::OnShutdown()
{
    RenderingSystem::GetInstance().DestroyFramebuffer(mhFrameBufferName);
}

void EditorCameraPreviewWindow::DrawUI()
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

    static constexpr ImGuiWindowFlags sFlags =
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;

    if (ImGui::Begin("Camera Preview", &mbOpen, sFlags))
    {
        SetFocused(ImGui::IsWindowFocused());

        drawConfigWindow();

        const SceneViewArea area = calculateSceneViewArea();

        drawSceneViewImage(area);
    }
    ImGui::End();

    ImGui::PopStyleVar(2);
}

SceneViewArea EditorCameraPreviewWindow::calculateSceneViewArea() const
{
    SceneViewArea area;
    area.Size = ImGui::GetContentRegionAvail();
    area.Position = ImGui::GetCursorPos();
    area.ScreenPosition = ImGui::GetCursorScreenPos();

    if (mbDisplayFitMode)
    {
        const float aspectRatio = static_cast<float>(mFrameBufferWidth) / static_cast<float>(mFrameBufferHeight);

        const float workAspectRatio = area.Size.x / area.Size.y;

        if (workAspectRatio > aspectRatio)
        {
            area.Size.x = area.Size.y * aspectRatio;
        }
        else
        {
            area.Size.y = area.Size.x / aspectRatio;
        }

        const ImVec2 offset((ImGui::GetContentRegionAvail().x - area.Size.x) * 0.5f,
                            (ImGui::GetContentRegionAvail().y - area.Size.y) * 0.5f);

        area.Position.x += offset.x;
        area.Position.y += offset.y;

        area.ScreenPosition.x += offset.x;
        area.ScreenPosition.y += offset.y;
    }

    return area;
}

void EditorCameraPreviewWindow::drawSceneViewImage(const SceneViewArea& area)
{
    const ImTextureID textureID = static_cast<ImTextureID>(
        reinterpret_cast<intptr_t>(RenderingSystem::GetInstance().GetRenderResultNativeHandle(mhFrameBufferName)));

    ImGui::SetCursorPos(area.Position);
    ImGui::SetNextItemAllowOverlap();

    ImGui::Image(textureID, area.Size, ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f));
}

void EditorCameraPreviewWindow::drawConfigWindow()
{
    static constexpr float sConfigWindowHeight = 30.0f;
    static constexpr float sPadding = sConfigWindowHeight * 0.1f;
    static constexpr float sButtonHeight = sConfigWindowHeight - sPadding * 2.0f;

    static constexpr ImGuiWindowFlags sFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
                                               ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoDocking |
                                               ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoSavedSettings;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVarY(ImGuiStyleVar_ItemSpacing, 0.0f);

    if (ImGui::BeginChild("##CameraPreviewConfigsWindow", ImVec2(0.0f, sConfigWindowHeight), 0, sFlags))
    {
        ImGui::SetCursorPos({sPadding, sPadding});

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));

        drawCameraSelectionCombo();

        ImGui::SameLine();

        drawSceneViewOptions(sButtonHeight, sPadding);

        ImGui::PopStyleColor();
    }
    ImGui::EndChild();
    ImGui::PopStyleVar(2);
}

void EditorCameraPreviewWindow::drawCameraSelectionCombo()
{
    const char* currentPreview = mhCameraComp.IsNULL() ? "None" : *(mhCameraComp->GetOwner()->GetName());
    if (ImGui::BeginCombo("##CameraCombo", currentPreview, ImGuiComboFlags_PopupAlignLeft))
    {
        if (mpOwner->GetCurrentScene().IsValid())
        {
            for (const auto& hCameraComp : mpOwner->GetCurrentScene()->GetComponents<const RCameraComponent>())
            {
                if (hCameraComp->IsDestroyed())
                {
                    continue;
                }
                const bool bSelected = (mhCameraComp == hCameraComp);
                if (ImGui::Selectable(*hCameraComp->GetOwner()->GetName(), bSelected))
                {
                    mhCameraComp = hCameraComp;
                }

                if (bSelected)
                {
                    ImGui::SetItemDefaultFocus();
                }
            }
        }
        ImGui::EndCombo();
    }
}

void EditorCameraPreviewWindow::drawSceneViewOptions(float buttonHeight, float padding)
{
    const char* renderingModeText = getRenderingModeName(mRenderingMode);
    const char* visibilityText = " " ICON_MDI_EYE_OUTLINE " ";
    const char* settingsText = " " ICON_MDI_COG " ";

    const ImGuiStyle& style = ImGui::GetStyle();
    const float framePaddingX = style.FramePadding.x;
    const float itemSpacingX = style.ItemSpacing.x;

    const float rightGroupWidth = ImGui::CalcTextSize(renderingModeText).x + framePaddingX * 2.0f +
                                  ImGui::CalcTextSize(visibilityText).x + framePaddingX * 2.0f +
                                  ImGui::CalcTextSize(settingsText).x + framePaddingX * 2.0f + itemSpacingX * 2.0f;

    const float contentRightX = ImGui::GetWindowContentRegionMax().x;
    const float groupStartX = contentRightX - rightGroupWidth - padding;

    if (groupStartX > ImGui::GetCursorPosX())
    {
        ImGui::SetCursorPosX(groupStartX);
    }

    if (ImGui::Button(renderingModeText, ImVec2(0.0f, buttonHeight)))
    {
        mbOpenRenderingModeSettingPopup = true;
    }
    drawRenderingModeSettingPopup();

    ImGui::SameLine();

    if (ImGui::Button(visibilityText, ImVec2(0.0f, buttonHeight)))
    {
        mbOpenVisibilitySettingPopup = true;
    }
    drawVisibilitySettingPopup();

    ImGui::SameLine();

    if (ImGui::Button(settingsText, ImVec2(0.0f, buttonHeight)))
    {
        mbOpenSettingPopup = true;
    }
    drawSettingPopup();
}

void EditorCameraPreviewWindow::drawRenderingModeSettingPopup()
{
    if (mbOpenRenderingModeSettingPopup)
    {
        const ImVec2 buttonMin = ImGui::GetItemRectMin();
        const ImVec2 buttonSize = ImGui::GetItemRectSize();

        ImGui::SetNextWindowPos({buttonMin.x, buttonMin.y + buttonSize.y});
        ImGui::OpenPopup("##RenderingModeSettingPopup");
        mbOpenRenderingModeSettingPopup = false;
    }

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.0f, 8.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(ImGui::GetStyle().ItemSpacing.x, 6.0f));

    if (ImGui::BeginPopup("##RenderingModeSettingPopup", ImGuiWindowFlags_NoDecoration))
    {
        ImGui::PushStyleVar(ImGuiStyleVar_SeparatorTextBorderSize, 1.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_SeparatorTextPadding, ImVec2(0.0f, 10.0f));

        ImGui::SeparatorText("VIEW MODE");

        static const eRenderingMode sRenderingModes[] = {
            eRenderingMode::Unlit,
            eRenderingMode::Lit,
            eRenderingMode::Wireframe,
            eRenderingMode::LitWireframe,
            eRenderingMode::DetailLighting,
            eRenderingMode::LightingOnly,
            eRenderingMode::Reflection,
        };

        for (auto renderingMode : sRenderingModes)
        {
            if (ImGui::MenuItem(getRenderingModeName(renderingMode), nullptr, mRenderingMode == renderingMode))
            {
                mRenderingMode = renderingMode;
            }
        }

        ImGui::PopStyleVar(2);
        ImGui::EndPopup();
    }

    ImGui::PopStyleVar(2);
}

void EditorCameraPreviewWindow::drawVisibilitySettingPopup()
{
    if (mbOpenVisibilitySettingPopup)
    {
        const ImVec2 buttonMin = ImGui::GetItemRectMin();
        const ImVec2 buttonSize = ImGui::GetItemRectSize();

        ImGui::SetNextWindowPos({buttonMin.x, buttonMin.y + buttonSize.y});
        ImGui::OpenPopup("##VisibilitySettingPopup");
        mbOpenVisibilitySettingPopup = false;
    }

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.0f, 8.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(ImGui::GetStyle().ItemSpacing.x, 6.0f));

    if (ImGui::BeginPopup("##VisibilitySettingPopup", ImGuiWindowFlags_NoDecoration))
    {
        ImGui::PushStyleVar(ImGuiStyleVar_SeparatorTextBorderSize, 1.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_SeparatorTextPadding, ImVec2(0.0f, 10.0f));

        ImGui::SeparatorText("VISIBLITY");

        static const char* const sVisibilityNames[] = {
            ICON_MDI_VIEW_GRID "  World Gizmo",
            ICON_MDI_HEAD_OUTLINE "  Object Outline",
            ICON_MDI_BORDER_ALL_VARIANT "  Mesh AABB Bounding Volume",
            ICON_MDI_SPHERE "  Mesh Sphere Bounding Volume",
            ICON_MDI_LIGHTBULB_MULTIPLE_OUTLINE "  Light Debug Mesh",
            ICON_MDI_CAMERA_OUTLINE "  Camera Debug Mesh",
        };

        static eRenderingFlags sVisibilityFlags[] = {
            RenderingFlags_RenderWorldGizmo,
            RenderingFlags_RenderObjectOutline,
            RenderingFlags_RenderMeshAabbBoundingVolume,
            RenderingFlags_RenderMeshSphereBoundingVolume,
            RenderingFlags_RenderLightDebugMesh,
            RenderingFlags_RenderCameraDebugMesh,
        };

        static_assert(HO_ARRAY_SIZE(sVisibilityNames) == HO_ARRAY_SIZE(sVisibilityFlags));

        for (int i = 0; i < HO_ARRAY_SIZE(sVisibilityNames); ++i)
        {
            if (ImGui::MenuItem(sVisibilityNames[i], nullptr, mRenderingFlags & sVisibilityFlags[i]))
            {
                mRenderingFlags = static_cast<eRenderingFlags>(mRenderingFlags ^ sVisibilityFlags[i]);
            }
        }

        ImGui::PopStyleVar(2);
        ImGui::EndPopup();
    }

    ImGui::PopStyleVar(2);
}

void EditorCameraPreviewWindow::drawSettingPopup()
{
    if (mbOpenSettingPopup)
    {
        const ImVec2 buttonMin = ImGui::GetItemRectMin();
        const ImVec2 buttonSize = ImGui::GetItemRectSize();

        ImGui::SetNextWindowPos({buttonMin.x, buttonMin.y + buttonSize.y});
        ImGui::OpenPopup("##SettingPopup");
        mbOpenSettingPopup = false;
    }

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.0f, 8.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(ImGui::GetStyle().ItemSpacing.x, 6.0f));

    if (ImGui::BeginPopup("##SettingPopup", ImGuiWindowFlags_NoDecoration))
    {
        ImGui::PushStyleVar(ImGuiStyleVar_SeparatorTextBorderSize, 1.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_SeparatorTextPadding, ImVec2(0.0f, 10.0f));

        ImGui::SeparatorText("VIEWPORT");

        if (ImGui::BeginTable("##ResolutionSettingTable", 2, ImGuiTableFlags_SizingFixedFit))
        {
            ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("Control", ImGuiTableColumnFlags_WidthFixed, 180.0f);
            static constexpr int32_t sResolutionWidths[] = {640, 854, 1280, 1600, 1920, 2560, 3840};

            static constexpr int32_t sResolutionHeights[] = {360, 480, 720, 900, 1080, 1440, 2160};

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Resolution");

            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(-1.0f);

            char resolutionBuf[126] = {};
            snprintf(resolutionBuf, HO_ARRAY_SIZE(resolutionBuf), "%d x %d", mFrameBufferWidth, mFrameBufferHeight);

            if (ImGui::BeginCombo("##ResolutionCombo", resolutionBuf))
            {
                for (int32_t i = 0; i < HO_ARRAY_SIZE(sResolutionWidths); ++i)
                {
                    const bool bSelected =
                        (mFrameBufferWidth == sResolutionWidths[i] && mFrameBufferHeight == sResolutionHeights[i]);

                    snprintf(resolutionBuf,
                             HO_ARRAY_SIZE(resolutionBuf),
                             "%d x %d",
                             sResolutionWidths[i],
                             sResolutionHeights[i]);

                    if (ImGui::Selectable(resolutionBuf, bSelected))
                    {
                        const int32_t prevWidth = mFrameBufferWidth;
                        const int32_t prevHeight = mFrameBufferHeight;

                        mFrameBufferWidth = sResolutionWidths[i];
                        mFrameBufferHeight = sResolutionHeights[i];

                        if (mFrameBufferWidth != prevWidth || mFrameBufferHeight != prevHeight)
                        {
                            mbNeedResizeFrameBuffer = true;
                        }
                    }

                    if (bSelected)
                    {
                        ImGui::SetItemDefaultFocus();
                    }
                }

                ImGui::EndCombo();
            }

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Display Mode");

            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(-1.0f);
            static const char* const sDisplayModeName[] = {"Fill", "Fit"};
            if (ImGui::BeginCombo("##DisplayModeCombo", sDisplayModeName[static_cast<int32_t>(mbDisplayFitMode)]))
            {
                for (int32_t i = 0; i < 2; ++i)
                {
                    const bool bSelected = mbDisplayFitMode == static_cast<bool>(i);

                    if (ImGui::Selectable(sDisplayModeName[i], bSelected))
                    {
                        mbDisplayFitMode = static_cast<bool>(i);
                    }

                    if (bSelected)
                    {
                        ImGui::SetItemDefaultFocus();
                    }
                }

                ImGui::EndCombo();
            }

            ImGui::EndTable();
        }

        ImGui::PopStyleVar(2);
        ImGui::EndPopup();
    }

    ImGui::PopStyleVar(2);
}

const char* EditorCameraPreviewWindow::getRenderingModeName(eRenderingMode mode) const
{
    switch (mode)
    {
        case eRenderingMode::Unlit:
            return ICON_MDI_CIRCLE_OUTLINE "  Unlit";
            break;
        case eRenderingMode::Lit:
            return ICON_MDI_CREATION_OUTLINE "  Lit";
            break;
        case eRenderingMode::Wireframe:
            return ICON_MDI_CHART_TIMELINE_VARIANT "  Wireframe";
            break;
        case eRenderingMode::LitWireframe:
            return ICON_MDI_CHART_TIMELINE_VARIANT_SHIMMER "  Lit WireFrame";
            break;
        case eRenderingMode::DetailLighting:
            return ICON_MDI_TRACK_LIGHT "  Detail Lighting";
            break;
        case eRenderingMode::LightingOnly:
            return ICON_MDI_LIGHTBULB "  Lighting Only";
            break;
        case eRenderingMode::Reflection:
            return ICON_MDI_CRYSTAL_BALL "  Reflection";
            break;
        default:
            HO_ASSERT(false, "Invalid view mode.");
            return "None";
    }
}

} // namespace ho