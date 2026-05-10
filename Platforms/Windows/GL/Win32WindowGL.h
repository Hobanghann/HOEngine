#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#define UNICODE
#define _UNICODE

#define NOMINMAX

#include <cstdint>
#include <windows.h>

class ImGuiViewport;

namespace ho
{
class Win32WindowGL final
{
  public:
    Win32WindowGL(HWND hWnd, int32_t clientWidth, int32_t clientHeight);
    ~Win32WindowGL();

    static bool DeleteGLContext();

    int32_t GetClientWidth() const;
    int32_t GetClientHeight() const;
    void Resize(int32_t clientWidth, int32_t clientHeight);
    void Present() const;
    void MakeCurrent() const;

    static void Hook_CreateWindow(ImGuiViewport* viewport);
    static void Hook_DestroyWindow(ImGuiViewport* viewport);
    static void Hook_RenderWindow(ImGuiViewport* viewport, void* unused);
    static void Hook_SwapBuffers(ImGuiViewport* viewport, void* unused);

  private:
    HWND mhWnd;
    HDC mhDC;
    int32_t mClientWidth;
    int32_t mClientHeight;

    static HGLRC shGLRC;
};
} // namespace ho