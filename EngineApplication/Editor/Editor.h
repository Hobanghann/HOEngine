#pragma once

#include <memory>

#include "../IEngineApplication.h"
#include "SceneDispatchers/SceneRenderer.h"
#include "Subsystems/Object/ObjectHandle.h"

namespace ho
{
class RScene;
class RGameObject;

class EditorMainWindow;

class Editor final : public IEngineApplication
{
    friend EditorMainWindow;

  public:
    Editor();
    ~Editor() override;

    bool OnInit() override;
    bool OnPreUpdate(float deltaTime) override;
    bool OnUpdate(float deltaTime) override;
    bool OnPostUpdate(float deltaTime) override;
    bool OnRender() override;
    void OnShutdown() override;

    void SetCurrentScene(ObjectHandle<RScene> hCurrentScene);
    ObjectHandle<RScene> GetCurrentScene() const;

    void SetSelectedGameObject(ObjectHandle<RGameObject> hSelectedGameObject);
    ObjectHandle<RGameObject> GetSelectedGameObject() const;

    SceneRenderer& GetSceneRenderer();

  private:
    ObjectHandle<RScene> mhCurrentScene;
    ObjectHandle<RGameObject> mhSelectedGameObject;

    SceneRenderer mSceneRenderer;
};
} // namespace ho