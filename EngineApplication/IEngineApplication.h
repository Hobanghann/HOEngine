#pragma once

#include <memory>

#include "Config.h"

namespace ho
{
class IEngineWindow;

class IEngineApplication
{
  public:
    static std::unique_ptr<IEngineApplication> CreateApplication(eEngineApplicationType appType);

    virtual ~IEngineApplication();

    virtual bool OnInit();
    virtual bool OnPreUpdate(float deltaTime);
    virtual bool OnUpdate(float deltaTime);
    virtual bool OnPostUpdate(float deltaTime);
    virtual bool OnRender();
    virtual void OnShutdown();

  protected:
    static const int32_t sMaxWindowCount = 8;

    IEngineApplication();
    // Window callback functions are called sequentially in array order.
    std::unique_ptr<IEngineWindow> mpWindows[sMaxWindowCount];
};
} // namespace ho