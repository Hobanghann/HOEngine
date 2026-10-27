#pragma once

#include "Class.h"

namespace ho
{
template <typename TClass, typename TParent>
class ClassRegister final
{
  public:
    ClassRegister(StringHandle hName, ClassFlags flags = ClassFlags_None)
      : mhName(hName)
      , mFlags(flags)
    {
        static_assert(std::is_class_v<TClass>, "TClass must be a class or struct type.");
        static_assert(std::is_default_constructible_v<TClass> || std::is_abstract_v<TClass>,
                      "Reflection system requires the concrete class to have a default constructor.");
        HO_ASSERT(hName.IsValid(), "Type name can't be empty.");
        // HO_ASSERT(TypeRegistryPool::GetInstance().mNameToTypeRegistryMap.find(hName) ==
        //               TypeRegistryPool::GetInstance().mNameToTypeRegistryMap.end(),
        //           "Duplicate type registration detected. Type name already exists in ReflectionSystem.");
        if constexpr (!std::is_abstract_v<TClass>)
        {
            mpInstantiator = []() -> IReflectable*
            {
                TClass* pInst = new TClass();
                HO_ASSERT(pInst, "Failed to initiate.");
                pInst->template setType<TClass>(); // Reflected class must inherit 'IReflectable'.
                return pInst;
            };
        }
        if constexpr (!std::is_abstract_v<TClass>)
        {
            mpInPlaceInstantiator = [](void* ptr) -> IReflectable*
            {
                HO_ASSERT(ptr, "Invalid destination ptr.");
                TClass* pInst = ::new (ptr) TClass();
                HO_ASSERT(pInst, "Failed to initiate.");
                pInst->template setType<TClass>(); // Reflected class must inherit 'IReflectable'.
                return pInst;
            };
        }
        HO_ASSERT(std::is_abstract_v<TClass> || mpInstantiator != nullptr,
                  "Non-abstract class must be default constructable for reflection.");
        mpDestroyer = [](IReflectable* pInstance) { static_cast<TClass*>(pInstance)->~TClass(); };
        mpDeleter = [](IReflectable* pInstance) { delete static_cast<TClass*>(pInstance); };
    }

    template <typename TargetType, typename Pack>
    using ExpandAs = std::string_view;

    template <typename TReturn, typename... TParams>
    void AddMethod(StringHandle hName,
                   TReturn (TClass::*pFunc)(TParams...),
                   ExpandAs<std::string_view, TParams>... paramNames,
                   MethodFlags flags = MethodFlags_None)
    {
        HO_ASSERT(hName.IsValid() && !hName.IsEmpty(), "Method must has name.");
        FixedArray<StringHandle> hParamNames = {StringHandle(paramNames)...};

        Method::InvokerFunc invoker = [pFunc](void* pInstance, void** ppArgs, void* pOutReturn)
        {
            TClass* pClassObj = static_cast<TClass*>(pInstance);
            ClassRegister::invokeInternal(pClassObj, pFunc, ppArgs, pOutReturn, std::index_sequence_for<TParams...>{});
        };

        mMethods.push_back(Method(TypeRegistryPool::GetInstance().getOrAddTypeRegistry<TClass>(),
                                  hName,
                                  std::move(hParamNames),
                                  invoker,
                                  flags));
    }

    template <typename TReturn, typename... TParams>
    void AddMethod(StringHandle hName,
                   TReturn (TClass::*pFunc)(TParams...) const,
                   ExpandAs<std::string_view, TParams>... paramNames,
                   MethodFlags flags = MethodFlags_None)
    {
        HO_ASSERT(hName.IsValid() && !hName.IsEmpty(), "Method must has name.");
        FixedArray<StringHandle> hParamNames = {StringHandle(paramNames)...};

        Method::InvokerFunc invoker = [pFunc](void* pInstance, void** ppArgs, void* pOutReturn)
        {
            TClass* pClassObj = static_cast<TClass*>(pInstance);
            ClassRegister::invokeInternal(pClassObj, pFunc, ppArgs, pOutReturn, std::index_sequence_for<TParams...>{});
        };

        mMethods.push_back(Method(TypeRegistryPool::GetInstance().getOrAddTypeRegistry<TClass>(),
                                  hName,
                                  std::move(hParamNames),
                                  invoker,
                                  flags));
    }

    template <typename TProp>
    void AddProperty(StringHandle hName,
                     int32_t offset,
                     PropertyFlags flags = PropertyFlags_None,
                     ePropertyUIStyle uiStyle = ePropertyUIStyle::None)
    {
        HO_ASSERT(hName.IsValid() && !hName.IsEmpty(), "Property must has name.");
        HO_ASSERT(offset + sizeof(TProp) <= sizeof(TClass), "Property offset and size exceed parent struct size.");
        mProperties.push_back(Property(TypeRegistryPool::GetInstance().getOrAddTypeRegistry<TClass>(),
                                       TypeRegistryPool::GetInstance().getOrAddTypeRegistry<TProp>(),
                                       hName,
                                       offset,
                                       sizeof(TProp),
                                       flags,
                                       uiStyle));
    }

    void SetPropertyMinMax(StringHandle hName, float minValue, float maxValue)
    {
        for (auto& property : mProperties)
        {
            if (hName == property.GetName())
            {
                property.setMinMax(minValue, maxValue);
                return;
            }
        }

        HO_ASSERT(false, "Property is not registered yet.");
    }

