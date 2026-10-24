#pragma once

#include "Core/String/StringHandle.h"
#include "ReflectionFlags.h"
#include "TypeRegistry.h"
#include "TypeRegistryPool.h"

namespace ho
{
class Type;

class PropertyValue final
{
    friend class Property;

  public:
    template <typename TProp>
    operator TProp() const
    {
        HO_ASSERT(sizeof(TProp) == mSize, "Property size mismatch.");
        return *reinterpret_cast<const TProp*>(mpValue);
    }

    template <typename TProp>
    TProp Get() const
    {
        HO_ASSERT(sizeof(TProp) == mSize, "Property size mismatch.");
        return *reinterpret_cast<const TProp*>(mpValue);
    }

    template <typename TProp>
    const TProp& GetAsRef() const
    {
        HO_ASSERT(sizeof(TProp) == mSize, "Property size mismatch.");
        return *reinterpret_cast<const TProp*>(mpValue);
    }

  private:
    PropertyValue(int32_t size, const void* pValue)
      : mSize(size)
      , mpValue(pValue)
    {
        HO_ASSERT(size > 0, "Property size must be greater than zero");
        HO_ASSERT(pValue != nullptr, "Pointer to value can't be null.");
    }

    int32_t mSize;
    const void* mpValue = nullptr;
};

class Property final
{
    friend class Struct;
    template <typename TStruct>
    friend class StructRegister;

    friend class Class;
    template <typename TClass, typename TParent>
    friend class ClassRegister;

  public:
    Property() = default;
    ~Property() = default;

    const TypeRegistry* GetType() const
    {
        return mpType;
    }

    StringHandle GetName() const
    {
        return mhName;
    }

    template <typename TOwner>
    PropertyValue GetValue(const TOwner* pInstance) const
    {
        HO_ASSERT(pInstance != nullptr, "Instance cannot be null.");
        HO_ASSERT(mpOwnerType->GetTypeKind() == eTypeKind::Struct
                      ? TypeRegistryPool::GetInstance().GetRegisteredTypeRegistryByType<TOwner>() == mpOwnerType
                      : TypeRegistryPool::GetInstance()
                            .GetRegisteredTypeRegistryByType<TOwner>()
                            ->GetAs<Class>()
                            .IsBaseOfRecursive(mpOwnerType->GetAs<Class>()),
                  "Instance doesn't have this property.");

        const uint8_t* pByte = reinterpret_cast<const uint8_t*>(pInstance);
        return PropertyValue(mSize, static_cast<const void*>(pByte + mOffset));
    }

    template <typename TProp, typename TOwner>
    void SetValue(TOwner* pOutInstance, const TProp& value) const
    {
        HO_ASSERT(pOutInstance != nullptr, "Instance cannot be null.");
        HO_ASSERT(mpOwnerType->GetTypeKind() == eTypeKind::Struct
                      ? TypeRegistryPool::GetInstance().GetRegisteredTypeRegistryByType<TOwner>() == mpOwnerType
                      : TypeRegistryPool::GetInstance()
                            .GetRegisteredTypeRegistryByType<TOwner>()
                            ->GetAs<Class>()
                            .IsBaseOfRecursive(mpOwnerType->GetAs<Class>()),
                  "Instance doesn't have this property.");
        HO_ASSERT(sizeof(TProp) == mSize, "Property size mismatch.");

        uint8_t* pByte = reinterpret_cast<uint8_t*>(pOutInstance);
        TProp* pValueAddr = reinterpret_cast<TProp*>(pByte + mOffset);
        *pValueAddr = value;
    }

    template <typename TOwner>
    void* GetPtr(TOwner* pOutInstance) const
    {
        HO_ASSERT(pOutInstance != nullptr, "Instance cannot be null.");
        HO_ASSERT(mpOwnerType->GetTypeKind() == eTypeKind::Struct
                      ? TypeRegistryPool::GetInstance().GetRegisteredTypeRegistryByType<TOwner>() == mpOwnerType
                      : TypeRegistryPool::GetInstance()
                            .GetRegisteredTypeRegistryByType<TOwner>()
                            ->template GetAs<Class>()
                            .IsBaseOfRecursive(mpOwnerType->GetAs<Class>()),
                  "Instance doesn't have this property.");

        uint8_t* pByte = reinterpret_cast<uint8_t*>(pOutInstance);
        return reinterpret_cast<void*>(pByte + mOffset);
    }

    template <typename TOwner>
    const void* GetPtr(const TOwner* pInstance) const
    {
        HO_ASSERT(pInstance != nullptr, "Instance cannot be null.");
        HO_ASSERT(mpOwnerType->GetTypeKind() == eTypeKind::Struct
                      ? TypeRegistryPool::GetInstance().GetRegisteredTypeRegistryByType<TOwner>() == mpOwnerType
                      : TypeRegistryPool::GetInstance()
                            .GetRegisteredTypeRegistryByType<TOwner>()
                            ->GetAs<Class>()
                            .IsBaseOfRecursive(mpOwnerType->GetAs<Class>()),
                  "Instance doesn't have this property.");

        const uint8_t* pByte = reinterpret_cast<const uint8_t*>(pInstance);
        return reinterpret_cast<const void*>(pByte + mOffset);
    }

    PropertyFlags GetFlag() const
    {
        return mFlags;
    }

    ePropertyUIStyle GetUIStyle() const
    {
        return mUIStyle;
    }

    float GetMin() const
    {
        return mMinValue;
    }

    float GetMax() const
    {
        return mMaxValue;
    }

  private:
    Property(const TypeRegistry* pOwnerType,
             const TypeRegistry* pType,
             StringHandle hName,
             int32_t offset,
             int32_t size,
             PropertyFlags flags,
             ePropertyUIStyle uiStyle)
      : mpOwnerType(pOwnerType)
      , mpType(pType)
      , mhName(hName)
      , mOffset(offset)
      , mSize(size)
      , mFlags(flags)
      , mUIStyle(uiStyle)
    {
    }

    void setMinMax(float minValue, float maxValue)
    {
        HO_ASSERT(minValue < maxValue, "max must be greater than min.");
        mMinValue = minValue;
        mMaxValue = maxValue;
    }

    const TypeRegistry* mpOwnerType = nullptr;
    const TypeRegistry* mpType = nullptr;
    StringHandle mhName;

    int32_t mOffset = 0;
    int32_t mSize = 0;
    PropertyFlags mFlags = PropertyFlags_None;
    ePropertyUIStyle mUIStyle = ePropertyUIStyle::None;

    float mMinValue = -FLT_MAX;
    float mMaxValue = FLT_MAX;
};
} // namespace ho