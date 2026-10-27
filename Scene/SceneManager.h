#pragma once

#include <unordered_map>

#include "Core/Templates/FixedQueue.h"
#include "Scene.h"
#include "Subsystems/Object/ObjectManager.h"

namespace ho
{
class SceneManager final
{
    friend class Engine;

  public:
    SceneManager(const SceneManager&) = delete;
    SceneManager& operator=(const SceneManager&) = delete;

    static SceneManager& GetInstance();

    [[nodiscard]] ObjectHandle<RScene> CreateScene(StringHandle hName);

    void DestroyScene(ObjectHandle<RScene> hScene);

    [[nodiscard]] bool HasScene(StringHandle hName) const;

    [[nodiscard]] ObjectHandle<RScene> GetSceneByName(StringHandle hName);

    [[nodiscard]] ObjectHandle<const RScene> GetSceneByName(StringHandle hName) const;

    [[nodiscard]] ObjectContainer<RScene> GetScenes();

    [[nodiscard]] ObjectContainer<const RScene> GetScenes() const;

    [[nodiscard]] int32_t GetSceneCount() const;

  private:
    static const int32_t sDestroyedSceneQueueSize = 64;

    SceneManager();
    ~SceneManager();

    static void createInstance();
    static void deleteInstance();

    bool init();
    void shutdown();

    void updateScenes(float deltaTime);
    void collectGarbage();
    void deleteDestroyedScenes();

    ObjectManagerHandle mhScenePool;
    std::unordered_map<StringHandle, ObjectHandle<RScene>> mNameToSceneMap;
    FixedQueue<ObjectHandle<RScene>> mpDestroyedSceneQueue;

    static SceneManager* spInstance;
};
} // namespace ho