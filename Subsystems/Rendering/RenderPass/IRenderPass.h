#pragma once
#include "../RenderStage/IRenderStage.h"
#include "Core/Math/Vector3.h"

namespace ho
{
class IRenderingBackend;
struct RenderResourceContext;

enum class eRenderPassType
{
    None = 0,
    DirLightShadowMapRenderPass,
    PointLightShadowMapRenderPass,
    SpotLightShadowMapRenderPass,
    ForwardRenderPass,
    DeferedRenderPass,
    ShadowMapBakePass,
    EnvironmentBakePass,
};

class IRenderPass
{
  public:
    IRenderPass();

    IRenderPass(IRenderingBackend* pBackend, RenderingBuiltInAssets* pBuiltInAssets);

    IRenderPass(const IRenderPass& other);

    IRenderPass& operator=(const IRenderPass& rhs);

    virtual ~IRenderPass();

    eRenderPassType GetType() const;

    // Setup is called by the main thread and constructs the pass.
    void Setup(const Vector3& cameraWorldPos);

    // PrepareExecute through FinishExecute are called by the rendering thread.
    // An asset judged by the main thread to be uploaded to the GPU during Setup
    // may be evicted by the rendering backend at the time the render queue is swapped.
    // Therefore, functions from PrepareExecute to FinishExecute must be implemented
    // considering that recorded GpuResourceHandles may be invalid.
    virtual void PrepareExecute(const RenderResourceContext& resourceContext);
    virtual void Execute(const RenderResourceContext& resourceContext);
    virtual void FinishExecute(const RenderResourceContext& resourceContext);

    // TearDown is called by the main thread and tear down the pass, stage.
    virtual void TearDown();

    void SetBound(bool bIsBound);

    bool IsBound() const;

    bool IsSetup() const;

    bool IsPrepared() const;

    virtual void AttachStage(eRenderStageType stageType);

    bool IsStageAttached(eRenderStageType stageType) const;

    IRenderStage* GetStage(eRenderStageType stageType) const;

    const Vector3& GetWorldCameraPos() const
    {
        return mCameraWorldPos;
    }

  protected:
    eRenderPassType mType = eRenderPassType::None;

    bool mbIsBound = false;
    bool mbSetup = false;
    bool mbPrepared = false;

    IRenderStage* mpStagePipeline[static_cast<int32_t>(eRenderStageType::Last)] = {};

    RenderingBuiltInAssets* mpBuiltInAssets = nullptr;
    IRenderingBackend* mpBackend = nullptr;

    // NOTE: This field exists solely for external code to sort instance data.
    // It is meaningless/unused in passes that do not execute internal draw commands.
    Vector3 mCameraWorldPos;
};
} // namespace ho