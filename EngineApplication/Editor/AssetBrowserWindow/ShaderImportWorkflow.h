#pragma once

#include <memory>
#include <string>

#include "Core/IO/Path.h"
#include "Core/IO/ShaderIR.h"
#include "Core/Templates/FixedArray.h"
#include "Subsystems/Asset/AssetDefs.h"
#include "Subsystems/Asset/ShaderAsset.h"
#include "Subsystems/Object/ObjectHandle.h"

namespace ho
{
class ShaderImportWorkflow
{
  public:
    struct ShaderParseContext
    {
        Path Path;
        char ShaderName[256] = {};
        std::unique_ptr<const parser::ShaderIR> pParsedShader;
    };

    struct ShaderImportContext
    {
        eShaderStage ShaderStage = eShaderStage::None;
        eMaterialAssetType CompatibleMaterialType = eMaterialAssetType::None;

        std::string CompileErrorMsg;

        int32_t TotalImportedShaderCount = 0;
        int32_t ImportedShaderCount = 0;

        ObjectHandle<RShaderAsset> hImportedShader;
    };

    void Begin();
    void Draw();
    void Process();

    bool IsSucceed() const
    {
        return mState == eState::Succeed;
    }

    const ShaderParseContext& GetParseContext() const
    {
        return mParseContext;
    }

    const ShaderImportContext& GetImportContext() const
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

    std::string showShaderFileOpenDialog();

    void initializeParseContext(const std::string& path);
    void clearParseContext();
    void initializeImportContext();
    void clearImportContext();
    void deleteImpotedShader() const;

    bool beginParsing();
    bool checkParsing();
    bool beginImporting();
    bool checkImporting();

    eState mState = eState::None;

    bool mbNeedToDeleteShader = false;

    ShaderParseContext mParseContext;
    ShaderImportContext mImportContext;
};
} // namespace ho