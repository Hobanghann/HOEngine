#pragma once

#include <memory>

#include "../IEngineApplication.h"

namespace ho
{
class TestAppMainWindow;

class TestApp final : public IEngineApplication
{
  public:
    TestApp();
    ~TestApp() override;

    bool OnInit() override;
    bool OnPreUpdate(float deltaTime) override;
    bool OnUpdate(float deltaTime) override;
    bool OnPostUpdate(float deltaTime) override;
    bool OnRender() override;
    void OnShutdown() override;

  private:
    std::unique_ptr<TestAppMainWindow> mpMainWindow;
};
} // namespace ho