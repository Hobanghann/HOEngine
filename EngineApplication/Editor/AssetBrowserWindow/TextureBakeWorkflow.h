#pragma once

#include <string>

#include "Core/Templates/FixedArray.h"
#include "Subsystems/Asset/AssetDefs.h"
#include "Subsystems/Asset/TextureAsset.h"
#include "Subsystems/Object/ObjectHandle.h"
#include "Subsystems/Rendering/RenderPass/EnvironmentBakePassDesc.h"

namespace ho
{
class TextureBakeWorkflow
{
  public:
    struct TextureBakeContext
    {
        ObjectHandle<RTextureAsset> hSrcTexture;

        char DstTextureName[256] = {};

        eEnvironmentBakeType BakeType = eEnvironmentBakeType::Irradiance;
        int32_t BakedMapSize = 0;
        float SampleDelta = 0.0f;
        int32_t SampleCount = 0;

        ObjectHandle<RTextureAsset> hDstTexture;
    };

    void Begin(ObjectHandle<RTextureAsset> hSrcTexture);
    void Draw();
    void Process();

    bool IsSucceed() const
    {
        return mState == eState::Succeed;
    }

    const TextureBakeContext& GetBakeContext() const
    {
        return mBakeContext;
    }

  private:
    enum class eState
    {
        None,
        Begin,
        BakeSetting,
        BakeBegin,
        Baking,
        Failed,
        Succeed,
        Finish,
    };

    bool mbOpenBakeSettingPopup = false;
    void drawBakeSettingPopup();
    bool mbOpenBakingPopup = false;
    void drawBakingPopup();
    bool mbOpenBakeFailedPopup = false;
    void drawBakeFailedPopup();

    std::string showTextureFileOpenDialog();

    void initializeBakeContext(ObjectHandle<RTextureAsset> hSrcTexture);
    void clearBakeContext();
    void deleteBakedTexture() const;

    bool beginBaking();
    bool checkBaking();

    ObjectHandle<RTextureAsset> mhSrcTexture = ObjectHandle<RTextureAsset>::sNULL;

    eState mState = eState::None;

    bool mbNeedToDeleteTexture = false;

    TextureBakeContext mBakeContext;
};
} // namespace ho