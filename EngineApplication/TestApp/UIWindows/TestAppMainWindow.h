#pragma once

#include "Subsystems/UI/IUIWindow.h"

namespace ho
{
class TestAppMainWindow final : public IUIWindow
{
  public:
    TestAppMainWindow();

    TestAppMainWindow(const TestAppMainWindow&) = delete;
    TestAppMainWindow& operator=(const TestAppMainWindow&) = delete;

    ~TestAppMainWindow() override = default;

  private:
    void onGUI() override;
};
} // namespace ho