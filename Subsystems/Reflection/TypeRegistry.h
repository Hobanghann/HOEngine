#pragma once

#include <cstdint>
#include <functional>

#include "Core/Memory/WeakTypedHandle.h"

namespace ho
{
class Type;

enum class eTypeKind : uint8_t
{
    None = 0,
    BuiltIn = 1,
    Enum = 2,
    Struct = 3,
    Class = 4,
};

// Wraps reflection types into a single container to allow order-independent
// static registration and unified type management via TypeRegistryHandle.

class TypeRegistry final
{
    friend class TypedObjectPool<TypeRegistry>;
    friend class TypeRegistryPool;

    template <typename TEnum>
    friend class EnumRegister;
    template <typename TStruct>
    friend class StructRegister;
    template <typename TClass, typename TParent>
    friend class ClassRegister;

    template <typename T>
    friend struct RegisterFuncRegister;

  public:
    TypeRegistry(const TypeRegistry&) = default;
    TypeRegistry& operator=(const TypeRegistry&) = default;
    ~TypeRegistry();

    uint64_t GetID() const
    {
        return mID;
    }

    bool IsRegistered() const
    {
        return mbRegistered;
    }

    eTypeKind GetTypeKind() const
    {
        HO_ASSERT(mbRegistered, "Type is not registered yet.");
        return mTypeKind;
    }

    Type& GetType()
    {
        HO_ASSERT(mbRegistered, "Type is not registered yet.");
        return *mpType;
    }

    template <typename T>
    T& GetAs()
    {
        static_assert(std::is_base_of_v<Type, T>, "T must be Type, Enum, Struct, or Class.");
        HO_ASSERT(mbRegistered, "Type is not registered yet.");
        return *static_cast<T*>(mpType);
    }

    const Type& GetType() const
    {
        HO_ASSERT(mbRegistered, "Type is not registered yet.");
        return *mpType;
    }

    template <typename T>
    const T& GetAs() const
    {
        static_assert(std::is_base_of_v<Type, T>, "T must be Type, Enum, Struct, or Class.");
        HO_ASSERT(mbRegistered, "Type is not registered yet.");
        return *static_cast<const T*>(mpType);
    }

  private:
    using RegisterFunc = std::function<void()>;

    void setRegisterFunc(RegisterFunc pFunc)
    {
        HO_ASSERT(!mbRegistered, "This type is already registred.");
        mpRegisterFunc = std::move(pFunc);
        mbRegistable = true;
    }

    void regist()
    {
        HO_ASSERT(mbRegistable, "This registry can't be registered. It's not have function for register.");
        HO_ASSERT(!mbRegistered, "This type is already registred.");
        mpRegisterFunc();
        mbRegistered = true;
    }

    TypeRegistry() = default;

    uint64_t mID = 0;
    bool mbRegistable = false;
    bool mbRegistered = false;
    eTypeKind mTypeKind = eTypeKind::None;
    Type* mpType = nullptr;
    RegisterFunc mpRegisterFunc = nullptr;
};
} // namespace ho