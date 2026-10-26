#include "EditorSceneViewWindow.h"

// clang-format off
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_internal.h>
#include <ImGuizmo.h>
#include <imoguizmo.hpp>
// clang-format on

#include <IconsMaterialDesignIcons.h>

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

EditorSceneViewWindow::EditorSceneViewWindow(Editor* pOwner)
  : IEngineWindow(StringHandle("EditorSceneViewWindow"))
  , mhFrameBufferName(StringHandle("SceneViewMainFrameBuffer"))
  , mPerspCamera(Vector3::sZero,
                 Vector3::sUnitZ,
                 Vector3::sUnitY,
                 math::PI / 3.0f,
                 static_cast<float>(IPlatformApplication::GetInstance().GetMainWindow()->GetClientWidth()) /
                     static_cast<float>(IPlatformApplication::GetInstance().GetMainWindow()->GetClientHeight()),
                 0.5f,
                 1000.0f)
  , mOrthoCamera(100.0f,
                 static_cast<float>(IPlatformApplication::GetInstance().GetMainWindow()->GetClientWidth()) /
                     static_cast<float>(IPlatformApplication::GetInstance().GetMainWindow()->GetClientHeight()),
                 0.5f,
                 1000.0f)
  , mCurrentGizmoOperation(ImGuizmo::OPERATION::TRANSLATE)
  , mCurrentGizmoMode(ImGuizmo::MODE::WORLD)
  , mpOwner(pOwner)
{
}

EditorSceneViewWindow::~EditorSceneViewWindow() {}

bool EditorSceneViewWindow::OnInit()
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

bool EditorSceneViewWindow::OnPreUpdate(float deltaTime)
{
    return true;
}

bool EditorSceneViewWindow::OnUpdate(float deltaTime)
{
    if (mbOpen)
    {
        handleInput(deltaTime);
    }

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

bool EditorSceneViewWindow::OnPostUpdate(float deltaTime)
{
    return true;
}

bool EditorSceneViewWindow::OnRender()
{
    if (!mbOpen)
    {
        return true;
    }

    const ObjectHandle<RScene> hCurrScene = mpOwner->GetCurrentScene();
    if (hCurrScene.IsValid())
    {
        if (mCameraViewType == eCameraViewType::Perspective)
        {
            mpOwner->GetSceneRenderer().RenderForwardWithExternalCamera(mRenderingMode,
                                                                        mRenderingFlags,
                                                                        hCurrScene,
                                                                        mhFrameBufferName,
                                                                        mPerspCamera.GetPosition(),
                                                                        mPerspCamera.GetViewMatrix(),
                                                                        mPerspCamera.GetProjection());
        }
        else
        {
            mpOwner->GetSceneRenderer().RenderForwardWithExternalCamera(mRenderingMode,
                                                                        mRenderingFlags,
                                                                        hCurrScene,
                                                                        mhFrameBufferName,
                                                                        mOrthoCamera.GetPosition(),
                                                                        mOrthoCamera.GetViewMatrix(),
                                                                        mOrthoCamera.GetProjection());
        }
    }

    return true;
}

void EditorSceneViewWindow::OnShutdown()
{
    RenderingSystem::GetInstance().DestroyFramebuffer(mhFrameBufferName);
}

void EditorSceneViewWindow::DrawUI()
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

    static constexpr ImGuiWindowFlags sFlags =
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;

    if (ImGui::Begin("Scene View", &mbOpen, sFlags))
    {
        SetFocused(ImGui::IsWindowFocused());

        drawConfigWindow();

        const SceneViewArea area = calculateSceneViewArea();

        drawSceneViewImage(area);

        const ObjectHandle<RScene> hCurrScene = mpOwner->GetCurrentScene();
        if (hCurrScene.IsValid())
        {
            drawCameraGizmo(area);
            drawTransformGizmo(area);
        }

        updateMouseInteraction(area);
    }

    ImGui::End();

    ImGui::PopStyleVar(2);
}

