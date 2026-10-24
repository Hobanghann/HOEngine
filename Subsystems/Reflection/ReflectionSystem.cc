#include "ReflectionSystem.h"

#include "Core/Log/Logger.h"

namespace ho
{
ReflectionSystem& ReflectionSystem::GetInstance()
{
    HO_ASSERT(spInstance, "Reflection system is not created");
    return *spInstance;
}

ReflectionSystem::ReflectionSystem()
{
    HO_ASSERT(!spInstance, "Reflection system muse be created by 'createInstance' function.");
}

void ReflectionSystem::createInstance()
{
    if (spInstance)
    {
        HO_ASSERT(false, "Reflection system is already created.");
    }
    else
    {
        spInstance = new ReflectionSystem();
    }
}

void ReflectionSystem::deleteInstance()
{
    if (!spInstance)
    {
        HO_ASSERT(false, "Reflection system is not created.");
    }
    else
    {
        delete spInstance;
        spInstance = nullptr;
    }
}

bool ReflectionSystem::init()
{
    TypeRegistryPool::GetInstance().registerAll();
    TypeRegistryPool::GetInstance();
    return true;
}

void ReflectionSystem::shutdown() {}

ReflectionSystem* ReflectionSystem::spInstance = nullptr;
} // namespace ho