#include "IPlatformApplication.h"

#include <Lib/IconsFontAwesome7.h>
#include <filesystem>
#include <imgui.h>

namespace ho
{
const IPlatformWindow* IPlatformApplication::GetMainWindow() const
{
    return spMainWindow.get();
}

void IPlatformApplication::initImGuiFonts()
{
    ImGuiIO& io = ImGui::GetIO();

    std::filesystem::path assetPath;

#ifdef HO_ASSET_ROOT
    assetPath = std::filesystem::path(std::string(HO_ASSET_ROOT));
#else
#error "HO_ASSET_ROOT must be defined"
#endif

    // load font
    const float fontSize = 16.0f;
    const ImWchar* pGlyphRangesKorean = io.Fonts->GetGlyphRangesKorean();
    const std::filesystem::path fontPath =
        assetPath / std::filesystem::path(std::string("Fonts/Inter_18pt-Regular.ttf"));
    if (!io.Fonts->AddFontFromFileTTF(fontPath.string().c_str(), fontSize, nullptr, pGlyphRangesKorean))
    {
        HO_ASSERT(false, "Failed to load font.");
    }

    // load icon
    ImFontConfig config;
    config.MergeMode = true;
    config.PixelSnapH = true;
    config.GlyphMinAdvanceX = 14.f;
    config.GlyphOffset = ImVec2(0.0f, 2.0f);
    static const ImWchar sIconRanges[] = {ICON_MIN_FA, ICON_MAX_FA, 0};
    const std::filesystem::path iconPath = assetPath / std::filesystem::path(std::string("Fonts/fa-solid-900.ttf"));
    if (!io.Fonts->AddFontFromFileTTF(iconPath.string().c_str(), 16.0f, &config, sIconRanges))
    {
        HO_ASSERT(false, "Failed to load font.");
    }

    if (!io.Fonts->Build())
    {
        HO_ASSERT(false, "Failed to build font.");
    }
}

std::unique_ptr<IPlatformWindow> IPlatformApplication::spMainWindow = nullptr;
IPlatformApplication* IPlatformApplication::spInstance = nullptr;
} // namespace ho