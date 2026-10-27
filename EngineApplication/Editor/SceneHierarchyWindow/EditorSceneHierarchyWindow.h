#pragma once

#include <imgui.h>

#include "../../IEngineWindow.h"
#include "Scene/Scene.h"

namespace ho
{
class Editor;
class RGameObject;

class EditorSceneHierarchyWindow final : public IEngineWindow
{
  public:
    EditorSceneHierarchyWindow(Editor* pOwner);

    EditorSceneHierarchyWindow(const EditorSceneHierarchyWindow&) = delete;
    EditorSceneHierarchyWindow& operator=(const EditorSceneHierarchyWindow&) = delete;

    ~EditorSceneHierarchyWindow() override;

    bool OnInit() override;
    bool OnPreUpdate(float deltaTime) override;
    bool OnUpdate(float deltaTime) override;
    bool OnPostUpdate(float deltaTime) override;
    bool OnRender() override;
    void OnShutdown() override;

    void DrawUI() override;

  private:
    void drawMainWindow();
    void drawSceneArea();
    void drawToolbar();
    void drawGameObjectHierarchy();
    bool mbOpenAddPopup = false;
    void drawAddPopup();

    void drawGameObjectTreeRecursive(ObjectHandle<RGameObject> hGameObj);
    void drawGameObjectRowBackground(ObjectHandle<RGameObject> hGameObj);
    void drawGameObjectVisibilityControls(ObjectHandle<RGameObject> hGameObj);
    bool drawGameObjectTreeNode(ObjectHandle<RGameObject> hGameObj);
    void processGameObjectDragAndDrop(ObjectHandle<RGameObject> hGameObj);
    void processGameObjectSelection(ObjectHandle<RGameObject> hGameObj);
    void drawGameObjectName(ObjectHandle<RGameObject> hGameObj);

    void drawPopups();
    bool mbOpenGameObjRightClickPopup = false;
    void drawGameObjRightClickPopup();
    bool mbOpenGameObjPropertiesPopup = false;
    void drawGameObjPropertiesPopup();

    void updateGameObjectFilteredRecursive(ObjectHandle<RGameObject> hGameObj) const;

    void setGameObjectVisibleInEditorRecursive(ObjectHandle<RGameObject> hGameObj, bool bVisible) const;
    void setGameObjectClickableInEditorRecursive(ObjectHandle<RGameObject> hGameObj, bool bClickable) const;

    std::string getUniqueGameObjectName(const std::string& baseNameStr) const;

    ObjectHandle<RGameObject> mhRenamingGameObject;

    ImGuiTextFilter mGameObjectFilter;

    Editor* mpOwner;
};
} // namespace ho