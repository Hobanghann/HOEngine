#include "Engine.h"

#include "Core/IO/Path.h"
#include "Core/Log/Logger.h"
#include "EngineApplication/IEngineApplication.h"
#include "Platforms/IPlatformApplication.h"
#include "Subsystems/Asset/AssetSystem.h"
#include "Subsystems/Rendering/IRenderingSystem.h"
#include "Subsystems/UI/UISystem.h"

namespace ho
{
Engine::Engine() = default;
Engine::~Engine() = default;

bool Engine::Init(const EngineInitParam& param)
{
    Logger::Init();

    // Init platform application
    Path iconPath;
    switch (param.ApplicationType)
    {
        case eEngineApplicationType::TestApp:
            iconPath = Path(std::string("Platforms/Resources/Icons/TestAppIcon64.png"));
            break;
        case eEngineApplicationType::ModelViewer:
            iconPath = Path(std::string("Platforms/Resources/Icons/ModelViewerIcon64.png"));
            break;
        case eEngineApplicationType::Editor:
            iconPath = Path(std::string("Platforms/Resources/Icons/EditorIcon64.png"));
            break;
        case eEngineApplicationType::Game:
            iconPath = Path(std::string("Platforms/Resources/Icons/GameIcon64.png"));
            break;
        default:
            HO_ASSERT(false, "Invalid application type.");
            break;
    }
    iconPath.ResolveProjectPath();

    if (!IPlatformApplication::GetInstance().Init(iconPath.ToString()))
    {
        HO_ASSERT(false, "Failed to initialize platform application.");
        return false;
    }

    // Create subsystems
    AssetSystem::createInstance();
    IRenderingSystem::createInstance(eGraphicsAPI::GL);
    UISystem::createInstance();

    // Init subsystems
    if (!AssetSystem::GetInstance().init())
    {
        HO_ASSERT(false, "Failed to initialize asset system.");
        return false;
    }
    if (!IRenderingSystem::GetInstance().init())
    {
        HO_ASSERT(false, "Failed to initialize rendering system.");
        return false;
    }

    if (!UISystem::GetInstance().init())
    {
        HO_ASSERT(false, "Failed to initialize UI system.");
        return false;
    }

    // Create main window
    std::string titleStr;
    switch (param.ApplicationType)
    {
        case eEngineApplicationType::TestApp:
            titleStr = "HOEngine: TestApp";
            break;
        case eEngineApplicationType::ModelViewer:
            titleStr = "HOEngine: ModelViewer";
            break;
        case eEngineApplicationType::Editor:
            titleStr = "HOEngine: Editor";
            break;
        case eEngineApplicationType::Game:
            titleStr = "HOEngine: Game";
            break;
        default:
            HO_ASSERT(false, "Invalid application type.");
            break;
    }
    if (!IPlatformApplication::GetInstance().CreateMainWindow(
            titleStr, UISystem::GetInstance().GetTitleBarTheme().Height, param.MainWindowWidth, param.MainWindowHeight))
    {
        HO_ASSERT(false, "Failed to create main window.");
        return false;
    }

    // Create engine application
    mpRunningApp = IEngineApplication::CreateApplication(param.ApplicationType);

    if (!mpRunningApp)
    {
        HO_ASSERT(false, "Failed to create engine application.");
        return false;
    }

    // Init engine application
    if (!mpRunningApp->OnInit())
    {
        HO_ASSERT(false, "Failed to initialize engine application.");
        return false;
    }

    // Run render thread
    IRenderingSystem::GetInstance().run();

    HO_LOG_INFO("Engine Init Complete.");

    return true;
}

void Engine::Run()
{
    mEngineTimer.Reset();

    while (mbRunning)
    {
        if (!IPlatformApplication::GetInstance().ProcessPlatformMessages())
        {
            mbRunning = false;
            break;
        }

        if (IPlatformApplication::GetInstance().IsPaused())
        {
            Thread::Sleep(16);
            continue;
        }

        IPlatformApplication::GetInstance().BeginFrame();

        mEngineTimer.Tick();
        ++mFrameCount;

        // ===========================================
        // Update
        // ===========================================
        if (!mpRunningApp->OnPreUpdate())
        {
            mbRunning = false;
        }

        UISystem::GetInstance().submitDrawCommandForUI();

        if (!mpRunningApp->OnUpdate())
        {
            mbRunning = false;
        }
        if (!mpRunningApp->OnPostUpdate())
        {
            mbRunning = false;
        }

        // ===========================================
        // Render
        // ===========================================

        if (!mpRunningApp->OnRender())
        {
            mbRunning = false;
        }

        IRenderingSystem::GetInstance().swapRenderQueues(mFrameCount);
    }
}

void Engine::Shutdown()
{
    mpRunningApp->OnShutdown();
    mpRunningApp.reset();

    UISystem::GetInstance().shutdown();
    IRenderingSystem::GetInstance().shutdown();
    AssetSystem::GetInstance().shutdown();

    UISystem::deleteInstance();
    IRenderingSystem::deleteInstance();
    AssetSystem::deleteInstance();

    IPlatformApplication::GetInstance().Shutdown();

    Logger::Shutdown();
}
} // namespace ho