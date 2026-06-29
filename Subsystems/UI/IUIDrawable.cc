#include "IUIDrawable.h"

#include <IconsMaterialDesignIcons.h>
#include <imgui.h>

#include "Platforms/IPlatformApplication.h"
#include "Platforms/IPlatformWindow.h"
#include "TitleBarTheme.h"
#include "UISystem.h"

namespace ho
{
void IUIDrawable::drawMainWindowTitleBar(const TitleBarTheme& theme) const
{
    const IPlatformWindow* pMainWindow = IPlatformApplication::GetInstance().GetMainWindow();
    ImGui::SetNextWindowSize(
        ImVec2(static_cast<float>(pMainWindow->GetClientWidth()), static_cast<float>(theme.Height)));

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(0.0f, 0.0f));

    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(theme.BgColor.R, theme.BgColor.G, theme.BgColor.B, theme.BgColor.A));

    static const ImGuiWindowFlags sFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
                                           ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoDocking |
                                           ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoSavedSettings;

    if (ImGui::BeginChild("##TitleBar",
                          ImVec2(static_cast<float>(pMainWindow->GetClientWidth()), static_cast<float>(theme.Height)),
                          0,
                          sFlags))
    {
        const float buttonStartX =
            (static_cast<float>(pMainWindow->GetClientWidth()) - (static_cast<float>(theme.Height) * 1.5f) * 3.0f);

        const float iconMargin = (static_cast<float>(theme.Height) - static_cast<float>(theme.IconHeight)) * 0.5f;
        ImGui::SetCursorPos(ImVec2(iconMargin, iconMargin));
        drawIcon(IPlatformApplication::GetInstance().GetIconNativeHandle(), theme.IconWidth, theme.IconHeight);

        ImGui::SameLine();

        const float textStartX = ImGui::GetCursorPosX();
        const ImVec2 textSize = ImGui::CalcTextSize(pMainWindow->GetTitle().c_str());

        if (textStartX + textSize.x < buttonStartX)
        {
            const float textMarginY = (static_cast<float>(theme.Height) - static_cast<float>(theme.TextSize)) * 0.5f;
            ImGui::SetCursorPosY(textMarginY);
            drawTitle(pMainWindow->GetTitle(), theme.TextColor, theme.TextSize);
        }

        ImGui::SameLine();

        ImGui::SetCursorPos({buttonStartX, 0.0f});

        drawMinimizeButton(theme.ButtonIconColor,
                           theme.ButtonHoveredColor,
                           theme.ButtonActiveColor,
                           static_cast<int32_t>(static_cast<float>(theme.Height) * 1.5f),
                           theme.Height);

        ImGui::SetCursorPos({buttonStartX + (static_cast<float>(theme.Height) * 1.5f), 0.0f});

        drawMaximizeButton(theme.ButtonIconColor,
                           theme.ButtonHoveredColor,
                           theme.ButtonActiveColor,
                           static_cast<int32_t>(static_cast<float>(theme.Height) * 1.5f),
                           theme.Height);

        ImGui::SetCursorPos({buttonStartX + (static_cast<float>(theme.Height) * 1.5f * 2.0f), 0.0f});

        drawCloseButton(theme.ButtonIconColor,
                        theme.CloseButtonHoveredColor,
                        theme.CloseButtonActiveColor,
                        static_cast<int32_t>(static_cast<float>(theme.Height) * 1.5f),
                        theme.Height);

        if (ImGui::IsWindowHovered() && !ImGui::IsAnyItemHovered() &&
            ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
        {
            if (pMainWindow->IsWindowMaximized())
            {
                pMainWindow->Restore();
            }
            else
            {
                pMainWindow->Maximize();
            }
        }

        if (ImGui::IsMouseDragging(ImGuiMouseButton_Left, 0.0f) && !ImGui::IsAnyItemHovered() &&
            !ImGui::IsAnyItemActive())
        {
            const float clickedY = ImGui::GetIO().MouseClickedPos[0].y;
            const float titleBarTop = static_cast<float>(pMainWindow->GetPosY());
            const float titleBarBottom = static_cast<float>(pMainWindow->GetPosY()) + static_cast<float>(theme.Height);

            if (clickedY >= titleBarTop && clickedY <= titleBarBottom)
            {
                pMainWindow->ProcessWindowDragging();
            }
        }
    }
    ImGui::EndChild();

    ImGui::PopStyleColor();
    ImGui::PopStyleVar(3);
}

