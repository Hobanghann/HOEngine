#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#define UNICODE
#define _UNICODE

#define NOMINMAX

#include <string>
#include <windows.h>

#include "../IPlatformApplication.h"
#include "Macros.h"

namespace ho
{
class IWin32Application : public IPlatformApplication
{
  public:
    ~IWin32Application() override = default;
    IWin32Application(const IWin32Application&) = delete;
    IWin32Application& operator=(const IWin32Application&) = delete;

    bool Init(const std::string& iconPathStr) override = 0;

    bool CreateMainWindow(const std::string& mainWindowTitleStr,
                          int32_t titleBarHeight,
                          int32_t mainWindowClientWidth,
                          int32_t mainWindowClientHeight) override = 0;

    void BeginFrame() override = 0;

    bool ProcessPlatformMessages() override;

    bool ShowOpenFileDialog(std::string* pOutPathStr,
                            const std::string& titleStr,
                            const std::string* pFilterNamesStr,
                            const std::string* pFilterExtensionsStr,
                            int32_t filterCount,
                            const std::string& initialDirPathStr) override;

    bool ShowOpenFilesDialog(std::string* pOutPathsStr,
                             int32_t* pOutPathCount,
                             int32_t maxOutPathCount,
                             const std::string& titleStr,
                             const std::string* pFilterNamesStr,
                             const std::string* pFilterExtensionsStr,
                             int32_t filterCount,
                             const std::string& initialDirPathStr) override;

    void Shutdown() override = 0;

    void* GetNativeHandle() const override
    {
        return reinterpret_cast<void*>(mhApp);
    }

  protected:
    IWin32Application(HINSTANCE hApp);
    void uploadIconTexture(const std::string& iconPathStr) override = 0;
    void loadWindowsIcon(const std::string& iconPathStr);

    static std::wstring toUTF16(const std::string& utf8Str);

    static std::string toUTF8(const std::wstring& utf16Str);

    static LRESULT CALLBACK wndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);

    HINSTANCE mhApp;
    std::string mMainWndNameStr;
    HWND mhMainWnd;
    HICON mhIcon;
};
} // namespace ho