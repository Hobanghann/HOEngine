#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#define UNICODE
#define _UNICODE

#define NOMINMAX

#include <windows.h>

#include "../IPlatformWindow.h"
#include "Macros.h"

namespace ho
{
class IWin32Window : public IPlatformWindow
{
  public:
    IWin32Window(
        int32_t posX, int32_t posY, int32_t titleBarHeight, int32_t clientWidth, int32_t clientHeight, HWND hWnd)
      : IPlatformWindow(posX, posY, titleBarHeight, clientWidth, clientHeight)
      , mhWnd(hWnd)
    {
        HO_ASSERT(mhWnd != nullptr, "Invalid Window Handle.");

        mhDC = ::GetDC(mhWnd);
        HO_ASSERT(mhWnd, "Failed to Get DC");

        PIXELFORMATDESCRIPTOR pfd{};
        pfd.nSize = sizeof(pfd);
        pfd.nVersion = 1;
        pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER | PFD_SWAP_EXCHANGE;
        pfd.iPixelType = PFD_TYPE_RGBA;
        pfd.cColorBits = 32;

        const int pixelFormat = ::ChoosePixelFormat(mhDC, &pfd);
        HO_ASSERT(pixelFormat != 0, "Current system not supports such pixel format.");
        const BOOL bSuccess = ::SetPixelFormat(mhDC, pixelFormat, &pfd);
        HO_ASSERT(bSuccess != FALSE, "Failed to setting pixel format.");
        (void)bSuccess;
    }

    ~IWin32Window() override
    {
        ::ReleaseDC(mhWnd, mhDC);
    }

    void* GetNativeHandle() const override
    {
        return reinterpret_cast<void*>(mhWnd);
    }

    void ActivateContext() const override = 0;

    void DeactivateContext() const override = 0;

    void Present() const override
    {
        ::SwapBuffers(mhDC);
    }

    void ProcessWindowDragging() const override;

    void Maximize() const override
    {
        ::ShowWindow(mhWnd, SW_MAXIMIZE);
    }

    void Minimize() const override
    {
        ::ShowWindow(mhWnd, SW_MINIMIZE);
    }

    void Restore() const override
    {
        ::ShowWindow(mhWnd, SW_RESTORE);
    }

    void Close() const override
    {
        ::PostMessageW(mhWnd, WM_CLOSE, 0, 0);
    }

  protected:
    HWND mhWnd = nullptr;
    HDC mhDC = nullptr;
};
} // namespace ho