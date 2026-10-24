#pragma once

#include <numeric>

#include "Core/Templates/FixedArray.h"
#include "ReflectionFlags.h"
#include "Type.h"
#include "TypeRegistry.h"

namespace ho
{
class EnumValue final
{
    friend class EnumEntry;

  public:
    template <typename T>
    operator T() const
    {
        if (mbSigned)
        {
            HO_ASSERT(mValueSigned >= static_cast<int64_t>(std::numeric_limits<T>::lowest()) &&
                          mValueSigned <= static_cast<int64_t>(std::numeric_limits<T>::max()),
                      "Enum value overflows target T");
            return static_cast<T>(mValueSigned);
        }
        else
        {
            HO_ASSERT(mValueUnsigned <= static_cast<uint64_t>(std::numeric_limits<T>::max()),
                      "Enum value overflows target T");
            return static_cast<T>(mValueUnsigned);
        }
    }

  private:
    EnumValue(int64_t valueSigned, uint64_t valueUnsigned, bool bSigned)
      : mValueSigned(valueSigned)
      , mValueUnsigned(valueUnsigned)
      , mbSigned(bSigned)
    {
    }

    int64_t mValueSigned;
    uint64_t mValueUnsigned;
    bool mbSigned;
};

class EnumEntry final
{
    friend class Enum;

    template <typename TEnum>
    friend class EnumRegister;

  public:
    EnumEntry() = default;

    StringHandle GetName() const
    {
        return mhName;
    }

    EnumValue GetValue() const
    {
        return EnumValue{mValueSigned, mValueUnsigned, mbSigned};
    }

    bool IsSigned() const
    {
        return mbSigned;
    }

  private:
    EnumEntry(StringHandle hName, int64_t value)
      : mhName(hName)
      , mValueSigned(value)
      , mbSigned(true)
    {
    }

    EnumEntry(StringHandle hName, uint64_t value)
      : mhName(hName)
      , mValueUnsigned(value)
      , mbSigned(false)
    {
    }

    StringHandle mhName;

    union
    {
        int64_t mValueSigned = 0;
        uint64_t mValueUnsigned;
    };

    bool mbSigned = false;
};

class Enum final : public Type
{
    friend class TypeRegistry;

    template <typename TEnum>
    friend class EnumRegister;

  public:
    ~Enum() override = default;

    bool operator==(const Enum& rhs) const
    {
        return mhName == rhs.mhName;
    }

    bool operator!=(const Enum& rhs) const
    {
        return mhName != rhs.mhName;
    }

    const TypeRegistry& GetUnderlyngType() const
    {
        return *mpUnderlyingType;
    }

    const EnumEntry& GetEntry(StringHandle hName) const
    {
        HO_ASSERT(hName.IsValid() && !hName.IsEmpty(), "Enum Entry must has name.");
        for (const auto& entry : mEntries)
        {
            if (entry.GetName() == hName)
            {
                return entry;
            }
        }

        HO_ASSERT(false, "There is no matched entry.");

        static const EnumEntry sFallbackEntry = EnumEntry(StringHandle("InvalidEntry"), static_cast<int64_t>(0));

        return sFallbackEntry;
    }

    const FixedArray<EnumEntry>& GetEntries() const
    {
        return mEntries;
    }

    EnumFlags GetFlags() const
    {
        return mFlags;
    }

  private:
    Enum(uint64_t typeID,
         StringHandle hName,
         int32_t size,
         int32_t alignment,
         const TypeRegistry* pUnderlyingType,
         FixedArray<EnumEntry>&& entries,
         EnumFlags flags)
      : Type(typeID, hName, size, alignment)
      , mpUnderlyingType(pUnderlyingType)
      , mEntries(std::move(entries))
      , mFlags(flags)
    {
    }

    const TypeRegistry* mpUnderlyingType;

    FixedArray<EnumEntry> mEntries;
    EnumFlags mFlags;
};
} // namespace ho