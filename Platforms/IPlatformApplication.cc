#include "IPlatformApplication.h"

#include <Lib/IconsFontAwesome7.h>
#include <filesystem>
#include <imgui.h>

namespace ho
{
int32_t IPlatformApplication::sMainWindowWidth = 0;
int32_t IPlatformApplication::sMainWindowHeight = 0;
IPlatformApplication* IPlatformApplication::spInstance = nullptr;

int32_t IPlatformApplication::GetMainWindowWidth() const
{
    return sMainWindowWidth;
}

int32_t IPlatformApplication::GetMainWindowHeight() const
{
    return sMainWindowHeight;
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
    const ImWchar* glyphRangesKorean = io.Fonts->GetGlyphRangesKorean();
    const std::filesystem::path fontPath =
        assetPath / std::filesystem::path(std::string("Fonts/Inter_18pt-Regular.ttf"));
    const ImFont* mainFont =
        io.Fonts->AddFontFromFileTTF(fontPath.string().c_str(), fontSize, nullptr, glyphRangesKorean);
    HO_ASSERT(mainFont, "");
    (void)mainFont;

    // load icon
    ImFontConfig config;
    config.MergeMode = true;
    config.PixelSnapH = true;
    config.GlyphMinAdvanceX = 14.f;
    config.GlyphOffset = ImVec2(0.0f, 2.0f);
    static const ImWchar sIconRanges[] = {ICON_MIN_FA, ICON_MAX_FA, 0};
    const std::filesystem::path iconPath = assetPath / std::filesystem::path(std::string("Fonts/fa-solid-900.ttf"));
    const ImFont* icon = io.Fonts->AddFontFromFileTTF(iconPath.string().c_str(), 16.0f, &config, sIconRanges);
    HO_ASSERT(icon, "");
    (void)icon;

    io.Fonts->Build();
}
} // namespace ho