#pragma once

#include "Subsystems/Reflection/Class.h"
#include "Subsystems/Reflection/IReflectable.h"

namespace ho
{
class RObject : public IReflectable
{
    HO_INJECT_REFLECTION_CODE(RObject);

    friend class ObjectSystem;

  public:
    RObject() = default;

    ~RObject() override = default;

    uint64_t GetGlobalID() const
    {
        return mGlobalID;
    }

    const Class& GetClass() const
    {
        return mpType->GetAs<Class>();
    }

  private:
    uint64_t mGlobalID = 0;
};
} // namespace ho