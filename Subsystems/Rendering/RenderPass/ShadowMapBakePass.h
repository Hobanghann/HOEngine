#pragma once

#include "IRenderPass.h"
#include "ShadowMapBakePassDesc.h"

namespace ho
{
class ShadowMapBakePass final : public IRenderPass
{
  public:
    ShadowMapBakePass() = default;

    ShadowMapBakePass(IRenderingBackend* pBackend, RenderingBuiltInAssets* pBuiltInAssets)
      : IRenderPass(pBackend, pBuiltInAssets)
    {
        mType = eRenderPassType::ShadowMapBakePass;
    }

    ShadowMapBakePass(const ShadowMapBakePass& other)
      : IRenderPass(other)
    {
    }

    ShadowMapBakePass& operator=(const ShadowMapBakePass& rhs)
    {
        if (this != &rhs)
        {
            IRenderPass::operator=(rhs);
        }
        return *this;
    }

    ~ShadowMapBakePass() override = default;

    void Setup(const ShadowMapBakePassDesc& desc);

    void PrepareExecute(const RenderResourceContext& resourceContext) override;
    void Execute(const RenderResourceContext& resourceContext) override;
    void FinishExecute(const RenderResourceContext& resourceContext) override;
    void TearDown() override;

  private:
    ShadowMapBakePassDesc mPassDesc;

    GpuTextureHandle mhBakedTexture;
};
} // namespace ho