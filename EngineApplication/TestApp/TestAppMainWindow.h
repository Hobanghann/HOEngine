#pragma once

#include "../IEngineWindow.h"

namespace ho
{
class TestAppMainWindow final : public IEngineWindow
{
  public:
    TestAppMainWindow();

    TestAppMainWindow(const TestAppMainWindow&) = delete;
    TestAppMainWindow& operator=(const TestAppMainWindow&) = delete;

    ~TestAppMainWindow() override = default;

    bool OnInit() override;
    bool OnPreUpdate(float deltaTime) override;
    bool OnUpdate(float deltaTime) override;
    bool OnPostUpdate(float deltaTime) override;
    bool OnRender() override;
    void OnShutdown() override;

    void DrawUI() override;

  private:
};
} // namespace ho