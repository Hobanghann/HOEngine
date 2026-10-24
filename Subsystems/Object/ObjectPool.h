#pragma once

#include <memory>

#include "Core/Memory/FixedBlockAllocator.h"
#include "Core/Templates/FixedArray.h"

namespace ho
{
class Class;
class RObject;

class ObjectPool final
{
  public:
    ObjectPool(const Class* pObjectType, int32_t size);
    ~ObjectPool();

    ObjectPool(const ObjectPool&) = delete;
    ObjectPool& operator=(const ObjectPool&) = delete;

    [[nodiscard]] uint64_t New();

    void Delete(uint64_t id);

    [[nodiscard]] RObject* Get(uint64_t id);

    [[nodiscard]] bool Has(uint64_t id) const;

    [[nodiscard]] uint32_t GetGeneration(uint32_t poolIndex) const;

    void Clear();

    const Class& GetObjectType() const;

    int32_t GetSize() const;

    int32_t GetFullSize() const;

    bool IsEmpty() const;

    void Expand();

    void Expand(int32_t newSize);

  private:
    const Class* mpObjectType;
    FixedArray<uint32_t> mGen; // generation trancking value per 'Block index'
    FixedBlockAllocator mAllocator;
    bool mbNeedToExpand = false;
};
} // namespace ho