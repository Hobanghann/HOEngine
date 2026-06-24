#pragma once

#include <memory>

#include "../IEngineApplication.h"
#include "TestAppMainWindow.h"

namespace ho
{
class TestApp final : public IEngineApplication
{
  public:
    TestApp();
    ~TestApp() override = default;

    bool OnInit() override;
    bool OnPreUpdate() override;
    bool OnUpdate() override;
    bool OnPostUpdate() override;
    bool OnRender() override;
    void OnShutdown() override;

  private:
    std::unique_ptr<TestAppMainWindow> mpMainWindow;
};
} // namespace ho