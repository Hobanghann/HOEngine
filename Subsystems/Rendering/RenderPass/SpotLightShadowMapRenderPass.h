#pragma once

#include "IRenderPass.h"
#include "SpotLightShadowMapRenderPassDesc.h"

namespace ho
{
class SpotLightShadowMapRenderPass final : public IRenderPass
{
  public:
    SpotLightShadowMapRenderPass() = default;

    SpotLightShadowMapRenderPass(IRenderingBackend* pBackend, RenderingBuiltInAssets* pBuiltInAssets)
      : IRenderPass(pBackend, pBuiltInAssets)
    {
        mType = eRenderPassType::SpotLightShadowMapRenderPass;
    }

    SpotLightShadowMapRenderPass(const SpotLightShadowMapRenderPass& other)
      : IRenderPass(other)
    {
    }

    SpotLightShadowMapRenderPass& operator=(const SpotLightShadowMapRenderPass& rhs)
    {
        if (this != &rhs)
        {
            IRenderPass::operator=(rhs);
        }
        return *this;
    }

    ~SpotLightShadowMapRenderPass() override = default;

    void Setup(const SpotLightShadowMapRenderPassDesc& desc);

    void PrepareExecute(const RenderResourceContext& resourceContext) override;
    void FinishExecute(const RenderResourceContext& resourceContext) override;
    void TearDown() override;

  private:
    SpotLightShadowMapRenderPassDesc mPassDesc;
};
} // namespace ho