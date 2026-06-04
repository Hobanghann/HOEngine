#pragma once

#include "../IWin32Window.h"

namespace ho
{
class Win32WindowGL final : public IWin32Window
{
  public:
    Win32WindowGL(int32_t clientWidth, int32_t clientHeight, HWND hWnd, HGLRC hGlContext)
      : IWin32Window(clientWidth, clientHeight, hWnd)
      , mhGlContext(hGlContext)
    {
    }

    ~Win32WindowGL() override
    {
        ::wglMakeCurrent(nullptr, nullptr);
    }

    void ActivateContext() const override
    {
        ::wglMakeCurrent(mhDC, mhGlContext);
    }

    void DeactivateContext() const override
    {
        ::wglMakeCurrent(mhDC, nullptr);
    }

  private:
    HGLRC mhGlContext;
};
} // namespace ho