void EditorSceneViewWindow::handleInput(float deltaTime)
{
    if (!mbFocused)
    {
        return;
    }

    const ImGuiIO& io = ImGui::GetIO();

    // Camera
    const ObjectHandle<RScene> hCurrScene = mpOwner->GetCurrentScene();
    if (hCurrScene.IsValid())
    {
        if (mCameraViewType == eCameraViewType::Perspective)
        {
            if (mbDragging || mbHovering)
            {
                if (io.MouseWheel != 0.0f)
                {
                    mPerspCamera.MoveForward(10.0f * io.MouseWheel * mCameraMoveSpeed * deltaTime);
                }

                if (!ImOGuizmo::internal::interaction.dragging)
                {
                    if (ImGui::IsMouseDown(ImGuiMouseButton_Left) && ImGui::IsMouseDown(ImGuiMouseButton_Right))
                    {
                        mPerspCamera.MoveRight(io.MouseDelta.x * mCameraMoveSpeed * deltaTime);
                        mPerspCamera.MoveUp(-io.MouseDelta.y * mCameraMoveSpeed * deltaTime);
                    }
                    else if (ImGui::IsMouseDown(ImGuiMouseButton_Left))
                    {
                        mPerspCamera.AddYaw(io.MouseDelta.x * mCameraRotateSpeed * deltaTime);
                        mPerspCamera.MoveForward(-io.MouseDelta.y * mCameraMoveSpeed * deltaTime);
                    }
                    else if (ImGui::IsMouseDown(ImGuiMouseButton_Right))
                    {
                        mPerspCamera.AddPitch(io.MouseDelta.y * mCameraRotateSpeed * deltaTime);
                        mPerspCamera.AddYaw(io.MouseDelta.x * mCameraRotateSpeed * deltaTime);

                        if (ImGui::IsKeyDown(ImGuiKey_W))
                        {
                            mPerspCamera.MoveForward(mCameraMoveSpeed * deltaTime);
                        }
                        if (ImGui::IsKeyDown(ImGuiKey_A))
                        {
                            mPerspCamera.MoveRight(-mCameraMoveSpeed * deltaTime);
                        }
                        if (ImGui::IsKeyDown(ImGuiKey_S))
                        {
                            mPerspCamera.MoveForward(-mCameraMoveSpeed * deltaTime);
                        }
                        if (ImGui::IsKeyDown(ImGuiKey_D))
                        {
                            mPerspCamera.MoveRight(mCameraMoveSpeed * deltaTime);
                        }
                    }
                }
            }
        }
        else
        {
            if (mbDragging || mbHovering)
            {
                if (io.MouseWheel != 0.0f)
                {
                    mOrthoCamera.Zoom(10.0f * -io.MouseWheel * mCameraMoveSpeed * deltaTime);
                }
                if (!ImOGuizmo::internal::interaction.dragging)
                {
                    if (ImGui::IsMouseDown(ImGuiMouseButton_Left) && ImGui::IsMouseDown(ImGuiMouseButton_Right))
                    {
                        mOrthoCamera.Zoom(io.MouseDelta.y * mCameraMoveSpeed * deltaTime);
                    }
                    else if (ImGui::IsMouseDown(ImGuiMouseButton_Left))
                    {
                        mOrthoCamera.MoveForward(-io.MouseDelta.y * mCameraMoveSpeed * deltaTime);
                    }
                    else if (ImGui::IsMouseDown(ImGuiMouseButton_Right))
                    {
                        mOrthoCamera.MoveUp(io.MouseDelta.y * mCameraMoveSpeed * deltaTime);
                        mOrthoCamera.MoveRight(-io.MouseDelta.x * mCameraMoveSpeed * deltaTime);

                        if (ImGui::IsKeyDown(ImGuiKey_W))
                        {
                            mOrthoCamera.MoveUp(mCameraMoveSpeed * deltaTime);
                        }
                        if (ImGui::IsKeyDown(ImGuiKey_A))
                        {
                            mOrthoCamera.MoveRight(-mCameraMoveSpeed * deltaTime);
                        }
                        if (ImGui::IsKeyDown(ImGuiKey_S))
                        {
                            mOrthoCamera.MoveUp(-mCameraMoveSpeed * deltaTime);
                        }
                        if (ImGui::IsKeyDown(ImGuiKey_D))
                        {
                            mOrthoCamera.MoveRight(mCameraMoveSpeed * deltaTime);
                        }
                    }
                }
            }
        }
    }

    // Gizmo
    if (ImGui::IsKeyPressed(ImGuiKey_Q))
    {
        mCurrentGizmoOperation = 0;
    }
    if (ImGui::IsKeyPressed(ImGuiKey_W))
    {
        mCurrentGizmoOperation = ImGuizmo::OPERATION::TRANSLATE;
    }
    if (ImGui::IsKeyPressed(ImGuiKey_E))
    {
        mCurrentGizmoOperation = ImGuizmo::OPERATION::ROTATE;
    }
    if (ImGui::IsKeyPressed(ImGuiKey_R))
    {
        mCurrentGizmoOperation = ImGuizmo::OPERATION::SCALE;
        mCurrentGizmoMode = ImGuizmo::MODE::LOCAL;
    }
}

void EditorSceneViewWindow::drawConfigWindow()
{
    static constexpr float sConfigWindowHeight = 30.0f;
    static constexpr float sPadding = sConfigWindowHeight * 0.1f;
    static constexpr float sButtonHeight = sConfigWindowHeight - sPadding * 2.0f;

    static constexpr ImGuiWindowFlags sFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
                                               ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoDocking |
                                               ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoSavedSettings;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVarY(ImGuiStyleVar_ItemSpacing, 0.0f);

    if (ImGui::BeginChild("##SceneViewConfigsWindow", ImVec2(0.0f, sConfigWindowHeight), 0, sFlags))
    {
        ImGui::SetCursorPos(ImVec2(sPadding, sPadding));

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));

        drawTransformToolButtons(sButtonHeight);
        ImGui::SameLine(0.0f, 30.0f);
        drawTranslateSnapControls(sButtonHeight);
        drawRotateSnapControls(sButtonHeight);
        drawScaleSnapControls(sButtonHeight);
        drawSceneViewOptions(sButtonHeight, sPadding);

        ImGui::PopStyleColor();
    }

    ImGui::EndChild();
    ImGui::PopStyleVar(2);
}

