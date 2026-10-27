#include "IRenderPass.h"

#include "../RenderBackend/IRenderingBackend.h"
#include "../RenderBackend/RenderResourceContext.h"
#include "../RenderStage/RenderStages.h"

namespace ho
{
IRenderPass::IRenderPass()
{
    mpStagePipeline[static_cast<int32_t>(eRenderStageType::Opaque)] = new OpaqueRenderStage();
    mpStagePipeline[static_cast<int32_t>(eRenderStageType::Masked)] = new MaskedRenderStage();
    mpStagePipeline[static_cast<int32_t>(eRenderStageType::Environment)] = new EnvironmentRenderStage();
    mpStagePipeline[static_cast<int32_t>(eRenderStageType::InWorldDebug)] = new InWorldDebugRenderStage();
    mpStagePipeline[static_cast<int32_t>(eRenderStageType::Blended)] = new BlendedRenderStage();
    mpStagePipeline[static_cast<int32_t>(eRenderStageType::Outline)] = new OutlineRenderStage();
    mpStagePipeline[static_cast<int32_t>(eRenderStageType::PostProcessing)] = new PostProcessingStage();
    mpStagePipeline[static_cast<int32_t>(eRenderStageType::WorldOverlayDebug)] = new WorldOverlayDebugRenderStage();

    for (const IRenderStage* pStage : mpStagePipeline)
    {
        (void)pStage;
        HO_ASSERT(pStage, "There is null pipeline. every stage instances must be created in constructor.");
    }
}

IRenderPass::IRenderPass(IRenderingBackend* pBackend, RenderingBuiltInAssets* pBuiltInAssets)
  : mType(eRenderPassType::None)
  , mpBuiltInAssets(pBuiltInAssets)
  , mpBackend(pBackend)
{
    HO_ASSERT(mpBackend, "Backend is null.");

    mpStagePipeline[static_cast<int32_t>(eRenderStageType::Opaque)] = new OpaqueRenderStage(mpBackend, mpBuiltInAssets);
    mpStagePipeline[static_cast<int32_t>(eRenderStageType::Masked)] = new MaskedRenderStage(mpBackend, mpBuiltInAssets);
    mpStagePipeline[static_cast<int32_t>(eRenderStageType::Environment)] =
        new EnvironmentRenderStage(mpBackend, mpBuiltInAssets);
    mpStagePipeline[static_cast<int32_t>(eRenderStageType::InWorldDebug)] =
        new InWorldDebugRenderStage(mpBackend, mpBuiltInAssets);
    mpStagePipeline[static_cast<int32_t>(eRenderStageType::Blended)] =
        new BlendedRenderStage(mpBackend, mpBuiltInAssets);
    mpStagePipeline[static_cast<int32_t>(eRenderStageType::Outline)] =
        new OutlineRenderStage(mpBackend, mpBuiltInAssets);
    mpStagePipeline[static_cast<int32_t>(eRenderStageType::PostProcessing)] =
        new PostProcessingStage(mpBackend, mpBuiltInAssets);
    mpStagePipeline[static_cast<int32_t>(eRenderStageType::WorldOverlayDebug)] =
        new WorldOverlayDebugRenderStage(mpBackend, mpBuiltInAssets);

    for (const IRenderStage* pStage : mpStagePipeline)
    {
        (void)pStage;
        HO_ASSERT(pStage, "There is null pipeline. every stage instances must be created in constructor.");
    }
}

IRenderPass::IRenderPass(const IRenderPass& other)
  : mType(other.mType)
  , mbIsBound(false)
  , mbSetup(false)
  , mbPrepared(false)
  , mpBuiltInAssets(other.mpBuiltInAssets)
  , mpBackend(other.mpBackend)
{
    HO_ASSERT(other.mpBackend, "Source render pass has invalid backend.");
    HO_ASSERT(!other.mbIsBound, "Source render pass must not be bound for copying.");
    HO_ASSERT(!other.mbSetup && !other.mbPrepared, "Source render pass must not be setup or prepared for copying.");

    mpStagePipeline[static_cast<int32_t>(eRenderStageType::Opaque)] = new OpaqueRenderStage(mpBackend, mpBuiltInAssets);
    mpStagePipeline[static_cast<int32_t>(eRenderStageType::Masked)] = new MaskedRenderStage(mpBackend, mpBuiltInAssets);
    mpStagePipeline[static_cast<int32_t>(eRenderStageType::Environment)] =
        new EnvironmentRenderStage(mpBackend, mpBuiltInAssets);
    mpStagePipeline[static_cast<int32_t>(eRenderStageType::InWorldDebug)] =
        new InWorldDebugRenderStage(mpBackend, mpBuiltInAssets);
    mpStagePipeline[static_cast<int32_t>(eRenderStageType::Blended)] =
        new BlendedRenderStage(mpBackend, mpBuiltInAssets);
    mpStagePipeline[static_cast<int32_t>(eRenderStageType::Outline)] =
        new OutlineRenderStage(mpBackend, mpBuiltInAssets);
    mpStagePipeline[static_cast<int32_t>(eRenderStageType::PostProcessing)] =
        new PostProcessingStage(mpBackend, mpBuiltInAssets);
    mpStagePipeline[static_cast<int32_t>(eRenderStageType::WorldOverlayDebug)] =
        new WorldOverlayDebugRenderStage(mpBackend, mpBuiltInAssets);

    for (const IRenderStage* pStage : mpStagePipeline)
    {
        (void)pStage;
        HO_ASSERT(pStage, "There is null pipeline. every stage instances must be created in constructor.");
    }
}

IRenderPass& IRenderPass::operator=(const IRenderPass& rhs)
{
    if (this == &rhs)
    {
        return *this;
    }

    HO_ASSERT(rhs.mpBackend, "RHS render pass has invalid backend.");
    HO_ASSERT(!rhs.mbIsBound, "RHS render pass must not be bound for copying.");
    HO_ASSERT(!rhs.mbSetup && !rhs.mbPrepared, "RHS render pass must not be setup or prepared for copying.");

    HO_ASSERT(!mbIsBound, "LHS render pass must not be bound for copying.");
    HO_ASSERT(!mbSetup && !mbPrepared, "LHS render pass must not be setup or prepared for copying.");

    mType = rhs.mType;
    mbIsBound = false;
    mbSetup = false;
    mbPrepared = false;
    mpBuiltInAssets = rhs.mpBuiltInAssets;
    mpBackend = rhs.mpBackend;

    for (const IRenderStage* pStage : mpStagePipeline)
    {
        delete pStage;
    }

    mpStagePipeline[static_cast<int32_t>(eRenderStageType::Opaque)] = new OpaqueRenderStage(mpBackend, mpBuiltInAssets);
    mpStagePipeline[static_cast<int32_t>(eRenderStageType::Masked)] = new MaskedRenderStage(mpBackend, mpBuiltInAssets);
    mpStagePipeline[static_cast<int32_t>(eRenderStageType::Environment)] =
        new EnvironmentRenderStage(mpBackend, mpBuiltInAssets);
    mpStagePipeline[static_cast<int32_t>(eRenderStageType::InWorldDebug)] =
        new InWorldDebugRenderStage(mpBackend, mpBuiltInAssets);
    mpStagePipeline[static_cast<int32_t>(eRenderStageType::Blended)] =
        new BlendedRenderStage(mpBackend, mpBuiltInAssets);
    mpStagePipeline[static_cast<int32_t>(eRenderStageType::Outline)] =
        new OutlineRenderStage(mpBackend, mpBuiltInAssets);
    mpStagePipeline[static_cast<int32_t>(eRenderStageType::PostProcessing)] =
        new PostProcessingStage(mpBackend, mpBuiltInAssets);
    mpStagePipeline[static_cast<int32_t>(eRenderStageType::WorldOverlayDebug)] =
        new WorldOverlayDebugRenderStage(mpBackend, mpBuiltInAssets);

    for (const IRenderStage* pStage : mpStagePipeline)
    {
        (void)pStage;
        HO_ASSERT(pStage, "There is null pipeline. every stage instances must be created in constructor.");
    }

    return *this;
}

IRenderPass::~IRenderPass()
{
    for (const IRenderStage* pStage : mpStagePipeline)
    {
        delete pStage;
    }
}

eRenderPassType IRenderPass::GetType() const
{
    return mType;
}

void IRenderPass::Setup(const Vector3& cameraWorldPos)
{
    HO_ASSERT(!mbSetup, "Pass has already been set up.");
    HO_ASSERT(!mbPrepared, "Cannot set up a pass while it is in a prepared state.");

    mbSetup = true;
    mCameraWorldPos = cameraWorldPos;
}

void IRenderPass::PrepareExecute(const RenderResourceContext& resourceContext)
{
    HO_ASSERT(mbSetup, "Pass must be set up before preparing execution.");
    HO_ASSERT(!mbPrepared, "Pass is already prepared for execution.");

    mbPrepared = true;

    mpBackend->BeginDrawSequence();
}

void IRenderPass::Execute(const RenderResourceContext& resourceContext)
{
    HO_ASSERT(mbSetup && mbPrepared, "Pass must be setup and prepared before execute.");

    for (auto& pStage : mpStagePipeline)
    {
        if (pStage->IsAttached())
        {
            pStage->PrepareExecute(resourceContext);
            pStage->Execute(resourceContext);
            pStage->FinishExecute(resourceContext);
        }
    }
}

void IRenderPass::FinishExecute(const RenderResourceContext& resourceContext)
{
    (void)resourceContext;
    HO_ASSERT(mbPrepared, "Cannot finish execution on a pass that is not prepared.");
    mbPrepared = false;

    mpBackend->EndDrawSequence();
}

void IRenderPass::TearDown()
{
    HO_ASSERT(mbSetup, "Cannot tear down a pass that has not been set up.");
    HO_ASSERT(!mbPrepared, "Cannot tear down a pass while execution is still prepared.");

    mbSetup = false;

    for (auto& pStage : mpStagePipeline)
    {
        if (pStage->IsAttached())
        {
            pStage->TearDown();
        }
    }
}

void IRenderPass::SetBound(bool bIsBound)
{
    mbIsBound = bIsBound;
}

bool IRenderPass::IsBound() const
{
    return mbIsBound;
}

bool IRenderPass::IsSetup() const
{
    return mbSetup;
}

bool IRenderPass::IsPrepared() const
{
    return mbPrepared;
}

void IRenderPass::AttachStage(eRenderStageType stageType)
{
    HO_ASSERT(mpStagePipeline[static_cast<int32_t>(stageType)],
              "Stage is not created. Render pass is created incompletely.");
    if (mpStagePipeline[static_cast<int32_t>(stageType)]->IsAttached())
    {
        return;
    }

    mpStagePipeline[static_cast<int32_t>(stageType)]->OnAttached(*this);
}

bool IRenderPass::IsStageAttached(eRenderStageType stageType) const
{
    HO_ASSERT(mpStagePipeline[static_cast<int32_t>(stageType)],
              "Stage is not created. Render pass is created incompletely.");
    return mpStagePipeline[static_cast<int32_t>(stageType)]->IsAttached();
}

IRenderStage* IRenderPass::GetStage(eRenderStageType stage) const
{
    HO_ASSERT(mpStagePipeline[static_cast<int32_t>(stage)],
              "Stage is not created. Render pass is created incompletely.");
    return mpStagePipeline[static_cast<int32_t>(stage)];
}
} // namespace ho