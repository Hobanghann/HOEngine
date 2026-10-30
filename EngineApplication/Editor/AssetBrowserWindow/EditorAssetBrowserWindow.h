#pragma once

#include "../../IEngineWindow.h"
#include "../Utils/DirectoryHierarchy.h"
#include "ModelImportWorkflow.h"
#include "ShaderImportWorkflow.h"
#include "TextureBakeWorkflow.h"
#include "TextureImportWorkflow.h"

namespace ho
{
class Editor;
class DirectoryHierarchyView;
class DirectoryItemView;

class EditorAssetBrowserWindow final : public IEngineWindow
{
  public:
    EditorAssetBrowserWindow();

    EditorAssetBrowserWindow(const EditorAssetBrowserWindow&) = delete;
    EditorAssetBrowserWindow& operator=(const EditorAssetBrowserWindow&) = delete;

    ~EditorAssetBrowserWindow() override;

    bool OnInit() override;
    bool OnPreUpdate(float deltaTime) override;
    bool OnUpdate(float deltaTime) override;
    bool OnPostUpdate(float deltaTime) override;
    bool OnRender() override;
    void OnShutdown() override;

    void DrawUI() override;

  private:
    bool mbOpenMenuBarAddPopup = false;
    void drawMenuBarAddPopup();
    bool mbOpenNewFolderPopup = false;
    void drawNewFolderPopup();
    bool mbOpenMenuBarImportPopup = false;
    void drawMenuBarImportPopup();

    void handleDirectoryHierarchyViewRequest();
    void handleDirectoryItemViewRequest();

    void addModelToDirectoryHierarchy();
    void addTextureToDirectoryHierarchy();
    void addShaderToDirectoryHierarchy();
    void addBakedTextureToDirectoryHierarchy();

    bool mbOpenDirectoryPropertiesPopup = false;
    void drawDirectoryPropertiesPopup();
    bool mbOpenItemPropertiesPopup = false;
    void drawItemPropertiesPopup();

    ModelImportWorkflow mModelImportWorkflow;
    TextureImportWorkflow mTextureImportWorkflow;
    ShaderImportWorkflow mShaderImportWorkflow;
    TextureBakeWorkflow mTextureBakeWorkflow;

    DirectoryHierarchy mDirectoryHierarchy;
    DirectoryHierarchy::Directory* mpOpenedDir = nullptr;
    DirectoryHierarchy::Directory* mpSelectedDir = nullptr;
    DirectoryHierarchy::DirectoryItem* mpSelectedItem = nullptr;

    std::unique_ptr<DirectoryHierarchyView> mpDirectoryHierarchyView;
    std::unique_ptr<DirectoryItemView> mpDirectoryItemView;
};
} // namespace ho