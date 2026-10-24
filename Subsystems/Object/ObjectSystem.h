#pragma once

#include "Core/Memory/TypedObjectPool.h"
#include "Object.h"
#include "ObjectHandle.h"
#include "ObjectManager.h"

namespace ho
{
class Class;

class ObjectSystem final
{
    friend class Engine;
    friend class ObjectPool;

  public:
    ObjectSystem(const ObjectSystem&) = delete;
    ObjectSystem& operator=(const ObjectSystem&) = delete;

    static ObjectSystem& GetInstance();

    ObjectManagerHandle CreateObjectManager();
    void DestroyObjectManager(ObjectManagerHandle hObjManager);

  private:
    ObjectSystem();
    ~ObjectSystem();

    static void createInstance();
    static void deleteInstance();

    bool init();
    void shutdown();

    void registerObject(ObjectHandle<RObject> hObj);
    void unregisterObject(uint64_t objID);

    void expandObjectManagers();

    TypedObjectPool<ObjectHandle<RObject>> mhObjects;
    TypedObjectPool<ObjectManager> mObjectManagerPool;

    static ObjectSystem* spInstance;
};
} // namespace ho