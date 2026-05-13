#pragma once

#include <cstdint>
#include <functional>
#include <limits>

#include "Macros.h"

namespace ho
{
template <typename T>
class ID
{
  public:
    constexpr ID()
      : mID(0)
    {
    }

    [[nodiscard]] constexpr uint64_t GetID() const
    {
        return mID;
    }

    [[nodiscard]] constexpr bool operator==(const ID<T>& rhs) const
    {
        return mID == rhs.mID;
    }

    [[nodiscard]] constexpr bool operator!=(const ID<T>& rhs) const
    {
        return !(*this == rhs);
    }

    [[nodiscard]] constexpr bool IsNULL() const
    {
        return mID == 0ull;
    }

    [[nodiscard]] constexpr bool IsValid() const
    {
        return !IsNULL();
    }

    constexpr ID& Increment()
    {
        HO_ASSERT(mID < std::numeric_limits<uint64_t>::max(), "ID exceeds maximum limit.");
        ++mID;
        return *this;
    }

    constexpr ID& Decrement()
    {
        HO_ASSERT(mID > 0ull, "ID is already zero.");
        --mID;
        return *this;
    }

    static const ID sNULL;

  private:
    uint64_t mID;
};

template <typename T>
INLINE const ID<T> ID<T>::sNULL{};

} // namespace ho

namespace std
{
template <typename T>
struct hash<ho::ID<T>>
{
    std::size_t operator()(const ho::ID<T>& id) const
    {
        return std::hash<uint64_t>{}(id.GetID());
    }
};
} // namespace std