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

    void DrawUI() override;

  private:
    StringHandle mhMainFrameBufferName;
};
} // namespace ho