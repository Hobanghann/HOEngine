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

    virtual bool Init(const std::wstring& mainWindowNameStr, int32_t mainWindowWidth, int32_t mainWindowHeight) = 0;

    virtual void BeginFrame() = 0;

    virtual bool ProcessPlatformMessages() = 0;

    virtual bool ShowOpenFileDialog(Path* pOutPaths,
                                    const std::wstring& titleStr,
                                    const std::wstring* pFilterNamesStr,
                                    const std::wstring* pFilterExtensionsStr,
                                    int32_t filterCount,
                                    const std::wstring& initialDirPathStr) = 0;

    virtual bool ShowOpenFilesDialog(Path* pOutPaths,
                                     int32_t* pOutPathCount,
                                     int32_t maxOutPathCount,
                                     const std::wstring& titleStr,
                                     const std::wstring* pFilterNamesStr,
                                     const std::wstring* pFilterExtensionsStr,
                                     int32_t filterCount,
                                     const std::wstring& initialDirPathStr) = 0;

    virtual void Shutdown() = 0;

    virtual void* GetNativeHandle() const = 0;

    const IPlatformWindow* GetMainWindow() const;

  protected:
    IPlatformApplication() = default;

    void initImGuiFonts();

    static std::unique_ptr<IPlatformWindow> spMainWindow;

    static IPlatformApplication* spInstance;
};
} // namespace ho
