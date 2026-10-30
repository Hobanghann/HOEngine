#include "TextureImportWorkflow.h"

#include <imgui.h>

#include "../Utils/EditorUtils.h"
#include "Core/IO/ResourceParseFuncs.h"
#include "Platforms/IPlatformApplication.h"
#include "Subsystems/Asset/AssetSystem.h"

namespace ho
{
static const char* getTextureFormatName(eTextureFormat format);
static const char* getImageFormatName(Image::eFormat format);
static const std::vector<eTextureFormat>& getAvailableTextureFormats(bool bIsLinear, bool bIsInteger, int numChannel);

void TextureImportWorkflow::Begin2D()
{
    if (mState != eState::None)
    {
        HO_ASSERT(false, "Workflow was already begun.");
        return;
    }
    mState = eState::Begin2D;
    mbNeedToDeleteTexture = false;
}

void TextureImportWorkflow::BeginCubemap()
{
    if (mState != eState::None)
    {
        HO_ASSERT(false, "Workflow was already begun.");
        return;
    }
    mState = eState::BeginCubemap;
    mbNeedToDeleteTexture = false;
}

void TextureImportWorkflow::Draw()
{
    switch (mState)
    {
        case eState::ParseSetting:
            mbOpenParseSettingPopup = true;
            break;
        case eState::Parsing:
            mbOpenParsingPopup = true;
            break;
        case eState::ImportSetting:
            mbOpenImportSettingPopup = true;
            break;
        case eState::Importing:
            mbOpenImportingPopup = true;
            break;
        case eState::ParseFailed:
            mbOpenParseFailedPopup = true;
            break;
        case eState::ImportFailed:
            mbOpenImportFailedPopup = true;
            break;
        case eState::None:
        case eState::Begin2D:
        case eState::BeginCubemap:
        case eState::ParseBegin:
        case eState::ImportBegin:
        case eState::Succeed:
        case eState::Finish:
            break;
        default:
            HO_ASSERT(false, "Invalid state.");
    }

    drawParseSettingPopup();
    drawParsingPopup();
    drawImportSettingPopup();
    drawImportingPopup();
    drawParseFailedPopup();
    drawImportFailedPopup();
}

void TextureImportWorkflow::Process()
{
    switch (mState)
    {
        case eState::Begin2D:
        {
            initializeParseContext(eTextureType::Texture2D);
            mState = eState::ParseSetting;
        }
        break;
        case eState::BeginCubemap:
        {
            initializeParseContext(eTextureType::TextureCubeMap);
            mState = eState::ParseSetting;
        }
        break;
        case eState::ParseBegin:
        {
            if (beginParsing())
            {
                mState = eState::Parsing;
            }
            else
            {
                mState = eState::ParseFailed;
            }
        }
        break;
        case eState::Parsing:
        {
            if (checkParsing())
            {
                if (initializeImportContext())
                {
                    mState = eState::ImportSetting;
                }
                else
                {
                    mState = eState::ParseFailed;
                }
            }
            else
            {
                mState = eState::ParseFailed;
            }
        }
        break;
        case eState::ImportBegin:
        {
            if (beginImporting())
            {
                mState = eState::Importing;
            }
            else
            {
                mState = eState::ImportFailed;
            }
        }
        break;
        case eState::Importing:
        {
            if (checkImporting())
            {
                if (mImportContext.TotalImportedTextureCount > mImportContext.ImportedTextureCount)
                {
                    mState = eState::Importing;
                }
                else
                {
                    mState = eState::Succeed;
                }
            }
            else
            {
                if (mImportContext.TotalImportedTextureCount > mImportContext.ImportedTextureCount)
                {
                    mState = eState::Importing;
                }
                else
                {
                    mState = eState::ImportFailed;
                }
            }
        }
        break;
        case eState::Succeed:
            mState = eState::Finish;
            break;
        case eState::Finish:
            clearParseContext();
            if (mbNeedToDeleteTexture)
            {
                deleteImpotedTexture();
            }
            clearImportContext();

            mState = eState::None;
            break;
        case eState::ImportFailed:
            mbNeedToDeleteTexture = true;
            break;
        case eState::None:
        case eState::ParseSetting:
        case eState::ImportSetting:
        case eState::ParseFailed:

            return;
        default:
            HO_ASSERT(false, "Invalid state.");
    }
}

void TextureImportWorkflow::drawParseSettingPopup()
{
    if (mState != eState::ParseSetting)
    {
        return;
    }

    if (mbOpenParseSettingPopup)
    {
        ImGui::SetNextWindowSize(ImVec2(600.0f, 0.0f), ImGuiCond_Appearing);
        ImGui::OpenPopup("Texture Parse Setting##TextureParseSettingPopup");
        mbOpenParseSettingPopup = false;
    }

    if (ImGui::BeginPopupModal("Texture Parse Setting##TextureParseSettingPopup",
                               nullptr,
                               ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse))
    {
        constexpr float labelColumnWidth = 140.0f;

        // =============================================================================================================
        // Draw Texture Path Settings
        ImGui::SeparatorText("TEXTURE PATH");
        if (ImGui::BeginTable("##TexturePathTable", 2, ImGuiTableFlags_SizingStretchProp))
        {
            ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed, labelColumnWidth);
            ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

            const int32_t texCount = (mParseContext.TextureType == eTextureType::Texture2D) ? 1 : 6;
            static const char* const sCubemapFaceNames[6] = {"X+", "X-", "Y+", "Y-", "Z+", "Z-"};

            for (int32_t i = 0; i < texCount; ++i)
            {
                ImGui::PushID(i);

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::AlignTextToFramePadding();

                if (texCount > 1 && i < 6)
                {
                    ImGui::Text("%s", sCubemapFaceNames[i]);
                }
                else
                {
                    ImGui::Text("Path");
                }

                ImGui::TableNextColumn();

                const float buttonWidth = 30.0f;
                const float spacing = ImGui::GetStyle().ItemSpacing.x;
                const float inputWidth = ImGui::GetContentRegionAvail().x - buttonWidth - spacing;

                ImGui::SetNextItemWidth(inputWidth);

                char pathBuffer[512];
                snprintf(pathBuffer, sizeof(pathBuffer), "%s", mParseContext.Path[i].ToString().c_str());

                ImGui::InputText("##PathInput", pathBuffer, sizeof(pathBuffer), ImGuiInputTextFlags_ReadOnly);

                ImGui::SameLine();

                if (ImGui::Button("...", ImVec2(buttonWidth, 0.0f)))
                {
                    const std::string selectedPath = showTextureFileOpenDialog();
                    if (!selectedPath.empty())
                    {
                        mParseContext.Path[i] = Path(selectedPath);
                    }
                }

                ImGui::PopID();
            }

            ImGui::EndTable();
        }

        // =============================================================================================================
        // Draw Texture Settings
        ImGui::Spacing();
        ImGui::SeparatorText("SETTINGS");

        if (ImGui::BeginTable("##TextureSettingsTable", 2, ImGuiTableFlags_SizingStretchProp))
        {
            ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed, labelColumnWidth);
            ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

            // Texture Name
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Texture Name");

            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(-1.0f);
            ImGui::InputText(
                "##TextureNameInputText", mParseContext.TextureName, HO_ARRAY_SIZE(mParseContext.TextureName));

            // Color Space Combo
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Color Space");

            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(-1.0f);
            if (ImGui::BeginCombo("##TextureCompatibleMaterialCombo",
                                  mParseContext.bIsLinear ? "LINEAR" : "SRGB",
                                  ImGuiComboFlags_PopupAlignLeft))
            {
                for (int32_t i = 0; i < 2; ++i)
                {
                    const bool bSelected = (static_cast<bool>(i) == mParseContext.bIsLinear);
                    if (ImGui::Selectable(static_cast<bool>(i) ? "LINEAR" : "SRGB", bSelected))
                    {
                        mParseContext.bIsLinear = static_cast<bool>(i);
                    }

                    if (bSelected)
                    {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }

            ImGui::EndTable();
        }

        // =============================================================================================================
        // Draw Buttons
        ImGui::Spacing();

        const float windowWidth = ImGui::GetContentRegionAvail().x;
        const float buttonWidth = 80.0f;
        const float allButtonWidth = buttonWidth + buttonWidth + ImGui::GetStyle().ItemSpacing.x;

        const float beginX = (windowWidth - allButtonWidth) * 0.5f;

        if (beginX > 0.0f)
        {
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + beginX);
        }

        if (ImGui::Button("Parse", ImVec2(buttonWidth, 0.0f)))
        {
            mState = eState::ParseBegin;
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();

        if (ImGui::Button("Cancel", ImVec2(buttonWidth, 0.0f)))
        {
            mState = eState::Finish;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void TextureImportWorkflow::drawParsingPopup()
{
    if (mbOpenParsingPopup)
    {
        ImGui::SetNextWindowSize(ImVec2(360.0f, 100.0f), ImGuiCond_Appearing);
        ImGui::OpenPopup("##TextureParsingPopup");
        mbOpenParsingPopup = false;
    }

    if (ImGui::BeginPopupModal("##TextureParsingPopup", nullptr, ImGuiWindowFlags_NoDecoration))
    {
        ImGui::PushFont(nullptr, 30.0f);

        const char* text = "Parsing Texture...";
        const ImVec2 textSize = ImGui::CalcTextSize(text);
        const ImVec2 windowSize = ImGui::GetContentRegionAvail();

        const float posX = (windowSize.x - textSize.x) * 0.5f;
        const float posY = ImGui::GetCursorPos().y + (windowSize.y - textSize.y) * 0.5f;

        ImGui::SetCursorPos(ImVec2(posX, posY));
        ImGui::TextUnformatted(text);

        ImGui::PopFont();

        if (mState != eState::Parsing)
        {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

void TextureImportWorkflow::drawImportSettingPopup()
{
    if (mbOpenImportSettingPopup)
    {
        ImGui::SetNextWindowSize(ImVec2(800.0f, 0.0f), ImGuiCond_Appearing);
        ImGui::OpenPopup("Texture Import Setting##TextureImportSettingPopup");
        mbOpenImportSettingPopup = false;
    }

    if (ImGui::BeginPopupModal("Texture Import Setting##TextureImportSettingPopup",
                               nullptr,
                               ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse))
    {
        constexpr float labelColumnWidth = 140.0f;

        // =============================================================================================================
        // Draw Main Texture Information
        ImGui::SeparatorText("TEXTURE");
        if (ImGui::BeginTable("TextureMainInfoTable", 2, ImGuiTableFlags_SizingStretchProp))
        {
            ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed, labelColumnWidth);
            ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Name");
            ImGui::TableNextColumn();
            ImGui::AlignTextToFramePadding();
            ImGui::Text("%s", mParseContext.pParsedTextures[0]->NameStr.c_str());

            ImGui::EndTable();
        }

        // =============================================================================================================
        // Draw Source Details
        ImGui::Spacing();
        ImGui::SeparatorText("SOURCE");
        ImGui::Spacing();

        const int32_t texCount = (mParseContext.TextureType == eTextureType::Texture2D) ? 1 : 6;
        static const char* const sCubemapFaceNames[6] = {"X+", "X-", "Y+", "Y-", "Z+", "Z-"};

        for (int32_t i = 0; i < texCount; ++i)
        {
            ImGui::PushID(i);

            const std::string headerTitle =
                (texCount > 1 && i < 6) ? sCubemapFaceNames[i] : std::string("Texture ").append(std::to_string(i));

            if (ImGui::CollapsingHeader(headerTitle.c_str(), ImGuiTreeNodeFlags_None))
            {
                if (ImGui::BeginTable("SourceInfoTable", 2, ImGuiTableFlags_SizingStretchProp))
                {
                    ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed, labelColumnWidth);
                    ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

                    const parser::TextureIR& textureIR = *(mParseContext.pParsedTextures[i]);

                    // Name
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::AlignTextToFramePadding();
                    ImGui::Text("Name");
                    ImGui::TableNextColumn();
                    ImGui::AlignTextToFramePadding();
                    ImGui::Text("%s", textureIR.NameStr.c_str());

                    // Source Path
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::AlignTextToFramePadding();
                    ImGui::Text("Source");
                    ImGui::TableNextColumn();
                    ImGui::AlignTextToFramePadding();
                    ImGui::Text("%s", textureIR.ResourcePath.ToString().c_str());

                    // Format
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::AlignTextToFramePadding();
                    ImGui::Text("Format");
                    ImGui::TableNextColumn();
                    ImGui::AlignTextToFramePadding();
                    ImGui::Text("%s", getImageFormatName(textureIR.Img.GetFormat()));

                    // Color Channels
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::AlignTextToFramePadding();
                    ImGui::Text("Color Channels");
                    ImGui::TableNextColumn();
                    ImGui::AlignTextToFramePadding();
                    ImGui::Text("%u", textureIR.Img.GetPhysicalChannelCount());

                    // Resolution
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::AlignTextToFramePadding();
                    ImGui::Text("Width");
                    ImGui::TableNextColumn();
                    ImGui::AlignTextToFramePadding();
                    ImGui::Text("%u", textureIR.Img.GetWidth());

                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::AlignTextToFramePadding();
                    ImGui::Text("Height");
                    ImGui::TableNextColumn();
                    ImGui::AlignTextToFramePadding();
                    ImGui::Text("%u", textureIR.Img.GetHeight());

                    ImGui::EndTable();
                }
            }

            ImGui::PopID();
            ImGui::Spacing();
        }

        // =============================================================================================================
        // Draw Import Settings
        ImGui::Spacing();
        ImGui::SeparatorText("SETTINGS");
        if (ImGui::BeginTable("ImportSettingsTable", 2, ImGuiTableFlags_SizingStretchProp))
        {
            ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed, labelColumnWidth);
            ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

            // Format Combo
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Format");
            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(-1.0f);
            if (ImGui::BeginCombo("##TextureFormatCombo", getTextureFormatName(mImportContext.TextureFormat)))
            {
                const auto& formats =
                    getAvailableTextureFormats(mParseContext.pParsedTextures[0]->Img.IsLinear(),
                                               mParseContext.pParsedTextures[0]->Img.IsInteger(),
                                               mParseContext.pParsedTextures[0]->Img.GetPhysicalChannelCount());

                for (auto format : formats)
                {
                    const bool bSelected = (mImportContext.TextureFormat == format);
                    if (ImGui::Selectable(getTextureFormatName(format), bSelected))
                    {
                        mImportContext.TextureFormat = format;
                    }

                    if (bSelected)
                    {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }

            // Generate Mipmap Checkbox
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Generate Mipmap");
            ImGui::TableNextColumn();
            ImGui::Checkbox("##GenerateMipmap", &mImportContext.bGenerateMipmap);

            ImGui::EndTable();
        }

        // =============================================================================================================
        // Draw Buttons
        ImGui::Spacing();

        const float windowWidth = ImGui::GetContentRegionAvail().x;
        const float buttonWidth = 80.0f;
        const float allButtonWidth = buttonWidth + buttonWidth + ImGui::GetStyle().ItemSpacing.x;

        const float beginX = (windowWidth - allButtonWidth) * 0.5f;

        if (beginX > 0.0f)
        {
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + beginX);
        }

        if (ImGui::Button("Import", ImVec2(buttonWidth, 0.0f)))
        {
            mState = eState::ImportBegin;
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();

        if (ImGui::Button("Cancel", ImVec2(buttonWidth, 0.0f)))
        {
            mState = eState::Finish;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void TextureImportWorkflow::drawImportingPopup()
{
    if (mbOpenImportingPopup)
    {
        ImGui::SetNextWindowSize(ImVec2(360.0f, 100.0f), ImGuiCond_Appearing);
        ImGui::OpenPopup("##TextureImportingPopup");
        mbOpenImportingPopup = false;
    }

    if (ImGui::BeginPopupModal("##TextureImportingPopup", nullptr, ImGuiWindowFlags_NoDecoration))
    {
        ImGui::PushFont(nullptr, 24.0f);

        const char* text = "Importing Texture...";
        const ImVec2 textSize = ImGui::CalcTextSize(text);
        const float posX = (ImGui::GetContentRegionAvail().x - textSize.x) * 0.5f;

        ImGui::SetCursorPosX(posX);
        ImGui::TextUnformatted(text);

        ImGui::PopFont();

        ImGui::Spacing();

        const int32_t totalCount = mImportContext.TotalImportedTextureCount;
        const int32_t importedCount = mImportContext.ImportedTextureCount;

        const float progress =
            totalCount > 0 ? static_cast<float>(importedCount) / static_cast<float>(totalCount) : 0.0f;

        char buf[32];
        snprintf(buf, sizeof(buf), "%d / %d", importedCount, totalCount);

        ImGui::ProgressBar(progress, ImVec2(-1.0f, 0.0f), buf);

        if (mState != eState::Importing)
        {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void TextureImportWorkflow::drawParseFailedPopup()
{
    if (mbOpenParseFailedPopup)
    {
        ImGui::SetNextWindowSize(ImVec2(360.0f, 100.0f), ImGuiCond_Appearing);
        ImGui::OpenPopup("##TextureParseFailedPopup");
        mbOpenParseFailedPopup = false;
    }

    if (ImGui::BeginPopupModal("##TextureParseFailedPopup", nullptr, ImGuiWindowFlags_NoDecoration))
    {
        ImGui::PushFont(nullptr, 24.0f);

        const char* text = "Failed to parse.";
        const ImVec2 textSize = ImGui::CalcTextSize(text);
        const float posX = (ImGui::GetContentRegionAvail().x - textSize.x) * 0.5f;

        ImGui::SetCursorPosX(posX);
        ImGui::TextUnformatted(text);

        ImGui::PopFont();

        // =============================================================================================================
        // Draw Close Button
        const float buttonWidth = 80.0f;
        const float buttonHeight = ImGui::GetFrameHeight();

        const float availableWidth = ImGui::GetContentRegionAvail().x;
        const float availableHeight = ImGui::GetContentRegionAvail().y;

        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + availableHeight - buttonHeight);

        const float beginX = (availableWidth - buttonWidth) * 0.5f;
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + beginX);

        if (ImGui::Button("Close", ImVec2(buttonWidth, 0.0f)))
        {
            mState = eState::Finish;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void TextureImportWorkflow::drawImportFailedPopup()
{
    if (mbOpenImportFailedPopup)
    {
        ImGui::SetNextWindowSize(ImVec2(600.0f, 0.0f), ImGuiCond_Appearing);
        ImGui::OpenPopup("##TextureImportFailedPopup");
        mbOpenImportFailedPopup = false;
    }

    if (ImGui::BeginPopupModal("##TextureImportFailedPopup", nullptr, ImGuiWindowFlags_NoDecoration))
    {
        constexpr float labelColumnWidth = 140.0f;

        // =============================================================================================================
        // Title
        ImGui::PushFont(nullptr, 20.0f);

        const char* titleText = "Texture Import Failed";
        const ImVec2 textSize = ImGui::CalcTextSize(titleText);
        const ImVec2 windowSize = ImGui::GetContentRegionAvail();

        const float posX = (windowSize.x - textSize.x) * 0.5f;

        ImGui::SetCursorPos(ImVec2(posX, 25.0f));
        ImGui::TextUnformatted(titleText);

        ImGui::PopFont();
        ImGui::Spacing();

        const ImVec4 colorFailed(0.95f, 0.3f, 0.3f, 1.0f);

        // =============================================================================================================
        // Draw Texture Status Table
        ImGui::SeparatorText("TEXTURE");
        if (ImGui::BeginTable("TextureFailedResultTable", 2, ImGuiTableFlags_SizingStretchProp))
        {
            ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed, labelColumnWidth);
            ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

            // Texture Name
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Name");
            ImGui::TableNextColumn();
            ImGui::AlignTextToFramePadding();

            if (mParseContext.pParsedTextures[0] != nullptr)
            {
                ImGui::Text("%s", mParseContext.pParsedTextures[0]->NameStr.c_str());
            }
            else
            {
                ImGui::TextDisabled("Unknown");
            }

            // Status
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Status");
            ImGui::TableNextColumn();
            ImGui::AlignTextToFramePadding();
            if (mImportContext.hImportedTexture.IsValid())
            {
                ImGui::TextColored(colorFailed, "Import Failed.");
            }
            else
            {
                ImGui::TextDisabled("Import Not Started. Check for duplicate asset names.");
            }

            ImGui::EndTable();
        }

        // =============================================================================================================
        // Draw Button
        ImGui::Spacing();

        const float windowWidth = ImGui::GetContentRegionAvail().x;
        const float buttonWidth = 80.0f;
        const float beginX = (windowWidth - buttonWidth) * 0.5f;

        if (beginX > 0.0f)
        {
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + beginX);
        }

        if (ImGui::Button("Close", ImVec2(buttonWidth, 0.0f)))
        {
            mState = eState::Finish;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

std::string TextureImportWorkflow::showTextureFileOpenDialog()
{
    std::string filterNamesStr[] = {
        "Texture File(*.png; *.jpg; *.jpeg; *.tga; *.bmp; *.hdr; *.psd)",
    };
    std::string filterExtensionsStr[] = {
        "*.png;*.jpg;*.jpeg;*.tga;*.bmp;*.hdr;*.psd",
    };
    static_assert(HO_ARRAY_SIZE(filterNamesStr) == HO_ARRAY_SIZE(filterExtensionsStr));

    std::string path;
    IPlatformApplication::GetInstance().ShowOpenFileDialog(
        &path, "Import Texture", filterNamesStr, filterExtensionsStr, HO_ARRAY_SIZE(filterNamesStr), std::string());

    return path;
}

void TextureImportWorkflow::initializeParseContext(eTextureType textureType)
{
    mParseContext.TextureType = textureType;

    for (auto& path : mParseContext.Path)
    {
        path = Path();
    }
    mParseContext.TextureName[0] = '\0';
    mParseContext.bIsLinear = true;
}

void TextureImportWorkflow::clearParseContext()
{
    for (auto& pParsedTexture : mParseContext.pParsedTextures)
    {
        pParsedTexture = nullptr;
    }
}

bool TextureImportWorkflow::initializeImportContext()
{
    mImportContext.TextureType = mParseContext.TextureType;

    bool bSucceed = true;
    const bool bIsLinear = mParseContext.pParsedTextures[0]->Img.IsLinear();
    const bool bIsInteger = mParseContext.pParsedTextures[0]->Img.IsInteger();
    const int32_t channelCount = mParseContext.pParsedTextures[0]->Img.GetPhysicalChannelCount();
    if (bIsInteger)
    {
        if (bIsLinear)
        {
            switch (channelCount)
            {
                case 1:
                    mImportContext.TextureFormat = eTextureFormat::BC4_UNORM;
                    break;
                case 2:
                    mImportContext.TextureFormat = eTextureFormat::BC5_UNORM;
                    break;
                case 3:
                case 4:
                    mImportContext.TextureFormat = eTextureFormat::BC7_UNORM;
                    break;
                default:
                    HO_ASSERT(false, "Invalid channel count.");
                    bSucceed = false;
            }
        }
        else
        {
            switch (channelCount)
            {
                case 1:
                    mImportContext.TextureFormat = eTextureFormat::R8_SRGB;
                    break;
                case 2:
                    mImportContext.TextureFormat = eTextureFormat::R8G8_SRGB;
                    break;
                case 3:
                case 4:
                    mImportContext.TextureFormat = eTextureFormat::BC7_SRGB;
                    break;
                default:
                    HO_ASSERT(false, "Invalid channel count.");
                    bSucceed = false;
            }
        }
    }
    else
    {
        if (bIsLinear)
        {
            switch (channelCount)
            {
                case 1:
                    mImportContext.TextureFormat = eTextureFormat::R16_FLOAT;
                    break;
                case 2:
                    mImportContext.TextureFormat = eTextureFormat::R16G16_FLOAT;
                    break;
                case 3:
                case 4:
                    mImportContext.TextureFormat = eTextureFormat::BC6H_UF16;
                    break;
                default:
                    HO_ASSERT(false, "Invalid channel count.");
                    bSucceed = false;
            }
        }
        else
        {
            HO_ASSERT(false, "Invalid channel count.");
            bSucceed = false;
        }
    }

    mImportContext.bGenerateMipmap = true;

    mImportContext.TotalImportedTextureCount = 0;
    mImportContext.ImportedTextureCount = 0;

    mImportContext.hImportedTexture = ObjectHandle<RTextureAsset>::sNULL;

    return bSucceed;
}

void TextureImportWorkflow::clearImportContext()
{
    mImportContext.hImportedTexture = ObjectHandle<RTextureAsset>::sNULL;
}

void TextureImportWorkflow::deleteImpotedTexture() const
{
    AssetSystem::GetInstance().DeleteAsset(mImportContext.hImportedTexture);
}

bool TextureImportWorkflow::beginParsing()
{
    if (mState != eState::ParseBegin)
    {
        return false;
    }

    const int32_t texCount = mParseContext.TextureType == eTextureType::Texture2D ? 1 : 6;
    for (int32_t i = 0; i < texCount; ++i)
    {
        mParseContext.pParsedTextures[i] = parser::parseTextureFile(
            mParseContext.Path[i], std::string(mParseContext.TextureName), mParseContext.bIsLinear);
        if (!mParseContext.pParsedTextures[i])
        {
            return false;
        }
    }

    return true;
}

bool TextureImportWorkflow::checkParsing()
{
    if (mState != eState::Parsing)
    {
        return false;
    }

    const int32_t texCount = mParseContext.TextureType == eTextureType::Texture2D ? 1 : 6;
    for (int32_t i = 0; i < texCount; ++i)
    {
        if (!mParseContext.pParsedTextures[i])
        {
            return false;
        }
    }

    return true;
}

bool TextureImportWorkflow::beginImporting()
{
    if (mState != eState::ImportBegin)
    {
        return false;
    }

    bool bSucceed = true;
    switch (mImportContext.TextureType)
    {
        case eTextureType::Texture2D:
        {
            if (!mParseContext.pParsedTextures[0])
            {
                HO_ASSERT(false, "Texture is parsed incompletely.");
                return false;
            }
            mImportContext.hImportedTexture = AssetSystem::GetInstance().ImportTexture2D(
                *mParseContext.pParsedTextures[0], mImportContext.TextureFormat, mImportContext.bGenerateMipmap);
            ++mImportContext.TotalImportedTextureCount;
            if (mImportContext.hImportedTexture.IsNULL())
            {
                bSucceed = false;
            }
        }
        break;
        case eTextureType::TextureCubeMap:
        {
            for (auto& pParsedTexture : mParseContext.pParsedTextures)
            {
                if (!pParsedTexture)
                {
                    HO_ASSERT(false, "Texture is parsed incompletely.");
                    bSucceed = false;
                }
            }

            const int32_t width = mParseContext.pParsedTextures[0]->Img.GetWidth();
            const int32_t height = mParseContext.pParsedTextures[0]->Img.GetHeight();
            if (width != height)
            {
                bSucceed = false;
            }

            for (int32_t i = 1; i < 6; ++i)
            {
                if (mParseContext.pParsedTextures[i]->Img.GetWidth() !=
                        mParseContext.pParsedTextures[i]->Img.GetHeight() ||
                    mParseContext.pParsedTextures[i]->Img.GetWidth() != width ||
                    mParseContext.pParsedTextures[i]->Img.GetHeight() != height)
                {
                    bSucceed = false;
                }
            }

            mImportContext.hImportedTexture =
                AssetSystem::GetInstance().ImportTextureCubemap(mParseContext.pParsedTextures[0]->NameStr,
                                                                *mParseContext.pParsedTextures[0],
                                                                *mParseContext.pParsedTextures[1],
                                                                *mParseContext.pParsedTextures[2],
                                                                *mParseContext.pParsedTextures[3],
                                                                *mParseContext.pParsedTextures[4],
                                                                *mParseContext.pParsedTextures[5],
                                                                mImportContext.TextureFormat,
                                                                mImportContext.bGenerateMipmap);

            ++mImportContext.TotalImportedTextureCount;

            if (mImportContext.hImportedTexture.IsNULL())
            {
                bSucceed = false;
            }
        }
        break;
        default:
            HO_ASSERT(false, "Invalid texture type.");
            bSucceed = false;
    }

    return bSucceed;
}

bool TextureImportWorkflow::checkImporting()
{
    if (mState != eState::Importing)
    {
        return false;
    }

    bool bSucceed = true;
    mImportContext.ImportedTextureCount = 0;

    if (mImportContext.hImportedTexture.IsValid())
    {
        switch (mImportContext.hImportedTexture->TransferState)
        {
            case eAssetTransferState::Ready:
                ++mImportContext.ImportedTextureCount;
                break;
            case eAssetTransferState::Failed:
                ++mImportContext.ImportedTextureCount;
                bSucceed = false;
                break;
            default:
                break;
        }
    }
    else
    {
        ++mImportContext.ImportedTextureCount;
    }

    return bSucceed;
}

const char* getTextureFormatName(eTextureFormat format)
{
    switch (format)
    {
        case eTextureFormat::None:
            return "None";
        case eTextureFormat::R8_UNORM:
            return "R8_UNORM";
        case eTextureFormat::R8G8_UNORM:
            return "R8G8_UNORM";
        case eTextureFormat::R8G8B8A8_UNORM:
            return "R8G8B8A8_UNORM";
        case eTextureFormat::B8G8R8A8_UNORM:
            return "B8G8R8A8_UNORM";
        case eTextureFormat::R8_SRGB:
            return "R8_SRGB";
        case eTextureFormat::R8G8_SRGB:
            return "R8G8_SRGB";
        case eTextureFormat::R8G8B8A8_SRGB:
            return "R8G8B8A8_SRGB";
        case eTextureFormat::R16_FLOAT:
            return "R16_FLOAT";
        case eTextureFormat::R16G16_FLOAT:
            return "R16G16_FLOAT";
        case eTextureFormat::R16G16B16A16_FLOAT:
            return "R16G16B16A16_FLOAT";
        case eTextureFormat::R32_FLOAT:
            return "R32_FLOAT";
        case eTextureFormat::R32G32_FLOAT:
            return "R32G32_FLOAT";
        case eTextureFormat::R32G32B32A32_FLOAT:
            return "R32G32B32A32_FLOAT";
        case eTextureFormat::D24_UNORM_S8_UINT:
            return "D24_UNORM_S8_UINT";
        case eTextureFormat::D32_FLOAT:
            return "D32_FLOAT";
        case eTextureFormat::BC1_UNORM:
            return "BC1_UNORM";
        case eTextureFormat::BC1_SRGB:
            return "BC1_SRGB";
        case eTextureFormat::BC3_UNORM:
            return "BC3_UNORM";
        case eTextureFormat::BC3_SRGB:
            return "BC3_SRGB";
        case eTextureFormat::BC4_UNORM:
            return "BC4_UNORM";
        case eTextureFormat::BC4_SNORM:
            return "BC4_SNORM";
        case eTextureFormat::BC5_UNORM:
            return "BC5_UNORM";
        case eTextureFormat::BC5_SNORM:
            return "BC5_SNORM";
        case eTextureFormat::BC6H_UF16:
            return "BC6H_UF16";
        case eTextureFormat::BC6H_SF16:
            return "BC6H_SF16";
        case eTextureFormat::BC7_UNORM:
            return "BC7_UNORM";
        case eTextureFormat::BC7_SRGB:
            return "BC7_SRGB";
        default:
            HO_ASSERT(false, "Invalid texture format.");
            return "Unknown";
    }
}

const char* getImageFormatName(Image::eFormat format)
{
    switch (format)
    {
        case Image::eFormat::None:
            return "None";
        case Image::eFormat::R8_UNORM:
            return "R8_UNORM";
        case Image::eFormat::R8_SRGB:
            return "R8_SRGB";
        case Image::eFormat::R8G8_UNORM:
            return "R8G8_UNORM";
        case Image::eFormat::R8G8_SRGB:
            return "R8G8_SRGB";
        case Image::eFormat::R8G8B8_UNORM:
            return "R8G8B8_UNORM";
        case Image::eFormat::R8G8B8_SRGB:
            return "R8G8B8_SRGB";
        case Image::eFormat::R8G8B8A8_UNORM:
            return "R8G8B8A8_UNORM";
        case Image::eFormat::R8G8B8A8_SRGB:
            return "R8G8B8A8_SRGB";
        case Image::eFormat::R32_FLOAT:
            return "R32_FLOAT";
        case Image::eFormat::R32G32_FLOAT:
            return "R32G32_FLOAT";
        case Image::eFormat::R32G32B32_FLOAT:
            return "R32G32B32_FLOAT";
        case Image::eFormat::R32G32B32A32_FLOAT:
            return "R32G32B32A32_FLOAT";
        default:
            HO_ASSERT(false, "Invalid image format.");
            return "Unknown";
    }
}

static const std::vector<eTextureFormat>& getAvailableTextureFormats(bool bIsLinear, bool bIsInteger, int numChannel)
{
    static const std::vector<eTextureFormat> sEmpty = {};

    if (bIsInteger)
    {
        if (bIsLinear)
        {
            switch (numChannel)
            {
                case 1:
                {
                    static const std::vector<eTextureFormat> sFormats = {
                        eTextureFormat::BC4_UNORM, eTextureFormat::BC4_SNORM, eTextureFormat::R8_UNORM};
                    return sFormats;
                }
                case 2:
                {
                    static const std::vector<eTextureFormat> sFormats = {
                        eTextureFormat::BC5_UNORM, eTextureFormat::BC5_SNORM, eTextureFormat::R8G8_UNORM};
                    return sFormats;
                }
                case 3:
                case 4:
                {
                    static const std::vector<eTextureFormat> sFormats = {eTextureFormat::BC7_UNORM,
                                                                         eTextureFormat::BC3_UNORM,
                                                                         eTextureFormat::BC1_UNORM,
                                                                         eTextureFormat::R8G8B8A8_UNORM,
                                                                         eTextureFormat::B8G8R8A8_UNORM};
                    return sFormats;
                }
                default:
                    return sEmpty;
            }
        }
        else
        {
            switch (numChannel)
            {
                case 1:
                {
                    static const std::vector<eTextureFormat> sFormats = {eTextureFormat::R8_SRGB};
                    return sFormats;
                }
                case 2:
                {
                    static const std::vector<eTextureFormat> sFormats = {eTextureFormat::R8G8_SRGB};
                    return sFormats;
                }
                case 3:
                case 4:
                {
                    static const std::vector<eTextureFormat> sFormats = {eTextureFormat::BC7_SRGB,
                                                                         eTextureFormat::BC3_SRGB,
                                                                         eTextureFormat::BC1_SRGB,
                                                                         eTextureFormat::R8G8B8A8_SRGB};
                    return sFormats;
                }
                default:
                    return sEmpty;
            }
        }
    }
    else
    {
        if (bIsLinear)
        {
            switch (numChannel)
            {
                case 1:
                {
                    static const std::vector<eTextureFormat> sFormats = {eTextureFormat::R16_FLOAT,
                                                                         eTextureFormat::R32_FLOAT};
                    return sFormats;
                }
                case 2:
                {
                    static const std::vector<eTextureFormat> sFormats = {eTextureFormat::R16G16_FLOAT,
                                                                         eTextureFormat::R32G32_FLOAT};
                    return sFormats;
                }
                case 3:
                case 4:
                {
                    static const std::vector<eTextureFormat> sFormats = {eTextureFormat::BC6H_UF16,
                                                                         eTextureFormat::BC6H_SF16,
                                                                         eTextureFormat::R16G16B16A16_FLOAT,
                                                                         eTextureFormat::R32G32B32A32_FLOAT};
                    return sFormats;
                }
                default:
                    return sEmpty;
            }
        }
        else
        {
            return sEmpty;
        }
    }
}

} // namespace ho