#pragma once

#include "../../IEngineWindow.h"
#include "../Utils/OrthographicCamera.h"
#include "../Utils/PerspectiveCamera.h"
#include "SceneDispatchers/SceneRenderer.h"

namespace ho
{
class Editor;

struct SceneViewArea;

class EditorSceneViewWindow final : public IEngineWindow
{
  public:
    EditorSceneViewWindow(Editor* pOwner);

    EditorSceneViewWindow(const EditorSceneViewWindow&) = delete;
    EditorSceneViewWindow& operator=(const EditorSceneViewWindow&) = delete;

    ~EditorSceneViewWindow() override;

    bool OnInit() override;
    bool OnPreUpdate(float deltaTime) override;
    bool OnUpdate(float deltaTime) override;
    bool OnPostUpdate(float deltaTime) override;
    bool OnRender() override;
    void OnShutdown() override;

    void DrawUI() override;

  private:
    enum class eCameraViewType
    {
        Perspective,
        OrthographicTop,
        OrthographicBottom,
        OrthographicLeft,
        OrthographicRight,
        OrthographicFront,
        OrthographicBack,
    };

    void handleInput(float deltaTime);

    void drawConfigWindow();
    void drawTransformToolButtons(float buttonHeight);
    void drawTranslateSnapControls(float buttonHeight);
    void drawRotateSnapControls(float buttonHeight);
    void drawScaleSnapControls(float buttonHeight);
    void drawSceneViewOptions(float buttonHeight, float padding);

    bool mbOpenTransformToolsPopup = false;
    void drawTransformToolsPopup();
    bool mbOpenTranslateSnapPopup = false;
    void drawTranslateSnapPopup();
    bool mbOpenRotateSnapPopup = false;
    void drawRotateSnapPopup();
    bool mbOpenScaleSnapPopup = false;
    void drawScaleSnapPopup();
    bool mbOpenCameraOptionPopup = false;
    void drawCameraOptionPopup();
    bool mbOpenCameraSpeedPopup = false;
    void drawCameraSpeedPopup();
    bool mbOpenRenderingModeSettingPopup = false;
    void drawRenderingModeSettingPopup();
    bool mbOpenVisibilitySettingPopup = false;
    void drawVisibilitySettingPopup();
    bool mbOpenSettingPopup = false;
    void drawSettingPopup();

    SceneViewArea calculateSceneViewArea() const;
    void drawSceneViewImage(const SceneViewArea& area);
    void drawCameraGizmo(const SceneViewArea& area);
    void drawTransformGizmo(const SceneViewArea& area);
    void updateMouseInteraction(const SceneViewArea& area);

    const char* getCameraViewTypeName(eCameraViewType type) const;
    const char* getRenderingModeName(eRenderingMode mode) const;

    StringHandle mhFrameBufferName;
    int32_t mFrameBufferWidth = 1280;
    int32_t mFrameBufferHeight = 720;
    bool mbNeedResizeFrameBuffer = false;
    bool mbDisplayFitMode = true;

    eCameraViewType mCameraViewType = eCameraViewType::Perspective;
    eRenderingMode mRenderingMode = eRenderingMode::Lit;
    eRenderingFlags mRenderingFlags =
        static_cast<eRenderingFlags>(RenderingFlags_RenderWorldGizmo | RenderingFlags_RenderObjectOutline |
                                     RenderingFlags_RenderLightDebugMesh | RenderingFlags_RenderCameraDebugMesh);

    // Input
    bool mbDragging = false;
    bool mbHovering = false;

    // Camera
    PerspectiveCamera mPerspCamera;
    OrthographicCamera mOrthoCamera;
    float mCameraMoveSpeed = 10.0f;
    float mCameraRotateSpeed = math::PI4;

    // ImGuizmo configs
    int mCurrentGizmoOperation = 0;
    int mCurrentGizmoMode = 0;
    bool mbUseTranslateSnap = false;
    Vector3 mTranslateSnap = Vector3(1.0f, 1.0f, 1.0f);
    bool mbUseRotateSnap = false;
    float mRotateSnap = 1.0f;
    bool mbUseScaleSnap = false;
    Vector3 mScaleSnap = Vector3(1.0f, 1.0f, 1.0f);
    bool mbShowTransformGizmo = true;
    float mGizmoScale = 1.0f;
    bool mbPreserveNonUniformScale = true;

    Editor* mpOwner;
};
} // namespace ho