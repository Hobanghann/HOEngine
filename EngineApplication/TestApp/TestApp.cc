#include "TestApp.h"

#include "Subsystems/UI/UISystem.h"
#include "TestAppMainWindow.h"

namespace ho
{
TestApp::TestApp()
  : IEngineApplication()
{
    mpWindows[0] = std::make_unique<TestAppMainWindow>();
}

TestApp::~TestApp() = default;

bool TestApp::OnInit()
{
    if (!IEngineApplication::OnInit())
    {
        return false;
    }
    mpWindows[0]->SetOpen(true);
    return true;
}

bool TestApp::OnPreUpdate(float deltaTime)
{
    if (!IEngineApplication::OnPreUpdate(deltaTime))
    {
        return false;
    }
    return true;
}

bool TestApp::OnUpdate(float deltaTime)
{
    if (!IEngineApplication::OnUpdate(deltaTime))
    {
        return false;
    }
    return true;
}

bool TestApp::OnPostUpdate(float deltaTime)
{
    if (!IEngineApplication::OnPostUpdate(deltaTime))
    {
        return false;
    }
    return true;
}

bool TestApp::OnRender()
{
    if (!IEngineApplication::OnRender())
    {
        return false;
    }
    return true;
}

void TestApp::OnShutdown()
{
    IEngineApplication::OnShutdown();
}
} // namespace ho