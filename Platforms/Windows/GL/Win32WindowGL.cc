#include "Win32WindowGL.h"

#include <glad/glad.h>
#include <imgui.h>

#include "Macros.h"

namespace ho
{
HGLRC Win32WindowGL::shGLRC = nullptr;

Win32WindowGL::Win32WindowGL(HWND hWnd, int32_t clientWidth, int32_t clientHeight)
  : mhWnd(hWnd)
  , mClientWidth(clientWidth)
  , mClientHeight(clientHeight)
{
    HO_ASSERT(mhWnd != nullptr, "Invalid Window Handle.");

    mhDC = ::GetDC(mhWnd);
    HO_ASSERT(mhWnd, "Failed to Get DC");

    PIXELFORMATDESCRIPTOR pfd = {0};
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

    if (!shGLRC)
    {
        shGLRC = wglCreateContext(mhDC);
        HO_ASSERT(shGLRC != nullptr, "");
    }
}

Win32WindowGL::~Win32WindowGL()
{
    wglMakeCurrent(nullptr, nullptr);
    ::ReleaseDC(mhWnd, mhDC);
}

bool Win32WindowGL::DeleteGLContext()
{
    return wglDeleteContext(shGLRC);
}

int32_t Win32WindowGL::GetClientWidth() const
{
    return mClientWidth;
}

int32_t Win32WindowGL::GetClientHeight() const
{
    return mClientHeight;
}

bool Win32WindowGL::Resize(int32_t clientWidth, int32_t clientHeight)
{
    mClientWidth = clientWidth;
    mClientHeight = clientHeight;
    return true;
}

bool Win32WindowGL::Present() const
{
    return ::SwapBuffers(mhDC);
}

bool Win32WindowGL::MakeCurrent() const
{
    return wglMakeCurrent(mhDC, shGLRC);
}

void Win32WindowGL::Hook_CreateWindow(ImGuiViewport* viewport)
{
    HO_ASSERT(viewport->RendererUserData == nullptr, "");

    Win32WindowGL* window = nullptr;
    window = new Win32WindowGL(
        (HWND)viewport->PlatformHandle, static_cast<int32_t>(viewport->Size.x), static_cast<int32_t>(viewport->Size.y));
    viewport->RendererUserData = reinterpret_cast<void*>(window);
}

void Win32WindowGL::Hook_DestroyWindow(ImGuiViewport* viewport)
{
    if (viewport->RendererUserData != nullptr)
    {
        const Win32WindowGL* window = (Win32WindowGL*)viewport->RendererUserData;
        delete window;
        viewport->RendererUserData = nullptr;
    }
}

void Win32WindowGL::Hook_RenderWindow(ImGuiViewport* viewport, void* unused)
{
    // Activate the platform window DC in the OpenGL rendering context
    if (const Win32WindowGL* window = (Win32WindowGL*)viewport->RendererUserData)
    {
        window->MakeCurrent();
    }
    (void)unused;
}

void Win32WindowGL::Hook_SwapBuffers(ImGuiViewport* viewport, void* unused)
{
    if (const Win32WindowGL* window = (Win32WindowGL*)viewport->RendererUserData)
    {
        window->Present();
    }
    (void)unused;
}
} // namespace ho