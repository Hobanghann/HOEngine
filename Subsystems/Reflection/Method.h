#pragma once

#include <functional>

#include "Core/String/StringHandle.h"
#include "Core/Templates/FixedArray.h"
#include "ReflectionFlags.h"
#include "TypeRegistry.h"
#include "TypeRegistryPool.h"

namespace ho
{
class Type;

class Method final
{
    friend class Class;
    template <typename TClass, typename TParent>
    friend class ClassRegister;

  public:
    using InvokerFunc = std::function<void(void* pInstance, void** ppArgs, void* pOutReturn)>;

    Method() = default;
    ~Method() = default;

    StringHandle GetName() const
    {
        return mhName;
    }

    int32_t GetParamCount() const
    {
        return mhParamNames.GetSize();
    }

    const FixedArray<StringHandle>& GetParamNames() const
    {
        return mhParamNames;
    }

    template <typename TReturn, typename TOwner, typename... TArgs>
    TReturn Invoke(TOwner* pInstance, TArgs... args) const
    {
        HO_ASSERT(pInstance != nullptr, "Instance cannot be null.");
        HO_ASSERT(TypeRegistryPool::GetInstance().GetRegisteredTypeRegistryByType<TOwner>() == mpOwnerType,
                  "Instance doesn't have this method.");
        HO_ASSERT(static_cast<int32_t>(sizeof...(TArgs)) == mhParamNames.GetSize(), "Parameter count mismatch.");

        if constexpr (std::is_same_v<TReturn, void>)
        {
            if constexpr (sizeof...(TArgs) > 0)
            {
                const void* pArgs[] = {static_cast<const void*>(&args)...};
                mInvoker(pInstance, pArgs, nullptr);
            }
            else
            {
                mInvoker(pInstance, nullptr, nullptr);
            }
        }
        else
        {
            TReturn ret{};
            if constexpr (sizeof...(TArgs) > 0)
            {
                const void* pArgs[] = {static_cast<const void*>(&args)...};
                mInvoker(pInstance, pArgs, &ret);
            }
            else
            {
                mInvoker(pInstance, nullptr, &ret);
            }
            return ret;
        }
    }

    MethodFlags GetFlags() const
    {
        return mFlags;
    }

  private:
    Method(const TypeRegistry* pOwnerType,
           StringHandle hName,
           FixedArray<StringHandle>&& hParamNames,
           InvokerFunc invoker,
           MethodFlags flags)
      : mpOwnerType(pOwnerType)
      , mhName(hName)
      , mhParamNames(std::move(hParamNames))
      , mInvoker(std::move(invoker))
      , mFlags(flags)
    {
    }

    Method(Method&& other) noexcept
      : mpOwnerType(other.mpOwnerType)
      , mhName(other.mhName)
      , mhParamNames(std::move(other.mhParamNames))
      , mInvoker(other.mInvoker)
      , mFlags(other.mFlags)
    {
    }

    Method& operator=(Method&& rhs) noexcept
    {
        if (this == &rhs)
        {
            return *this;
        }
        mpOwnerType = rhs.mpOwnerType;
        mhName = rhs.mhName;
        mhParamNames = std::move(rhs.mhParamNames);
        mInvoker = rhs.mInvoker;
        mFlags = rhs.mFlags;
        return *this;
    }

    const TypeRegistry* mpOwnerType = nullptr;
    StringHandle mhName;
    FixedArray<StringHandle> mhParamNames;

    InvokerFunc mInvoker;
    MethodFlags mFlags = MethodFlags_None;
};
} // namespace ho