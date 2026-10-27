#pragma once

#include "../../IEngineWindow.h"
#include "../Utils/DirectoryHierarchy.h"
#include "AssetLoadContext.h"

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
    enum class eModelLoadState
    {
        None,
        ParseSetting,
        Parsing,
        ImportSetting,
        ImportBegin,
        Importing,
        Fail,
    };

    enum class eTextureLoadState
    {
        None,
        ParseSetting,
        Parsing,
        ImportSetting,
        ImportBegin,
        Importing,
        Fail,
    };

    enum class eShaderLoadState
    {
        None,
        ParseSetting,
        Parsing,
        ImportSetting,
        Importing,
        Fail,
    };

    enum class eTextureBakeState
    {
        None,
        BakeSetting,
        Baking,
        Fail,
    };

    void processModelLoadPipeline();
    void processTextureLoadPipeline();
    void processShaderLoadPipeline();

    void processTextureBakePipeline();

    bool mbOpenMenuBarAddPopup = false;
    void drawMenuBarAddPopup();
    bool mbOpenNewFolderPopup = false;
    void drawNewFolderPopup();
    bool mbOpenMenuBarImportPopup = false;
    void drawMenuBarImportPopup();

    void handleDirectoryHierarchyViewRequest();
    void handleDirectoryItemViewRequest();

    // Model import functions
    void showModelFileOpenDialog();

    void initializeModelParseContext(const std::string& importPathStr);
    void resetModelParseContext();
    void initializeModelImportContext();
    void resetModelImportContext();
    void cleanModelImportContext();

    void drawModelImportWizard();
    void parseModel();
    void beginModelImport();
    void checkModelImport();
    void addModelToDirectoryHierarchy();

    // Shader import functions
    void showShaderFileOpenDialog();

    void initializeShaderParseContext(const std::string& importPathStr);
    void resetShaderParseContext();
    void initializeShaderImportContext();
    void resetShaderImportContext();
    void cleanShaderImportContext();

    void drawShaderImportWizard();
    void parseShader();
    void importShader();
    void addShaderToDirectoryHierarchy();

    // Texture import functions
    void resetTextureParseContext();
    void initializeTextureImportContext();
    void resetTextureImportContext();
    void cleanTextureImportContext();

    void drawTextureImportWizard();
    void parseTexture();
    void beginTextureImport();
    void checkTextureImport();
    void addTextureToDirectoryHierarchy();

    // Texture bake functions
    void initializeTextureBakeContext(ObjectHandle<RTextureAsset> hSrcTexture);
    void resetTextureBakeContext();
    void cleanTextureBakeContext();

    void drawTextureBakeWizard();
    void bakeTexture();
    void addBakedTextureToDirectoryHierarchy();

    void drawDirectoryPropertiesPopup();
    void drawItemPropertiesPopup();

    std::string getUniqueAssetName(const std::string& baseNameStr) const;

    eModelLoadState mModelLoadState = eModelLoadState::None;
    ModelParseContext mModelParseContext;
    ModelImportContext mModelImportContext;

    eShaderLoadState mShaderLoadState = eShaderLoadState::None;
    ShaderParseContext mShaderParseContext;
    ShaderImportContext mShaderImportContext;

    eTextureLoadState mTextureLoadState = eTextureLoadState::None;
    TextureParseContext mTextureParseContext;
    TextureImportContext mTextureImportContext;

    eTextureBakeState mTextureBakeState = eTextureBakeState::None;
    TextureBakeContext mTextureBakeContext;

    DirectoryHierarchy mDirectoryHierarchy;
    DirectoryHierarchy::Directory* mpOpenedDir = nullptr;
    DirectoryHierarchy::Directory* mpSelectedDir = nullptr;
    DirectoryHierarchy::DirectoryItem* mpSelectedItem = nullptr;

    std::unique_ptr<DirectoryHierarchyView> mpDirectoryHierarchyView;
    std::unique_ptr<DirectoryItemView> mpDirectoryItemView;
};
} // namespace ho