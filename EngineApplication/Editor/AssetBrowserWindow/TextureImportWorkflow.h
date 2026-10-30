#pragma once

#include <memory>
#include <string>

#include "Core/IO/Path.h"
#include "Core/IO/TextureIR.h"
#include "Core/Templates/FixedArray.h"
#include "Subsystems/Asset/AssetDefs.h"
#include "Subsystems/Asset/TextureAsset.h"
#include "Subsystems/Object/ObjectHandle.h"

namespace ho
{
class TextureImportWorkflow
{
  public:
    struct TextureParseContext
    {
        eTextureType TextureType = eTextureType::None;

        Path Path[6] = {};
        char TextureName[256] = {};

        bool bIsLinear = false;

        std::unique_ptr<const parser::TextureIR> pParsedTextures[6] = {};
    };

    struct TextureImportContext
    {
        eTextureType TextureType = eTextureType::None;

        eTextureFormat TextureFormat = eTextureFormat::R8G8B8A8_SRGB;
        bool bGenerateMipmap = true;

        int32_t TotalImportedTextureCount = 0;
        int32_t ImportedTextureCount = 0;

        ObjectHandle<RTextureAsset> hImportedTexture;
    };

    void Begin2D();
    void BeginCubemap();
    void Draw();
    void Process();

    bool IsSucceed() const
    {
        return mState == eState::Succeed;
    }

    const TextureParseContext& GetParseContext() const
    {
        return mParseContext;
    }

    const TextureImportContext& GetImportContext() const
    {
        return mImportContext;
    }

  private:
    enum class eState
    {
        None,
        Begin2D,
        BeginCubemap,
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

    std::string showTextureFileOpenDialog();

    void initializeParseContext(eTextureType textureType);
    void clearParseContext();
    bool initializeImportContext();
    void clearImportContext();
    void deleteImpotedTexture() const;

    bool beginParsing();
    bool checkParsing();
    bool beginImporting();
    bool checkImporting();

    eState mState = eState::None;

    bool mbNeedToDeleteTexture = false;

    TextureParseContext mParseContext;
    TextureImportContext mImportContext;
};
} // namespace ho