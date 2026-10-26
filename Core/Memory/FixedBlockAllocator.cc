#include "FixedBlockAllocator.h"

#include <cstdlib>

#include "Macros.h"

namespace ho
{
FixedBlockAllocator::FixedBlockAllocator(int32_t blockSize, int32_t maxBlockCount, int32_t alignment)
  : mBlockSize(HO_ALIGN_CEIL(blockSize, alignment))
  , mAlignment(alignment)
  , mMaxBlockCount(maxBlockCount)
  , mAllocatedBlockCount(0)
{
    HO_ASSERT(mBlockSize > 0 && mAlignment > 0, "Must be greater than zero.");
    mpMemoryPool = malloc(mBlockSize * mMaxBlockCount + mAlignment);
    // NOLINTBEGIN(performance-no-int-to-ptr)
    HO_ASSERT(mpMemoryPool, "Failed to allocate memory.");
    mpMemoryBegin = reinterpret_cast<uint8_t*>(HO_ALIGN_CEIL(reinterpret_cast<uintptr_t>(mpMemoryPool), mAlignment));
    // NOLINTEND(performance-no-int-to-ptr)
    mpMemoryEnd = mpMemoryBegin + mBlockSize * mMaxBlockCount;
    mFreeAddress.reserve(mMaxBlockCount);
    mbIsAllocated.resize(mMaxBlockCount);
}

FixedBlockAllocator::FixedBlockAllocator(FixedBlockAllocator&& other) noexcept
  : mBlockSize(other.mBlockSize)
  , mAlignment(other.mAlignment)
  , mMaxBlockCount(other.mMaxBlockCount)
  , mAllocatedBlockCount(other.mAllocatedBlockCount)
  , mpMemoryPool(other.mpMemoryPool)
  , mpMemoryBegin(other.mpMemoryBegin)
  , mpMemoryEnd(other.mpMemoryEnd)
  , mFreeAddress(std::move(other.mFreeAddress))
  , mbIsAllocated(std::move(other.mbIsAllocated))
{
    other.mpMemoryPool = nullptr;
    other.mpMemoryBegin = nullptr;
    other.mpMemoryEnd = nullptr;
    other.mBlockSize = 0;
    other.mAlignment = 0;
    other.mMaxBlockCount = 0;
    other.mAllocatedBlockCount = 0;
}

FixedBlockAllocator& FixedBlockAllocator::operator=(FixedBlockAllocator&& rhs) noexcept
{
    if (this == &rhs)
    {
        return *this;
    }

    free(mpMemoryPool);

    mBlockSize = rhs.mBlockSize;
    mAlignment = rhs.mAlignment;
    mMaxBlockCount = rhs.mMaxBlockCount;
    mAllocatedBlockCount = rhs.mAllocatedBlockCount;

    mpMemoryPool = rhs.mpMemoryPool;
    mpMemoryBegin = rhs.mpMemoryBegin;
    mpMemoryEnd = rhs.mpMemoryEnd;

    mFreeAddress = std::move(rhs.mFreeAddress);
    mbIsAllocated = std::move(rhs.mbIsAllocated);

    rhs.mpMemoryPool = nullptr;
    rhs.mpMemoryBegin = nullptr;
    rhs.mpMemoryEnd = nullptr;
    rhs.mBlockSize = 0;
    rhs.mAlignment = 0;
    rhs.mMaxBlockCount = 0;
    rhs.mAllocatedBlockCount = 0;

    return *this;
}

FixedBlockAllocator::~FixedBlockAllocator()
{
    free(mpMemoryPool);
}

void* FixedBlockAllocator::Allocate()
{
    if (IsFull())
    {
        return nullptr;
    }

    uint8_t* pBlock = nullptr; // NOLINT(misc-const-correctness)

    if (!mFreeAddress.empty())
    {
        pBlock = mFreeAddress.back();
        mFreeAddress.pop_back();
    }
    else
    {
        pBlock = mpMemoryBegin + mAllocatedBlockCount * mBlockSize;
    }

    const int32_t blockIdx = static_cast<int32_t>(
        (reinterpret_cast<uintptr_t>(pBlock) - reinterpret_cast<uintptr_t>(mpMemoryBegin)) / mBlockSize);
    HO_ASSERT(!mbIsAllocated[blockIdx], "This block already used.");
    mbIsAllocated[blockIdx] = true;
    ++mAllocatedBlockCount;
    return static_cast<void*>(pBlock);
}

void FixedBlockAllocator::Free(void* pBlock)
{
    if (!pBlock)
    {
        return;
    }

    uint8_t* pBlockByte = static_cast<uint8_t*>(pBlock); // NOLINT(misc-const-correctness)

    if (pBlockByte < mpMemoryBegin || pBlockByte >= mpMemoryEnd)
    {
        return;
    }

    const uintptr_t blockOffset = pBlockByte - mpMemoryBegin;
    if (blockOffset % mBlockSize != 0)
    {
        return;
    }

    const int32_t blockIdx = static_cast<int32_t>(blockOffset / mBlockSize);
    if (!mbIsAllocated[blockIdx])
    {
        return;
    }

    mFreeAddress.push_back(pBlockByte);
    mbIsAllocated[blockIdx] = false;
    --mAllocatedBlockCount;
}

void* FixedBlockAllocator::GetAllocatedBlock(uint32_t index) const
{
    HO_ASSERT(index < static_cast<uint32_t>(mMaxBlockCount), "Index out of bounds.");
    HO_ASSERT(mbIsAllocated[index], "Attempted to access an unallocated memory block.");

    return mpMemoryBegin + (index * mBlockSize);
}

uint32_t FixedBlockAllocator::GetAllocatedBlockIndex(void* pBlock) const
{
    HO_ASSERT(pBlock != nullptr, "Pointer is null.");
    HO_ASSERT(pBlock >= mpMemoryBegin && pBlock < mpMemoryEnd, "Pointer is out of allocator range.");

    const auto offset = static_cast<uint8_t*>(pBlock) - mpMemoryBegin;
    HO_ASSERT(offset % mBlockSize == 0, "Pointer is not aligned to the start of a block.");

    const uint32_t index = static_cast<uint32_t>(offset / mBlockSize);
    HO_ASSERT(mbIsAllocated[index], "Attempted to get index from an unallocated memory block.");

    return index;
}

int32_t FixedBlockAllocator::GetBlockSize() const
{
    return mBlockSize;
}

int32_t FixedBlockAllocator::GetAllocatedBlockCount() const
{
    return mAllocatedBlockCount;
}

int32_t FixedBlockAllocator::GetMaxBlockCount() const
{
    return mMaxBlockCount;
}

bool FixedBlockAllocator::IsAllocated(uint32_t index) const
{
    HO_ASSERT(index < static_cast<uint32_t>(mMaxBlockCount), "Index out of bounds.");
    return mbIsAllocated[index];
}

bool FixedBlockAllocator::IsEmpty() const
{
    return mAllocatedBlockCount == 0;
}

bool FixedBlockAllocator::IsFull() const
{
    return mAllocatedBlockCount == mMaxBlockCount;
}

void FixedBlockAllocator::CopyStateFrom(const FixedBlockAllocator& other)
{
    if (mBlockSize != other.mBlockSize || mMaxBlockCount < other.mMaxBlockCount || mAlignment != other.mAlignment)
    {
        HO_ASSERT(false,
                  "To copy state, both allocators must have the same block size and alignment, "
                  "and the destination allocator must have sufficient capacity to preserve block indices.");
        return;
    }

    std::memcpy(mpMemoryBegin, other.mpMemoryBegin, static_cast<size_t>(other.mBlockSize) * other.mMaxBlockCount);

    mAllocatedBlockCount = other.mAllocatedBlockCount;
    mFreeAddress.clear();
    mFreeAddress.reserve(mMaxBlockCount);
    mbIsAllocated.assign(mMaxBlockCount, false);

    for (int32_t i = 0; i < other.mMaxBlockCount; ++i)
    {
        mbIsAllocated[i] = other.mbIsAllocated[i];

        if (!mbIsAllocated[i])
        {
            mFreeAddress.push_back(mpMemoryBegin + i * mBlockSize);
        }
    }

    for (int32_t i = other.mMaxBlockCount; i < mMaxBlockCount; ++i)
    {
        mFreeAddress.push_back(mpMemoryBegin + i * mBlockSize);
    }
}
} // namespace ho