void EditorSceneViewWindow::drawTransformToolButtons(float buttonHeight)
{
    if (ImGui::Button(ICON_MDI_DOTS_VERTICAL ICON_MDI_CHEVRON_DOWN, ImVec2(0.0f, buttonHeight)))
    {
        mbOpenTransformToolsPopup = true;
    }

    drawTransformToolsPopup();

    ImGui::SameLine(0.0f, 20.0f);

    const auto drawOperationButton = [this, buttonHeight](const char* label, int32_t operation)
    {
        const bool bIsActive = mCurrentGizmoOperation == operation;

        if (bIsActive)
        {
            ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyle().Colors[ImGuiCol_ButtonActive]);
        }

        if (ImGui::Button(label, ImVec2(0.0f, buttonHeight)))
        {
            mCurrentGizmoOperation = operation;

            if (operation == ImGuizmo::OPERATION::SCALE)
            {
                mCurrentGizmoMode = ImGuizmo::MODE::LOCAL;
            }
        }

        if (bIsActive)
        {
            ImGui::PopStyleColor();
        }
    };

    drawOperationButton(" " ICON_MDI_CURSOR_DEFAULT " ", 0);
    ImGui::SameLine();

    drawOperationButton(" " ICON_MDI_ARROW_ALL " ", ImGuizmo::OPERATION::TRANSLATE);
    ImGui::SameLine();

    drawOperationButton(" " ICON_MDI_RELOAD " ", ImGuizmo::OPERATION::ROTATE);
    ImGui::SameLine();

    drawOperationButton(" " ICON_MDI_ARROW_TOP_RIGHT_BOTTOM_LEFT " ", ImGuizmo::OPERATION::SCALE);

    ImGui::SameLine();

    ImGui::PushStyleColor(ImGuiCol_Separator, ImGui::GetStyleColorVec4(ImGuiCol_Text));

    ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);

    ImGui::PopStyleColor();
    ImGui::SameLine();

    const char* modeIcon =
        mCurrentGizmoMode == ImGuizmo::MODE::WORLD ? " " ICON_MDI_WEB " " : " " ICON_MDI_AXIS_ARROW " ";

    if (ImGui::Button(modeIcon, ImVec2(0.0f, buttonHeight)))
    {
        if (mCurrentGizmoMode == ImGuizmo::MODE::WORLD)
        {
            mCurrentGizmoMode = ImGuizmo::MODE::LOCAL;
        }
        else if (mCurrentGizmoOperation != ImGuizmo::OPERATION::SCALE)
        {
            mCurrentGizmoMode = ImGuizmo::MODE::WORLD;
        }
    }
}

void EditorSceneViewWindow::drawTranslateSnapControls(float buttonHeight)
{
    char buf[32] = {};
    const ImVec2 buttonStartPos = ImGui::GetCursorPos();

    static const float sIconButtonWidth = ImGui::CalcTextSize(ICON_MDI_GRID).x +
                                          ImGui::GetStyle().FramePadding.x * 2.0f + ImGui::GetStyle().ItemSpacing.x;

    snprintf(buf, sizeof(buf), "%g" ICON_MDI_CHEVRON_DOWN, mTranslateSnap.X);

    const float snapWidth = sIconButtonWidth + ImGui::CalcTextSize(buf).x + ImGui::GetStyle().FramePadding.x * 2.0f +
                            ImGui::GetStyle().ItemSpacing.x;

    ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.9f, 0.5f));
    ImGui::SetNextItemAllowOverlap();

    if (ImGui::Button(std::string(buf).append("##TranslateSnapButton").c_str(), ImVec2(snapWidth, buttonHeight)))
    {
        mbOpenTranslateSnapPopup = true;
    }

    ImGui::PopStyleVar();

    drawTranslateSnapPopup();

    ImGui::SameLine();

    const ImVec2 nextButtonPos = ImGui::GetCursorPos();

    ImGui::SetCursorPos(buttonStartPos);

    const bool bIsActive = mbUseTranslateSnap;

    if (bIsActive)
    {
        ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyle().Colors[ImGuiCol_ButtonActive]);
    }

    if (ImGui::Button(ICON_MDI_GRID, ImVec2(0.0f, buttonHeight)))
    {
        mbUseTranslateSnap = !mbUseTranslateSnap;
    }

    if (bIsActive)
    {
        ImGui::PopStyleColor();
    }

    ImGui::SetCursorPos(nextButtonPos);
}

void EditorSceneViewWindow::drawRotateSnapControls(float buttonHeight)
{
    char buf[32] = {};
    const ImVec2 buttonStartPos = ImGui::GetCursorPos();

    static const float sIconButtonWidth = ImGui::CalcTextSize(ICON_MDI_ROTATE_RIGHT).x +
                                          ImGui::GetStyle().FramePadding.x * 2.0f + ImGui::GetStyle().ItemSpacing.x;

    snprintf(buf, sizeof(buf), "%g" ICON_MDI_CHEVRON_DOWN, mRotateSnap);

    const float snapWidth = sIconButtonWidth + ImGui::CalcTextSize(buf).x + ImGui::GetStyle().FramePadding.x * 2.0f +
                            ImGui::GetStyle().ItemSpacing.x;

    ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.9f, 0.5f));
    ImGui::SetNextItemAllowOverlap();

    if (ImGui::Button(std::string(buf).append("##RotateSnapButton").c_str(), ImVec2(snapWidth, buttonHeight)))
    {
        mbOpenRotateSnapPopup = true;
    }

    ImGui::PopStyleVar();

    drawRotateSnapPopup();

    ImGui::SameLine();

    const ImVec2 nextButtonPos = ImGui::GetCursorPos();

    ImGui::SetCursorPos(buttonStartPos);

    const bool bIsActive = mbUseRotateSnap;

    if (bIsActive)
    {
        ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyle().Colors[ImGuiCol_ButtonActive]);
    }

    if (ImGui::Button(ICON_MDI_ROTATE_RIGHT, ImVec2(0.0f, buttonHeight)))
    {
        mbUseRotateSnap = !mbUseRotateSnap;
    }

    if (bIsActive)
    {
        ImGui::PopStyleColor();
    }

    ImGui::SetCursorPos(nextButtonPos);
}

