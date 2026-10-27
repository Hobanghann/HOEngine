#pragma once

#include "Core/Templates/FixedArray.h"
#include "Property.h"
#include "ReflectionFlags.h"
#include "Type.h"
#include "TypeRegistryPool.h"

namespace ho
{
class IReflectable;

class Struct final : public Type
{
    friend class TypeRegistry;

    template <typename TStruct>
    friend class StructRegister;

  public:
    ~Struct() override = default;

    bool operator==(const Struct& rhs) const
    {
        return mhName == rhs.mhName;
    }

    bool operator!=(const Struct& rhs) const
    {
        return mhName != rhs.mhName;
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

    StructFlags GetFlags() const
    {
        return mFlags;
    }

  private:
    Struct(uint64_t typeID,
           StringHandle hName,
           int32_t size,
           int32_t alignment,
           FixedArray<Property>&& properties,
           StructFlags flags)
      : Type(typeID, hName, size, alignment)
      , mProperties(std::move(properties))
      , mFlags(flags)
    {
    }

    FixedArray<Property> mProperties;
    StructFlags mFlags = StructFlags_None;
};

} // namespace ho