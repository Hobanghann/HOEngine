#pragma once

#include <memory>
#include <string>

#include "IPlatformWindow.h"
#include "Macros.h"

namespace ho
{
class Path;

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

    virtual bool Init(const std::string& iconPathStr) = 0;

    virtual bool CreateMainWindow(const std::string& mainWindowTitleStr,
                                  int32_t titleBarHeight,
                                  int32_t mainWindowClientWidth,
                                  int32_t mainWindowClientHeight) = 0;

    virtual void BeginFrame() = 0;

    virtual bool ProcessPlatformMessages() = 0;

    virtual bool ShowOpenFileDialog(std::string* pOutPathStr,
                                    const std::string& titleStr,
                                    const std::string* pFilterNamesStr,
                                    const std::string* pFilterExtensionsStr,
                                    int32_t filterCount,
                                    const std::string& initialDirPathStr) = 0;

    virtual bool ShowOpenFilesDialog(std::string* pOutPathsStr,
                                     int32_t* pOutPathCount,
                                     int32_t maxOutPathCount,
                                     const std::string& titleStr,
                                     const std::string* pFilterNamesStr,
                                     const std::string* pFilterExtensionsStr,
                                     int32_t filterCount,
                                     const std::string& initialDirPathStr) = 0;

    virtual void Shutdown() = 0;

    virtual void* GetNativeHandle() const = 0;

    const IPlatformWindow* GetMainWindow() const
    {
        return spMainWindow.get();
    }

    void* GetIconNativeHandle() const
    {
        return mIconNativeHandle;
    }

    int32_t GetIconImageWidth() const
    {
        return mIconWidth;
    }

    int32_t GetIconHeight() const
    {
        return mIconHeight;
    }

    void SetPaused(bool bPaused)
    {
        mbPaused = bPaused;
    }

    bool IsPaused() const
    {
        return mbPaused;
    }

  protected:
    IPlatformApplication() = default;

    virtual void uploadIconTexture(const std::string& iconPathStr) = 0;

    static std::unique_ptr<IPlatformWindow> spMainWindow;

    void* mIconNativeHandle = nullptr;
    int32_t mIconWidth = 0;
    int32_t mIconHeight = 0;

    bool mbPaused = false;

    static IPlatformApplication* spInstance;
};
} // namespace ho