void IUIDrawable::drawIcon(void* iconNativeHandle, int32_t width, int32_t height) const
{
    if (iconNativeHandle != nullptr)
    {
        const ImTextureID textureID = static_cast<ImTextureID>(reinterpret_cast<intptr_t>(iconNativeHandle));

        const ImVec2 uv0(0.0f, 0.0f);
        const ImVec2 uv1(1.0f, 1.0f);

        ImGui::Image(textureID, ImVec2(static_cast<float>(width), static_cast<float>(height)), uv0, uv1);
    }
    else
    {
        ImGui::PushFont(nullptr, static_cast<float>(width));
        ImGui::TextUnformatted(ICON_MDI_SQUARE);
        ImGui::PopFont();
    }
}

void IUIDrawable::drawTitle(const std::string& titleStr, const Color128& textColor, float textSize) const
{
    if (titleStr.empty())
    {
        return;
    }

    ImGui::PushStyleColor(ImGuiCol_Text, {textColor.R, textColor.G, textColor.B, textColor.A});
    ImGui::PushFont(nullptr, textSize);
    ImGui::Text(titleStr.c_str(), ImGui::GetFontSize(), textSize);
    ImGui::PopFont();
    ImGui::PopStyleColor();
}

void IUIDrawable::drawMinimizeButton(const Color128& iconColor,
                                     const Color128& hoveredColor,
                                     const Color128& activateColor,
                                     int32_t width,
                                     int32_t height) const
{
    const IPlatformWindow* pMainWindow = IPlatformApplication::GetInstance().GetMainWindow();

    const char* buttonText = ICON_MDI_WINDOW_MINIMIZE;
    const ImVec2 buttonSize(static_cast<float>(width), static_cast<float>(height));

    const bool bPressed = ImGui::InvisibleButton("##MinimizeButton", buttonSize, ImGuiButtonFlags_AllowOverlap);

    const bool bHovered = ImGui::IsItemHovered(ImGuiHoveredFlags_RectOnly);
    const bool bActive = bHovered && ImGui::IsItemActive();

    ImVec4 finalBgColor = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    if (bActive)
    {
        finalBgColor = ImVec4(activateColor.R, activateColor.G, activateColor.B, activateColor.A);
    }
    else if (bHovered)
    {
        finalBgColor = ImVec4(hoveredColor.R, hoveredColor.G, hoveredColor.B, hoveredColor.A);
    }

    const ImVec2 pMin = ImGui::GetItemRectMin();
    const ImVec2 pMax = ImGui::GetItemRectMax();
    ImGui::GetWindowDrawList()->AddRectFilled(pMin, pMax, ImGui::ColorConvertFloat4ToU32(finalBgColor));

    ImGui::PushFont(nullptr, static_cast<float>(height) * 0.6f);

    const ImVec2 textSize = ImGui::CalcTextSize(buttonText);
    const ImVec2 textPos =
        ImVec2(pMin.x + (buttonSize.x - textSize.x) * 0.5f, pMin.y + (buttonSize.y - textSize.y) * 0.5f);
    const ImVec4 textColor = ImVec4(iconColor.R, iconColor.G, iconColor.B, iconColor.A);
    ImGui::GetWindowDrawList()->AddText(textPos, ImGui::ColorConvertFloat4ToU32(textColor), buttonText);

    ImGui::PopFont();

    if (bPressed)
    {
        pMainWindow->Minimize();
    }
}

