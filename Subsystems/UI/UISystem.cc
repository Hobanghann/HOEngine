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

void UISystem::SubmitRenderWindow(IUIWindow* pWindow)
{
    HO_ASSERT(pWindow, "Window is null.");
    if (mRenderWindowQueue.IsFull())
    {
        HO_ASSERT(false, "Render window queue is full.");
        return;
    }
    mRenderWindowQueue.Push(pWindow);
}

UISystem::UISystem()
{
    HO_ASSERT(!spInstance, "UI system muse be created by 'createInstance' function.");
}

void UISystem::createInstance()
{
    HO_ASSERT(!spInstance, "UISystem was already created.");
    spInstance = new UISystem();

    spInstance->mRenderWindowQueue = FixedQueue<IUIWindow*>(sRenderWindowQueueSize);
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
    while (!mRenderWindowQueue.IsEmpty())
    {
        IUIWindow* pWindow = mRenderWindowQueue.Front();
        if (pWindow->IsVisible())
        {
            pWindow->onGUI();
        }
        mRenderWindowQueue.Pop();
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