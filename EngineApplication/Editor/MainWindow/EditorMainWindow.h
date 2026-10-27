#pragma once

#include "../../IEngineWindow.h"
#include "Core/Templates/FixedQueue.h"
#include "Scene/Components/LightComponent.h"
#include "Scene/Scene.h"

namespace ho
{
class Editor;

class EditorMainWindow final : public IEngineWindow
{
  public:
    EditorMainWindow(Editor* pOwner);

    EditorMainWindow(const EditorMainWindow&) = delete;
    EditorMainWindow& operator=(const EditorMainWindow&) = delete;

    ~EditorMainWindow() override;

    bool OnInit() override;
    bool OnPreUpdate(float deltaTime) override;
    bool OnUpdate(float deltaTime) override;
    bool OnPostUpdate(float deltaTime) override;
    bool OnRender() override;
    void OnShutdown() override;

    void DrawUI() override;

  private:
    static const int32_t sMaxPendingBakingLightQueueSize = 256;
    static const int32_t sMaxPendingShadowMapResetSceneQueueSize = 32;

    void drawMainWindow();
    void drawMenuBar();
    void drawFileMenu();
    void drawProjectMenu();
    void drawBuildMenu();
    void drawWindowMenu();
    void drawSettingsMenu();

    void drawPopups();
    bool mbOpenNewScenePopup = false;
    void drawNewScenePopup();
    bool mbOpenImGuiMetricsWindow = false;

    std::string getUniqueSceneName(const std::string& baseNameStr) const;

    FixedQueue<ObjectHandle<RLightComponent>> mPendingBakingLightQueue;
    FixedQueue<ObjectHandle<RScene>> mhPendingShadowMapResetSceneQueue;
    Editor* mpOwner;
};
} // namespace ho