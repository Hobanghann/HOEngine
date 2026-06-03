#pragma once

#include <memory>

#include "../IWin32Application.h"

class ImGuiViewport;

namespace ho
{
class Win32ApplicationGL final : public IWin32Application
{
  public:
    static void CreateInstance(HINSTANCE hApp);

    ~Win32ApplicationGL() override = default;
    Win32ApplicationGL(const Win32ApplicationGL&) = delete;
    Win32ApplicationGL& operator=(const Win32ApplicationGL&) = delete;

    bool Init(const std::wstring& mainWindowNameStr, int32_t mainWindowWidth, int32_t mainWindowHeight) override;

    void BeginFrame() override;

    void Shutdown() override;

    static void Hook_CreateWindow(ImGuiViewport* pViewport);
    static void Hook_DestroyWindow(ImGuiViewport* pViewport);
    static void Hook_RenderWindow(ImGuiViewport* pViewport, void* pUnused);
    static void Hook_SwapBuffers(ImGuiViewport* pViewport, void* pUnused);

  private:
    Win32ApplicationGL(HINSTANCE hApp);

    static HGLRC shGlContext;
};
} // namespace ho