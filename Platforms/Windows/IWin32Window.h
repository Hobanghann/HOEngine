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
    IWin32Window(int32_t clientWidth, int32_t clientHeight, HWND hWnd)
      : IPlatformWindow(clientWidth, clientHeight)
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

    virtual void ActivateContext() const = 0;

    virtual void DeactivateContext() const = 0;

    void Present() const
    {
        ::SwapBuffers(mhDC);
    }

    void SetTitle(const std::wstring& titleStr) const
    {
        if (!SetWindowTextW(mhWnd, titleStr.c_str()))
        {
            HO_ASSERT(false, "Failed to set title text.");
        }
    }

  protected:
    HWND mhWnd = nullptr;
    HDC mhDC = nullptr;
};
} // namespace ho