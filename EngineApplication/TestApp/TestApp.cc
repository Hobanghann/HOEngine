#include "TestApp.h"

namespace ho
{
TestApp::TestApp()
  : IEngineApplication(L"TestApp")
{
}

bool TestApp::OnInit()
{
    return true;
}

bool TestApp::OnPreUpdate()
{
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