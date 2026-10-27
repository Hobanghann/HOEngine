#pragma once

#include "Struct.h"

namespace ho
{

template <typename TStruct>
class StructRegister final
{
  public:
    StructRegister(StringHandle hName, StructFlags flags = StructFlags_None)
      : mhName(hName)
      , mFlags(flags)
    {
        static_assert(std::is_class_v<TStruct>, "TStruct must be a class or struct type.");
        static_assert(!std::is_abstract_v<TStruct>, "Struct cannot be an abstract class.");
        static_assert(std::is_default_constructible_v<TStruct>,
                      "Reflection system requires the struct to have a default constructor.");
        HO_ASSERT(mhName.IsValid(), "Type name can't be empty.");
        HO_ASSERT(TypeRegistryPool::GetInstance().mNameToTypeRegistryMap.find(mhName) ==
                      TypeRegistryPool::GetInstance().mNameToTypeRegistryMap.end(),
                  "Duplicate type registration detected. Type name already exists in ReflectionSystem.");
    }

    template <typename TProp>
    void AddProperty(StringHandle hName,
                     int32_t offset,
                     PropertyFlags flags = PropertyFlags_None,
                     ePropertyUIStyle uiStyle = ePropertyUIStyle::None)
    {
        HO_ASSERT(hName.IsValid() && !hName.IsEmpty(), "Property must has name.");
        HO_ASSERT(offset + sizeof(TProp) <= sizeof(TStruct), "Property offset and size exceed parent struct size.");
        mProperties.push_back(Property(TypeRegistryPool::GetInstance().getOrAddTypeRegistry<TStruct>(),
                                       TypeRegistryPool::GetInstance().getOrAddTypeRegistry<TProp>(),
                                       hName,
                                       offset,
                                       sizeof(TProp),
                                       flags,
                                       uiStyle));
    }

    void SetPropertyMinMax(StringHandle hName, float minValue, float maxValue)
    {
        for (auto& property : mProperties)
        {
            if (hName == property.GetName())
            {
                property.setMinMax(minValue, maxValue);
                return;
            }
        }

        HO_ASSERT(false, "Property is not registered yet.");
    }

    void Register()
    {
        TypeRegistry* pStructRegistry = TypeRegistryPool::GetInstance().getOrAddTypeRegistry<TStruct>();

        // If the type was already registered, this acts as a type alias mapping
        if (!pStructRegistry->IsRegistered())
        {
            FixedArray<Property> props(static_cast<int32_t>(mProperties.size()));
            for (int32_t i = 0; i < props.GetSize(); ++i)
            {
                props[i] = mProperties[i];
            }

            pStructRegistry->mbRegistered = true;
            pStructRegistry->mTypeKind = eTypeKind::Struct;

            Struct* pStruct = new Struct(pStructRegistry->GetID(),
                                         mhName,
                                         static_cast<int32_t>(sizeof(TStruct)),
                                         static_cast<int32_t>(alignof(TStruct)),
                                         std::move(props),
                                         mFlags);
            pStructRegistry->mpType = pStruct;
        }

        TypeRegistryPool::GetInstance().mNameToTypeRegistryMap[mhName] = pStructRegistry;

        std::vector<Property>().swap(mProperties);
    }

  private:
    StringHandle mhName;
    std::vector<Property> mProperties;
    StructFlags mFlags;
};

template <typename T>
struct RegisterFuncRegister;

#define HO_REGISTER_STRUCT_BEGIN(Type, Name, ...)                                                                      \
    template <>                                                                                                        \
    struct RegisterFuncRegister<Type>                                                                                  \
    {                                                                                                                  \
        static void SetRegisterFunc()                                                                                  \
        {                                                                                                              \
            TypeRegistry* pRegistry = TypeRegistryPool::GetInstance().getOrAddTypeRegistry<Type>();                    \
            pRegistry->setRegisterFunc([]() { const RegisterFuncRegister<Type> reg; });                                \
        }                                                                                                              \
                                                                                                                       \
        RegisterFuncRegister()                                                                                         \
        {                                                                                                              \
            using TStruct = Type;                                                                                      \
            StructRegister<TStruct> builder(StringHandle(Name), ##__VA_ARGS__);

#define HO_REGISTER_STRUCT_PROPERTY(MemberIdentifier, Name, ...)                                                       \
    builder.AddProperty<decltype(TStruct::MemberIdentifier)>(                                                          \
        StringHandle(Name), static_cast<int32_t>(offsetof(TStruct, MemberIdentifier)), ##__VA_ARGS__);

#define HO_REGISTER_STRUCT_END()                                                                                       \
    builder.Register();                                                                                                \
    }                                                                                                                  \
    inline static const bool sbRegistered = []()                                                                       \
    {                                                                                                                  \
        SetRegisterFunc();                                                                                             \
        return true;                                                                                                   \
    }();                                                                                                               \
    }                                                                                                                  \
    ;
} // namespace ho
