#pragma once

#include <string>
#include <utility>

#include "Core/Memory/GlobalTypedHandle.h"

namespace ho
{
class StringHandle final
{
  public:
    static const StringHandle sNULL;

    StringHandle()
    {
        *this = sNULL;
    }

    StringHandle(const char* str);

    StringHandle(const std::string& str);

    [[nodiscard]] bool operator==(const StringHandle& rhs) const;

    [[nodiscard]] bool operator!=(const StringHandle& rhs) const;

    [[nodiscard]] const char* operator*() const;

    [[nodiscard]] const char* Get() const;

    [[nodiscard]] bool IsNULL() const;

    [[nodiscard]] bool IsValid() const;

    [[nodiscard]] uint64_t GetID() const;

    [[nodiscard]] int32_t GetLength() const;

    [[nodiscard]] bool IsEmpty() const;

  private:
    GlobalTypedHandle<std::string, UniqueTypedObjectPool> mhString =
        GlobalTypedHandle<std::string, UniqueTypedObjectPool>::sNULL;
};
} // namespace ho

namespace std
{
template <>
struct hash<ho::StringHandle>
{
    size_t operator()(const ho::StringHandle& k) const noexcept
    {
        return std::hash<uint64_t>{}(k.GetID());
    }
};
} // namespace std