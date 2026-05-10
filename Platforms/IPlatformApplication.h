#pragma once

#include <string>

#include "Macros.h"

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

    IPlatformApplication(const IPlatformApplication&) = delete;
    IPlatformApplication& operator=(const IPlatformApplication&) = delete;

    virtual ~IPlatformApplication() = default;

    virtual bool Init(const std::wstring& mainWindowName, int32_t mainWindowWidth, int32_t mainWindowHeight) = 0;

    virtual bool BeginFrame() = 0;

    virtual bool ProcessPlatformMessages() = 0;

    virtual bool EndFrame() = 0;

    virtual bool Shutdown() = 0;

    int32_t GetMainWindowWidth() const;
    int32_t GetMainWindowHeight() const;

    virtual bool SetWindowTitle(std::wstring& title) = 0;

  protected:
    IPlatformApplication() = default;

    void initImGuiFonts();

    static int32_t sMainWindowWidth;
    static int32_t sMainWindowHeight;

    static IPlatformApplication* spInstance;
};
} // namespace ho