void IUIDrawable::drawMaximizeButton(const Color128& iconColor,
                                     const Color128& hoveredColor,
                                     const Color128& activateColor,
                                     int32_t width,
                                     int32_t height) const
{
    const IPlatformWindow* pMainWindow = IPlatformApplication::GetInstance().GetMainWindow();

    const char* buttonText = pMainWindow->IsWindowMaximized() ? ICON_MDI_WINDOW_RESTORE : ICON_MDI_WINDOW_MAXIMIZE;
    const ImVec2 buttonSize(static_cast<float>(width), static_cast<float>(height));

    const bool bPressed = ImGui::InvisibleButton("##MaximizeButton", buttonSize, ImGuiButtonFlags_AllowOverlap);

    const bool bHovered = ImGui::IsItemHovered(ImGuiHoveredFlags_RectOnly);
    const bool bActive = bHovered && ImGui::IsItemActive();

    ImVec4 finalBgColor = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    if (bActive)
    {
        finalBgColor = ImVec4(activateColor.R, activateColor.G, activateColor.B, activateColor.A);
    }
    else if (bHovered)
    {
        finalBgColor = ImVec4(hoveredColor.R, hoveredColor.G, hoveredColor.B, hoveredColor.A);
    }

    const ImVec2 pMin = ImGui::GetItemRectMin();
    const ImVec2 pMax = ImGui::GetItemRectMax();
    ImGui::GetWindowDrawList()->AddRectFilled(pMin, pMax, ImGui::ColorConvertFloat4ToU32(finalBgColor));

    ImGui::PushFont(nullptr, static_cast<float>(height) * 0.6f);

    const ImVec2 textSize = ImGui::CalcTextSize(buttonText);
    const ImVec2 textPos =
        ImVec2(pMin.x + (buttonSize.x - textSize.x) * 0.5f, pMin.y + (buttonSize.y - textSize.y) * 0.5f);
    const ImVec4 textColor = ImVec4(iconColor.R, iconColor.G, iconColor.B, iconColor.A);
    ImGui::GetWindowDrawList()->AddText(textPos, ImGui::ColorConvertFloat4ToU32(textColor), buttonText);

    ImGui::PopFont();

    if (bPressed)
    {
        if (pMainWindow->IsWindowMaximized())
        {
            pMainWindow->Restore();
        }
        else
        {
            pMainWindow->Maximize();
        }
    }
}

void IUIDrawable::drawCloseButton(const Color128& iconColor,
                                  const Color128& hoveredColor,
                                  const Color128& activateColor,
                                  int32_t width,
                                  int32_t height) const
{
    const IPlatformWindow* pMainWindow = IPlatformApplication::GetInstance().GetMainWindow();

    const char* buttonText = ICON_MDI_WINDOW_CLOSE;
    const ImVec2 buttonSize(static_cast<float>(width), static_cast<float>(height));

    const bool bPressed = ImGui::InvisibleButton("##CloseButton", buttonSize, ImGuiButtonFlags_AllowOverlap);

    const bool bHovered = ImGui::IsItemHovered(ImGuiHoveredFlags_RectOnly);
    const bool bActive = bHovered && ImGui::IsItemActive();

    ImVec4 finalBgColor = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);

    if (bActive)
    {
        finalBgColor = ImVec4(activateColor.R, activateColor.G, activateColor.B, activateColor.A);
    }
    else if (bHovered)
    {
        finalBgColor = ImVec4(hoveredColor.R, hoveredColor.G, hoveredColor.B, hoveredColor.A);
    }

    const ImVec2 pMin = ImGui::GetItemRectMin();
    const ImVec2 pMax = ImGui::GetItemRectMax();
    ImGui::GetWindowDrawList()->AddRectFilled(pMin, pMax, ImGui::ColorConvertFloat4ToU32(finalBgColor));

    ImGui::PushFont(nullptr, static_cast<float>(height) * 0.6f);

    const ImVec2 textSize = ImGui::CalcTextSize(buttonText);
    const ImVec2 textPos =
        ImVec2(pMin.x + (buttonSize.x - textSize.x) * 0.5f, pMin.y + (buttonSize.y - textSize.y) * 0.5f);
    const ImVec4 textColor = ImVec4(iconColor.R, iconColor.G, iconColor.B, iconColor.A);
    ImGui::GetWindowDrawList()->AddText(textPos, ImGui::ColorConvertFloat4ToU32(textColor), buttonText);

    ImGui::PopFont();

    if (bPressed)
    {
        pMainWindow->Close();
    }
}
} // namespace ho