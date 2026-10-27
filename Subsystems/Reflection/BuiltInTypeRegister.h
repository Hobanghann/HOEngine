#pragma once

#include "ReflectionFlags.h"
#include "Type.h"
#include "TypeRegistryPool.h"

namespace ho
{
template <typename T>
struct RegisterFuncRegister;

#define HO_REGISTER_VOID()                                                                                             \
    template <>                                                                                                        \
    struct RegisterFuncRegister<void>                                                                                  \
    {                                                                                                                  \
        static void SetRegisterFunc()                                                                                  \
        {                                                                                                              \
            TypeRegistry* pRegistry = TypeRegistryPool::GetInstance().getOrAddTypeRegistry<void>();                    \
            pRegistry->setRegisterFunc(                                                                                \
                []()                                                                                                   \
                {                                                                                                      \
                    TypeRegistry* pReg = TypeRegistryPool::GetInstance().getOrAddTypeRegistry<void>();                 \
                    if (!pReg->IsRegistered())                                                                         \
                    {                                                                                                  \
                        pReg->mbRegistered = true;                                                                     \
                        pReg->mTypeKind = eTypeKind::BuiltIn;                                                          \
                        const int32_t typeSize = 0;                                                                    \
                        const int32_t typeAlignment = 0;                                                               \
                        Type* pBuiltInType = new Type(pReg->GetID(), StringHandle("void"), typeSize, typeAlignment);   \
                        pReg->mpType = pBuiltInType;                                                                   \
                    }                                                                                                  \
                    TypeRegistryPool::GetInstance().mNameToTypeRegistryMap[StringHandle("void")] = pReg;               \
                });                                                                                                    \
        }                                                                                                              \
        inline static const bool HO_CONCAT(s_bRegistered_, __COUNTER__) = []()                                         \
        {                                                                                                              \
            SetRegisterFunc();                                                                                         \
            return true;                                                                                               \
        }();                                                                                                           \
    };

#define HO_REGISTER_BUILTIN_TYPE(T)                                                                                    \
    template <>                                                                                                        \
    struct RegisterFuncRegister<T>                                                                                     \
    {                                                                                                                  \
        static void SetRegisterFunc()                                                                                  \
        {                                                                                                              \
            TypeRegistry* pRegistry = TypeRegistryPool::GetInstance().getOrAddTypeRegistry<T>();                       \
            pRegistry->setRegisterFunc(                                                                                \
                []()                                                                                                   \
                {                                                                                                      \
                    TypeRegistry* pReg = TypeRegistryPool::GetInstance().getOrAddTypeRegistry<T>();                    \
                    if (!pReg->IsRegistered())                                                                         \
                    {                                                                                                  \
                        pReg->mbRegistered = true;                                                                     \
                        pReg->mTypeKind = eTypeKind::BuiltIn;                                                          \
                        const int32_t typeSize = static_cast<int32_t>(sizeof(T));                                      \
                        const int32_t typeAlignment = static_cast<int32_t>(alignof(T));                                \
                        Type* pBuiltInType = new Type(pReg->GetID(), StringHandle(#T), typeSize, typeAlignment);       \
                        pReg->mpType = pBuiltInType;                                                                   \
                    }                                                                                                  \
                    TypeRegistryPool::GetInstance().mNameToTypeRegistryMap[StringHandle(#T)] = pReg;                   \
                });                                                                                                    \
        }                                                                                                              \
        inline static const bool HO_CONCAT(s_bRegistered_, __COUNTER__) = []()                                         \
        {                                                                                                              \
            SetRegisterFunc();                                                                                         \
            return true;                                                                                               \
        }();                                                                                                           \
    };
} // namespace ho