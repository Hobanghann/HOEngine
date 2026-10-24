#pragma once

#include "EnvironmentBakePassDesc.h"
#include "IRenderPass.h"

namespace ho
{
class EnvironmentBakePass final : public IRenderPass
{
  public:
    EnvironmentBakePass() = default;

    EnvironmentBakePass(IRenderingBackend* pBackend, RenderingBuiltInAssets* pBuiltInAssets)
      : IRenderPass(pBackend, pBuiltInAssets)
    {
        mType = eRenderPassType::EnvironmentBakePass;
    }

    EnvironmentBakePass(const EnvironmentBakePass& other)
      : IRenderPass(other)
    {
    }

    EnvironmentBakePass& operator=(const EnvironmentBakePass& rhs)
    {
        if (this != &rhs)
        {
            IRenderPass::operator=(rhs);
        }
        return *this;
    }

    ~EnvironmentBakePass() override = default;

    void Setup(const EnvironmentBakePassDesc& desc);

    void PrepareExecute(const RenderResourceContext& resourceContext) override;
    void Execute(const RenderResourceContext& resourceContext) override;
    void FinishExecute(const RenderResourceContext& resourceContext) override;
    void TearDown() override;

  private:
    EnvironmentBakePassDesc mPassDesc;

    GpuTextureHandle mhBakedTexture;
};
} // namespace ho