#pragma once

#include "Enum.h"
#include "TypeRegistryPool.h"

namespace ho
{
template <typename TEnum>
class EnumRegister final
{
  public:
    EnumRegister(StringHandle hName, EnumFlags flags = EnumFlags_None)
      : mhName(hName)
      , mFlags(flags)
    {
        static_assert(std::is_enum_v<TEnum>, "TEnum must be an enum type.");
        HO_ASSERT(mhName.IsValid() && !hName.IsEmpty(), "Type name can't be empty.");
        HO_ASSERT(TypeRegistryPool::GetInstance().mNameToTypeRegistryMap.find(mhName) ==
                      TypeRegistryPool::GetInstance().mNameToTypeRegistryMap.end(),
                  "Duplicate type registration detected. Type name already exists in ReflectionSystem.");
    }

    void AddEntry(StringHandle hName, TEnum value)
    {
        HO_ASSERT(hName.IsValid() && !hName.IsEmpty(), "Enum Entry must has name.");

        if constexpr (std::is_signed_v<std::underlying_type_t<TEnum>>)
        {
            mEntries.push_back(EnumEntry(hName, static_cast<int64_t>(value)));
        }
        else
        {
            mEntries.push_back(EnumEntry(hName, static_cast<uint64_t>(value)));
        }
    }

    void Register()
    {
        TypeRegistry* pEnumRegistry = TypeRegistryPool::GetInstance().getOrAddTypeRegistry<TEnum>();

        // If the type was already registered, this acts as a type alias mapping
        if (!pEnumRegistry->IsRegistered())
        {
            FixedArray<EnumEntry> entries(static_cast<int32_t>(mEntries.size()));
            for (int32_t i = 0; i < entries.GetSize(); ++i)
            {
                entries[i] = mEntries[i];
            }

            const TypeRegistry* pUnderlyingRegistry =
                TypeRegistryPool::GetInstance().getOrAddTypeRegistry<std::underlying_type_t<TEnum>>();

            pEnumRegistry->mbRegistered = true;
            pEnumRegistry->mTypeKind = eTypeKind::Enum;

            Enum* pEnum = new Enum(pEnumRegistry->GetID(),
                                   mhName,
                                   static_cast<int32_t>(sizeof(TEnum)),
                                   static_cast<int32_t>(alignof(TEnum)),
                                   pUnderlyingRegistry,
                                   std::move(entries),
                                   mFlags);
            pEnumRegistry->mpType = pEnum;
        }

        TypeRegistryPool::GetInstance().mNameToTypeRegistryMap[mhName] = pEnumRegistry;

        std::vector<EnumEntry>().swap(mEntries);
    }

  private:
    StringHandle mhName;
    std::vector<EnumEntry> mEntries;
    EnumFlags mFlags;
};

template <typename T>
struct RegisterFuncRegister;

#define HO_REGISTER_ENUM_BEGIN(Type, Name, ...)                                                                        \
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
            using TEnum = Type;                                                                                        \
            EnumRegister<TEnum> builder(StringHandle(Name), ##__VA_ARGS__);

#define HO_REGISTER_ENUM_ENTRY(Name, Value) builder.AddEntry(StringHandle(Name), Value);

#define HO_REGISTER_ENUM_END()                                                                                         \
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
