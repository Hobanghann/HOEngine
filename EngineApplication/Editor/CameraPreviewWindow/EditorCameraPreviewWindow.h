#pragma once

#include "../../IEngineWindow.h"
#include "SceneDispatchers/SceneRenderer.h"

namespace ho
{
class Editor;
struct SceneViewArea;

class EditorCameraPreviewWindow final : public IEngineWindow
{
  public:
    EditorCameraPreviewWindow(Editor* pOwner);

    EditorCameraPreviewWindow(const EditorCameraPreviewWindow&) = delete;
    EditorCameraPreviewWindow& operator=(const EditorCameraPreviewWindow&) = delete;

    ~EditorCameraPreviewWindow() override;

    bool OnInit() override;
    bool OnPreUpdate(float deltaTime) override;
    bool OnUpdate(float deltaTime) override;
    bool OnPostUpdate(float deltaTime) override;
    bool OnRender() override;
    void OnShutdown() override;

    void DrawUI() override;

  private:
    void drawConfigWindow();
    void drawCameraSelectionCombo();
    void drawSceneViewOptions(float buttonHeight, float padding);

    bool mbOpenRenderingModeSettingPopup = false;
    void drawRenderingModeSettingPopup();
    bool mbOpenVisibilitySettingPopup = false;
    void drawVisibilitySettingPopup();
    bool mbOpenSettingPopup = false;
    void drawSettingPopup();

    SceneViewArea calculateSceneViewArea() const;
    void drawSceneViewImage(const SceneViewArea& area);

    const char* getRenderingModeName(eRenderingMode mode) const;

    StringHandle mhFrameBufferName;
    int32_t mFrameBufferWidth = 1280;
    int32_t mFrameBufferHeight = 720;
    bool mbNeedResizeFrameBuffer = false;
    bool mbDisplayFitMode = true;

    eRenderingMode mRenderingMode = eRenderingMode::Lit;
    eRenderingFlags mRenderingFlags = RenderingFlags_None;

    ObjectHandle<const RCameraComponent> mhCameraComp;

    Editor* mpOwner;
};
} // namespace ho