#pragma once

#include <memory>

#include "../IWin32Application.h"

struct ImGuiViewport;

namespace ho
{
class Win32ApplicationGL final : public IWin32Application
{
  public:
    static void CreateInstance(HINSTANCE hApp);

    ~Win32ApplicationGL() override = default;
    Win32ApplicationGL(const Win32ApplicationGL&) = delete;
    Win32ApplicationGL& operator=(const Win32ApplicationGL&) = delete;

    bool Init(const std::string& pIconPathStr) override;

    bool CreateMainWindow(const std::string& mainWindowTitleStr,
                          int32_t titleBarHeight,
                          int32_t mainWindowClientWidth,
                          int32_t mainWindowClientHeight) override;

    void BeginFrame() override;

    void Shutdown() override;

    // Sets platform-specific OpenGL V-Sync configuration.
    // Placed at the platform application level since OpenGL lacks a universal cross-platform method for V-Sync control
    // (Windows uses WGL, Linux uses GLX, macOS uses NSOpenGLContext).
    // Renderering system should downcast the platform instance to call this method during API-specific initialization.
    void SetVSync(bool bEnable);

    static void RendererCreateWindow_Hook(ImGuiViewport* pViewport);
    static void RendererDestroyWindow_Hook(ImGuiViewport* pViewport);
    static void RendererSwapBuffers_Hook(ImGuiViewport* pViewport, void* pUnused);
    static void PlatformRenderWindow_Hook(ImGuiViewport* pViewport, void* pUnused);

  private:
    Win32ApplicationGL(HINSTANCE hApp);

    void uploadIconTexture(const std::string& path) override;

    static void (*pOriginalPlatformCreateWindow_Hook)(ImGuiViewport*);

    // Dummy window and device context handles.
    // These are temporary scaffolding used to correctly initialize GLAD
    // and keep the OpenGL context active until the main window is fully created.
    HWND mhDummyWnd;
    HDC mhDummyDC;

    static HGLRC shGlContext;
};
} // namespace ho