void EditorSceneViewWindow::drawScaleSnapControls(float buttonHeight)
{
    char buf[32] = {};
    const ImVec2 buttonStartPos = ImGui::GetCursorPos();

    static const float sIconButtonWidth = ImGui::CalcTextSize(ICON_MDI_ARROW_EXPAND).x +
                                          ImGui::GetStyle().FramePadding.x * 2.0f + ImGui::GetStyle().ItemSpacing.x;

    snprintf(buf, sizeof(buf), "%g" ICON_MDI_CHEVRON_DOWN, mScaleSnap.X);

    const float snapWidth = sIconButtonWidth + ImGui::CalcTextSize(buf).x + ImGui::GetStyle().FramePadding.x * 2.0f +
                            ImGui::GetStyle().ItemSpacing.x;

    ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.9f, 0.5f));
    ImGui::SetNextItemAllowOverlap();

    if (ImGui::Button(std::string(buf).append("##ScaleSnapButton").c_str(), ImVec2(snapWidth, buttonHeight)))
    {
        mbOpenScaleSnapPopup = true;
    }

    ImGui::PopStyleVar();

    drawScaleSnapPopup();

    ImGui::SameLine();

    const ImVec2 nextButtonPos = ImGui::GetCursorPos();

    ImGui::SetCursorPos(buttonStartPos);

    const bool bIsActive = mbUseScaleSnap;

    if (bIsActive)
    {
        ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyle().Colors[ImGuiCol_ButtonActive]);
    }

    if (ImGui::Button(ICON_MDI_ARROW_EXPAND, ImVec2(0.0f, buttonHeight)))
    {
        mbUseScaleSnap = !mbUseScaleSnap;
    }

    if (bIsActive)
    {
        ImGui::PopStyleColor();
    }

    ImGui::SetCursorPos(nextButtonPos);
}

void EditorSceneViewWindow::drawSceneViewOptions(float buttonHeight, float padding)
{
    const char* cameraViewText = getCameraViewTypeName(mCameraViewType);
    const char* cameraSpeedText = " " ICON_MDI_CAMERA_SWITCH " ";
    const char* renderingModeText = getRenderingModeName(mRenderingMode);
    const char* visibilityText = " " ICON_MDI_EYE_OUTLINE " ";
    const char* settingsText = " " ICON_MDI_COG " ";

    const float framePaddingX = ImGui::GetStyle().FramePadding.x;
    const float itemSpacingX = ImGui::GetStyle().ItemSpacing.x;

    const float rightGroupWidth = (ImGui::CalcTextSize(cameraViewText).x + framePaddingX * 2.0f) +
                                  (ImGui::CalcTextSize(cameraSpeedText).x + framePaddingX * 2.0f) +
                                  (ImGui::CalcTextSize(renderingModeText).x + framePaddingX * 2.0f) +
                                  (ImGui::CalcTextSize(visibilityText).x + framePaddingX * 2.0f) +
                                  (ImGui::CalcTextSize(settingsText).x + framePaddingX * 2.0f) + itemSpacingX * 4.0f;

    const float availableWidth = ImGui::GetContentRegionAvail().x;

    if (availableWidth > rightGroupWidth + padding)
    {
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + availableWidth - rightGroupWidth - padding);
    }

    if (ImGui::Button(cameraViewText, ImVec2(0.0f, buttonHeight)))
    {
        mbOpenCameraOptionPopup = true;
    }
    drawCameraOptionPopup();

    ImGui::SameLine();

    if (ImGui::Button(cameraSpeedText, ImVec2(0.0f, buttonHeight)))
    {
        mbOpenCameraSpeedPopup = true;
    }
    drawCameraSpeedPopup();

    ImGui::SameLine();

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

SceneViewArea EditorSceneViewWindow::calculateSceneViewArea() const
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

void EditorSceneViewWindow::drawSceneViewImage(const SceneViewArea& area)
{
    const ImTextureID textureID = static_cast<ImTextureID>(
        reinterpret_cast<intptr_t>(RenderingSystem::GetInstance().GetRenderResultNativeHandle(mhFrameBufferName)));

    ImGui::SetCursorPos(area.Position);
    ImGui::SetNextItemAllowOverlap();

    ImGui::Image(textureID, area.Size, ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f));
}

void EditorSceneViewWindow::drawCameraGizmo(const SceneViewArea& area)
{
    Matrix4x4 viewMat = mPerspCamera.GetViewMatrix().Transposed();
    const Matrix4x4 projMat = mPerspCamera.GetProjMatrix().Transposed();

    if (mCameraViewType != eCameraViewType::Perspective)
    {
        return;
    }

    const float gizmoSize = area.Size.x > area.Size.y ? area.Size.x / 16.0f : area.Size.y / 16.0f;

    ImOGuizmo::SetRect(area.ScreenPosition.x + gizmoSize * 0.2f,
                       area.ScreenPosition.y + area.Size.y - gizmoSize - gizmoSize * 0.2f,
                       gizmoSize);

    const float pivotDistance = mPerspCamera.GetPosition().Magnitude();

    if (ImOGuizmo::DrawGizmo(&viewMat.Data[0][0], &projMat.Data[0][0], pivotDistance, ImOGuizmo::CoordinateSystem::ZYX))
    {
        const Transform3D newCameraWorld(viewMat.Transposed().Inverse());

        Vector3 newForward = newCameraWorld.GetForward().Normalized();

        if (ImOGuizmo::internal::interaction.dragged)
        {
            newForward = -newForward;
        }

        mPerspCamera.SetPosition(-(newForward * pivotDistance));
        mPerspCamera.SetPitch(math::Asin(-newForward.Y));
        mPerspCamera.SetYaw(math::Atan2(newForward.X, newForward.Z));
    }
}

