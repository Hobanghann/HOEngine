#include "ObjectPool.h"

#include "Object.h"
#include "ObjectSystem.h"
#include "Subsystems/Reflection/Class.h"

namespace ho
{
ObjectPool::ObjectPool(const Class* pObjectType, int32_t size)
  : mpObjectType(pObjectType)
  , mGen(size)
  , mAllocator(pObjectType->GetSize(), size, pObjectType->GetAlignment())
{
    std::fill_n(mGen.Data(), mGen.GetSize(), 0);
}

ObjectPool::~ObjectPool()
{
    Clear();
}

uint64_t ObjectPool::New()
{
    void* pNewDst = mAllocator.Allocate();
    if (!pNewDst)
    {
        HO_ASSERT(false, "Allocator is full. Check Allocator size.");
        return 0;
    }

    RObject* pNewObj = static_cast<RObject*>(mpObjectType->NewAt(pNewDst));

    const uint32_t blockIndex = mAllocator.GetAllocatedBlockIndex(pNewObj);
    const uint32_t gen = mGen[static_cast<int32_t>(blockIndex)];
    const uint32_t poolIndex = blockIndex + 1;

    if (static_cast<float>(mAllocator.GetAllocatedBlockCount()) / static_cast<float>(mAllocator.GetMaxBlockCount()) >
        0.75f)
    {
        mbNeedToExpand = true;
    }

    const uint64_t newObjID = (static_cast<uint64_t>(gen) << 32) | poolIndex;

    const ObjectHandle<RObject> hNewObj(mpObjectType, newObjID, this);
    ObjectSystem::GetInstance().registerObject(hNewObj);

    return newObjID;
}

void ObjectPool::Delete(uint64_t id)
{
    if (id == 0)
    {
        return;
    }

    HO_ASSERT(Has(id), "There is no object in allocator matched with ID.");
    const uint32_t poolIndex = static_cast<uint32_t>(id);
    const uint32_t allocIndex = poolIndex - 1;
    RObject* pDeleteObj = static_cast<RObject*>(mAllocator.GetAllocatedBlock(allocIndex));

    ObjectSystem::GetInstance().unregisterObject(pDeleteObj->GetGlobalID());
    mpObjectType->Destroy(pDeleteObj);

    mAllocator.Free(pDeleteObj);
    ++mGen[static_cast<int32_t>(allocIndex)];

    if (static_cast<float>(mAllocator.GetAllocatedBlockCount()) / static_cast<float>(mAllocator.GetMaxBlockCount()) <=
        0.75f)
    {
        mbNeedToExpand = false;
    }
}

RObject* ObjectPool::Get(uint64_t id)
{
    if (!Has(id))
    {
        return nullptr;
    }

    const uint32_t index = static_cast<uint32_t>(id);
    const uint32_t allocIndex = index - 1;
    return static_cast<RObject*>(mAllocator.GetAllocatedBlock(allocIndex));
}

bool ObjectPool::Has(uint64_t id) const
{
    const uint32_t gen = static_cast<uint32_t>(id >> 32);
    const uint32_t poolIndex = static_cast<uint32_t>(id);
    const uint32_t allocIndex = poolIndex - 1;

    if (poolIndex == 0 || poolIndex > static_cast<uint32_t>(mAllocator.GetMaxBlockCount()) ||
        mGen[static_cast<int32_t>(allocIndex)] != gen)
    {
        return false;
    }

    return mAllocator.IsAllocated(allocIndex);
}

uint32_t ObjectPool::GetGeneration(uint32_t poolIndex) const
{
    if (poolIndex == 0 || poolIndex > static_cast<uint32_t>(mAllocator.GetMaxBlockCount()))
    {
        return 0;
    }

    const uint32_t allocIndex = poolIndex - 1;
    return mGen[static_cast<int32_t>(allocIndex)];
}

void ObjectPool::Clear()
{
    const uint32_t maxBlockCount = static_cast<uint32_t>(mAllocator.GetMaxBlockCount());
    for (uint32_t allocIndex = 0; allocIndex < maxBlockCount; ++allocIndex)
    {
        if (mAllocator.IsAllocated(allocIndex))
        {
            const uint32_t poolIndex = allocIndex + 1;
            const uint64_t id = (static_cast<uint64_t>(mGen[static_cast<int32_t>(allocIndex)]) << 32) | poolIndex;
            Delete(id);
        }
    }
}

const Class& ObjectPool::GetObjectType() const
{
    return *mpObjectType;
}

int32_t ObjectPool::GetSize() const
{
    return mAllocator.GetAllocatedBlockCount();
}

int32_t ObjectPool::GetFullSize() const
{
    return mAllocator.GetMaxBlockCount();
}

bool ObjectPool::IsEmpty() const
{
    return mAllocator.IsEmpty();
}

void ObjectPool::Expand()
{
    if (!mbNeedToExpand)
    {
        return;
    }
    FixedBlockAllocator newAllocator(
        mpObjectType->GetSize(), mAllocator.GetMaxBlockCount() * 2, mpObjectType->GetAlignment());
    newAllocator.CopyStateFrom(mAllocator);
    mAllocator = std::move(newAllocator);

    FixedArray<uint32_t> newGen(mAllocator.GetMaxBlockCount() * 2);
    for (int32_t i = 0; i < mGen.GetSize(); ++i)
    {
        newGen[i] = mGen[i];
    }
    mGen = std::move(newGen);

    mbNeedToExpand = false;
}

void ObjectPool::Expand(int32_t newSize)
{
    if (newSize < mAllocator.GetMaxBlockCount())
    {
        return;
    }

    FixedBlockAllocator newAllocator(mpObjectType->GetSize(), newSize, mpObjectType->GetAlignment());
    newAllocator.CopyStateFrom(mAllocator);
    mAllocator = std::move(newAllocator);

    FixedArray<uint32_t> newGen(newSize);
    for (int32_t i = 0; i < mGen.GetSize(); ++i)
    {
        newGen[i] = mGen[i];
    }
    mGen = std::move(newGen);

    mbNeedToExpand = false;
}
} // namespace ho