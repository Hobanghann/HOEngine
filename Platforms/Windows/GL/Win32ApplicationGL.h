#pragma once

#include <memory>

#include "../IWin32Application.h"
#include "Win32WindowGL.h"

namespace ho
{
class Win32ApplicationGL final : public IWin32Application
{
  public:
    static void CreateInstance(HINSTANCE hApp);

    Win32ApplicationGL(const Win32ApplicationGL&) = delete;
    Win32ApplicationGL& operator=(const Win32ApplicationGL&) = delete;
    ~Win32ApplicationGL() override = default;

    bool Init(const std::wstring& mainWindowName, int32_t mainwWindowWidth, int32_t mainwWindowHeight) override;

    bool BeginFrame() override;

    bool EndFrame() override;

    bool Shutdown() override;

  private:
    Win32ApplicationGL(HINSTANCE hApp);

    std::unique_ptr<Win32WindowGL> pMainWindow;
};
} // namespace ho