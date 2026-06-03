#pragma once

#include <memory>
#include <string>

#include "Macros.h"
#include "IPlatformWindow.h"

namespace ho
{

class IPlatformApplication
{
  public:
    static IPlatformApplication& GetInstance()
    {
        HO_ASSERT(spInstance, "PlatformApplication is not created");
        return *spInstance;
    }

    static void DeleteInstance()
    {
        HO_ASSERT(spInstance, "PlatformApplication is not created");
        delete spInstance;
        spInstance = nullptr;
    }

    virtual ~IPlatformApplication() = default;

    IPlatformApplication(const IPlatformApplication&) = delete;
    IPlatformApplication& operator=(const IPlatformApplication&) = delete;

    virtual bool Init(const std::wstring& mainWindowNameStr, int32_t mainWindowWidth, int32_t mainWindowHeight) = 0;

    virtual void BeginFrame() = 0;

    virtual bool ProcessPlatformMessages() = 0;

    virtual void Shutdown() = 0;

    const IPlatformWindow* GetMainWindow() const;

  protected:
    IPlatformApplication() = default;

    void initImGuiFonts();

    static std::unique_ptr<IPlatformWindow> spMainWindow;

    static IPlatformApplication* spInstance;
};
} // namespace ho
