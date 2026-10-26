#include "Engine.h"

#include "Core/IO/Path.h"
#include "Core/Log/Logger.h"
#include "EngineApplication/IEngineApplication.h"
#include "Platforms/IPlatformApplication.h"
#include "Scene/SceneManager.h"
#include "Subsystems/Asset/AssetSystem.h"
#include "Subsystems/Object/ObjectSystem.h"
#include "Subsystems/Reflection/ReflectionSystem.h"
#include "Subsystems/Rendering/RenderingSystem.h"
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

    // Create & Init Subsystems
    ReflectionSystem::createInstance();
    if (!ReflectionSystem::GetInstance().init())
    {
        HO_ASSERT(false, "Failed to initialize reflection system.");
        return false;
    }

    ObjectSystem::createInstance();
    if (!ObjectSystem::GetInstance().init())
    {
        HO_ASSERT(false, "Failed to initialize object system.");
        return false;
    }

    AssetSystem::createInstance();
    if (!AssetSystem::GetInstance().init())
    {
        HO_ASSERT(false, "Failed to initialize asset system.");
        return false;
    }
    RenderingSystem::createInstance(param.GraphicsAPI);
    if (!RenderingSystem::GetInstance().init())
    {
        HO_ASSERT(false, "Failed to initialize rendering system.");
        return false;
    }

    UISystem::createInstance();
    if (!UISystem::GetInstance().init())
    {
        HO_ASSERT(false, "Failed to initialize UI system.");
        return false;
    }

    // Create & Init SceneManager
    SceneManager::createInstance();
    if (!SceneManager::GetInstance().init())
    {
        HO_ASSERT(false, "Failed to initialize scene manager.");
        return false;
    }

    // Create main window
    std::string titleStr;
    switch (param.ApplicationType)
    {
        case eEngineApplicationType::TestApp:
            titleStr = "HOEngine: TestApp";
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

    switch (param.GraphicsAPI)
    {
        case eGraphicsAPI::GL:
            titleStr.append(" -  OpenGL 4.6");
            break;
        default:
            HO_ASSERT(false, "Invalid graphics API.");
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
    RenderingSystem::GetInstance().run();

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
        updateWindowTitleStats();

        AssetSystem::GetInstance().uploadPendingAssets();
        RenderingSystem::GetInstance().BeginFrame();
        UISystem::GetInstance().BeginFrame();

        if (!mpRunningApp->OnPreUpdate(mEngineTimer.GetDeltaTime()))
        {
            mbRunning = false;
        }

        UISystem::GetInstance().draw();

        if (!mpRunningApp->OnUpdate(mEngineTimer.GetDeltaTime()))
        {
            mbRunning = false;
        }

        SceneManager::GetInstance().updateScenes(mEngineTimer.GetDeltaTime());

        if (!mpRunningApp->OnPostUpdate(mEngineTimer.GetDeltaTime()))
        {
            mbRunning = false;
        }

        // ===========================================
        // Render
        // ===========================================

        UISystem::GetInstance().submitDrawCommandForUI();

        if (!mpRunningApp->OnRender())
        {
            mbRunning = false;
        }

        RenderingSystem::GetInstance().EndFrame();
        RenderingSystem::GetInstance().swapRenderQueues(mFrameCount);

        SceneManager::GetInstance().collectGarbage();
        SceneManager::GetInstance().deleteDestroyedScenes();
        AssetSystem::GetInstance().deletePendingAssets();
        ObjectSystem::GetInstance().expandObjectManagers();

        IPlatformApplication::GetInstance().DestroyPendingSubWindows();
    }
}

void Engine::Shutdown()
{
    mpRunningApp->OnShutdown();
    mpRunningApp.reset();

    SceneManager::GetInstance().shutdown();
    SceneManager::deleteInstance();

    UISystem::GetInstance().shutdown();
    RenderingSystem::GetInstance().shutdown();
    AssetSystem::GetInstance().shutdown();
    ObjectSystem::GetInstance().shutdown();
    ReflectionSystem::GetInstance().shutdown();

    UISystem::deleteInstance();
    RenderingSystem::deleteInstance();
    AssetSystem::deleteInstance();
    ObjectSystem::deleteInstance();
    ReflectionSystem::deleteInstance();

    IPlatformApplication::GetInstance().Shutdown();

    Logger::Shutdown();
}

void Engine::updateWindowTitleStats()
{
    ++mStatsFrameCount;
    mStatsElapsedTime += mEngineTimer.GetDeltaTime();

    if (mStatsElapsedTime >= 0.5f)
    {
        const int32_t fps = static_cast<int32_t>(
            math::Round(static_cast<float>(mStatsFrameCount) / static_cast<float>(mStatsElapsedTime)));
        const float frameTimeMs =
            (static_cast<float>(mStatsElapsedTime) / static_cast<float>(mStatsFrameCount)) * 1000.0f;

        char buf[100] = {};
        snprintf(buf, sizeof(buf), "FPS: %d (%.2f ms)", fps, frameTimeMs);
        IPlatformApplication::GetInstance().GetMainWindow()->SetDynamicTitle(buf);

        mStatsFrameCount = 0;
        mStatsElapsedTime = 0.0f;
    }
}
} // namespace ho