void EditorSceneViewWindow::drawTransformGizmo(const SceneViewArea& area)
{
    const ObjectHandle<RGameObject> hSelectedObj = mpOwner->GetSelectedGameObject();

    if (!hSelectedObj.IsValid() || hSelectedObj->GetComponent<RTransformComponent>()->IsDirty() ||
        !mbShowTransformGizmo || mCameraViewType != eCameraViewType::Perspective || mCurrentGizmoOperation == 0)
    {
        return;
    }

    bool bUseSnap = false;
    const float* pSnapData = nullptr;

    switch (mCurrentGizmoOperation)
    {
        case ImGuizmo::OPERATION::TRANSLATE:
            bUseSnap = mbUseTranslateSnap;
            pSnapData = mTranslateSnap.Data;
            break;

        case ImGuizmo::OPERATION::ROTATE:
            bUseSnap = mbUseRotateSnap;
            pSnapData = &mRotateSnap;
            break;

        case ImGuizmo::OPERATION::SCALE:
            bUseSnap = mbUseScaleSnap;
            pSnapData = mScaleSnap.Data;
            break;

        default:
            HO_ASSERT(false, "Invalid operation.");
            mCurrentGizmoOperation = 0;
            return;
    }

    ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());
    ImGuizmo::SetRect(area.ScreenPosition.x, area.ScreenPosition.y, area.Size.x, area.Size.y);

    Matrix4x4 worldMat = hSelectedObj->GetComponent<RTransformComponent>()->GetWorldTransform().Matrix.Transposed();

    const Matrix4x4 viewMat = mPerspCamera.GetViewMatrix().Transposed();
    const Matrix4x4 projMat = mPerspCamera.GetProjMatrix().Transposed();

    ImGuizmo::Manipulate(&viewMat.Data[0][0],
                         &projMat.Data[0][0],
                         static_cast<ImGuizmo::OPERATION>(mCurrentGizmoOperation),
                         static_cast<ImGuizmo::MODE>(mCurrentGizmoMode),
                         &worldMat.Data[0][0],
                         nullptr,
                         bUseSnap ? pSnapData : nullptr);

    hSelectedObj->GetComponent<RTransformComponent>()->SetWorldTransform(worldMat.Transposed());
}

void EditorSceneViewWindow::updateMouseInteraction(const SceneViewArea& area)
{
    if (mbDragging)
    {
        mbDragging = ImGui::IsMouseDown(ImGuiMouseButton_Left) || ImGui::IsMouseDown(ImGuiMouseButton_Right);
    }

    if (!mbDragging && (ImGuizmo::IsOver() || ImGuizmo::IsUsing()))
    {
        mbHovering = false;
        return;
    }

    ImGui::SetCursorPos(area.Position);
    ImGui::InvisibleButton("##SceneDraggingRect", area.Size);

    mbHovering = ImGui::IsItemHovered();
    mbDragging = ImGui::IsItemActive();
}

void EditorSceneViewWindow::drawTransformToolsPopup()
{
    if (mbOpenTransformToolsPopup)
    {
        const ImVec2 buttonMin = ImGui::GetItemRectMin();
        const ImVec2 buttonSize = ImGui::GetItemRectSize();

        ImGui::SetNextWindowPos({buttonMin.x, buttonMin.y + buttonSize.y});
        ImGui::OpenPopup("##TransformToolsPopup");
        mbOpenTransformToolsPopup = false;
    }

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4.0f, 8.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(ImGui::GetStyle().ItemSpacing.x, 6.0f));
    if (ImGui::BeginPopup("##TransformToolsPopup", ImGuiWindowFlags_NoDecoration))
    {
        ImGui::PushStyleVar(ImGuiStyleVar_SeparatorTextBorderSize, 1.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_SeparatorTextPadding, ImVec2(0.0f, 10.0f));
        ImGui::SeparatorText("TRANSFORM TOOLS");
        ImGui::PopStyleVar(2);

        if (ImGui::MenuItem(ICON_MDI_CURSOR_DEFAULT "  Select Mode", "Q", mCurrentGizmoOperation == 0))
        {
            mCurrentGizmoOperation = 0;
        }

        if (ImGui::MenuItem(
                ICON_MDI_ARROW_ALL "  Translate Mode", "W", mCurrentGizmoOperation == ImGuizmo::OPERATION::TRANSLATE))
        {
            mCurrentGizmoOperation = ImGuizmo::OPERATION::TRANSLATE;
        }

        if (ImGui::MenuItem(
                ICON_MDI_RELOAD "  Rotate Mode", "E", mCurrentGizmoOperation == ImGuizmo::OPERATION::ROTATE))
        {
            mCurrentGizmoOperation = ImGuizmo::OPERATION::ROTATE;
        }

        if (ImGui::MenuItem(ICON_MDI_ARROW_TOP_RIGHT_BOTTOM_LEFT "  Scale Mode",
                            "R",
                            mCurrentGizmoOperation == ImGuizmo::OPERATION::SCALE))
        {
            mCurrentGizmoOperation = ImGuizmo::OPERATION::SCALE;
            mCurrentGizmoMode = ImGuizmo::MODE::LOCAL;
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (ImGui::BeginMenu(ICON_MDI_AXIS_ARROW "  Coordinate System"))
        {
            if (ImGui::MenuItem(ICON_MDI_WEB "  World Space", nullptr, mCurrentGizmoMode == ImGuizmo::MODE::WORLD))
            {
                if (mCurrentGizmoOperation != ImGuizmo::OPERATION::SCALE)
                {
                    mCurrentGizmoMode = ImGuizmo::MODE::WORLD;
                }
            }

            if (ImGui::MenuItem(
                    ICON_MDI_AXIS_ARROW "  Local Space", nullptr, mCurrentGizmoMode == ImGuizmo::MODE::LOCAL))
            {
                mCurrentGizmoMode = ImGuizmo::MODE::LOCAL;
            }
            ImGui::EndMenu();
        }

        ImGui::PushStyleVar(ImGuiStyleVar_SeparatorTextBorderSize, 1.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_SeparatorTextPadding, ImVec2(0.0f, 10.0f));
        ImGui::SeparatorText("GIZMO");
        ImGui::PopStyleVar(2);

        if (ImGui::MenuItem(ICON_MDI_EYE "  Show Transform Gizmo", nullptr, mbShowTransformGizmo))
        {
            mbShowTransformGizmo = !mbShowTransformGizmo;
        }

        ImGui::Text(ICON_MDI_TUNE_VARIANT "  Gizmo Scale");
        ImGui::SameLine();
        if (ImGui::SliderFloat("##GizmoScaleSlider", &mGizmoScale, 0.1f, 3.0f))
        {
            static const ImGuizmo::Style sDefaultGizmoStyle = ImGuizmo::GetStyle();
            ImGuizmo::GetStyle().TranslationLineThickness = sDefaultGizmoStyle.TranslationLineThickness * mGizmoScale;
            ImGuizmo::GetStyle().TranslationLineArrowSize = sDefaultGizmoStyle.TranslationLineArrowSize * mGizmoScale;
            ImGuizmo::GetStyle().RotationLineThickness = sDefaultGizmoStyle.RotationLineThickness * mGizmoScale;
            ImGuizmo::GetStyle().RotationOuterLineThickness =
                sDefaultGizmoStyle.RotationOuterLineThickness * mGizmoScale;
            ImGuizmo::GetStyle().ScaleLineThickness = sDefaultGizmoStyle.ScaleLineThickness * mGizmoScale;
            ImGuizmo::GetStyle().ScaleLineCircleSize = sDefaultGizmoStyle.ScaleLineCircleSize * mGizmoScale;
            ImGuizmo::GetStyle().HatchedAxisLineThickness = sDefaultGizmoStyle.HatchedAxisLineThickness * mGizmoScale;
            ImGuizmo::GetStyle().CenterCircleSize = sDefaultGizmoStyle.CenterCircleSize * mGizmoScale;
        }

        if (ImGui::MenuItem(ICON_MDI_RESIZE "  Preserve Non-Uniform Scale", nullptr, mbPreserveNonUniformScale))
        {
            mbPreserveNonUniformScale = !mbPreserveNonUniformScale;
        }

        ImGui::EndPopup();
    }
    ImGui::PopStyleVar(2);
}

