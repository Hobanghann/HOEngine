#pragma once

#include "../IWin32Window.h"

namespace ho
{
class Win32WindowGL final : public IWin32Window
{
  public:
    Win32WindowGL(int32_t posX,
                  int32_t posY,
                  int32_t titleBarHeight,
                  int32_t clientWidth,
                  int32_t clientHeight,
                  HWND hWnd,
                  HGLRC hGlContext)
      : IWin32Window(posX, posY, titleBarHeight, clientWidth, clientHeight, hWnd)
      , mhGlContext(hGlContext)
    {
    }

    ~Win32WindowGL() override
    {
        ::wglMakeCurrent(nullptr, nullptr);
    }

    void ActivateContext() const override
    {
        const BOOL bSuccess = ::wglMakeCurrent(mhDC, mhGlContext);
        HO_ASSERT(bSuccess != FALSE, "wglMakeCurrent failed.");
        (void)bSuccess;
    }

    void DeactivateContext() const override
    {
        const BOOL bSuccess = ::wglMakeCurrent(mhDC, nullptr);
        HO_ASSERT(bSuccess != FALSE, "wglMakeCurrent failed.");
        (void)bSuccess;
    }

  private:
    HGLRC mhGlContext;
};
} // namespace ho