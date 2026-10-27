#pragma once

#include "../../IEngineWindow.h"
#include "Subsystems/Object/ObjectHandle.h"

namespace ho
{
class Editor;
class RGameObject;
class RComponent;
class RTransformComponent;

class EditorInspectorWindow final : public IEngineWindow
{
  public:
    EditorInspectorWindow(Editor* pOwner);

    EditorInspectorWindow(const EditorInspectorWindow&) = delete;
    EditorInspectorWindow& operator=(const EditorInspectorWindow&) = delete;

    ~EditorInspectorWindow() override;

    bool OnInit() override;
    bool OnPreUpdate(float deltaTime) override;
    bool OnUpdate(float deltaTime) override;
    bool OnPostUpdate(float deltaTime) override;
    bool OnRender() override;
    void OnShutdown() override;

    void DrawUI() override;

  private:
    bool mbOpenAddComponentPopup = false;
    void drawAddComponentPopup();

    void drawComponent(ObjectHandle<RComponent> hComp);

    void drawTransformComponent(ObjectHandle<RTransformComponent> hTransformComp);

    Editor* mpOwner;
};
} // namespace ho