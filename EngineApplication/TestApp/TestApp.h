#pragma once

#include "../IEngineApplication.h"

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
};
} // namespace ho