void EditorSceneViewWindow::drawTranslateSnapPopup()
{
    if (mbOpenTranslateSnapPopup)
    {
        const ImVec2 buttonMin = ImGui::GetItemRectMin();
        const ImVec2 buttonSize = ImGui::GetItemRectSize();

        ImGui::SetNextWindowPos({buttonMin.x, buttonMin.y + buttonSize.y});
        ImGui::OpenPopup("##TranslateSnapPopup");
        mbOpenTranslateSnapPopup = false;
    }

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4.0f, 8.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(ImGui::GetStyle().ItemSpacing.x, 6.0f));
    if (ImGui::BeginPopup("##TranslateSnapPopup", ImGuiWindowFlags_NoDecoration))
    {
        ImGui::PushStyleVar(ImGuiStyleVar_SeparatorTextBorderSize, 1.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_SeparatorTextPadding, ImVec2(0.0f, 10.0f));
        ImGui::SeparatorText("SNAP SIZES");
        ImGui::PopStyleVar(2);

        char buf[32] = {};
        float snapSize = 1.0f;
        snprintf(buf, sizeof(buf), "%g", snapSize);
        if (ImGui::MenuItem(buf, nullptr, mTranslateSnap.X == snapSize))
        {
            mTranslateSnap = Vector3(snapSize, snapSize, snapSize);
        }
        while (snapSize < 10000)
        {
            snapSize *= 5.0f;
            snprintf(buf, sizeof(buf), "%g", snapSize);
            if (ImGui::MenuItem(buf, nullptr, mTranslateSnap.X == snapSize))
            {
                mTranslateSnap = Vector3(snapSize, snapSize, snapSize);
            }
            snapSize *= 2.0f;
            snprintf(buf, sizeof(buf), "%g", snapSize);
            if (ImGui::MenuItem(buf, nullptr, mTranslateSnap.X == snapSize))
            {
                mTranslateSnap = Vector3(snapSize, snapSize, snapSize);
            }
        }

        ImGui::EndPopup();
    }
    ImGui::PopStyleVar(2);
}

void EditorSceneViewWindow::drawRotateSnapPopup()
{
    if (mbOpenRotateSnapPopup)
    {
        const ImVec2 buttonMin = ImGui::GetItemRectMin();
        const ImVec2 buttonSize = ImGui::GetItemRectSize();

        ImGui::SetNextWindowPos({buttonMin.x, buttonMin.y + buttonSize.y});
        ImGui::OpenPopup("##RotateSnapPopup");
        mbOpenRotateSnapPopup = false;
    }

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.0f, 8.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(ImGui::GetStyle().ItemSpacing.x, 6.0f));

    if (ImGui::BeginPopup("##RotateSnapPopup", ImGuiWindowFlags_NoDecoration))
    {
        ImGui::PushStyleVar(ImGuiStyleVar_SeparatorTextBorderSize, 1.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_SeparatorTextPadding, ImVec2(0.0f, 10.0f));
        char buf[32] = {};

        ImGui::SeparatorText("ROTATION INCREMENT");
        static const float sIncrements[] = {1.0f, 5.0f, 10.0f, 15.0f, 30.0f, 45.0f, 60.0f, 90.0f, 120.0f};
        for (const float snapVal : sIncrements)
        {
            snprintf(buf, sizeof(buf), "%g°", snapVal);

            if (ImGui::MenuItem(buf, nullptr, mRotateSnap == snapVal))
            {
                mRotateSnap = snapVal;
            }
        }

        ImGui::SeparatorText("DIVISION OF 360°");

        static const float sDivisions[] = {22.5f, 11.25f, 5.625f, 2.8125f};
        for (const float snapVal : sDivisions)
        {
            snprintf(buf, sizeof(buf), "%g°", snapVal);

            if (ImGui::MenuItem(buf, nullptr, math::IsEqualApprox(mRotateSnap, snapVal)))
            {
                mRotateSnap = snapVal;
            }
        }

        ImGui::PopStyleVar(2);
        ImGui::EndPopup();
    }

    ImGui::PopStyleVar(2);
}

