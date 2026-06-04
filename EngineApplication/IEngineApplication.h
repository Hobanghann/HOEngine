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

    std::wstring GetTitleStr() const
    {
        return mTitleStr;
    }

  protected:
    IEngineApplication(const std::wstring& titleStr);
    std::wstring mTitleStr;
};
} // namespace ho