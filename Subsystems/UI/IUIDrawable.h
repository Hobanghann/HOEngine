#pragma once

#include <string>

#include "Core/Templates/GlobalPoolIndex.h"

namespace ho
{
class UISystem;
struct TitleBarTheme;
struct Color128;

class IUIDrawable
{
    friend UISystem;

  public:
    IUIDrawable() = default;

    IUIDrawable(const IUIDrawable&) = delete;
    IUIDrawable& operator=(const IUIDrawable&) = delete;
    virtual ~IUIDrawable() = default;

    // Function to handle the UI rendering logic.
    // Implementations must enclose their UI widgets between ImGui::Begin() and ImGui::End()
    // internally to properly construct and render the window.
    virtual void DrawUI() = 0;

  protected:
    // This function draws the title bar as an ImGui child window.
    // Therefore, it must be called immediately after ImGui::Begin() before any other widgets.
    // WARN: This function must be used with main window.
    void drawMainWindowTitleBar(const TitleBarTheme& theme) const;

  private:
    void drawTitleBarIcon(void* iconNativeHandle, int32_t width, int32_t height) const;
    void drawTitleBarTitle(const std::string& titleStr, const Color128& textColor, float textSize) const;
    void drawTitleBarMinimizeButton(const Color128& iconColor,
                                    const Color128& hoveredColor,
                                    const Color128& activateColor,
                                    int32_t width,
                                    int32_t height) const;
    void drawTitleBarMaximizeButton(const Color128& iconColor,
                                    const Color128& hoveredColor,
                                    const Color128& activateColor,
                                    int32_t width,
                                    int32_t height) const;
    void drawTitleBarCloseButton(const Color128& iconColor,
                                 const Color128& hoveredColor,
                                 const Color128& activateColor,
                                 int32_t width,
                                 int32_t height) const;
};

} // namespace ho