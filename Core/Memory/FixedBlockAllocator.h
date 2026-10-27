#pragma once

#include <cstdint>
#include <memory>
#include <vector>

namespace ho
{
class FixedBlockAllocator final
{
  public:
    FixedBlockAllocator(int32_t blockSize, int32_t maxBlockCount, int32_t alignment);
    FixedBlockAllocator(const FixedBlockAllocator&) = delete;
    FixedBlockAllocator& operator=(const FixedBlockAllocator&) = delete;

    FixedBlockAllocator(FixedBlockAllocator&& other) noexcept;
    FixedBlockAllocator& operator=(FixedBlockAllocator&& rhs) noexcept;

    ~FixedBlockAllocator();

    void* Allocate();
    void Free(void* pBlock);

    [[nodiscard]] void* GetAllocatedBlock(uint32_t index) const;
    [[nodiscard]] uint32_t GetAllocatedBlockIndex(void* pBlock) const;

    [[nodiscard]] int32_t GetBlockSize() const;

    [[nodiscard]] int32_t GetAllocatedBlockCount() const;

    [[nodiscard]] int32_t GetMaxBlockCount() const;

    [[nodiscard]] bool IsAllocated(uint32_t index) const;

    [[nodiscard]] bool IsEmpty() const;

    [[nodiscard]] bool IsFull() const;

    void CopyStateFrom(const FixedBlockAllocator& other);

  private:
    int32_t mBlockSize;
    int32_t mAlignment;
    int32_t mMaxBlockCount;
    int32_t mAllocatedBlockCount;

    void* mpMemoryPool = nullptr;
    uint8_t* mpMemoryBegin = nullptr;
    uint8_t* mpMemoryEnd = nullptr;
    std::vector<uint8_t*> mFreeAddress;
    std::vector<bool> mbIsAllocated;
};
} // namespace ho