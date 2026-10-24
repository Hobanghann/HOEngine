#pragma once

#include "ForwardRenderPassDesc.h"
#include "IRenderPass.h"

namespace ho
{
class ForwardRenderPass : public IRenderPass
{
  public:
    ForwardRenderPass() = default;

    ForwardRenderPass(IRenderingBackend* pBackend, RenderingBuiltInAssets* pBuiltInAssets)
      : IRenderPass(pBackend, pBuiltInAssets)
    {
        mType = eRenderPassType::ForwardRenderPass;
    }

    ForwardRenderPass(const ForwardRenderPass& other)
      : IRenderPass(other)
    {
    }

    ForwardRenderPass& operator=(const ForwardRenderPass& rhs)
    {
        if (this != &rhs)
        {
            IRenderPass::operator=(rhs);
        }
        return *this;
    }

    ~ForwardRenderPass() override = default;

    void Setup(FrameBufferHandle hFrameBuffer, const ForwardRenderPassDesc& desc);

    void PrepareExecute(const RenderResourceContext& resourceContext) override;
    void Execute(const RenderResourceContext& resourceContext) override;
    void FinishExecute(const RenderResourceContext& resourceContext) override;
    void TearDown() override;

    FrameBufferHandle GetFrameBuffer() const
    {
        return mhFrameBuffer;
    }

  private:
    ForwardRenderPassDesc mPassDesc;

    FrameBufferHandle mhFrameBuffer;
};
} // namespace ho