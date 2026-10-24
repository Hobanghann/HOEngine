#include "EnvironmentRenderStage.h"

#include "../RenderBackend/GpuBufferLayouts.h"
#include "../RenderBackend/IRenderingBackend.h"
#include "../RenderBackend/RenderResourceContext.h"
#include "../RenderData/DrawCommand.h"
#include "../RenderingBuiltInAssets.h"
#include "EnvironmentRenderStageDesc.h"
#include "Subsystems/Asset/TextureAsset.h"

namespace ho
{
void EnvironmentRenderStage::Setup(const EnvironmentRenderStageDesc& desc)
{
    IRenderStage::Setup();

    mStageDesc = desc;
}

void EnvironmentRenderStage::PrepareExecute(const RenderResourceContext& resourceContext)
{
    if (!mbSetup)
    {
        return;
    }

    IRenderStage::PrepareExecute(resourceContext);

    mpBackend->SetCullMode(IRenderingBackend::eCullMode::None);

    mpBackend->SetDepthTest(true);
    mpBackend->SetDepthWrite(false);
    mpBackend->SetDepthFunc(IRenderingBackend::eComparisonFunc::LEqual);

    mpBackend->SetStencilTest(false);

    mpBackend->SetAlphaBlendMode(IRenderingBackend::eAlphaBlendMode::None);
}

void EnvironmentRenderStage::Execute(const RenderResourceContext& resourceContext)
{
    HO_ASSERT(isExecutable(), "Stage is not executable");

    mpBackend->BindShader(mpBuiltInAssets->hRadianceMapVS, eShaderStage::VertexShader);
    mpBackend->BindShader(mpBuiltInAssets->hRadianceMapFS, eShaderStage::FragmentShader);

    GpuTextureHandle hSkyMap = mStageDesc.hSkyMap;
    if (hSkyMap.IsNULL())
    {
        hSkyMap = mpBuiltInAssets->hFallbackCubeMap;
    }
    else if (hSkyMap->State == eGpuResourceState::PendingUpload)
    {
        hSkyMap = mpBuiltInAssets->hCheckerBoardCubeMap;
    }
    mpBackend->BindTexture(hSkyMap, eTextureBindingIndex::RadianceMap);

    mpBackend->DrawProcedural(36);
}

void EnvironmentRenderStage::FinishExecute(const RenderResourceContext& resourceContext)
{
    IRenderStage::FinishExecute(resourceContext);
}

void EnvironmentRenderStage::TearDown()
{
    IRenderStage::TearDown();
}

void EnvironmentRenderStage::SortInstanceData(InstanceData* pInstanceDataBufferBegin,
                                              int32_t instanceCount,
                                              const Vector3& worldCameraPos)
{
    std::sort(pInstanceDataBufferBegin,
              pInstanceDataBufferBegin + instanceCount,
              [&](const InstanceData& a, const InstanceData& b)
              {
                  const float aDist = (worldCameraPos - Vector3(a.WorldMat.GetCol3())).SqrdMagnitude();
                  const float bDist = (worldCameraPos - Vector3(b.WorldMat.GetCol3())).SqrdMagnitude();
                  return aDist < bDist;
              });
}

void EnvironmentRenderStage::sortDrawCommands()
{
    if (!mpDrawCommandsBegin)
    {
        return;
    }

    std::sort(mpDrawCommandsBegin,
              mpDrawCommandsBegin + mDrawCommandCount,
              [](const DrawCommand& a, const DrawCommand& b)
              {
                  if (a.hGpuMaterial != b.hGpuMaterial)
                  {
                      return a.hGpuMaterial.GetID() < b.hGpuMaterial.GetID();
                  }
                  if (a.StencilWriteBitIndex != b.StencilWriteBitIndex)
                  {
                      return a.StencilWriteBitIndex < b.StencilWriteBitIndex;
                  }
                  return a.SortDepth < b.SortDepth;
              });
}
} // namespace ho