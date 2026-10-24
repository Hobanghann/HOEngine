#include "EnvironmentBakePass.h"

#include "../RenderBackend/GpuBufferLayouts.h"
#include "../RenderBackend/IRenderingBackend.h"
#include "../RenderBackend/RenderResourceContext.h"
#include "../RenderingBuiltInAssets.h"

namespace ho
{
void EnvironmentBakePass::Setup(const EnvironmentBakePassDesc& desc)
{
    IRenderPass::Setup(Vector3::sZero);

    HO_ASSERT(desc.SampleCount > 0, "Sample count must be greater than zero.");
    HO_ASSERT((desc.SampleCount & (desc.SampleCount - 1)) == 0,
              "Sample count should be a power of 2 for optimal Hammersley sampling.");
    HO_ASSERT(desc.SampleDelta > 0.0f && desc.SampleDelta < 1.0f, "Sample delta must be a positive fraction.");

    mPassDesc = desc;
}

void EnvironmentBakePass::PrepareExecute(const RenderResourceContext& resourceContext)
{
    HO_ASSERT(mPassDesc.hSrcTexture.IsValid() && mPassDesc.hSrcTexture->State != eGpuResourceState::PendingUpload,
              "Invalid source texture.");

    if (!mbSetup)
    {
        return;
    }

    IRenderPass::PrepareExecute(resourceContext);

    EnvironmentBakeDataLayout bakeData;
    if (mPassDesc.BakeType == eEnvironmentBakeType::Irradiance)
    {
        HO_ASSERT(mPassDesc.BakedMapSize >= 16 && mPassDesc.BakedMapSize <= 256,
                  "Irradiance map size is typically between 16 and 256.");
        mhBakedTexture = mpBackend->PrepareCreateTextureCubeMap(
            eTextureFormat::R16G16B16A16_FLOAT, mPassDesc.BakedMapSize, mPassDesc.BakedMapSize, 1, false);
        mpBackend->CreateTexture(mhBakedTexture);

        mpBackend->BindShader(mpBuiltInAssets->hIrradianceConvolutionCS, eShaderStage::ComputeShader);

        bakeData.SampleDelta = mPassDesc.SampleDelta;
        mpBackend->UploadBufferData(resourceContext.hEnvironmentBakeDataBuffer,
                                    &bakeData,
                                    static_cast<int32_t>(sizeof(EnvironmentBakeDataLayout)));
        mpBackend->BindBuffer(resourceContext.hEnvironmentBakeDataBuffer, eBufferBindingIndex::EnvironmentBakeData);

        mpBackend->BindTexture(mPassDesc.hSrcTexture, eTextureBindingIndex::RadianceMap);
        mpBackend->BindStorageTexture(mhBakedTexture,
                                      eStorageTextureBindingIndex::IrradianceMap,
                                      0,
                                      0,
                                      IRenderingBackend::eResourceAccessType::WriteOnly);
    }
    else if (mPassDesc.BakeType == eEnvironmentBakeType::PrefilteredEnvMap)
    {
        HO_ASSERT(mPassDesc.BakedMapSize >= 128,
                  "Prefiltered map requires a higher resolution (>= 128) to support mipmap chain properly.");
        mhBakedTexture = mpBackend->PrepareCreateTextureCubeMap(
            eTextureFormat::R16G16B16A16_FLOAT, mPassDesc.BakedMapSize, mPassDesc.BakedMapSize, 5, false);
        mpBackend->CreateTexture(mhBakedTexture);

        mpBackend->BindShader(mpBuiltInAssets->hPrefilteredEnvironmentConvolutionCS, eShaderStage::ComputeShader);
        mpBackend->BindBuffer(resourceContext.hEnvironmentBakeDataBuffer, eBufferBindingIndex::EnvironmentBakeData);
        mpBackend->BindTexture(mPassDesc.hSrcTexture, eTextureBindingIndex::RadianceMap);
    }
    else
    {
        HO_ASSERT(false, "Invalid bake type.");
    }
}

void EnvironmentBakePass::Execute(const RenderResourceContext& resourceContext)
{
    IRenderPass::Execute(resourceContext);

    if (mPassDesc.hSrcTexture.IsNULL())
    {
        return;
    }

    EnvironmentBakeDataLayout bakeData;
    if (mPassDesc.BakeType == eEnvironmentBakeType::Irradiance)
    {
        mpBackend->Dispatch(static_cast<uint32_t>(math::Ceil(static_cast<float>(mPassDesc.BakedMapSize) / 16.0f)),
                            static_cast<uint32_t>(math::Ceil(static_cast<float>(mPassDesc.BakedMapSize) / 16.0f)),
                            6);
    }
    else if (mPassDesc.BakeType == eEnvironmentBakeType::PrefilteredEnvMap)
    {
        const int32_t totalMipLevels = mhBakedTexture->MipLevels;
        const float maxMipIndex = static_cast<float>(totalMipLevels > 1 ? totalMipLevels - 1 : 1);

        for (int32_t level = 0; level < totalMipLevels; ++level)
        {
            const float roughness = static_cast<float>(level) / maxMipIndex;
            bakeData.Roughness = roughness;
            bakeData.SampleCount = mPassDesc.SampleCount;
            mpBackend->UploadBufferData(resourceContext.hEnvironmentBakeDataBuffer,
                                        &bakeData,
                                        static_cast<int32_t>(sizeof(EnvironmentBakeDataLayout)));

            mpBackend->BindStorageTexture(mhBakedTexture,
                                          eStorageTextureBindingIndex::PrefilteredEnvironmentMap,
                                          level,
                                          0,
                                          IRenderingBackend::eResourceAccessType::WriteOnly);

            const int32_t currentMipSize = math::Max(1, mPassDesc.BakedMapSize >> static_cast<uint32_t>(level));
            mpBackend->Dispatch(static_cast<uint32_t>(math::Ceil(static_cast<float>(currentMipSize) / 16.0f)),
                                static_cast<uint32_t>(math::Ceil(static_cast<float>(currentMipSize) / 16.0f)),
                                6);
        }
    }
    else
    {
        HO_ASSERT(false, "Invalid bake type.");
    }

    mpBackend->DownloadTexture(mhBakedTexture, mPassDesc.hDstTexture);
}

void EnvironmentBakePass::FinishExecute(const RenderResourceContext& resourceContext)
{
    IRenderPass::FinishExecute(resourceContext);
    mpBackend->UnloadTexture(mhBakedTexture);
    mpBackend->Flush();
}

void EnvironmentBakePass::TearDown()
{
    IRenderPass::TearDown();
}
} // namespace ho