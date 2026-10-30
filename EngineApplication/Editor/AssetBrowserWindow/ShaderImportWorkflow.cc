#include "ShaderImportWorkflow.h"

#include <imgui.h>

#include "Core/IO/ResourceParseFuncs.h"
#include "Platforms/IPlatformApplication.h"
#include "Subsystems/Asset/AssetSystem.h"

namespace ho
{
static const char* getMatTypeName(eMaterialAssetType type);
static const char* getShaderStageName(eShaderStage stage);

void ShaderImportWorkflow::Begin()
{
    if (mState != eState::None)
    {
        HO_ASSERT(false, "Workflow was already begun.");
        return;
    }
    mState = eState::Begin;
    mbNeedToDeleteShader = false;
}

void ShaderImportWorkflow::Draw()
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
        case eState::Begin:
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

void ShaderImportWorkflow::Process()
{
    switch (mState)
    {
        case eState::Begin:
        {
            const std::string path = showShaderFileOpenDialog();
            if (path.empty())
            {
                mState = eState::Finish;
                return;
            }
            initializeParseContext(path);
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
                initializeImportContext();
                mState = eState::ImportSetting;
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
                if (mImportContext.TotalImportedShaderCount > mImportContext.ImportedShaderCount)
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
                if (mImportContext.TotalImportedShaderCount > mImportContext.ImportedShaderCount)
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
            if (mbNeedToDeleteShader)
            {
                deleteImpotedShader();
            }
            clearImportContext();
            mState = eState::None;
            break;
        case eState::ImportFailed:
            mbNeedToDeleteShader = true;
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

void ShaderImportWorkflow::drawParseSettingPopup()
{
    if (mState != eState::ParseSetting)
    {
        return;
    }

    if (mbOpenParseSettingPopup)
    {
        ImGui::SetNextWindowSize(ImVec2(600.0f, 0.0f), ImGuiCond_Appearing);
        ImGui::OpenPopup("Shader Parse Setting##ShaderParseSettingPopup");
        mbOpenParseSettingPopup = false;
    }

    if (ImGui::BeginPopupModal("Shader Parse Setting##ShaderParseSettingPopup",
                               nullptr,
                               ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse))
    {
        constexpr float labelColumnWidth = 140.0f;

        // =============================================================================================================
        // Draw Parse Settings
        ImGui::SeparatorText("SHADER SETTING");

        if (ImGui::BeginTable("##ShaderParseSettingTable", 2, ImGuiTableFlags_SizingStretchProp))
        {
            ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed, labelColumnWidth);
            ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

            // Shader Source Path
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("Shader Source");
            ImGui::TableNextColumn();
            ImGui::Text("%s", mParseContext.Path.ToString().c_str());

            // Shader Name
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Shader Name");

            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(-1.0f);
            ImGui::InputText(
                "##ShaderNameInputText", mParseContext.ShaderName, HO_ARRAY_SIZE(mParseContext.ShaderName));

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

void ShaderImportWorkflow::drawParsingPopup()
{
    if (mbOpenParsingPopup)
    {
        ImGui::SetNextWindowSize(ImVec2(360.0f, 100.0f), ImGuiCond_Appearing);
        ImGui::OpenPopup("##ShaderParsingPopup");
        mbOpenParsingPopup = false;
    }

    if (ImGui::BeginPopupModal("##ShaderParsingPopup", nullptr, ImGuiWindowFlags_NoDecoration))
    {
        ImGui::PushFont(nullptr, 30.0f);

        const char* text = "Parsing Shader...";
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

void ShaderImportWorkflow::drawImportSettingPopup()
{
    if (mbOpenImportSettingPopup)
    {
        ImGui::SetNextWindowSize(ImVec2(800.0f, 0.0f), ImGuiCond_Appearing);
        ImGui::OpenPopup("Shader Import Setting##ShaderImportSettingPopup");
        mbOpenImportSettingPopup = false;
    }

    if (ImGui::BeginPopupModal("Shader Import Setting##ShaderImportSettingPopup",
                               nullptr,
                               ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse))
    {
        constexpr float labelColumnWidth = 190.0f;

        // =============================================================================================================
        // Draw Import Settings (SHADER)
        ImGui::SeparatorText("SHADER");
        if (ImGui::BeginTable("##ShaderInfoTable", 2, ImGuiTableFlags_SizingStretchProp))
        {
            ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed, labelColumnWidth);
            ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("Name");
            ImGui::TableNextColumn();
            ImGui::Text("%s", mParseContext.pParsedShader->NameStr.c_str());

            ImGui::EndTable();
        }

        // =============================================================================================================
        // Draw Source Code
        ImGui::Spacing();
        ImGui::SeparatorText("SOURCE");
        ImGui::Spacing();

        ImGui::InputTextMultiline("##ShaderSource",
                                  const_cast<char*>( // NOLINT(cppcoreguidelines-pro-type-const-cast)
                                      mParseContext.pParsedShader->SourceStr.c_str()),
                                  mParseContext.pParsedShader->SourceStr.size() + 1,
                                  ImVec2(-FLT_MIN, 350.0f),
                                  ImGuiInputTextFlags_ReadOnly);

        // =============================================================================================================
        // Draw Settings
        ImGui::Spacing();
        ImGui::SeparatorText("SETTINGS");

        if (ImGui::BeginTable("##ShaderSettingsTable", 2, ImGuiTableFlags_SizingStretchProp))
        {
            ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed, labelColumnWidth);
            ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

            // Stage Combo
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Stage");

            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(-1.0f);
            if (ImGui::BeginCombo("##ShaderStageCombo",
                                  getShaderStageName(mImportContext.ShaderStage),
                                  ImGuiComboFlags_PopupAlignLeft))
            {
                for (int32_t i = static_cast<int32_t>(eShaderStage::VertexShader);
                     i < static_cast<int32_t>(eShaderStage::Last);
                     ++i)
                {
                    const bool bSelected = (i == static_cast<int32_t>(mImportContext.ShaderStage));
                    if (ImGui::Selectable(getShaderStageName(static_cast<eShaderStage>(i)), bSelected))
                    {
                        mImportContext.ShaderStage = static_cast<eShaderStage>(i);
                    }

                    if (bSelected)
                    {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }

            // Compatible Material Type Combo
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Compatible Material Type");

            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(-1.0f);
            if (ImGui::BeginCombo("##ShaderCompatibleMaterialCombo",
                                  getMatTypeName(mImportContext.CompatibleMaterialType),
                                  ImGuiComboFlags_PopupAlignLeft))
            {
                for (int32_t i = static_cast<int32_t>(eMaterialAssetType::Unlit);
                     i <= static_cast<int32_t>(eMaterialAssetType::StandardLit);
                     ++i)
                {
                    const bool bSelected = (i == static_cast<int32_t>(mImportContext.CompatibleMaterialType));
                    if (ImGui::Selectable(getMatTypeName(static_cast<eMaterialAssetType>(i)), bSelected))
                    {
                        mImportContext.CompatibleMaterialType = static_cast<eMaterialAssetType>(i);
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

        // =============================================================================================================
        ImGui::EndPopup();
    }
}

void ShaderImportWorkflow::drawImportingPopup()
{
    if (mbOpenImportingPopup)
    {
        ImGui::SetNextWindowSize(ImVec2(360.0f, 100.0f), ImGuiCond_Appearing);
        ImGui::OpenPopup("##ShaderImportingPopup");
        mbOpenImportingPopup = false;
    }

    if (ImGui::BeginPopupModal("##ShaderImportingPopup", nullptr, ImGuiWindowFlags_NoDecoration))
    {
        ImGui::PushFont(nullptr, 24.0f);

        const char* text = "Importing Shader...";
        const ImVec2 textSize = ImGui::CalcTextSize(text);
        const float posX = (ImGui::GetContentRegionAvail().x - textSize.x) * 0.5f;

        ImGui::SetCursorPosX(posX);
        ImGui::TextUnformatted(text);

        ImGui::PopFont();

        ImGui::Spacing();

        const int32_t totalCount = mImportContext.TotalImportedShaderCount;
        const int32_t importedCount = mImportContext.ImportedShaderCount;

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

void ShaderImportWorkflow::drawParseFailedPopup()
{
    if (mbOpenParseFailedPopup)
    {
        ImGui::SetNextWindowSize(ImVec2(360.0f, 100.0f), ImGuiCond_Appearing);
        ImGui::OpenPopup("##ShaderParseFailedPopup");
        mbOpenParseFailedPopup = false;
    }

    if (ImGui::BeginPopupModal("##ShaderParseFailedPopup", nullptr, ImGuiWindowFlags_NoDecoration))
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

void ShaderImportWorkflow::drawImportFailedPopup()
{
    if (mbOpenImportFailedPopup)
    {
        ImGui::SetNextWindowSize(ImVec2(600.0f, 0.0f), ImGuiCond_Appearing);
        ImGui::OpenPopup("##ShaderImportFailedPopup");
        mbOpenImportFailedPopup = false;
    }

    if (ImGui::BeginPopupModal("##ShaderImportFailedPopup", nullptr, ImGuiWindowFlags_NoDecoration))
    {
        constexpr float labelColumnWidth = 140.0f;
        constexpr float buttonWidth = 80.0f;

        // =============================================================================================================
        // Title
        ImGui::PushFont(nullptr, 20.0f);

        const char* titleText = "Shader Import Failed";
        const ImVec2 textSize = ImGui::CalcTextSize(titleText);
        const ImVec2 windowSize = ImGui::GetContentRegionAvail();

        const float posX = (windowSize.x - textSize.x) * 0.5f;

        ImGui::SetCursorPos(ImVec2(posX, 25.0f));
        ImGui::TextUnformatted(titleText);

        ImGui::PopFont();
        ImGui::Spacing();

        // =============================================================================================================
        // Draw Shader Status Table
        ImGui::SeparatorText("SHADER");

        if (ImGui::BeginTable("##ShaderFailedResultTable", 2, ImGuiTableFlags_SizingStretchProp))
        {
            ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed, labelColumnWidth);
            ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

            // Shader Name
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("Name");
            ImGui::TableNextColumn();

            if (mParseContext.pParsedShader != nullptr)
            {
                ImGui::Text("%s", mParseContext.pParsedShader->NameStr.c_str());
            }
            else
            {
                ImGui::TextDisabled("Unknown");
            }

            // Status
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("Status");
            ImGui::TableNextColumn();

            const ImVec4 colorFailed(0.95f, 0.3f, 0.3f, 1.0f);
            if (mImportContext.hImportedShader.IsValid())
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
        // Draw Error Message
        if (mImportContext.hImportedShader.IsValid())
        {
            ImGui::Spacing();
            ImGui::SeparatorText("ERROR LOG");
            ImGui::Spacing();

            ImGui::InputTextMultiline("##ErrorMessage",
                                      const_cast<char*>( // NOLINT(cppcoreguidelines-pro-type-const-cast)
                                          mImportContext.CompileErrorMsg.c_str()),
                                      mImportContext.CompileErrorMsg.size() + 1,
                                      ImVec2(-FLT_MIN, 350.0f),
                                      ImGuiInputTextFlags_ReadOnly);
        }

        // =============================================================================================================
        // Draw Button
        ImGui::Spacing();

        const float windowWidth = ImGui::GetContentRegionAvail().x;
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

std::string ShaderImportWorkflow::showShaderFileOpenDialog()
{
    std::string filterNamesStr[] = {
        "Shader File(*.vert; *.frag; *.comp)",
    };
    std::string filterExtensionsStr[] = {
        "*.vert;*.frag;*.comp",
    };
    static_assert(HO_ARRAY_SIZE(filterNamesStr) == HO_ARRAY_SIZE(filterExtensionsStr));

    std::string path;
    IPlatformApplication::GetInstance().ShowOpenFileDialog(
        &path, "Import Shader", filterNamesStr, filterExtensionsStr, HO_ARRAY_SIZE(filterNamesStr), std::string());
    return path;
}

void ShaderImportWorkflow::initializeParseContext(const std::string& path)
{
    mParseContext.Path = Path(path);

    const std::string defaultName = mParseContext.Path.GetFileName().RemovedExtension().ToString();
    strncpy_s(mParseContext.ShaderName, defaultName.c_str(), sizeof(mParseContext.ShaderName) - 1);

    mParseContext.pParsedShader = nullptr;
}

void ShaderImportWorkflow::clearParseContext()
{
    mParseContext.pParsedShader = nullptr;
}

void ShaderImportWorkflow::initializeImportContext()
{
    static const Path sVertExt = Path(std::string(".vert"));
    static const Path sGeomExt = Path(std::string(".geom"));
    static const Path sFragExt = Path(std::string(".frag"));
    static const Path sCompExt = Path(std::string(".comp"));
    const Path ext = mParseContext.pParsedShader->ResourcePath.GetExtension();
    if (ext == sVertExt)
    {
        mImportContext.ShaderStage = eShaderStage::VertexShader;
    }
    else if (ext == sGeomExt)
    {
        mImportContext.ShaderStage = eShaderStage::GeometryShader;
    }
    else if (ext == sFragExt)
    {
        mImportContext.ShaderStage = eShaderStage::FragmentShader;
    }
    else if (ext == sCompExt)
    {
        mImportContext.ShaderStage = eShaderStage::ComputeShader;
    }
    else
    {
        HO_ASSERT(false, "Invalid extension. extension must be '.vert' or '.geom' or '.frag' or '.comp'.");
    }

    mImportContext.CompatibleMaterialType = eMaterialAssetType::StandardLit;

    mImportContext.CompileErrorMsg.clear();

    mImportContext.TotalImportedShaderCount = 0;
    mImportContext.ImportedShaderCount = 0;

    mImportContext.hImportedShader = ObjectHandle<RShaderAsset>::sNULL;
}

void ShaderImportWorkflow::clearImportContext()
{
    mImportContext.CompileErrorMsg.clear();
    mImportContext.hImportedShader = ObjectHandle<RShaderAsset>::sNULL;
}

void ShaderImportWorkflow::deleteImpotedShader() const
{
    AssetSystem::GetInstance().DeleteAsset(mImportContext.hImportedShader);
}

bool ShaderImportWorkflow::beginParsing()
{
    if (mState != eState::ParseBegin)
    {
        return false;
    }

    mParseContext.pParsedShader = parser::parseShaderFile(mParseContext.Path, std::string(mParseContext.ShaderName));
    if (!mParseContext.pParsedShader)
    {
        return false;
    }
    return true;
}

bool ShaderImportWorkflow::checkParsing()
{
    if (mState != eState::Parsing)
    {
        return false;
    }

    if (!mParseContext.pParsedShader)
    {
        return false;
    }

    return true;
}

bool ShaderImportWorkflow::beginImporting()
{
    if (mState != eState::ImportBegin)
    {
        return false;
    }

    bool bSucceed = true;

    mImportContext.hImportedShader = AssetSystem::GetInstance().ImportShader(&mImportContext.CompileErrorMsg,
                                                                             *mParseContext.pParsedShader,
                                                                             mImportContext.ShaderStage,
                                                                             mImportContext.CompatibleMaterialType);
    ++mImportContext.TotalImportedShaderCount;

    if (mImportContext.hImportedShader.IsNULL())
    {
        bSucceed = false;
    }

    return bSucceed;
}

bool ShaderImportWorkflow::checkImporting()
{
    if (mState != eState::Importing)
    {
        return false;
    }

    bool bSucceed = true;
    mImportContext.ImportedShaderCount = 0;

    if (mImportContext.hImportedShader.IsValid())
    {
        switch (mImportContext.hImportedShader->TransferState)
        {
            case eAssetTransferState::Ready:
                ++mImportContext.ImportedShaderCount;
                break;
            case eAssetTransferState::Failed:
                ++mImportContext.ImportedShaderCount;
                bSucceed = false;
                break;
            default:
                break;
        }
    }
    else
    {
        ++mImportContext.ImportedShaderCount;
    }

    return bSucceed;
}

const char* getMatTypeName(eMaterialAssetType type)
{
    switch (type)
    {
        case eMaterialAssetType::Unlit:
            return "Unlit";
        case eMaterialAssetType::Legacy:
            return "Legacy";
        case eMaterialAssetType::StandardLit:
            return "Standard Lit";
        default:
            HO_ASSERT(false, "Invalid material type.");
            return "None";
    }
}

const char* getShaderStageName(eShaderStage stage)
{
    switch (stage)
    {
        case eShaderStage::VertexShader:
            return "Vertex Shader";
        case eShaderStage::GeometryShader:
            return "Geometry Shader";
        case eShaderStage::FragmentShader:
            return "Fragment Shader";
        case eShaderStage::ComputeShader:
            return "Compute Shader";
        default:
            HO_ASSERT(false, "Invalid shader stage.");
            return "None";
    }
}

} // namespace ho