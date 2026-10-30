#pragma once

#include <memory>
#include <string>

#include "Core/IO/ModelIR.h"
#include "Core/IO/Path.h"
#include "Core/Templates/FixedArray.h"
#include "Subsystems/Asset/AssetDefs.h"
#include "Subsystems/Asset/Assets.h"
#include "Subsystems/Object/ObjectHandle.h"

namespace ho
{
class ModelImportWorkflow
{
  public:
    struct ModelParseContext
    {
        Path Path;
        char ModelName[256] = {};
        bool bMakeStatic = false;

        std::unique_ptr<const parser::ModelIR> pParsedModel;
    };

    struct ModelImportContext
    {
        bool bImportAsStaticMesh = true;

        FixedArray<eMaterialAssetType> MaterialTypes;

        FixedArray<eMaterialTextureUsage> TextureUsage;
        FixedArray<eTextureFormat> TextureFormats;
        FixedArray<bool> bGenerateMipmap;

        bool bHasMorphTargets = false;
        bool bImportMorphTargets = false;

        bool bHasMorphAnimation = false;
        FixedArray<bool> bImportMorphAnimation;

        int32_t TotalImportedAssetCount = 0;
        int32_t ImportedAssetCount = 0;

        ObjectHandle<RStaticMeshAsset> hImportedStaticMesh;
        FixedArray<ObjectHandle<RMaterialAsset>> hImportedMaterials;
        FixedArray<ObjectHandle<RTextureAsset>> hImportedTextures;
        ObjectHandle<RMorphTargetsAsset> hImportedMorphTargets;
        FixedArray<ObjectHandle<RMorphAnimationAsset>> hImportedMorphAnims;
    };

    void Begin();
    void Draw();
    void Process();

    bool IsSucceed() const
    {
        return mState == eState::Succeed;
    }

    const ModelParseContext& GetParseContext() const
    {
        return mParseContext;
    }

    const ModelImportContext& GetImportContext() const
    {
        return mImportContext;
    }

  private:
    enum class eState
    {
        None,
        Begin,
        ParseSetting,
        ParseBegin,
        Parsing,
        ImportSetting,
        ImportBegin,
        Importing,
        Succeed,
        ParseFailed,
        ImportFailed,
        Finish,
    };
    bool mbOpenParseSettingPopup = false;
    void drawParseSettingPopup();

    bool mbOpenParsingPopup = false;
    void drawParsingPopup();

    bool mbOpenImportSettingPopup = false;
    void drawImportSettingPopup();

    bool mbOpenImportingPopup = false;
    void drawImportingPopup();

    bool mbOpenParseFailedPopup = false;
    void drawParseFailedPopup();

    bool mbOpenImportFailedPopup = false;
    void drawImportFailedPopup();

    std::string showModelFileOpenDialog();

    void initializeParseContext(const std::string& importPathStr);
    void clearParseContext();
    void initializeImportContext();
    void clearImportContext();
    void deleteImpotedAssets();

    bool beginParsing();
    bool checkParsing();
    bool beginImporting();
    bool checkImporting();

    eState mState = eState::None;

    bool mbNeedToDeleteAssets = false;

    ModelParseContext mParseContext;
    ModelImportContext mImportContext;
};
} // namespace ho