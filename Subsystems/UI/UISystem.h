#pragma once

#include "Core/Templates/FixedQueue.h"
#include "IUIWindow.h"

namespace ho
{
class Engine;

class UISystem final
{
    friend Engine;

  public:
    UISystem(const UISystem&) = delete;
    UISystem& operator=(const UISystem&) = delete;

    static UISystem& GetInstance();

    void SubmitRenderWindow(IUIWindow* pWindow);

  private:
    static const int32_t sRenderWindowQueueSize = 32;

    UISystem();
    ~UISystem() = default;

    static void createInstance();

    static void deleteInstance();

    bool init();

    void submitDrawCommandForUI();

    void shutdown();

    FixedQueue<IUIWindow*> mRenderWindowQueue;

    static UISystem* spInstance;
};
} // namespace ho