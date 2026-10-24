#pragma once

#include "Core/Templates/FixedArray.h"
#include "Method.h"
#include "Property.h"
#include "ReflectionFlags.h"
#include "Type.h"
#include "TypeRegistryPool.h"

namespace ho
{
class IReflectable;

class Class final : public Type
{
    friend class TypeRegistry;

    template <typename TClass, typename TParent>
    friend class ClassRegister;

  public:
    ~Class() override = default;

    bool operator==(const Class& rhs) const
    {
        return mhName == rhs.mhName;
    }

    bool operator!=(const Class& rhs) const
    {
        return mhName != rhs.mhName;
    }

    IReflectable* New() const;

    IReflectable* NewAt(void* ptr) const;

    void Destroy(IReflectable* pInstance) const;

    void Delete(IReflectable* pInstance) const;

    const Class* GetParent() const
    {
        return mpParent ? &mpParent->GetAs<Class>() : nullptr;
    }

    const Class* GetFirstChild() const
    {
        return mpFirstChild ? &mpFirstChild->GetAs<Class>() : nullptr;
    }

    const Class* GetPrevSibling() const
    {
        return mpPrevSibling ? &mpPrevSibling->GetAs<Class>() : nullptr;
    }

    const Class* GetNextSibling() const
    {
        return mpNextSibling ? &mpNextSibling->GetAs<Class>() : nullptr;
    }

    const Method& GetMethod(StringHandle hName) const
    {
        HO_ASSERT(hName.IsValid() && !hName.IsEmpty(), "Method must has name.");
        for (const auto& method : mMethods)
        {
            if (method.GetName() == hName)
            {
                return method;
            }
        }

        HO_ASSERT(false, "There is no matched method.");

        static const Method sFallbackMethod = Method(nullptr,
                                                     StringHandle("InvalidProperty"),
                                                     FixedArray<StringHandle>(),
                                                     Method::InvokerFunc(),
                                                     MethodFlags_None);

        return sFallbackMethod;
    }

    const FixedArray<Method>& GetMethods() const
    {
        return mMethods;
    }

    const Property& GetProperty(StringHandle hName) const
    {
        HO_ASSERT(hName.IsValid() && !hName.IsEmpty(), "Property must has name.");
        for (const auto& property : mProperties)
        {
            if (property.GetName() == hName)
            {
                return property;
            }
        }

        HO_ASSERT(false, "There is no matched property.");

        static const Property sFallbackProperty = Property(
            nullptr, nullptr, StringHandle("InvalidProperty"), 0, 0, PropertyFlags_None, ePropertyUIStyle::None);

        return sFallbackProperty;
    }

    const FixedArray<Property>& GetProperties() const
    {
        return mProperties;
    }

    bool IsAbstract() const
    {
        return mbIsAbstract;
    }

    ClassFlags GetFlags() const
    {
        return mFlags;
    }

    bool IsBaseOfRecursive(const Class& descendant) const
    {
        if (this == &descendant)
        {
            return true;
        }

        if (descendant.mpParent == nullptr)
        {
            return false;
        }

        const Class& desParent = descendant.mpParent->GetAs<Class>();
        if (mhName == desParent.GetName())
        {
            return true;
        }

        return IsBaseOfRecursive(desParent);
    }

  private:
    Class(uint64_t typeID,
          StringHandle hName,
          int32_t size,
          int32_t alignment,
          TypeRegistry* pParent,
          TypeRegistry* pFirstChild,
          TypeRegistry* pPrevSibling,
          TypeRegistry* pNextSibling,
          IReflectable* (*pInstantiator)(),
          IReflectable* (*pInPlaceInstantiator)(void*),
          void (*pDestroyer)(IReflectable*),
          void (*pDeleter)(IReflectable*),
          FixedArray<Method>&& methods,
          FixedArray<Property>&& properties,
          bool bIsAbstract,
          StructFlags flags)
      : Type(typeID, hName, size, alignment)
      , mpParent(pParent)
      , mpFirstChild(pFirstChild)
      , mpPrevSibling(pPrevSibling)
      , mpNextSibling(pNextSibling)
      , mpInstantiator(pInstantiator)
      , mpInPlaceInstantiator(pInPlaceInstantiator)
      , mpDestroyer(pDestroyer)
      , mpDeleter(pDeleter)
      , mMethods(std::move(methods))
      , mProperties(std::move(properties))
      , mbIsAbstract(bIsAbstract)
      , mFlags(flags)
    {
    }

    TypeRegistry* mpParent = nullptr;
    TypeRegistry* mpFirstChild = nullptr;
    TypeRegistry* mpPrevSibling = nullptr;
    TypeRegistry* mpNextSibling = nullptr;
    IReflectable* (*mpInstantiator)() = nullptr;
    IReflectable* (*mpInPlaceInstantiator)(void*) = nullptr;
    void (*mpDestroyer)(IReflectable*) = nullptr;
    void (*mpDeleter)(IReflectable*) = nullptr;
    FixedArray<Method> mMethods;
    FixedArray<Property> mProperties;
    bool mbIsAbstract = false;
    ClassFlags mFlags = ClassFlags_None;
};
} // namespace ho