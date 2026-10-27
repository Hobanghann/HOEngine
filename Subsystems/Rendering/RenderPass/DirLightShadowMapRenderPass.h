#pragma once

#include "DirLightShadowMapRenderPassDesc.h"
#include "IRenderPass.h"

namespace ho
{
class DirLightShadowMapRenderPass final : public IRenderPass
{
  public:
    DirLightShadowMapRenderPass() = default;

    DirLightShadowMapRenderPass(IRenderingBackend* pBackend, RenderingBuiltInAssets* pBuiltInAssets)
      : IRenderPass(pBackend, pBuiltInAssets)
    {
        mType = eRenderPassType::DirLightShadowMapRenderPass;
    }

    DirLightShadowMapRenderPass(const DirLightShadowMapRenderPass& other)
      : IRenderPass(other)
    {
    }

    DirLightShadowMapRenderPass& operator=(const DirLightShadowMapRenderPass& rhs)
    {
        if (this != &rhs)
        {
            IRenderPass::operator=(rhs);
        }
        return *this;
    }

    ~DirLightShadowMapRenderPass() override = default;

    void Setup(const DirLightShadowMapRenderPassDesc& desc);

    void PrepareExecute(const RenderResourceContext& resourceContext) override;
    void FinishExecute(const RenderResourceContext& resourceContext) override;
    void TearDown() override;

  private:
    DirLightShadowMapRenderPassDesc mPassDesc;
};
} // namespace ho