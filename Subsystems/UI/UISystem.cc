#include "UISystem.h"

#include <IconsFontAwesome7.h>
#include <imgui.h>

#include "Core/IO/Path.h"
#include "Platforms/IPlatformApplication.h"
#include "Subsystems/Rendering/IRenderingSystem.h"

namespace ho
{
UISystem& UISystem::GetInstance()
{
    HO_ASSERT(spInstance, "UISystem is not created");
    return *spInstance;
}

void UISystem::SubmitUIDrawable(IUIDrawable* pWindow)
{
    HO_ASSERT(pWindow, "Window is null.");
    if (mUIDrawableQueue.IsFull())
    {
        HO_ASSERT(false, "Render window queue is full.");
        return;
    }
    mUIDrawableQueue.Push(pWindow);
}

UISystem::UISystem()
{
    HO_ASSERT(!spInstance, "UI system muse be created by 'createInstance' function.");
}

void UISystem::createInstance()
{
    HO_ASSERT(!spInstance, "UISystem was already created.");
    spInstance = new UISystem();

    spInstance->mUIDrawableQueue = FixedQueue<IUIDrawable*>(sUIDrawableQueueSize);
}

void UISystem::deleteInstance()
{
    HO_ASSERT(spInstance, "UISystem is not created");
    delete spInstance;
    spInstance = nullptr;
}

bool UISystem::init()
{
    ImGuiIO& io = ImGui::GetIO();

    // Load text font
    const float fontSize = 16.0f;
    const ImWchar* pGlyphRangesKorean = io.Fonts->GetGlyphRangesKorean();
    Path fontPath = Path(std::string("Platforms/Resources/Fonts/Inter_18pt-Regular.ttf"));
    fontPath.ResolveProjectPath();
    if (!io.Fonts->AddFontFromFileTTF(fontPath.ToString().c_str(), fontSize, nullptr, pGlyphRangesKorean))
    {
        HO_ASSERT(false, "Failed to load font.");
        return false;
    }

    // Load icon font
    ImFontConfig iconConfig;
    iconConfig.MergeMode = true;
    iconConfig.PixelSnapH = true;
    iconConfig.GlyphMinAdvanceX = 14.f;
    iconConfig.GlyphOffset = ImVec2(0.0f, 2.0f);
    static const ImWchar sIconRanges[] = {ICON_MIN_FA, ICON_MAX_FA, 0};

    mpFaSolidFont = io.Fonts->AddFontFromFileTTF(fontPath.ToString().c_str(), fontSize, nullptr, pGlyphRangesKorean);
    if (!mpFaSolidFont)
    {
        HO_ASSERT(false, "Failed to load base font for Solid.");
        return false;
    }

    Path iconPath;
    iconPath = Path(std::string("Platforms/Resources/Fonts/fa-solid-900.ttf"));
    iconPath.ResolveProjectPath();
    if (!io.Fonts->AddFontFromFileTTF(iconPath.ToString().c_str(), 16.0f, &iconConfig, sIconRanges))
    {
        HO_ASSERT(false, "Failed to merge FaSolid font.");
        return false;
    }

    mpFaRegularFont = io.Fonts->AddFontFromFileTTF(fontPath.ToString().c_str(), fontSize, nullptr, pGlyphRangesKorean);
    if (!mpFaRegularFont)
    {
        HO_ASSERT(false, "Failed to load base font for Regular.");
        return false;
    }

    iconPath = Path(std::string("Platforms/Resources/Fonts/fa-regular-400.ttf"));
    iconPath.ResolveProjectPath();
    if (!io.Fonts->AddFontFromFileTTF(iconPath.ToString().c_str(), 16.0f, &iconConfig, sIconRanges))
    {
        HO_ASSERT(false, "Failed to merge FaRegular font.");
        return false;
    }

    mpFaBrandsFont = io.Fonts->AddFontFromFileTTF(fontPath.ToString().c_str(), fontSize, nullptr, pGlyphRangesKorean);
    if (!mpFaBrandsFont)
    {
        HO_ASSERT(false, "Failed to load base font for Brands.");
        return false;
    }

    iconPath = Path(std::string("Platforms/Resources/Fonts/fa-brands-400.ttf"));
    iconPath.ResolveProjectPath();
    if (!io.Fonts->AddFontFromFileTTF(iconPath.ToString().c_str(), 16.0f, &iconConfig, sIconRanges))
    {
        HO_ASSERT(false, "Failed to merge FaBrands font.");
        return false;
    }

    // Setup imgui colors
    ImGuiStyle& style = ImGui::GetStyle();

    style.Colors[ImGuiCol_Text] = {1.0f, 1.0f, 1.0f, 1.0f};
    style.Colors[ImGuiCol_TextDisabled] = {0.5f, 0.5f, 0.5f, 1.0f};
    style.Colors[ImGuiCol_TextSelectedBg] = {mTitleBarTheme.ButtonActiveColor.R,
                                             mTitleBarTheme.ButtonActiveColor.G,
                                             mTitleBarTheme.ButtonActiveColor.B,
                                             0.6f};
    style.Colors[ImGuiCol_TextLink] = {0.3f, 0.6f, 0.9f, 1.0f};

    style.Colors[ImGuiCol_WindowBg] = {mTitleBarTheme.BgColor.R * 0.5f,
                                       mTitleBarTheme.BgColor.G * 0.5f,
                                       mTitleBarTheme.BgColor.B * 0.5f,
                                       mTitleBarTheme.BgColor.A};
    style.Colors[ImGuiCol_ChildBg] = {
        mTitleBarTheme.BgColor.R * 0.4f, mTitleBarTheme.BgColor.G * 0.4f, mTitleBarTheme.BgColor.B * 0.4f, 1.0f};
    style.Colors[ImGuiCol_PopupBg] = {
        mTitleBarTheme.BgColor.R * 0.6f, mTitleBarTheme.BgColor.G * 0.6f, mTitleBarTheme.BgColor.B * 0.6f, 0.95f};

    style.Colors[ImGuiCol_Border] = {
        mTitleBarTheme.BgColor.R * 0.3f, mTitleBarTheme.BgColor.G * 0.3f, mTitleBarTheme.BgColor.B * 0.3f, 1.0f};
    style.Colors[ImGuiCol_BorderShadow] = {0.0f, 0.0f, 0.0f, 0.0f};

    style.Colors[ImGuiCol_FrameBg] = {
        mTitleBarTheme.BgColor.R * 0.3f, mTitleBarTheme.BgColor.G * 0.3f, mTitleBarTheme.BgColor.B * 0.3f, 1.0f};
    style.Colors[ImGuiCol_FrameBgHovered] = {mTitleBarTheme.ButtonHoveredColor.R,
                                             mTitleBarTheme.ButtonHoveredColor.G,
                                             mTitleBarTheme.ButtonHoveredColor.B,
                                             mTitleBarTheme.ButtonHoveredColor.A};
    style.Colors[ImGuiCol_FrameBgActive] = {mTitleBarTheme.ButtonActiveColor.R,
                                            mTitleBarTheme.ButtonActiveColor.G,
                                            mTitleBarTheme.ButtonActiveColor.B,
                                            mTitleBarTheme.ButtonActiveColor.A};

    style.Colors[ImGuiCol_TitleBg] = {mTitleBarTheme.BgColor.R * 1.5f,
                                      mTitleBarTheme.BgColor.G * 1.5f,
                                      mTitleBarTheme.BgColor.B * 1.5f,
                                      mTitleBarTheme.BgColor.A};
    style.Colors[ImGuiCol_TitleBgActive] = {mTitleBarTheme.BgColor.R * 2.0f,
                                            mTitleBarTheme.BgColor.G * 2.0f,
                                            mTitleBarTheme.BgColor.B * 2.0f,
                                            mTitleBarTheme.BgColor.A};
    style.Colors[ImGuiCol_TitleBgCollapsed] = {mTitleBarTheme.BgColor.R * 1.5f,
                                               mTitleBarTheme.BgColor.G * 1.5f,
                                               mTitleBarTheme.BgColor.B * 1.5f,
                                               mTitleBarTheme.BgColor.A};
    style.Colors[ImGuiCol_MenuBarBg] = {mTitleBarTheme.BgColor.R * 1.2f,
                                        mTitleBarTheme.BgColor.G * 1.2f,
                                        mTitleBarTheme.BgColor.B * 1.2f,
                                        mTitleBarTheme.BgColor.A};

    style.Colors[ImGuiCol_ScrollbarBg] = {mTitleBarTheme.BgColor.R * 0.8f,
                                          mTitleBarTheme.BgColor.G * 0.8f,
                                          mTitleBarTheme.BgColor.B * 0.8f,
                                          mTitleBarTheme.BgColor.A};
    style.Colors[ImGuiCol_ScrollbarGrab] = {
        mTitleBarTheme.BgColor.R, mTitleBarTheme.BgColor.G, mTitleBarTheme.BgColor.B, mTitleBarTheme.BgColor.A * 0.5f};
    style.Colors[ImGuiCol_ScrollbarGrabHovered] = {mTitleBarTheme.ButtonHoveredColor.R,
                                                   mTitleBarTheme.ButtonHoveredColor.G,
                                                   mTitleBarTheme.ButtonHoveredColor.B,
                                                   mTitleBarTheme.ButtonHoveredColor.A};
    style.Colors[ImGuiCol_ScrollbarGrabActive] = {mTitleBarTheme.ButtonActiveColor.R,
                                                  mTitleBarTheme.ButtonActiveColor.G,
                                                  mTitleBarTheme.ButtonActiveColor.B,
                                                  mTitleBarTheme.ButtonActiveColor.A};

    style.Colors[ImGuiCol_CheckMark] = {1.0f, 1.0f, 1.0f, 1.0f};
    style.Colors[ImGuiCol_SliderGrab] = {mTitleBarTheme.ButtonHoveredColor.R,
                                         mTitleBarTheme.ButtonHoveredColor.G,
                                         mTitleBarTheme.ButtonHoveredColor.B,
                                         1.0f};
    style.Colors[ImGuiCol_SliderGrabActive] = {mTitleBarTheme.ButtonActiveColor.R,
                                               mTitleBarTheme.ButtonActiveColor.G,
                                               mTitleBarTheme.ButtonActiveColor.B,
                                               1.0f};
    style.Colors[ImGuiCol_Button] = {
        mTitleBarTheme.BgColor.R * 1.0f, mTitleBarTheme.BgColor.G * 1.0f, mTitleBarTheme.BgColor.B * 1.0f, 1.0f};
    style.Colors[ImGuiCol_ButtonHovered] = {mTitleBarTheme.ButtonHoveredColor.R,
                                            mTitleBarTheme.ButtonHoveredColor.G,
                                            mTitleBarTheme.ButtonHoveredColor.B,
                                            mTitleBarTheme.ButtonHoveredColor.A};
    style.Colors[ImGuiCol_ButtonActive] = {mTitleBarTheme.ButtonActiveColor.R,
                                           mTitleBarTheme.ButtonActiveColor.G,
                                           mTitleBarTheme.ButtonActiveColor.B,
                                           mTitleBarTheme.ButtonActiveColor.A};

    style.Colors[ImGuiCol_Header] = {
        mTitleBarTheme.BgColor.R * 1.1f, mTitleBarTheme.BgColor.G * 1.1f, mTitleBarTheme.BgColor.B * 1.1f, 1.0f};
    style.Colors[ImGuiCol_HeaderHovered] = {mTitleBarTheme.ButtonHoveredColor.R,
                                            mTitleBarTheme.ButtonHoveredColor.G,
                                            mTitleBarTheme.ButtonHoveredColor.B,
                                            mTitleBarTheme.ButtonHoveredColor.A};
    style.Colors[ImGuiCol_HeaderActive] = {mTitleBarTheme.ButtonActiveColor.R,
                                           mTitleBarTheme.ButtonActiveColor.G,
                                           mTitleBarTheme.ButtonActiveColor.B,
                                           mTitleBarTheme.ButtonActiveColor.A};

    style.Colors[ImGuiCol_Separator] = {
        mTitleBarTheme.BgColor.R * 0.4f, mTitleBarTheme.BgColor.G * 0.4f, mTitleBarTheme.BgColor.B * 0.4f, 1.0f};
    style.Colors[ImGuiCol_SeparatorHovered] = {mTitleBarTheme.ButtonHoveredColor.R,
                                               mTitleBarTheme.ButtonHoveredColor.G,
                                               mTitleBarTheme.ButtonHoveredColor.B,
                                               1.0f};
    style.Colors[ImGuiCol_SeparatorActive] = {mTitleBarTheme.ButtonActiveColor.R,
                                              mTitleBarTheme.ButtonActiveColor.G,
                                              mTitleBarTheme.ButtonActiveColor.B,
                                              1.0f};
    style.Colors[ImGuiCol_ResizeGrip] = {
        mTitleBarTheme.BgColor.R * 0.6f, mTitleBarTheme.BgColor.G * 0.6f, mTitleBarTheme.BgColor.B * 0.6f, 0.4f};
    style.Colors[ImGuiCol_ResizeGripHovered] = {mTitleBarTheme.ButtonHoveredColor.R,
                                                mTitleBarTheme.ButtonHoveredColor.G,
                                                mTitleBarTheme.ButtonHoveredColor.B,
                                                0.8f};
    style.Colors[ImGuiCol_ResizeGripActive] = {mTitleBarTheme.ButtonActiveColor.R,
                                               mTitleBarTheme.ButtonActiveColor.G,
                                               mTitleBarTheme.ButtonActiveColor.B,
                                               1.0f};

    style.Colors[ImGuiCol_Tab] = {
        mTitleBarTheme.BgColor.R * 0.6f, mTitleBarTheme.BgColor.G * 0.6f, mTitleBarTheme.BgColor.B * 0.6f, 1.0f};
    style.Colors[ImGuiCol_TabHovered] = {mTitleBarTheme.ButtonHoveredColor.R,
                                         mTitleBarTheme.ButtonHoveredColor.G,
                                         mTitleBarTheme.ButtonHoveredColor.B,
                                         1.0f};
    style.Colors[ImGuiCol_TabSelected] = {
        mTitleBarTheme.BgColor.R * 1.0f, mTitleBarTheme.BgColor.G * 1.0f, mTitleBarTheme.BgColor.B * 1.0f, 1.0f};
    style.Colors[ImGuiCol_TabSelectedOverline] = {1.0f, 1.0f, 1.0f, 1.0f};
    style.Colors[ImGuiCol_TabDimmed] = {
        mTitleBarTheme.BgColor.R * 0.4f, mTitleBarTheme.BgColor.G * 0.4f, mTitleBarTheme.BgColor.B * 0.4f, 1.0f};
    style.Colors[ImGuiCol_TabDimmedSelected] = {
        mTitleBarTheme.BgColor.R * 0.6f, mTitleBarTheme.BgColor.G * 0.6f, mTitleBarTheme.BgColor.B * 0.6f, 1.0f};
    style.Colors[ImGuiCol_TabDimmedSelectedOverline] = {0.8f, 0.8f, 0.8f, 1.0f};

    style.Colors[ImGuiCol_DockingPreview] = {mTitleBarTheme.ButtonHoveredColor.R,
                                             mTitleBarTheme.ButtonHoveredColor.G,
                                             mTitleBarTheme.ButtonHoveredColor.B,
                                             0.6f}; // 가이드 사각형 반투명 회색톤
    style.Colors[ImGuiCol_DockingEmptyBg] = {mTitleBarTheme.BgColor.R * 0.8f,
                                             mTitleBarTheme.BgColor.G * 0.8f,
                                             mTitleBarTheme.BgColor.B * 0.8f,
                                             mTitleBarTheme.BgColor.A};

    style.Colors[ImGuiCol_TableHeaderBg] = {
        mTitleBarTheme.BgColor.R * 0.8f, mTitleBarTheme.BgColor.G * 0.8f, mTitleBarTheme.BgColor.B * 0.8f, 1.0f};
    style.Colors[ImGuiCol_TableBorderStrong] = {
        mTitleBarTheme.BgColor.R * 0.5f, mTitleBarTheme.BgColor.G * 0.5f, mTitleBarTheme.BgColor.B * 0.5f, 1.0f};
    style.Colors[ImGuiCol_TableBorderLight] = {
        mTitleBarTheme.BgColor.R * 0.3f, mTitleBarTheme.BgColor.G * 0.3f, mTitleBarTheme.BgColor.B * 0.3f, 1.0f};
    style.Colors[ImGuiCol_TableRowBg] = {0.0f, 0.0f, 0.0f, 0.0f};
    style.Colors[ImGuiCol_TableRowBgAlt] = {1.0f, 1.0f, 1.0f, 0.04f};

    style.Colors[ImGuiCol_PlotLines] = {0.8f, 0.8f, 0.8f, 1.0f};
    style.Colors[ImGuiCol_PlotLinesHovered] = {1.0f, 1.0f, 1.0f, 1.0f};
    style.Colors[ImGuiCol_PlotHistogram] = {mTitleBarTheme.ButtonHoveredColor.R,
                                            mTitleBarTheme.ButtonHoveredColor.G,
                                            mTitleBarTheme.ButtonHoveredColor.B,
                                            1.0f};
    style.Colors[ImGuiCol_PlotHistogramHovered] = {mTitleBarTheme.ButtonActiveColor.R,
                                                   mTitleBarTheme.ButtonActiveColor.G,
                                                   mTitleBarTheme.ButtonActiveColor.B,
                                                   1.0f};
    style.Colors[ImGuiCol_InputTextCursor] = {1.0f, 1.0f, 1.0f, 1.0f};
    style.Colors[ImGuiCol_TreeLines] = {
        mTitleBarTheme.BgColor.R * 0.8f, mTitleBarTheme.BgColor.G * 0.8f, mTitleBarTheme.BgColor.B * 0.8f, 1.0f};

    style.Colors[ImGuiCol_DragDropTarget] = {mTitleBarTheme.ButtonActiveColor.R,
                                             mTitleBarTheme.ButtonActiveColor.G,
                                             mTitleBarTheme.ButtonActiveColor.B,
                                             1.0f};
    style.Colors[ImGuiCol_DragDropTargetBg] = {mTitleBarTheme.ButtonHoveredColor.R,
                                               mTitleBarTheme.ButtonHoveredColor.G,
                                               mTitleBarTheme.ButtonHoveredColor.B,
                                               0.4f};
    style.Colors[ImGuiCol_UnsavedMarker] = {mTitleBarTheme.CloseButtonHoveredColor.R,
                                            mTitleBarTheme.CloseButtonHoveredColor.G,
                                            mTitleBarTheme.CloseButtonHoveredColor.B,
                                            1.0f}; // 저장안됨 마커는 닫기버튼 빨간색 계열 매칭

    style.Colors[ImGuiCol_NavCursor] = {mTitleBarTheme.ButtonActiveColor.R,
                                        mTitleBarTheme.ButtonActiveColor.G,
                                        mTitleBarTheme.ButtonActiveColor.B,
                                        1.0f};
    style.Colors[ImGuiCol_NavWindowingHighlight] = {1.0f, 1.0f, 1.0f, 0.7f};
    style.Colors[ImGuiCol_NavWindowingDimBg] = {0.0f, 0.0f, 0.0f, 0.2f};
    style.Colors[ImGuiCol_ModalWindowDimBg] = {0.0f, 0.0f, 0.0f, 0.6f};
    return true;
}

void UISystem::ActivateFaSolid()
{
    HO_ASSERT(mpFaSolidFont, "Font is not loaded.");
    ImGui::PushFont(mpFaSolidFont);
}

void UISystem::ActivateFaRegular()
{
    HO_ASSERT(mpFaRegularFont, "Font is not loaded.");
    ImGui::PushFont(mpFaRegularFont);
}

void UISystem::ActivateFaBrands()
{
    HO_ASSERT(mpFaBrandsFont, "Font is not loaded.");
    ImGui::PushFont(mpFaBrandsFont);
}

void UISystem::DeactivateIconFont()
{
    ImGui::PopFont();
}

void UISystem::submitDrawCommandForUI()
{
    while (!mUIDrawableQueue.IsEmpty())
    {
        mUIDrawableQueue.Front()->DrawUI();
        mUIDrawableQueue.Pop();
    }

    ImGui::Render();

    const ImGuiIO& io = ImGui::GetIO();
    // Update and Render additional Platform Windows
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        ImGui::UpdatePlatformWindows();
    }

    const ImDrawData* mainDrawData = ImGui::GetDrawData();

    IRenderingSystem::GetInstance().SubmitUIViewportData(IPlatformApplication::GetInstance().GetMainWindow(),
                                                         *mainDrawData);

    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        ImGuiPlatformIO& platformIO = ImGui::GetPlatformIO();
        for (int32_t i = 0; i < platformIO.Viewports.Size; ++i)
        {
            const ImGuiViewport* viewport = platformIO.Viewports[i];

            if (viewport->ID == ImGui::GetMainViewport()->ID)
            {
                continue;
            }

            const IPlatformWindow* pTargetWindow = static_cast<IPlatformWindow*>(viewport->RendererUserData);

            if (viewport->DrawData && pTargetWindow)
            {
                IRenderingSystem::GetInstance().SubmitUIViewportData(pTargetWindow, *viewport->DrawData);
            }
        }
    }
}

void UISystem::shutdown() {}

UISystem* UISystem::spInstance = nullptr;
} // namespace ho