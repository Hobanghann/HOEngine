#pragma once

// TODO: Need to decoupling
#include "Subsystems/Reflection/TypeRegistryPool.h"

namespace ho
{
class IReflectable
{
    template <typename TClass, typename TParent>
    friend class ClassRegister;

  public:
    IReflectable() = default;
    virtual ~IReflectable() = default;

  protected:
    template <typename T>
    void setType()
    {
        mpType = TypeRegistryPool::GetInstance().GetRegisteredTypeRegistryByType<T>();
    }

    const TypeRegistry* mpType = nullptr;
};
} // namespace ho

#define HO_INJECT_REFLECTION_CODE(Type)                                                                                \
                                                                                                                       \
    template <typename Type>                                                                                           \
    friend struct RegisterFuncRegister;
