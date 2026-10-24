#pragma once

#include <memory>

#include "../IEngineApplication.h"

namespace ho
{
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
};
} // namespace ho