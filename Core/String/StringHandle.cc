#include "StringHandle.h"

namespace ho
{
const StringHandle StringHandle::sNULL = StringHandle();

StringHandle::StringHandle(const char* str)
  : mhString(std::string(str))
{
}

StringHandle::StringHandle(const std::string& str)
  : mhString(str)
{
}

bool StringHandle::operator==(const StringHandle& rhs) const
{
    return mhString == rhs.mhString;
}

bool StringHandle::operator!=(const StringHandle& rhs) const
{
    return mhString != rhs.mhString;
}

const char* StringHandle::operator*() const
{
    HO_ASSERT(mhString.IsValid(), "String is invalid.");
    return Get();
}

const char* StringHandle::Get() const
{
    HO_ASSERT(mhString.IsValid(), "String is invalid.");
    return mhString.Get()->c_str();
}

bool StringHandle::IsNULL() const
{
    return mhString.IsNULL();
}

bool StringHandle::IsValid() const
{
    return mhString.IsValid();
}

uint64_t StringHandle::GetID() const
{
    return mhString.GetID();
}

int32_t StringHandle::GetLength() const
{
    HO_ASSERT(mhString.IsValid(), "String is invalid.");
    return static_cast<int32_t>(mhString.Get()->length());
}

bool StringHandle::IsEmpty() const
{
    HO_ASSERT(mhString.IsValid(), "String is invalid.");
    return mhString.Get()->empty();
}
} // namespace ho