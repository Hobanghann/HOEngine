#pragma once

#include <string>

#include "Core/Templates/GlobalPoolIndex.h"

namespace ho
{
class UISystem;
struct TitleBarTheme;
struct Color128;

class IUIWindow
{
    friend UISystem;

  public:
    IUIWindow(StringHandle hName)
      : mhName(hName)
    {
    }

    IUIWindow(const IUIWindow&) = delete;
    IUIWindow& operator=(const IUIWindow&) = delete;
    virtual ~IUIWindow() = default;

    StringHandle GetName() const
    {
        return mhName;
    }

    bool IsVisible() const
    {
        return mbVisible;
    }

    void SetVisible(bool bFlag)
    {
        mbVisible = bFlag;
    }

  protected:
    virtual void onGUI() = 0;

    // This function draws the title bar as an ImGui child window.
    // Therefore, it must be called immediately after ImGui::Begin() before any other widgets.
    // WARN: This function must be used with main window.
    void drawMainWindowTitleBar(const TitleBarTheme& theme) const;

    StringHandle mhName;
    bool mbVisible = false;

  private:
    void drawIcon(void* iconNativeHandle, int32_t width, int32_t height) const;
    void drawTitle(const std::string& titleStr, const Color128& textColor, float textSize) const;
    void drawMinimizeButton(const Color128& iconColor,
                            const Color128& hoveredColor,
                            const Color128& activateColor,
                            int32_t width,
                            int32_t height) const;
    void drawMaximizeButton(const Color128& iconColor,
                            const Color128& hoveredColor,
                            const Color128& activateColor,
                            int32_t width,
                            int32_t height) const;
    void drawCloseButton(const Color128& iconColor,
                         const Color128& hoveredColor,
                         const Color128& activateColor,
                         int32_t width,
                         int32_t height) const;
};

} // namespace ho