void EditorSceneViewWindow::drawScaleSnapPopup()
{
    if (mbOpenScaleSnapPopup)
    {
        const ImVec2 buttonMin = ImGui::GetItemRectMin();
        const ImVec2 buttonSize = ImGui::GetItemRectSize();

        ImGui::SetNextWindowPos({buttonMin.x, buttonMin.y + buttonSize.y});
        ImGui::OpenPopup("##ScaleSnapPopup");
        mbOpenScaleSnapPopup = false;
    }

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.0f, 8.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(ImGui::GetStyle().ItemSpacing.x, 6.0f));

    if (ImGui::BeginPopup("##ScaleSnapPopup", ImGuiWindowFlags_NoDecoration))
    {
        ImGui::PushStyleVar(ImGuiStyleVar_SeparatorTextBorderSize, 1.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_SeparatorTextPadding, ImVec2(0.0f, 10.0f));
        char buf[32] = {};

        ImGui::SeparatorText("SNAP SIZES");

        static const float sSnapSizes[] = {10.0f, 1.0f, 0.5f, 0.25f, 0.125f, 0.1f, 0.0625f, 0.03125f};
        for (const float snapSize : sSnapSizes)
        {
            snprintf(buf, sizeof(buf), "%g", snapSize);

            if (ImGui::MenuItem(buf, nullptr, mScaleSnap.X == snapSize))
            {
                mScaleSnap = Vector3(snapSize, snapSize, snapSize);
            }
        }

        ImGui::PopStyleVar(2);
        ImGui::EndPopup();
    }

    ImGui::PopStyleVar(2);
}

void EditorSceneViewWindow::drawCameraOptionPopup()
{
    if (mbOpenCameraOptionPopup)
    {
        const ImVec2 buttonMin = ImGui::GetItemRectMin();
        const ImVec2 buttonSize = ImGui::GetItemRectSize();

        ImGui::SetNextWindowPos({buttonMin.x, buttonMin.y + buttonSize.y});
        ImGui::OpenPopup("##CameraOptionPopup");
        mbOpenCameraOptionPopup = false;
    }

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.0f, 8.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(ImGui::GetStyle().ItemSpacing.x, 6.0f));

    if (ImGui::BeginPopup("##CameraOptionPopup", ImGuiWindowFlags_NoDecoration))
    {
        ImGui::PushStyleVar(ImGuiStyleVar_SeparatorTextBorderSize, 1.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_SeparatorTextPadding, ImVec2(0.0f, 10.0f));

        ImGui::SeparatorText("PERSPECTIVE");

        if (ImGui::MenuItem(getCameraViewTypeName(eCameraViewType::Perspective),
                            nullptr,
                            mCameraViewType == eCameraViewType::Perspective))
        {
            mCameraViewType = eCameraViewType::Perspective;
        }

        ImGui::SeparatorText("ORTHOGRAPHIC");

        static const eCameraViewType sOrthoViewTypes[] = {eCameraViewType::OrthographicTop,
                                                          eCameraViewType::OrthographicBottom,
                                                          eCameraViewType::OrthographicLeft,
                                                          eCameraViewType::OrthographicRight,
                                                          eCameraViewType::OrthographicFront,
                                                          eCameraViewType::OrthographicBack};

        for (auto orthoViewType : sOrthoViewTypes)
        {
            if (ImGui::MenuItem(getCameraViewTypeName(orthoViewType), nullptr, mCameraViewType == orthoViewType))
            {
                mCameraViewType = orthoViewType;
                switch (mCameraViewType)
                {
                    case eCameraViewType::OrthographicTop:
                        mOrthoCamera.SetRotation(math::PI2, 0.0f, 0.0f);
                        mOrthoCamera.SetPosition({0.0f, 100.0f, 0.0f});
                        break;
                    case eCameraViewType::OrthographicBottom:
                        mOrthoCamera.SetRotation(-math::PI2, math::PI, 0.0f);
                        mOrthoCamera.SetPosition({0.0f, -100.0f, 0.0f});
                        break;
                    case eCameraViewType::OrthographicLeft:
                        mOrthoCamera.SetRotation(0.0f, math::PI2, 0.0f);
                        mOrthoCamera.SetPosition({-100.0f, 0.0f, 0.0f});
                        break;
                    case eCameraViewType::OrthographicRight:
                        mOrthoCamera.SetRotation(0.0f, -math::PI2, 0.0f);
                        mOrthoCamera.SetPosition({100.0f, 0.0f, 0.0f});
                        break;
                    case eCameraViewType::OrthographicFront:
                        mOrthoCamera.SetRotation(0.0f, math::PI, 0.0f);
                        mOrthoCamera.SetPosition({0.0f, 0.0f, 100.0f});
                        break;
                    case eCameraViewType::OrthographicBack:
                        mOrthoCamera.SetRotation(0.0f, 0.0f, 0.0f);
                        mOrthoCamera.SetPosition({0.0f, 0.0f, -100.0f});
                        break;
                    default:
                        HO_ASSERT(false, "Invalid view type.");
                }
            }
        }

        ImGui::SeparatorText("VIEW");

        if (ImGui::BeginTable("##CameraPropertiesTable", 2, ImGuiTableFlags_SizingFixedFit))
        {
            ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("Control", ImGuiTableColumnFlags_WidthFixed, 180.0f);

            if (mCameraViewType == eCameraViewType::Perspective)
            {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text(ICON_MDI_ANGLE_ACUTE "  Field of View");

                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(-1.0f);
                float fovDeg = math::RadToDeg(mPerspCamera.GetFOV());
                if (ImGui::SliderFloat("##FOVSlider", &fovDeg, 5.0f, 160.0f))
                {
                    mPerspCamera.SetFOV(math::DegToRad(fovDeg));
                }

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text(ICON_MDI_WIFI_STRENGTH_1 "  Near Plane Distance");

                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(-1.0f);
                float nearDist = mPerspCamera.GetNearDistance();
                if (ImGui::SliderFloat("##NearPlaneDistanceSlider", &nearDist, 0.001f, 100.0f))
                {
                    mPerspCamera.SetNearDistance(nearDist);
                }

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text(ICON_MDI_WIFI_STRENGTH_4 "  Far Plane Distance");

                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(-1.0f);
                float farDist = mPerspCamera.GetFarDistance();
                if (ImGui::SliderFloat("##FarPlaneDistanceSlider", &farDist, 0.01f, 10000.0f))
                {
                    mPerspCamera.SetFarDistance(farDist);
                }
            }
            else
            {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text(ICON_MDI_ARROW_EXPAND_VERTICAL "  View Height");

                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(-1.0f);
                float viewHeight = mOrthoCamera.GetHeight();
                if (ImGui::SliderFloat("##OrthoSizeSlider", &viewHeight, 0.1f, 1000.0f))
                {
                    mOrthoCamera.SetHeight(viewHeight);
                }

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text(ICON_MDI_WIFI_STRENGTH_1 "  Near Plane Distance");

                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(-1.0f);
                float nearDist = mOrthoCamera.GetNearDistance();
                if (ImGui::SliderFloat("##NearPlaneDistanceSlider", &nearDist, 0.001f, 100.0f))
                {
                    mOrthoCamera.SetNearDistance(nearDist);
                }

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text(ICON_MDI_WIFI_STRENGTH_4 "  Far Plane Distance");

                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(-1.0f);
                float farDist = mOrthoCamera.GetFarDistance();
                if (ImGui::SliderFloat("##FarPlaneDistanceSlider", &farDist, 0.01f, 10000.0f))
                {
                    mOrthoCamera.SetFarDistance(farDist);
                }
            }

            ImGui::EndTable();
        }

        ImGui::PopStyleVar(2);
        ImGui::EndPopup();
    }

    ImGui::PopStyleVar(2);
}

