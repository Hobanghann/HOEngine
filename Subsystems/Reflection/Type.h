#pragma once

#include "Core/String/StringHandle.h"

namespace ho
{

class Type
{
    friend class TypeRegistry;

    template <typename T>
    friend struct RegisterFuncRegister;

  public:
    virtual ~Type() = default;

    bool operator==(const Type& rhs) const
    {
        return mhName == rhs.mhName;
    }

    bool operator!=(const Type& rhs) const
    {
        return mhName != rhs.mhName;
    }

    uint64_t GetTypeID() const
    {
        return mTypeID;
    }

    StringHandle GetName() const
    {
        return mhName;
    }

    int32_t GetSize() const
    {
        return mSize;
    }

    int32_t GetAlignment() const
    {
        return mAlignment;
    }

  protected:
    Type(uint64_t typeID, StringHandle hName, int32_t size, int32_t alignment)
      : mTypeID(typeID)
      , mhName(hName)
      , mSize(size)
      , mAlignment(alignment)
    {
    }

    uint64_t mTypeID;
    StringHandle mhName;
    int32_t mSize;
    int32_t mAlignment;
};
} // namespace ho