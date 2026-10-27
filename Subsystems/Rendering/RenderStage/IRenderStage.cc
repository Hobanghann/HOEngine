#include "IRenderStage.h"

#include "../RenderBackend/RenderResourceContext.h"
#include "../RenderData/DrawCommand.h"

namespace ho
{
IRenderStage::IRenderStage() = default;

IRenderStage::IRenderStage(IRenderingBackend* pBackend, RenderingBuiltInAssets* pBuiltInAssets)
  : mType(eRenderStageType::Last)
  , mpBuiltInAssets(pBuiltInAssets)
  , mpBackend(pBackend)
{
    HO_ASSERT(mpBackend, "Backend is null.");
}

IRenderStage::IRenderStage(const IRenderStage& other)
  : mType(other.mType)
  , mbIsAttached(false)
  , mbSetup(false)
  , mbPrepared(false)
  , mDrawCommandCount(0)
  , mpDrawCommandsBegin(nullptr)
  , mpBuiltInAssets(other.mpBuiltInAssets)
  , mpBackend(other.mpBackend)
{
    HO_ASSERT(other.mpBackend, "Source render stage has invalid backend.");
    HO_ASSERT(!other.mbIsAttached, "Source render stage must not be attached for copying.");
    HO_ASSERT(!other.mbSetup && !other.mbPrepared, "Source render stage must not be setup or prepared for copying.");
    HO_ASSERT(other.mDrawCommandCount == 0, "Source render stabe must be empty.");
}

IRenderStage& IRenderStage::operator=(const IRenderStage& rhs)
{
    if (this == &rhs)
    {
        return *this;
    }

    HO_ASSERT(rhs.mpBackend, "RHS render stage has invalid backend.");
    HO_ASSERT(!rhs.mbIsAttached, "RHS render stage must not be attached for copying.");
    HO_ASSERT(!rhs.mbSetup && !rhs.mbPrepared, "RHS render stage must not be setup or prepared for copying.");
    HO_ASSERT(rhs.mDrawCommandCount == 0, "RHS render stabe must be empty.");

    HO_ASSERT(!mbIsAttached, "LHS render stage must not be attached for copying.");
    HO_ASSERT(!mbSetup && !mbPrepared, "LHS render stage must not be setup or prepared for copying.");
    HO_ASSERT(mDrawCommandCount == 0, "LHS render stabe must be empty.");

    mType = rhs.mType;
    mbIsAttached = false;
    mbSetup = false;
    mbPrepared = false;
    mDrawCommandCount = 0;
    mpDrawCommandsBegin = nullptr;
    mpBuiltInAssets = rhs.mpBuiltInAssets;
    mpBackend = rhs.mpBackend;

    return *this;
}

IRenderStage::~IRenderStage() = default;

eRenderStageType IRenderStage::GetType() const
{
    return mType;
}

void IRenderStage::Setup()
{
    HO_ASSERT(!mbSetup, "Stage has already been set up.");
    HO_ASSERT(!mbPrepared, "Cannot set up a stage while it is in a prepared state.");

    mbSetup = true;
}

void IRenderStage::PrepareExecute(const RenderResourceContext& resourceContext)
{
    (void)resourceContext;
    HO_ASSERT(mbIsAttached, "Stage must be attached before preparing execution.");
    HO_ASSERT(mbSetup, "Stage must be set up before preparing execution.");
    HO_ASSERT(!mbPrepared, "Stage is already prepared for execution.");
    mbPrepared = true;

    sortDrawCommands();
}

void IRenderStage::FinishExecute(const RenderResourceContext& resourceContext)
{
    (void)resourceContext;
    HO_ASSERT(mbPrepared, "Cannot finish execution on a stage that is not prepared.");
    mbPrepared = false;
}

void IRenderStage::TearDown()
{
    HO_ASSERT(mbSetup, "Cannot tear down a stage that has not been set up.");
    HO_ASSERT(!mbPrepared, "Cannot tear down a stage while execution is still prepared.");

    mbSetup = false;
    mbIsAttached = false;
    mpDrawCommandsBegin = nullptr;
    mDrawCommandCount = 0;
}

void IRenderStage::OnAttached(const IRenderPass& owner)
{
    mbIsAttached = true;
}

bool IRenderStage::IsAttached() const
{
    return mbIsAttached;
}

bool IRenderStage::IsSetup() const
{
    return mbSetup;
}

bool IRenderStage::IsPrepared() const
{
    return mbPrepared;
}

void IRenderStage::SetDrawCommands(DrawCommand* pDrawCommandsBegin, int32_t drawCommandCount)
{
    HO_ASSERT(pDrawCommandsBegin, "It can't be null.");
    mpDrawCommandsBegin = pDrawCommandsBegin;
    mDrawCommandCount = drawCommandCount;
}

bool IRenderStage::isExecutable() const
{
    return mbSetup && mbPrepared;
}
} // namespace ho
