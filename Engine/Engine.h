#pragma once

#include <memory>

#include "Config.h"
#include "Core/Time/GameTimer.h"

namespace ho
{
class IEngineApplication;

struct EngineInitParam
{
    eEngineApplicationType ApplicationType = eEngineApplicationType::None;
    int32_t MainWindowWidth = 0;
    int32_t MainWindowHeight = 0;
};

class Engine final
{
  public:
    Engine();
    ~Engine();

    bool Init(const EngineInitParam& param);

    void Run();

    void Shutdown();

  private:
    GameTimer mEngineTimer;

    std::unique_ptr<IEngineApplication> mpRunningApp = nullptr;

    uint64_t mFrameCount = 0;
    bool mbRunning = true;
};
} // namespace ho