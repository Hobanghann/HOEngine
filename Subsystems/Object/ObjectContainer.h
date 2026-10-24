#pragma once

#include "ObjectIterator.h"

namespace ho
{
template <typename T>
class ObjectContainer final
{
    friend class ObjectManager;

  public:
    // NOLINTBEGIN(readability-identifier-naming)
    ObjectIterator<T> begin() const
    {
        return ObjectIterator<T>(mpRootType, mpObjectPools, mbExactOnly);
    }

    ObjectIterator<T> end() const
    {
        return ObjectIterator<T>();
    }

    ObjectIterator<const T> cbegin() const
    {
        return ObjectIterator<const T>(mpRootType, mpObjectPools, mbExactOnly);
    }

    ObjectIterator<const T> cend() const
    {
        return ObjectIterator<const T>();
    }

    // NOLINTEND(readability-identifier-naming)

  private:
    ObjectContainer(const Class* pRootType, const FixedArray<ObjectPool*>* pObjectPools, bool bExactOnly)
      : mbExactOnly(bExactOnly)
      , mpRootType(pRootType)
      , mpObjectPools(pObjectPools)
    {
        HO_ASSERT(mpRootType && mpObjectPools, "It can't be null.");
    }

    bool mbExactOnly = false;
    const Class* mpRootType = nullptr;
    const FixedArray<ObjectPool*>* mpObjectPools = nullptr;
};
} // namespace ho