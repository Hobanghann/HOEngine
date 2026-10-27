#pragma once

#include "Class.h"
#include "Enum.h"
#include "Struct.h"
#include "Type.h"
#include "TypeRegistryPool.h"

namespace ho
{
class Engine;

// All types are fully registered via static registration (HO_REGISTER_*) prior to calling createInstance().
class ReflectionSystem final
{
    friend Engine;

  public:
    ReflectionSystem(const ReflectionSystem&) = delete;
    ReflectionSystem& operator=(const ReflectionSystem&) = delete;

    static ReflectionSystem& GetInstance();

    template <typename TBuiltIn>
    const Type& GetBuiltInTypeByType() const
    {
        const TypeRegistry* pType = TypeRegistryPool::GetInstance().GetRegisteredTypeRegistryByType<TBuiltIn>();
        HO_ASSERT(pType->GetTypeKind() == eTypeKind::BuiltIn, "Type is not built-in type.");
        return pType->GetType();
    }

    const Type& GetBuiltInTypeByName(StringHandle hName) const
    {
        const TypeRegistry* pType = TypeRegistryPool::GetInstance().GetRegisteredTypeRegistryByName(hName);
        HO_ASSERT(pType->GetTypeKind() == eTypeKind::BuiltIn, "Type is not built-in type.");
        return pType->GetType();
    }

    template <typename TEnum>
    const Enum& GetEnumByType() const
    {
        const TypeRegistry* pType = TypeRegistryPool::GetInstance().GetRegisteredTypeRegistryByType<TEnum>();
        HO_ASSERT(pType->GetTypeKind() == eTypeKind::Enum, "Type is not enum.");
        return pType->GetAs<Enum>();
    }

    const Enum& GetEnumByName(StringHandle hName) const
    {
        const TypeRegistry* pType = TypeRegistryPool::GetInstance().GetRegisteredTypeRegistryByName(hName);
        HO_ASSERT(pType->GetTypeKind() == eTypeKind::Enum, "Type is not enum.");
        return pType->GetAs<Enum>();
    }

    template <typename TStruct>
    const Struct& GetStructByType() const
    {
        const TypeRegistry* pType = TypeRegistryPool::GetInstance().GetRegisteredTypeRegistryByType<TStruct>();
        HO_ASSERT(pType->GetTypeKind() == eTypeKind::Struct, "Type is not struct.");
        return pType->GetAs<Struct>();
    }

    const Struct& GetStructByName(StringHandle hName) const
    {
        const TypeRegistry* pType = TypeRegistryPool::GetInstance().GetRegisteredTypeRegistryByName(hName);
        HO_ASSERT(pType->GetTypeKind() == eTypeKind::Struct, "Type is not struct.");
        return pType->GetAs<Struct>();
    }

    template <typename TClass>
    const Class& GetClassByType() const
    {
        const TypeRegistry* pType = TypeRegistryPool::GetInstance().GetRegisteredTypeRegistryByType<TClass>();
        HO_ASSERT(pType->GetTypeKind() == eTypeKind::Class, "Type is not class.");
        return pType->GetAs<Class>();
    }

    const Class& GetClassByName(StringHandle hName) const
    {
        const TypeRegistry* pType = TypeRegistryPool::GetInstance().GetRegisteredTypeRegistryByName(hName);
        HO_ASSERT(pType->GetTypeKind() == eTypeKind::Class, "Type is not class.");
        return pType->GetAs<Class>();
    }

    // Returns a unique ID for each type, regardless of whether it is registered in reflection.
    template <typename T>
    uint32_t GetTypeID()
    {
        const TypeRegistry* pType = TypeRegistryPool::GetInstance().getOrAddTypeRegistry<T>();
        return pType->GetID();
    }

    int32_t GetRegisteredTypeCount() const
    {
        int32_t registeredCount = 0;
        for (const auto& typeRegistry : TypeRegistryPool::GetInstance().mTypeRegistryPool)
        {
            if (typeRegistry.IsRegistered())
            {
                ++registeredCount;
            }
        }
        return registeredCount;
    }

  private:
    ReflectionSystem();
    ~ReflectionSystem() = default;

    static void createInstance();
    static void deleteInstance();

    bool init();
    void shutdown();

    static ReflectionSystem* spInstance;
};

} // namespace ho