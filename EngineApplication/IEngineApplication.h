#pragma once

#include <memory>
#include <string>

#include "Config.h"

namespace ho
{
class IEngineApplication
{
  public:
    static std::unique_ptr<IEngineApplication> CreateApplication(eEngineApplicationType appType);

    virtual ~IEngineApplication() = default;

    virtual bool OnInit() = 0;
    virtual bool OnPreUpdate() = 0;
    virtual bool OnUpdate() = 0;
    virtual bool OnPostUpdate() = 0;
    virtual bool OnRender() = 0;
    virtual void OnShutdown() = 0;

    std::string GetTitleStr() const
    {
        return mTitleStr;
    }

  protected:
    IEngineApplication(const std::string& titleStr);
    std::string mTitleStr;
};
} // namespace ho