void EditorSceneViewWindow::drawCameraSpeedPopup()
{
    if (mbOpenCameraSpeedPopup)
    {
        const ImVec2 buttonMin = ImGui::GetItemRectMin();
        const ImVec2 buttonSize = ImGui::GetItemRectSize();

        ImGui::SetNextWindowPos({buttonMin.x, buttonMin.y + buttonSize.y});
        ImGui::OpenPopup("##CameraSpeedPopup");
        mbOpenCameraSpeedPopup = false;
    }

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.0f, 8.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(ImGui::GetStyle().ItemSpacing.x, 6.0f));

    if (ImGui::BeginPopup("##CameraSpeedPopup", ImGuiWindowFlags_NoDecoration))
    {
        ImGui::PushStyleVar(ImGuiStyleVar_SeparatorTextBorderSize, 1.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_SeparatorTextPadding, ImVec2(0.0f, 10.0f));

        ImGui::SeparatorText("CAMERA SPEED");

        ImGui::Text("Speed");
        ImGui::SameLine();
        ImGui::SliderFloat("##CameraSpeedSlider", &mCameraMoveSpeed, 0.0f, 64.0f);

        ImGui::PopStyleVar(2);
        ImGui::EndPopup();
    }

    ImGui::PopStyleVar(2);
}

void EditorSceneViewWindow::drawRenderingModeSettingPopup()
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

void EditorSceneViewWindow::drawVisibilitySettingPopup()
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

void EditorSceneViewWindow::drawSettingPopup()
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

const char* EditorSceneViewWindow::getCameraViewTypeName(eCameraViewType type) const
{
    switch (type)
    {
        case eCameraViewType::Perspective:
            return ICON_MDI_CUBE "  Perspective";
            break;
        case eCameraViewType::OrthographicTop:
            return ICON_MDI_BORDER_TOP_VARIANT "  Top";
            break;
        case eCameraViewType::OrthographicBottom:
            return ICON_MDI_BORDER_BOTTOM_VARIANT "  Bottom";
            break;
        case eCameraViewType::OrthographicLeft:
            return ICON_MDI_BORDER_LEFT_VARIANT "  Left";
            break;
        case eCameraViewType::OrthographicRight:
            return ICON_MDI_BORDER_RIGHT_VARIANT "  Right";
            break;
        case eCameraViewType::OrthographicFront:
            return ICON_MDI_FLIP_TO_FRONT "  Front";
            break;
        case eCameraViewType::OrthographicBack:
            return ICON_MDI_FLIP_TO_BACK "  Back";
            break;
        default:
            HO_ASSERT(false, "Invalid view mode.");
            return "None";
    }
}

const char* EditorSceneViewWindow::getRenderingModeName(eRenderingMode mode) const
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