#include "TestApp.h"

#include <imgui.h>

#include "Subsystems/UI/UISystem.h"

namespace ho
{
TestApp::TestApp()
  : IEngineApplication("HOEngine: Test App")
{
}

bool TestApp::OnInit()
{
    mpMainWindow = std::make_unique<TestAppMainWindow>();
    mpMainWindow->SetOpen(true);

    return true;
}

bool TestApp::OnPreUpdate()
{
    UISystem::GetInstance().SubmitUIDrawable(mpMainWindow.get());
    return true;
}

bool TestApp::OnUpdate()
{
    return true;
}

bool TestApp::OnPostUpdate()
{
    return true;
}

bool TestApp::OnRender()
{
    return true;
}

void TestApp::OnShutdown() {}
} // namespace ho