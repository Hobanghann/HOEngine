#include "EngineMain.h"

#include "Core/Thread/Thread.h"
#include "Engine/Engine.h"

namespace ho
{
int EngineMain(const EngineMainParam& param)
{
    Thread::SetCurrentName("Main Thread");

    EngineInitParam engineInitParam;

#if defined(HO_APP_TEST_APP)
    engineInitParam.ApplicationType = eEngineApplicationType::TestApp;
#elif defined(HO_APP_MODEL_VIEWER)
    engineInitParam.ApplicationType = eEngineApplicationType::ModelViewer;
#elif defined(HO_APP_EDITOR)
    engineInitParam.ApplicationType = eEngineApplicationType::Editor;
#elif defined(HO_APP_GAME)
    engineInitParam.ApplicationType = eEngineApplicationType::Game;
#endif

    engineInitParam.MainWindowWidth = 1280;
    engineInitParam.MainWindowHeight = 720;

    Engine engine;
    if (engine.Init(engineInitParam))
    {
        engine.Run();
        engine.Shutdown();
    }
    return 0;
}
} // namespace ho