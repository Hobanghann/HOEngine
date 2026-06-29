#include "UISystem.h"

#include <IconsMaterialDesignIcons.h>
// clang-format off
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_internal.h>
#include <ImGuizmo.h>
#include <imoguizmo.hpp>
// clang-format on

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

void UISystem::BeginFrame()
{
    ImGui::NewFrame();
    ImGuizmo::BeginFrame();
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

    // ====================================================================
    // Load text font (Default - Inter & Korean)
    // ====================================================================
    const float fontSize = 16.0f;
    const ImWchar* pGlyphRangesKorean = io.Fonts->GetGlyphRangesKorean();
    Path fontPath = Path(std::string("Platforms/Resources/Fonts/Inter_18pt-Regular.ttf"));
    fontPath.ResolveProjectPath();

    mpIconsMaterialDesignIcons =
        io.Fonts->AddFontFromFileTTF(fontPath.ToString().c_str(), fontSize, nullptr, pGlyphRangesKorean);
    if (!mpIconsMaterialDesignIcons)
    {
        HO_ASSERT(false, "Failed to load base font.");
        return false;
    }
    ImFontConfig iconConfig;
    iconConfig.MergeMode = true;
    iconConfig.PixelSnapH = true;
    iconConfig.GlyphMinAdvanceX = 14.f;
    iconConfig.GlyphOffset = ImVec2(0.0f, 2.0f);

    static const ImWchar sIconRangesMDI[] = {static_cast<ImWchar>(ICON_MIN_MDI), static_cast<ImWchar>(ICON_MAX_MDI), 0};

    Path iconPath = Path(std::string("Platforms/Resources/Fonts/materialdesignicons-webfont.ttf"));
    iconPath.ResolveProjectPath();

    if (!io.Fonts->AddFontFromFileTTF(iconPath.ToString().c_str(), 16.0f, &iconConfig, sIconRangesMDI))
    {
        HO_ASSERT(false, "Failed to merge icons.");
        return false;
    }

    // Setup imgui IO config
    ImGui::GetIO().ConfigWindowsMoveFromTitleBarOnly = true;

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
    style.Colors[ImGuiCol_MenuBarBg] = {
        mTitleBarTheme.BgColor.R, mTitleBarTheme.BgColor.G, mTitleBarTheme.BgColor.B, mTitleBarTheme.BgColor.A};

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
                                             0.6f};
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
                                            1.0f};

    style.Colors[ImGuiCol_NavCursor] = {mTitleBarTheme.ButtonActiveColor.R,
                                        mTitleBarTheme.ButtonActiveColor.G,
                                        mTitleBarTheme.ButtonActiveColor.B,
                                        1.0f};
    style.Colors[ImGuiCol_NavWindowingHighlight] = {1.0f, 1.0f, 1.0f, 0.7f};
    style.Colors[ImGuiCol_NavWindowingDimBg] = {0.0f, 0.0f, 0.0f, 0.2f};
    style.Colors[ImGuiCol_ModalWindowDimBg] = {0.0f, 0.0f, 0.0f, 0.6f};

    // Setup ImGuizmo config
    ImGuizmo::Style& gizmoStyle = ImGuizmo::GetStyle();
    gizmoStyle.TranslationLineThickness = 3.0f;
    gizmoStyle.TranslationLineArrowSize = 6.0f;
    gizmoStyle.RotationLineThickness = 2.0f;
    gizmoStyle.RotationOuterLineThickness = 3.0f;
    gizmoStyle.ScaleLineThickness = 3.0f;
    gizmoStyle.ScaleLineCircleSize = 6.0f;
    gizmoStyle.HatchedAxisLineThickness = 6.0f;
    gizmoStyle.CenterCircleSize = 6.0f;

    // Setup ImOGuizmo config
    ImOGuizmo::config.lineThicknessScale = 0.03f;
    ImOGuizmo::config.axisLengthScale = 0.33f;
    ImOGuizmo::config.positiveRadiusScale = 0.1f;
    ImOGuizmo::config.negativeRadiusScale = 0.08f;
    ImOGuizmo::config.hoverCircleRadiusScale = 0.9f;
    ImOGuizmo::config.dragThreshold = 3.0f;
    ImOGuizmo::config.dragSensitivity = -0.01f;
    ImOGuizmo::config.drag = true;
    ImOGuizmo::config.click = true;
    return true;
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