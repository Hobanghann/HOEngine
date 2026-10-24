#pragma once

#include "IRenderPass.h"
#include "PointLightShadowMapRenderPassDesc.h"

namespace ho
{
class PointLightShadowMapRenderPass : public IRenderPass
{
  public:
    PointLightShadowMapRenderPass() = default;

    PointLightShadowMapRenderPass(IRenderingBackend* pBackend, RenderingBuiltInAssets* pBuiltInAssets)
      : IRenderPass(pBackend, pBuiltInAssets)
    {
        mType = eRenderPassType::PointLightShadowMapRenderPass;
    }

    PointLightShadowMapRenderPass(const PointLightShadowMapRenderPass& other)
      : IRenderPass(other)
    {
    }

    PointLightShadowMapRenderPass& operator=(const PointLightShadowMapRenderPass& rhs)
    {
        if (this != &rhs)
        {
            IRenderPass::operator=(rhs);
        }
        return *this;
    }

    ~PointLightShadowMapRenderPass() override = default;

    void Setup(const PointLightShadowMapRenderPassDesc& desc);

    void PrepareExecute(const RenderResourceContext& resourceContext) override;
    void FinishExecute(const RenderResourceContext& resourceContext) override;
    void TearDown() override;

  private:
    PointLightShadowMapRenderPassDesc mPassDesc;
};
} // namespace ho