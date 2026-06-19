#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#define UNICODE
#define _UNICODE

#define NOMINMAX

#include <objbase.h>
#include <shlobj.h>
#include <windows.h>

#include "../IPlatformApplication.h"

namespace ho
{
class IWin32Application : public IPlatformApplication
{
  public:
    ~IWin32Application() override = default;
    IWin32Application(const IWin32Application&) = delete;
    IWin32Application& operator=(const IWin32Application&) = delete;

    bool Init(const std::wstring& mainWindowNameStr, int32_t mainWindowWidth, int32_t mainWindowHeigh) override = 0;

    void BeginFrame() override = 0;

    bool ProcessPlatformMessages() override;

    bool ShowOpenFileDialog(Path* pOutPaths,
                            const std::wstring& titleStr,
                            const std::wstring* pFilterNamesStr,
                            const std::wstring* pFilterExtensionsStr,
                            int32_t filterCount,
                            const std::wstring& initialDirPathStr) override;

    bool ShowOpenFilesDialog(Path* pOutPaths,
                             int32_t* pOutPathCount,
                             int32_t maxOutPathCount,
                             const std::wstring& titleStr,
                             const std::wstring* pFilterNamesStr,
                             const std::wstring* pFilterExtensionsStr,
                             int32_t filterCount,
                             const std::wstring& initialDirPathStr) override;

    void Shutdown() override = 0;

    void* GetNativeHandle() const override
    {
        return reinterpret_cast<void*>(mhApp);
    }

  protected:
    IWin32Application(HINSTANCE hApp);

    static LRESULT CALLBACK wndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);

    HINSTANCE mhApp;
    std::wstring mMainWndNameStr;
    HWND mhMainWnd;
};
} // namespace ho