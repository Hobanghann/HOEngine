#include "IEngineApplication.h"

#include "Editor/Editor.h"
#include "IEngineWindow.h"
#include "Macros.h"
#include "Subsystems/UI/UISystem.h"
#include "TestApp/TestApp.h"

namespace ho
{
std::unique_ptr<IEngineApplication> IEngineApplication::CreateApplication(eEngineApplicationType appType)
{
    switch (appType)
    {
        case eEngineApplicationType::TestApp:
            return std::make_unique<TestApp>();
        case eEngineApplicationType::Editor:
            return std::make_unique<Editor>();
        // case eEngineApplicationType::Game:
        //     HO_ASSERT(false, "Currently not supported.");
        //     return nullptr;
        default:
            HO_ASSERT(false, "Invalid application type.");
            return nullptr;
    }
}

IEngineApplication::IEngineApplication() = default;

IEngineApplication::~IEngineApplication() = default;

bool IEngineApplication::OnInit()
{
    bool bSucceed = true;
    for (const auto& mpWindow : mpWindows)
    {
        if (mpWindow)
        {
            bSucceed &= mpWindow->OnInit();
            HO_ASSERT(bSucceed, "Failed to initialize window.");
        }
    }
    return bSucceed;
}

bool IEngineApplication::OnPreUpdate(float deltaTime)
{
    for (const auto& mpWindow : mpWindows)
    {
        if (mpWindow && mpWindow->IsOpen())
        {
            UISystem::GetInstance().SubmitUIDrawable(mpWindow.get());
        }
    }

    bool bSucceed = true;
    for (const auto& mpWindow : mpWindows)
    {
        if (mpWindow)
        {
            bSucceed &= mpWindow->OnPreUpdate(deltaTime);
            HO_ASSERT(bSucceed, "Failed to update window.");
        }
    }
    return bSucceed;
}

bool IEngineApplication::OnUpdate(float deltaTime)
{
    bool bSucceed = true;
    for (const auto& mpWindow : mpWindows)
    {
        if (mpWindow)
        {
            bSucceed &= mpWindow->OnUpdate(deltaTime);
            HO_ASSERT(bSucceed, "Failed to update window.");
        }
    }
    return bSucceed;
}

bool IEngineApplication::OnPostUpdate(float deltaTime)
{
    bool bSucceed = true;
    for (const auto& mpWindow : mpWindows)
    {
        if (mpWindow)
        {
            bSucceed &= mpWindow->OnPostUpdate(deltaTime);
            HO_ASSERT(bSucceed, "Failed to update window.");
        }
    }
    return bSucceed;
}

bool IEngineApplication::OnRender()
{
    bool bSucceed = true;
    for (const auto& mpWindow : mpWindows)
    {
        if (mpWindow)
        {
            bSucceed &= mpWindow->OnRender();
            HO_ASSERT(bSucceed, "Failed to render window.");
        }
    }
    return bSucceed;
}

void IEngineApplication::OnShutdown()
{
    for (const auto& mpWindow : mpWindows)
    {
        if (mpWindow)
        {
            mpWindow->OnShutdown();
        }
    }
}
} // namespace ho