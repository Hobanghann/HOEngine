#include "Engine.h"

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

    // Create engine application
    mpRunningApp = IEngineApplication::CreateApplication(param.ApplicationType);

    if (!mpRunningApp)
    {
        HO_ASSERT(false, "Failed to create engine application.");
        return false;
    }

    // Init platform application
    if (!IPlatformApplication::GetInstance().Init(
            mpRunningApp->GetTitleStr(), param.MainWindowWidth, param.MainWindowHeight))
    {
        HO_ASSERT(false, "Failed to create platform application.");
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

    // Init engine application
    IPlatformApplication::GetInstance().GetMainWindow()->ActivateContext();
    if (!mpRunningApp->OnInit())
    {
        HO_ASSERT(false, "Failed to initialize engine application.");
        return false;
    }
    IPlatformApplication::GetInstance().GetMainWindow()->DeactivateContext();

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
        IPlatformApplication::GetInstance().BeginFrame();

        ++mFrameCount;

        if (!IPlatformApplication::GetInstance().ProcessPlatformMessages())
        {
            mbRunning = false;
            break;
        }

        mEngineTimer.Tick();

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