    void Register()
    {
        TypeRegistry* pClassRegistry = TypeRegistryPool::GetInstance().getOrAddTypeRegistry<TClass>();

        // If the type was already registered, this acts as a type alias mapping
        if (!pClassRegistry->IsRegistered())
        {
            FixedArray<Method> methods(static_cast<int32_t>(mMethods.size()));
            for (int32_t i = 0; i < methods.GetSize(); ++i)
            {
                methods[i] = std::move(mMethods[i]);
            }

            FixedArray<Property> props(static_cast<int32_t>(mProperties.size()));
            for (int32_t i = 0; i < props.GetSize(); ++i)
            {
                props[i] = mProperties[i];
            }

            TypeRegistry* pParentRegistry = nullptr;
            TypeRegistry* pPrevSiblingRegistry = nullptr;
            if constexpr (!std::is_same_v<TParent, void>)
            {
                pParentRegistry = TypeRegistryPool::GetInstance().getOrAddTypeRegistry<TParent>();

                HO_ASSERT(pParentRegistry->mbRegistable,
                          "Parent class's register function is not registered. Regist parent class using macros");
                if (!pParentRegistry->mbRegistered)
                {
                    pParentRegistry->regist();
                }

                Class& parentClass = pParentRegistry->GetAs<Class>();
                if (!parentClass.mpFirstChild)
                {
                    parentClass.mpFirstChild = pClassRegistry;
                }
                else
                {
                    TypeRegistry* pChild = parentClass.mpFirstChild;
                    while (true)
                    {
                        HO_ASSERT(pChild->mbRegistable,
                                  "Child class's register function is not registered. Regist child class using macros");
                        if (!pChild->mbRegistered)
                        {
                            pChild->regist();
                        }

                        if (pChild->GetAs<Class>().mpNextSibling == nullptr)
                        {
                            break;
                        }

                        pChild = pChild->GetAs<Class>().mpNextSibling;
                    }

                    pChild->GetAs<Class>().mpNextSibling = pClassRegistry;
                    pPrevSiblingRegistry = pChild;
                }
            }

            pClassRegistry->mbRegistered = true;
            pClassRegistry->mTypeKind = eTypeKind::Class;

            Class* pClass = new Class(pClassRegistry->GetID(),
                                      mhName,
                                      static_cast<int32_t>(sizeof(TClass)),
                                      static_cast<int32_t>(alignof(TClass)),
                                      pParentRegistry,
                                      nullptr,
                                      pPrevSiblingRegistry,
                                      nullptr,
                                      mpInstantiator,
                                      mpInPlaceInstantiator,
                                      mpDestroyer,
                                      mpDeleter,
                                      std::move(methods),
                                      std::move(props),
                                      std::is_abstract_v<TClass>,
                                      mFlags);
            pClassRegistry->mpType = pClass;
        }

        TypeRegistryPool::GetInstance().mNameToTypeRegistryMap[mhName] = pClassRegistry;

        std::vector<Method>().swap(mMethods);
        std::vector<Property>().swap(mProperties);
    }

  private:
    template <typename TReturn, typename... TParams, std::size_t... ParamIndices>
    static void invokeInternal(TClass* pClassObj,
                               TReturn (TClass::*pFunc)(TParams...),
                               void** ppArgs,
                               void* pOutReturn,
                               std::index_sequence<ParamIndices...> indices)
    {
        if constexpr (std::is_void_v<TReturn>)
        {
            (pClassObj->*pFunc)(*static_cast<std::decay_t<TParams>*>(ppArgs[ParamIndices])...);
        }
        else
        {
            HO_ASSERT(pOutReturn, "There is no memory for return value.");
            *static_cast<std::decay_t<TReturn>*>(pOutReturn) =
                (pClassObj->*pFunc)(*static_cast<std::decay_t<TParams>*>(ppArgs[ParamIndices])...);
        }
    }

    StringHandle mhName;
    IReflectable* (*mpInstantiator)() = nullptr;
    IReflectable* (*mpInPlaceInstantiator)(void*) = nullptr;
    void (*mpDestroyer)(IReflectable*) = nullptr;
    void (*mpDeleter)(IReflectable*) = nullptr;
    std::vector<Method> mMethods;
    std::vector<Property> mProperties;
    ClassFlags mFlags;
};

template <typename T>
struct RegisterFuncRegister;

#define HO_REGISTER_CLASS_BEGIN(Type, Name, ...)                                                                       \
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
            using TClass = Type;                                                                                       \
            ClassRegister<TClass, void> builder(StringHandle(Name), ##__VA_ARGS__);

#define HO_REGISTER_CLASS_DERIVED_BEGIN(Type, Name, TParent, ...)                                                      \
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
            using TClass = Type;                                                                                       \
            ClassRegister<TClass, TParent> builder(StringHandle(Name), ##__VA_ARGS__);

#define HO_REGISTER_CLASS_METHOD(MemberIdentifier, Name, ...)                                                          \
    builder.AddMethod(StringHandle(Name), &TClass::MemberIdentifier, ##__VA_ARGS__);

#define HO_REGISTER_CLASS_PROPERTY(MemberIdentifier, Name, ...)                                                        \
    builder.AddProperty<decltype(TClass::MemberIdentifier)>(                                                           \
        StringHandle(Name), static_cast<int32_t>(offsetof(TClass, MemberIdentifier)), ##__VA_ARGS__);

#define HO_REGISTER_CLASS_PROPERTY_MINMAX(Name, Min, Max) builder.SetPropertyMinMax(StringHandle(Name), Min, Max);

#define HO_REGISTER_CLASS_END()                                                                                        \
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
