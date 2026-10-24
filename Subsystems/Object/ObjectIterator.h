#pragma once

#include "Core/Templates/FixedArray.h"
#include "ObjectHandle.h"
#include "Subsystems/Reflection/Class.h"

namespace ho
{

template <typename T>
class ObjectIterator final
{
  public:
    template <typename U>
    friend class ObjectContainer;

    ObjectHandle<T> operator*() const
    {
        if (mpCurrType && mpObjectPools)
        {
            ObjectPool* pPool = (*mpObjectPools)[static_cast<int32_t>(mpCurrType->GetTypeID())];
            if (pPool)
            {
                uint64_t id = makeID(pPool, mCurrPoolIndex);
                return ObjectHandle<T>(id, pPool);
            }
        }
        return ObjectHandle<T>::sNULL;
    }

    ObjectIterator& operator++()
    {
        advance();
        return *this;
    }

    ObjectIterator operator++(int)
    {
        ObjectIterator temp = *this;
        advance();
        return temp;
    }

    bool operator==(const ObjectIterator& rhs) const
    {
        if (mpCurrType == nullptr && rhs.mpCurrType == nullptr)
        {
            return true;
        }

        return mbExactOnly == rhs.mbExactOnly && mpRootType == rhs.mpRootType && mpCurrType == rhs.mpCurrType &&
               mCurrPoolIndex == rhs.mCurrPoolIndex && mpObjectPools == rhs.mpObjectPools;
    }

    bool operator!=(const ObjectIterator& rhs) const
    {
        return !(*this == rhs);
    }

  private:
    ObjectIterator(const Class* pRootType, const FixedArray<ObjectPool*>* pObjectPools, bool bExactOnly)
      : mbExactOnly(bExactOnly)
      , mpRootType(pRootType)
      , mpCurrType(pRootType)
      , mCurrPoolIndex(0)
      , mpObjectPools(pObjectPools)
    {
        HO_ASSERT(mpRootType && mpObjectPools, "It can't be null.");
        advance();
    }

    ObjectIterator() = default;

    static uint64_t makeID(const ObjectPool* pPool, uint32_t poolIndex)
    {
        if (pPool == nullptr || poolIndex == 0)
        {
            return 0;
        }
        uint32_t gen = pPool->GetGeneration(poolIndex);
        return (static_cast<uint64_t>(gen) << 32) | static_cast<uint64_t>(poolIndex);
    }

    void advance()
    {
        while (mpCurrType != nullptr)
        {
            ObjectPool* pPool = (*mpObjectPools)[static_cast<int32_t>(mpCurrType->GetTypeID())];

            if (pPool != nullptr)
            {
                while (mCurrPoolIndex < static_cast<uint32_t>(pPool->GetFullSize()))
                {
                    ++mCurrPoolIndex;
                    uint64_t id = makeID(pPool, mCurrPoolIndex);
                    if (pPool->Has(id))
                    {
                        return;
                    }
                }
            }

            if (mbExactOnly)
            {
                mpCurrType = nullptr;
                return;
            }

            mpCurrType = getNextTypeInSubtree(mpCurrType, mpRootType);
            mCurrPoolIndex = 0;
        }
    }

    const Class* getNextTypeInSubtree(const Class* pCurrent, const Class* pRoot) const
    {
        if (pCurrent == nullptr)
        {
            return nullptr;
        }

        if (pCurrent->GetFirstChild() != nullptr)
        {
            return pCurrent->GetFirstChild();
        }

        const Class* pNode = pCurrent;
        while (pNode != nullptr && pNode != pRoot)
        {
            if (pNode->GetNextSibling() != nullptr)
            {
                return pNode->GetNextSibling();
            }
            pNode = pNode->GetParent();
        }

        return nullptr;
    }

    bool mbExactOnly = false;
    const Class* mpRootType = nullptr;
    const Class* mpCurrType = nullptr;
    uint32_t mCurrPoolIndex = 0;
    const FixedArray<ObjectPool*>* mpObjectPools = nullptr;
};

} // namespace ho