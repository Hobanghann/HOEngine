#pragma once

#include <unordered_map>
#include <vector>

#include "Core/Memory/TypedObjectPool.h"
#include "Core/String/StringHandle.h"
#include "TypeRegistry.h"

namespace ho
{

// This is intentionally designed to support static registration (HO_REGISTER_* macros)
// before main() or Engine startup.
class TypeRegistryPool final
{
    template <typename TEnum>
    friend class EnumRegister;
    template <typename TStruct>
    friend class StructRegister;
    template <typename TClass, typename TParent>
    friend class ClassRegister;

    friend class ReflectionSystem;

    template <typename T>
    friend struct RegisterFuncRegister;

  public:
    static TypeRegistryPool& GetInstance()
    {
        static TypeRegistryPool sInstance;
        return sInstance;
    }

    template <typename T>
    const TypeRegistry* GetRegisteredTypeRegistryByType()
    {
        const TypeRegistry* type = getOrAddTypeRegistry<T>();
        if (!type->IsRegistered())
        {
            HO_ASSERT(false, "Type not registered.");
            return nullptr;
        }
        return type;
    }

    const TypeRegistry* GetRegisteredTypeRegistryByName(StringHandle hName)
    {
        auto it = mNameToTypeRegistryMap.find(hName);
        if (it == mNameToTypeRegistryMap.end() || !it->second->IsRegistered())
        {
            HO_ASSERT(false, "Invalid type name or type not registered.");
            return nullptr;
        }
        return it->second;
    }

    template <typename T>
    bool CheckRegisteredByType()
    {
        const TypeRegistry* type = getOrAddTypeRegistry<T>();
        const bool bIsRegistered = type.IsRegistered();
        if (!bIsRegistered)
        {
            mTypeRegistryPool.Remove(*type);
        }
        return bIsRegistered;
    }

    bool CheckRegisteredByName(StringHandle hName)
    {
        return mNameToTypeRegistryMap.find(hName) != mNameToTypeRegistryMap.end();
    }

  private:
    TypeRegistryPool() = default;

    // Type registry returned by this function might be unregisterd.
    template <typename T>
    TypeRegistry* getOrAddTypeRegistry()
    {
        return getOrAddTypeRegistryInternal<std::decay_t<T>>();
    }

    template <typename TRawType>
    TypeRegistry* getOrAddTypeRegistryInternal()
    {
        static const uint64_t typeID = mTypeRegistryPool.Add(TypeRegistry());
        mTypeRegistryPool.Get(typeID)->mID = typeID;
        return mTypeRegistryPool.Get(typeID);
    }

    void registerAll()
    {
        for (auto& registry : mTypeRegistryPool)
        {
            if (registry.mbRegistable && !registry.mbRegistered)
            {
                registry.regist();
            }
        }
    }

    // Each type occupies exactly one TypeRegistry in this pool.
    TypedObjectPool<TypeRegistry> mTypeRegistryPool;

    std::unordered_map<StringHandle, TypeRegistry*> mNameToTypeRegistryMap;
};

} // namespace ho