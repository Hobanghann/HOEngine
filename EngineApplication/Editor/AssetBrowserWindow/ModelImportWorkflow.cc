#include "ModelImportWorkflow.h"

#include <imgui.h>

#include "Core/IO/ResourceParseFuncs.h"
#include "Platforms/IPlatformApplication.h"
#include "Subsystems/Asset/AssetSystem.h"

namespace ho
{
static eTextureFormat getDefaultTextureFormatPerUsage(eMaterialTextureUsage usage);
static parser::MaterialIR::eTextureUsage toIRTextureUsage(eMaterialTextureUsage usage);

static const char* getMatTypeName(eMaterialAssetType type);
static const char* getTextureFormatName(eTextureFormat format);
static const std::vector<eTextureFormat>& getAvailableTextureFormatsPerUsage(eMaterialTextureUsage usage);

void ModelImportWorkflow::Begin()
{
    if (mState != eState::None)
    {
        HO_ASSERT(false, "Workflow was already begun.");
        return;
    }
    mState = eState::Begin;
    mbNeedToDeleteAssets = false;
}

void ModelImportWorkflow::Draw()
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

void ModelImportWorkflow::Process()
{
    switch (mState)
    {
        case eState::Begin:
        {
            const std::string path = showModelFileOpenDialog();
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
                if (mImportContext.TotalImportedAssetCount > mImportContext.ImportedAssetCount)
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
                if (mImportContext.TotalImportedAssetCount > mImportContext.ImportedAssetCount)
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
            if (mbNeedToDeleteAssets)
            {
                deleteImpotedAssets();
            }
            clearImportContext();
            mState = eState::None;
            break;
        case eState::ImportFailed:
            mbNeedToDeleteAssets = true;
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

void ModelImportWorkflow::drawParseSettingPopup()
{
    if (mState != eState::ParseSetting)
    {
        return;
    }

    if (mbOpenParseSettingPopup)
    {
        ImGui::OpenPopup("Model Parse Setting##ModelParseSettingPopup");
        mbOpenParseSettingPopup = false;
    }

    if (ImGui::BeginPopupModal("Model Parse Setting##ModelParseSettingPopup",
                               nullptr,
                               ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse))
    {
        constexpr float labelColumnWidth = 120.0f;
        // =============================================================================================================
        // Section 1: Model Source
        if (ImGui::BeginTable("##ModelSourceTable", 2))
        {
            ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed, labelColumnWidth);
            ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Model Source:");

            ImGui::TableNextColumn();
            ImGui::AlignTextToFramePadding();
            ImGui::TextUnformatted(mParseContext.Path.ToString().c_str());

            ImGui::EndTable();
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // Section 2: Model Name
        if (ImGui::BeginTable("##ModelNameTable", 2))
        {
            ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed, labelColumnWidth);
            ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Model Name:");

            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(-1.0f);
            ImGui::InputText("##ModelNameInputText", mParseContext.ModelName, HO_ARRAY_SIZE(mParseContext.ModelName));

            ImGui::EndTable();
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // Section 3: Mesh Settings
        if (ImGui::BeginTable("##MeshSettingsTable", 2))
        {
            ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed, labelColumnWidth);
            ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Mesh Settings:");

            ImGui::TableNextColumn();
            ImGui::Checkbox("Make Static", &mParseContext.bMakeStatic);

            ImGui::EndTable();
        }
        // =============================================================================================================

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::Spacing();

        // =============================================================================================================
        // Draw Buttons
        const float windowWidth = ImGui::GetContentRegionAvail().x;
        const float buttonWidth = 80.0f;
        const float allButtonWidth = buttonWidth + buttonWidth + ImGui::GetStyle().ItemSpacing.x;

        const float beginX = (windowWidth - allButtonWidth) * 0.5f;

        if (beginX > 0.0f)
        {
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + beginX);
        }

        if (ImGui::Button("Parse", ImVec2(buttonWidth, 0)))
        {
            mState = eState::ParseBegin;
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();

        if (ImGui::Button("Cancel", ImVec2(buttonWidth, 0)))
        {
            mState = eState::Finish;
            ImGui::CloseCurrentPopup();
        }
        // =============================================================================================================
        ImGui::EndPopup();
    }
}

void ModelImportWorkflow::drawParsingPopup()
{
    if (mbOpenParsingPopup)
    {
        ImGui::SetNextWindowSize(ImVec2(360.0f, 100.0f), ImGuiCond_Appearing);
        ImGui::OpenPopup("##ModelParsingPopup");
        mbOpenParsingPopup = false;
    }

    if (ImGui::BeginPopupModal("##ModelParsingPopup", nullptr, ImGuiWindowFlags_NoDecoration))
    {
        ImGui::PushFont(nullptr, 30.0f);

        const char* text = "Parsing Model...";
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

void ModelImportWorkflow::drawImportSettingPopup()
{
    if (mbOpenImportSettingPopup)
    {
        ImGui::SetNextWindowSize(ImVec2(800.0f, 600.0f), ImGuiCond_Appearing);
        ImGui::OpenPopup("Model Import Setting##ModelImportSettingPopup");
        mbOpenImportSettingPopup = false;
    }

    if (ImGui::BeginPopupModal("Model Import Setting##ModelImportSettingPopup",
                               nullptr,
                               ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse |
                                   ImGuiWindowFlags_HorizontalScrollbar))
    {
        constexpr float labelColumnWidth = 140.0f;

        // =============================================================================================================
        // Draw Import Settings (MODEL)
        ImGui::SeparatorText("MODEL");
        if (ImGui::BeginTable("##ModelTable", 2, ImGuiTableFlags_SizingStretchProp))
        {
            ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed, labelColumnWidth);
            ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("Name");
            ImGui::TableNextColumn();
            ImGui::Text("%s", mParseContext.pParsedModel->NameStr.c_str());

            ImGui::EndTable();
        }

        // =============================================================================================================
        // Draw Mesh Settings
        ImGui::Spacing();
        ImGui::SeparatorText("MESH");
        if (ImGui::BeginTable("##MeshTable", 2, ImGuiTableFlags_SizingStretchProp))
        {
            ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed, labelColumnWidth);
            ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

            // Mesh Name
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("Name");
            ImGui::TableNextColumn();
            ImGui::Text("%s", mParseContext.pParsedModel->pMeshIR->NameStr.c_str());

            // SubMeshes
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::AlignTextToFramePadding();
            ImGui::Text("SubMeshes");
            ImGui::TableNextColumn();
            if (ImGui::BeginChild("##SubMeshInfoArea", ImVec2(0, 120.0f), ImGuiChildFlags_Borders, 0))
            {
                for (const auto& subMeshIR : mParseContext.pParsedModel->pMeshIR->SubMeshes)
                {
                    ImGui::Text("%s", subMeshIR.NameStr.c_str());
                    ImGui::Spacing();
                }
            }
            ImGui::EndChild();

            // Mesh Type Combo
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Mesh Type");
            ImGui::TableNextColumn();
            {
                static const char* const sMeshTypeNames[] = {"Static Mesh", "Skeletal Mesh"};
                const char* currentPreview = mImportContext.bImportAsStaticMesh ? sMeshTypeNames[0] : sMeshTypeNames[1];

                ImGui::SetNextItemWidth(-1.0f);
                if (ImGui::BeginCombo("##MeshTypeCombo", currentPreview, ImGuiComboFlags_PopupAlignLeft))
                {
                    for (int32_t i = 0; i < 2; ++i)
                    {
                        const bool bIsStatic = (i == 0);
                        const bool bSelected = (mImportContext.bImportAsStaticMesh == bIsStatic);

                        if (ImGui::Selectable(sMeshTypeNames[i], bSelected))
                        {
                            mImportContext.bImportAsStaticMesh = bIsStatic;
                        }

                        if (bSelected)
                        {
                            ImGui::SetItemDefaultFocus();
                        }
                    }
                    ImGui::EndCombo();
                }
            }

            ImGui::EndTable();
        }

        // =============================================================================================================
        // Draw Material Settings
        ImGui::Spacing();
        ImGui::SeparatorText("MATERIAL");
        for (int32_t i = 0; i < static_cast<int32_t>(mParseContext.pParsedModel->pMaterialIRs.size()); ++i)
        {
            const parser::MaterialIR& materialIR = *(mParseContext.pParsedModel->pMaterialIRs[i]);
            if (ImGui::CollapsingHeader(materialIR.NameStr.c_str(), ImGuiTreeNodeFlags_None))
            {
                ImGui::PushID(i);
                if (ImGui::BeginTable("##MaterialItemTable", 2, ImGuiTableFlags_SizingStretchProp))
                {
                    ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed, labelColumnWidth);
                    ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

                    // Name
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::Text("Name");
                    ImGui::TableNextColumn();
                    ImGui::Text("%s", materialIR.NameStr.c_str());

                    // Type
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::AlignTextToFramePadding();
                    ImGui::Text("Type");
                    ImGui::TableNextColumn();
                    ImGui::SetNextItemWidth(-1.0f);
                    if (ImGui::BeginCombo("##MaterialCombo",
                                          getMatTypeName(mImportContext.MaterialTypes[i]),
                                          ImGuiComboFlags_PopupAlignLeft))
                    {
                        for (int32_t j = static_cast<int32_t>(eMaterialAssetType::Unlit);
                             j <= static_cast<int32_t>(eMaterialAssetType::StandardLit);
                             ++j)
                        {
                            const bool bSelected = (j == static_cast<int32_t>(mImportContext.MaterialTypes[i]));
                            if (ImGui::Selectable(getMatTypeName(static_cast<eMaterialAssetType>(j)), bSelected))
                            {
                                mImportContext.MaterialTypes[i] = static_cast<eMaterialAssetType>(j);
                            }

                            if (bSelected)
                            {
                                ImGui::SetItemDefaultFocus();
                            }
                        }
                        ImGui::EndCombo();
                    }

                    // Textures
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::AlignTextToFramePadding();
                    ImGui::Text("Bind Textures");
                    ImGui::TableNextColumn();

                    if (ImGui::TreeNode("View Textures##MaterialTree"))
                    {
                        if (ImGui::BeginTable("##BindTextureTable", 2, ImGuiTableFlags_SizingStretchProp))
                        {
                            ImGui::TableSetupColumn("Usage", ImGuiTableColumnFlags_WidthFixed, 120.0f);
                            ImGui::TableSetupColumn("Texture", ImGuiTableColumnFlags_WidthStretch);

                            auto drawTextureRow =
                                [&](const char* usageLabel, parser::MaterialIR::eTextureUsage texUsage)
                            {
                                ImGui::TableNextRow();
                                ImGui::TableNextColumn();
                                ImGui::Text("%s", usageLabel);
                                ImGui::TableNextColumn();

                                const int32_t texIdx = materialIR.TextureIRIndices[static_cast<int32_t>(texUsage)];
                                if (texIdx == -1)
                                {
                                    ImGui::TextDisabled("None");
                                }
                                else
                                {
                                    ImGui::Text("%s",
                                                mParseContext.pParsedModel->pTextureIRs[texIdx]
                                                    ->ResourcePath.GetFileName()
                                                    .ToString()
                                                    .c_str());
                                }
                            };

                            drawTextureRow("Diffuse", toIRTextureUsage(eMaterialTextureUsage::Diffuse));
                            drawTextureRow("Albedo", toIRTextureUsage(eMaterialTextureUsage::Albedo));
                            drawTextureRow("Specular", toIRTextureUsage(eMaterialTextureUsage::Specular));
                            drawTextureRow("Emissive", toIRTextureUsage(eMaterialTextureUsage::Emissive));
                            drawTextureRow("Normal", toIRTextureUsage(eMaterialTextureUsage::Normal));
                            drawTextureRow("Opacity", toIRTextureUsage(eMaterialTextureUsage::Opacity));
                            drawTextureRow("MetallicRoughness",
                                           toIRTextureUsage(eMaterialTextureUsage::MetallicRoughness));
                            drawTextureRow("AmbientOcclusion",
                                           toIRTextureUsage(eMaterialTextureUsage::AmbientOcclusion));

                            ImGui::EndTable();
                        }
                        ImGui::TreePop();
                    }

                    ImGui::EndTable();
                }
                ImGui::PopID();
                ImGui::Spacing();
            }
        }

        // =============================================================================================================
        // Draw Texture Settings
        ImGui::Spacing();
        ImGui::SeparatorText("TEXTURE");
        if (mParseContext.pParsedModel->pTextureIRs.empty())
        {
            ImGui::Text("There is no textures.");
        }
        else
        {
            for (int32_t i = 0; i < static_cast<int32_t>(mParseContext.pParsedModel->pTextureIRs.size()); ++i)
            {
                const parser::TextureIR& textureIR = *(mParseContext.pParsedModel->pTextureIRs[i]);
                if (ImGui::CollapsingHeader(textureIR.NameStr.c_str(), ImGuiTreeNodeFlags_None))
                {
                    ImGui::PushID(i);
                    if (ImGui::BeginTable("##TexturesTable", 2, ImGuiTableFlags_SizingStretchProp))
                    {
                        ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed, labelColumnWidth);
                        ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

                        // Name
                        ImGui::TableNextRow();
                        ImGui::TableNextColumn();
                        ImGui::Text("Name");
                        ImGui::TableNextColumn();
                        ImGui::Text("%s", textureIR.NameStr.c_str());

                        // Source Path
                        ImGui::TableNextRow();
                        ImGui::TableNextColumn();
                        ImGui::Text("Source");
                        ImGui::TableNextColumn();
                        ImGui::Text("%s", textureIR.ResourcePath.ToString().c_str());

                        // Format Combo
                        ImGui::TableNextRow();
                        ImGui::TableNextColumn();
                        ImGui::AlignTextToFramePadding();
                        ImGui::Text("Format");
                        ImGui::TableNextColumn();

                        const eMaterialTextureUsage currentUsage = mImportContext.TextureUsage[i];
                        eTextureFormat& currentFormat = mImportContext.TextureFormats[i];
                        const char* previewFormatName = getTextureFormatName(currentFormat);

                        ImGui::SetNextItemWidth(-1.0f);
                        if (ImGui::BeginCombo("##FormatCombo", previewFormatName))
                        {
                            const auto& availableFormats = getAvailableTextureFormatsPerUsage(currentUsage);
                            for (const eTextureFormat fmt : availableFormats)
                            {
                                const bool bSelected = (currentFormat == fmt);
                                if (ImGui::Selectable(getTextureFormatName(fmt), bSelected))
                                {
                                    currentFormat = fmt;
                                }

                                if (bSelected)
                                {
                                    ImGui::SetItemDefaultFocus();
                                }
                            }
                            ImGui::EndCombo();
                        }

                        // Generate Mipmap
                        ImGui::TableNextRow();
                        ImGui::TableNextColumn();
                        ImGui::AlignTextToFramePadding();
                        ImGui::Text("Mipmap");
                        ImGui::TableNextColumn();
                        ImGui::Checkbox("Generate Mipmap", &mImportContext.bGenerateMipmap[i]);

                        ImGui::EndTable();
                    }
                    ImGui::PopID();
                    ImGui::Spacing();
                }
            }
        }

        // =============================================================================================================
        // Draw Morph Targets Settings
        ImGui::Spacing();
        ImGui::SeparatorText("MORPH TARGETS");
        if (!mImportContext.bHasMorphTargets)
        {
            ImGui::AlignTextToFramePadding();
            ImGui::Text("There is no morph targets.");
        }
        else
        {
            if (ImGui::BeginTable("##MorphTargetsTable", 2, ImGuiTableFlags_SizingStretchProp))
            {
                ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed, labelColumnWidth);
                ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::AlignTextToFramePadding();
                ImGui::Text("Targets Per SubMesh");
                ImGui::TableNextColumn();
                if (ImGui::BeginChild("##MorphTargetsInfoArea",
                                      ImVec2(0, 150.0f),
                                      ImGuiChildFlags_Borders,
                                      ImGuiWindowFlags_AlwaysHorizontalScrollbar))
                {
                    for (int32_t i = 0; i < static_cast<int32_t>(mParseContext.pParsedModel->pMeshIR->SubMeshes.size());
                         ++i)
                    {
                        const auto& subMeshIR = mParseContext.pParsedModel->pMeshIR->SubMeshes[i];

                        if (subMeshIR.MorphTargets.empty())
                        {
                            continue;
                        }

                        ImGui::PushID(i);
                        const bool bTreeOpen =
                            ImGui::TreeNodeEx(subMeshIR.NameStr.c_str(),
                                              ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_OpenOnArrow);

                        if (bTreeOpen)
                        {
                            for (const auto& morphTarget : subMeshIR.MorphTargets)
                            {
                                ImGui::Text("%s", morphTarget.NameStr.c_str());
                            }
                            ImGui::TreePop();
                        }
                        ImGui::PopID();
                    }
                }
                ImGui::EndChild();

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::AlignTextToFramePadding();
                ImGui::Text("Import Settings");
                ImGui::TableNextColumn();
                ImGui::Checkbox("Import morph targets", &mImportContext.bImportMorphTargets);

                ImGui::EndTable();
            }
        }

        // =============================================================================================================
        // Draw Morph Animation Settings
        ImGui::Spacing();
        ImGui::SeparatorText("MORPH ANIMATIONS");
        if (!mImportContext.bHasMorphAnimation)
        {
            ImGui::Text("There is no morph animation.");
        }
        else
        {
            if (ImGui::BeginTable(
                    "##MorphAnimTable", 3, ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_BordersInnerH))
            {
                ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn("Duration", ImGuiTableColumnFlags_WidthFixed, 100.0f);
                ImGui::TableSetupColumn("Import", ImGuiTableColumnFlags_WidthFixed, 80.0f);
                ImGui::TableHeadersRow();

                for (int32_t i = 0; i < static_cast<int32_t>(mParseContext.pParsedModel->pAnimationIRs.size()); ++i)
                {
                    const auto& animIR = mParseContext.pParsedModel->pAnimationIRs[i];
                    if (animIR->MorphTargetTracks.empty())
                    {
                        continue;
                    }

                    ImGui::PushID(i);
                    ImGui::TableNextRow();

                    ImGui::TableNextColumn();
                    ImGui::AlignTextToFramePadding();
                    ImGui::Text("%s", animIR->NameStr.c_str());

                    ImGui::TableNextColumn();
                    ImGui::AlignTextToFramePadding();
                    ImGui::Text("%.2f sec", animIR->Duration);

                    ImGui::TableNextColumn();
                    ImGui::AlignTextToFramePadding();
                    ImGui::Checkbox("##ImportMorphAnimCheckbox", &mImportContext.bImportMorphAnimation[i]);

                    ImGui::PopID();
                }
                ImGui::EndTable();
            }
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

void ModelImportWorkflow::drawImportingPopup()
{
    if (mbOpenImportingPopup)
    {
        ImGui::SetNextWindowSize(ImVec2(360.0f, 100.0f), ImGuiCond_Appearing);
        ImGui::OpenPopup("##ModelImportingPopup");
        mbOpenImportingPopup = false;
    }

    if (ImGui::BeginPopupModal("##ModelImportingPopup", nullptr, ImGuiWindowFlags_NoDecoration))
    {
        ImGui::PushFont(nullptr, 24.0f);

        const char* text = "Importing Model...";
        const ImVec2 textSize = ImGui::CalcTextSize(text);
        const float posX = (ImGui::GetContentRegionAvail().x - textSize.x) * 0.5f;

        ImGui::SetCursorPosX(posX);
        ImGui::TextUnformatted(text);

        ImGui::PopFont();

        ImGui::Spacing();

        const int32_t totalCount = mImportContext.TotalImportedAssetCount;
        const int32_t importedCount = mImportContext.ImportedAssetCount;

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

void ModelImportWorkflow::drawParseFailedPopup()
{
    if (mbOpenParseFailedPopup)
    {
        ImGui::SetNextWindowSize(ImVec2(360.0f, 100.0f), ImGuiCond_Appearing);
        ImGui::OpenPopup("##ModelParseFailedPopup");
        mbOpenParseFailedPopup = false;
    }

    if (ImGui::BeginPopupModal("##ModelParseFailedPopup", nullptr, ImGuiWindowFlags_NoDecoration))
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

void ModelImportWorkflow::drawImportFailedPopup()
{
    if (mbOpenImportFailedPopup)
    {
        ImGui::SetNextWindowSize(ImVec2(600.0f, 0.0f), ImGuiCond_Appearing);
        ImGui::OpenPopup("##ModelImportFailedPopup");
        mbOpenImportFailedPopup = false;
    }

    if (ImGui::BeginPopupModal("##ModelImportFailedPopup", nullptr, ImGuiWindowFlags_NoDecoration))
    {
        constexpr float labelColumnWidth = 200.0f;

        // =============================================================================================================
        // Title
        ImGui::PushFont(nullptr, 20.0f);

        const char* titleText = "Model Import Failed";
        const ImVec2 textSize = ImGui::CalcTextSize(titleText);
        const ImVec2 windowSize = ImGui::GetContentRegionAvail();

        const float posX = (windowSize.x - textSize.x) * 0.5f;

        ImGui::SetCursorPos(ImVec2(posX, 25.0f));
        ImGui::TextUnformatted(titleText);

        ImGui::PopFont();
        ImGui::Spacing();

        const ImVec4 colorSuccess(0.3f, 0.85f, 0.3f, 1.0f);
        const ImVec4 colorFailed(0.95f, 0.3f, 0.3f, 1.0f);

        // =============================================================================================================
        // Draw Asset Status
        const auto drawAssetStatus = [&](const auto& handle)
        {
            ImGui::AlignTextToFramePadding();

            if (handle.IsNULL())
            {
                ImGui::TextDisabled("Import Not Started. Check for duplicate asset names.");
                return;
            }

            if (!handle.IsValid())
            {
                ImGui::TextColored(colorFailed, "Import Failed.");
                return;
            }

            switch (handle->TransferState)
            {
                case eAssetTransferState::Ready:
                    ImGui::TextColored(colorSuccess, "Import Succeed.");
                    break;

                case eAssetTransferState::Failed:
                    ImGui::TextColored(colorFailed, "Import Failed.");
                    break;

                default:
                    ImGui::Text("Import Pending.");
                    break;
            }
        };

        // =============================================================================================================
        // MESH
        ImGui::SeparatorText("MESH");
        if (ImGui::BeginTable("##MeshResultTable", 2, ImGuiTableFlags_SizingStretchProp))
        {
            ImGui::TableSetupColumn("Target", ImGuiTableColumnFlags_WidthFixed, labelColumnWidth);
            ImGui::TableSetupColumn("Status", ImGuiTableColumnFlags_WidthStretch);

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("%s", mParseContext.pParsedModel->pMeshIR->NameStr.c_str());

            ImGui::TableNextColumn();
            drawAssetStatus(mImportContext.hImportedStaticMesh);

            ImGui::EndTable();
        }

        // =============================================================================================================
        // MATERIAL
        ImGui::Spacing();
        ImGui::SeparatorText("MATERIAL");
        if (ImGui::BeginTable("##MaterialResultTable", 2, ImGuiTableFlags_SizingStretchProp))
        {
            ImGui::TableSetupColumn("Target", ImGuiTableColumnFlags_WidthFixed, labelColumnWidth);
            ImGui::TableSetupColumn("Status", ImGuiTableColumnFlags_WidthStretch);

            for (int32_t i = 0; i < static_cast<int32_t>(mParseContext.pParsedModel->pMaterialIRs.size()); ++i)
            {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("%s", mParseContext.pParsedModel->pMaterialIRs[i]->NameStr.c_str());

                ImGui::TableNextColumn();

                if (i < mImportContext.hImportedMaterials.GetSize())
                {
                    drawAssetStatus(mImportContext.hImportedMaterials[i]);
                }
                else
                {
                    ImGui::AlignTextToFramePadding();
                    ImGui::TextDisabled("Import Not Started.");
                }
            }

            ImGui::EndTable();
        }

        // =============================================================================================================
        // TEXTURE
        ImGui::Spacing();
        ImGui::SeparatorText("TEXTURE");
        if (ImGui::BeginTable("##TextureResultTable", 2, ImGuiTableFlags_SizingStretchProp))
        {
            ImGui::TableSetupColumn("Target", ImGuiTableColumnFlags_WidthFixed, labelColumnWidth);
            ImGui::TableSetupColumn("Status", ImGuiTableColumnFlags_WidthStretch);

            for (int32_t i = 0; i < static_cast<int32_t>(mParseContext.pParsedModel->pTextureIRs.size()); ++i)
            {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("%s", mParseContext.pParsedModel->pTextureIRs[i]->NameStr.c_str());

                ImGui::TableNextColumn();

                if (i < mImportContext.hImportedTextures.GetSize())
                {
                    drawAssetStatus(mImportContext.hImportedTextures[i]);
                }
                else
                {
                    ImGui::AlignTextToFramePadding();
                    ImGui::TextDisabled("Import Not Started.");
                }
            }

            ImGui::EndTable();
        }

        // =============================================================================================================
        // MORPH TARGETS
        ImGui::Spacing();
        ImGui::SeparatorText("MORPH TARGETS");
        if (ImGui::BeginTable("##MorphTargetResultTable", 2, ImGuiTableFlags_SizingStretchProp))
        {
            ImGui::TableSetupColumn("Target", ImGuiTableColumnFlags_WidthFixed, labelColumnWidth);
            ImGui::TableSetupColumn("Status", ImGuiTableColumnFlags_WidthStretch);

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("%s", mParseContext.pParsedModel->pMeshIR->NameStr.c_str());

            ImGui::TableNextColumn();
            drawAssetStatus(mImportContext.hImportedMorphTargets);

            ImGui::EndTable();
        }

        // =============================================================================================================
        // MORPH ANIMATIONS
        ImGui::Spacing();
        ImGui::SeparatorText("MORPH ANIMATIONS");
        if (ImGui::BeginTable("##MorphAnimResultTable", 2, ImGuiTableFlags_SizingStretchProp))
        {
            ImGui::TableSetupColumn("Target", ImGuiTableColumnFlags_WidthFixed, labelColumnWidth);
            ImGui::TableSetupColumn("Status", ImGuiTableColumnFlags_WidthStretch);

            for (int32_t i = 0; i < static_cast<int32_t>(mParseContext.pParsedModel->pAnimationIRs.size()); ++i)
            {
                const auto& animIR = mParseContext.pParsedModel->pAnimationIRs[i];

                if (animIR->MorphTargetTracks.empty())
                {
                    continue;
                }

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("%s", animIR->NameStr.c_str());

                ImGui::TableNextColumn();

                if (i < mImportContext.hImportedMorphAnims.GetSize())
                {
                    drawAssetStatus(mImportContext.hImportedMorphAnims[i]);
                }
                else
                {
                    ImGui::AlignTextToFramePadding();
                    ImGui::TextDisabled("Import Not Started.");
                }
            }

            ImGui::EndTable();
        }

        // =============================================================================================================
        // Draw Close Button
        ImGui::Spacing();

        const float btnWindowWidth = ImGui::GetContentRegionAvail().x;
        const float buttonWidth = 80.0f;
        const float beginX = (btnWindowWidth - buttonWidth) * 0.5f;

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

std::string ModelImportWorkflow::showModelFileOpenDialog()
{
    std::string filterNamesStr[] = {
        "GLTF File(*.gltf; *.glb)",
        "OBJ File(*.obj)",
    };
    std::string filterExtensionsStr[] = {
        "*.gltf;*.glb",
        "*.obj",
    };
    static_assert(HO_ARRAY_SIZE(filterNamesStr) == HO_ARRAY_SIZE(filterExtensionsStr));

    std::string path;
    IPlatformApplication::GetInstance().ShowOpenFileDialog(
        &path, "Import Model", filterNamesStr, filterExtensionsStr, HO_ARRAY_SIZE(filterNamesStr), std::string());
    return path;
}

void ModelImportWorkflow::initializeParseContext(const std::string& importPathStr)
{
    mParseContext.Path = Path(importPathStr);

    const std::string defaultName = mParseContext.Path.GetFileName().RemovedExtension().ToString();
    strncpy_s(mParseContext.ModelName, defaultName.c_str(), sizeof(mParseContext.ModelName) - 1);
    mParseContext.bMakeStatic = false;

    mParseContext.pParsedModel = nullptr;
}

void ModelImportWorkflow::clearParseContext()
{
    mParseContext.pParsedModel = nullptr;
}

void ModelImportWorkflow::initializeImportContext()
{
    mImportContext.bImportAsStaticMesh = true;

    if (mParseContext.pParsedModel)
    {
        mImportContext.MaterialTypes = FixedArray<eMaterialAssetType>(
            static_cast<int32_t>(mParseContext.pParsedModel->pMaterialIRs.size()), eMaterialAssetType::StandardLit);

        mImportContext.TextureUsage =
            FixedArray<eMaterialTextureUsage>(static_cast<int32_t>(mParseContext.pParsedModel->pTextureIRs.size()));
        mImportContext.TextureFormats =
            FixedArray<eTextureFormat>(static_cast<int32_t>(mParseContext.pParsedModel->pTextureIRs.size()));

        for (int32_t texIdx = 0;
             texIdx < static_cast<int32_t>(static_cast<int32_t>(mParseContext.pParsedModel->pTextureIRs.size()));
             ++texIdx)
        {
            for (const auto& pMaterialIR : mParseContext.pParsedModel->pMaterialIRs)
            {
                for (int32_t usageIdx = 1; usageIdx < static_cast<int32_t>(eMaterialTextureUsage::Last); ++usageIdx)
                {
                    if (pMaterialIR->TextureIRIndices[static_cast<int32_t>(
                            toIRTextureUsage(static_cast<eMaterialTextureUsage>(usageIdx)))] == texIdx)
                    {
                        mImportContext.TextureUsage[texIdx] = static_cast<eMaterialTextureUsage>(usageIdx);
                        mImportContext.TextureFormats[texIdx] =
                            getDefaultTextureFormatPerUsage(static_cast<eMaterialTextureUsage>(usageIdx));
                    }
                }
            }
        }
        mImportContext.bGenerateMipmap =
            FixedArray<bool>(static_cast<int32_t>(mParseContext.pParsedModel->pTextureIRs.size()), true);

        mImportContext.bHasMorphTargets = false;
        for (const auto& subMesh : mParseContext.pParsedModel->pMeshIR->SubMeshes)
        {
            if (!subMesh.MorphTargets.empty())
            {
                mImportContext.bHasMorphTargets = true;
                break;
            }
        }
        mImportContext.bImportMorphTargets = true;

        mImportContext.bHasMorphAnimation = false;
        for (const auto& pAnimIR : mParseContext.pParsedModel->pAnimationIRs)
        {
            if (!pAnimIR->MorphTargetTracks.empty())
            {
                mImportContext.bHasMorphAnimation = true;
                break;
            }
        }

        mImportContext.bImportMorphAnimation =
            FixedArray<bool>(static_cast<int32_t>(mParseContext.pParsedModel->pAnimationIRs.size()), true);
    }

    mImportContext.TotalImportedAssetCount = 0;
    mImportContext.ImportedAssetCount = 0;
}

void ModelImportWorkflow::clearImportContext()
{
    mImportContext.MaterialTypes.Release();
    mImportContext.TextureUsage.Release();
    mImportContext.TextureFormats.Release();
    mImportContext.bGenerateMipmap.Release();
    mImportContext.bImportMorphAnimation.Release();

    mImportContext.hImportedStaticMesh = ObjectHandle<RStaticMeshAsset>::sNULL;
    mImportContext.hImportedMaterials.Release();
    mImportContext.hImportedTextures.Release();
    mImportContext.hImportedMorphTargets = ObjectHandle<RMorphTargetsAsset>::sNULL;
    mImportContext.hImportedMorphAnims.Release();
}

void ModelImportWorkflow::deleteImpotedAssets()
{
    AssetSystem::GetInstance().DeleteAsset(mImportContext.hImportedStaticMesh);

    for (int32_t i = 0; i < mImportContext.hImportedMaterials.GetSize(); ++i)
    {
        AssetSystem::GetInstance().DeleteAsset(mImportContext.hImportedMaterials[i]);
    }

    for (int32_t i = 0; i < mImportContext.hImportedTextures.GetSize(); ++i)
    {
        AssetSystem::GetInstance().DeleteAsset(mImportContext.hImportedTextures[i]);
    }

    AssetSystem::GetInstance().DeleteAsset(mImportContext.hImportedMorphTargets);

    for (int32_t i = 0; i < mImportContext.hImportedMorphAnims.GetSize(); ++i)
    {
        AssetSystem::GetInstance().DeleteAsset(mImportContext.hImportedMorphAnims[i]);
    }
}

bool ModelImportWorkflow::beginParsing()
{
    if (mState != eState::ParseBegin)
    {
        return false;
    }
    mParseContext.pParsedModel =
        parser::parseModelFile(mParseContext.Path, std::string(mParseContext.ModelName), mParseContext.bMakeStatic);
    return true;
}

bool ModelImportWorkflow::checkParsing()
{
    if (mState != eState::Parsing)
    {
        return false;
    }
    return mParseContext.pParsedModel != nullptr;
}

bool ModelImportWorkflow::beginImporting()
{
    if (mState != eState::ImportBegin)
    {
        return false;
    }

    if (!mParseContext.pParsedModel)
    {
        HO_ASSERT(false, "There is no parsed model.");
        return false;
    }

    bool bSucceed = true;
    // Import mesh
    if (mImportContext.bImportAsStaticMesh)
    {
        mImportContext.hImportedStaticMesh =
            AssetSystem::GetInstance().ImportStaticMesh(*mParseContext.pParsedModel->pMeshIR,
                                                        mParseContext.pParsedModel->pMaterialIRs,
                                                        *mParseContext.pParsedModel->pSkeletonIR,
                                                        *mParseContext.pParsedModel->pSkinIR);
        ++mImportContext.TotalImportedAssetCount;
        if (mImportContext.hImportedStaticMesh.IsNULL())
        {
            bSucceed = false;
        }
    }
    else
    {
        HO_ASSERT(false, "Currently not supported skeletal mesh.");
        bSucceed = false;
    }

    // Import textures
    mImportContext.hImportedTextures = FixedArray<ObjectHandle<RTextureAsset>>(mImportContext.TextureFormats.GetSize());
    for (int32_t i = 0; i < mImportContext.TextureFormats.GetSize(); ++i)
    {
        mImportContext.hImportedTextures[i] =
            AssetSystem::GetInstance().ImportTexture2D(*mParseContext.pParsedModel->pTextureIRs[i],
                                                       mImportContext.TextureFormats[i],
                                                       mImportContext.bGenerateMipmap[i]);
        ++mImportContext.TotalImportedAssetCount;
        if (mImportContext.hImportedTextures[i].IsNULL())
        {
            bSucceed = false;
        }
    }

    // Import materials
    mImportContext.hImportedMaterials =
        FixedArray<ObjectHandle<RMaterialAsset>>(mImportContext.MaterialTypes.GetSize());
    for (int32_t i = 0; i < mImportContext.MaterialTypes.GetSize(); ++i)
    {
        mImportContext.hImportedMaterials[i] =
            AssetSystem::GetInstance().ImportTexturedMaterial(*mParseContext.pParsedModel->pMaterialIRs[i],
                                                              mImportContext.MaterialTypes[i],
                                                              mParseContext.pParsedModel->pTextureIRs);
        ++mImportContext.TotalImportedAssetCount;
        if (mImportContext.hImportedMaterials[i].IsNULL())
        {
            bSucceed = false;
        }
    }

    // Import morph targets
    if (mImportContext.bHasMorphTargets && mImportContext.bImportMorphTargets)
    {
        mImportContext.hImportedMorphTargets =
            AssetSystem::GetInstance().ImportMorphTargets(*mParseContext.pParsedModel->pMeshIR);
        ++mImportContext.TotalImportedAssetCount;
        if (mImportContext.hImportedMorphTargets.IsNULL())
        {
            bSucceed = false;
        }
    }

    // Import morph animations
    mImportContext.hImportedMorphAnims =
        FixedArray<ObjectHandle<RMorphAnimationAsset>>(mImportContext.bImportMorphAnimation.GetSize());
    if (mImportContext.bHasMorphAnimation)
    {
        for (int32_t i = 0; i < mImportContext.bImportMorphAnimation.GetSize(); ++i)
        {
            if (mImportContext.bImportMorphAnimation[i])
            {
                mImportContext.hImportedMorphAnims[i] =
                    AssetSystem::GetInstance().ImportMorphAnimation(*mParseContext.pParsedModel->pAnimationIRs[i]);
                ++mImportContext.TotalImportedAssetCount;
                if (mImportContext.hImportedMorphAnims[i].IsNULL())
                {
                    bSucceed = false;
                }
            }
        }
    }

    return bSucceed;
}

bool ModelImportWorkflow::checkImporting()
{
    if (mState != eState::Importing)
    {
        return false;
    }

    bool bSucceed = true;
    mImportContext.ImportedAssetCount = 0;

    if (mImportContext.hImportedStaticMesh.IsValid())
    {
        switch (mImportContext.hImportedStaticMesh->TransferState)
        {
            case eAssetTransferState::Ready:
                ++mImportContext.ImportedAssetCount;
                break;
            case eAssetTransferState::Failed:
                ++mImportContext.ImportedAssetCount;
                bSucceed = false;
                break;
            default:
                break;
        }
    }
    else
    {
        ++mImportContext.ImportedAssetCount;
    }

    for (int32_t i = 0; i < mImportContext.hImportedMaterials.GetSize(); ++i)
    {
        if (mImportContext.hImportedMaterials[i].IsValid())
        {
            switch (mImportContext.hImportedMaterials[i]->TransferState)
            {
                case eAssetTransferState::Ready:
                    ++mImportContext.ImportedAssetCount;
                    break;
                case eAssetTransferState::Failed:
                    ++mImportContext.ImportedAssetCount;
                    bSucceed = false;
                    break;
                default:
                    break;
            }
        }
        else
        {
            ++mImportContext.ImportedAssetCount;
        }
    }

    for (int32_t i = 0; i < mImportContext.hImportedTextures.GetSize(); ++i)
    {
        if (mImportContext.hImportedTextures[i].IsValid())
        {
            switch (mImportContext.hImportedTextures[i]->TransferState)
            {
                case eAssetTransferState::Ready:
                    ++mImportContext.ImportedAssetCount;
                    break;
                case eAssetTransferState::Failed:
                    ++mImportContext.ImportedAssetCount;
                    bSucceed = false;
                    break;
                default:
                    break;
            }
        }
        else
        {
            ++mImportContext.ImportedAssetCount;
        }
    }

    if (mImportContext.hImportedMorphTargets.IsValid())
    {
        switch (mImportContext.hImportedMorphTargets->TransferState)
        {
            case eAssetTransferState::Ready:
                ++mImportContext.ImportedAssetCount;
                break;
            case eAssetTransferState::Failed:
                ++mImportContext.ImportedAssetCount;
                bSucceed = false;
                break;
            default:
                break;
        }
    }
    else
    {
        ++mImportContext.ImportedAssetCount;
    }

    for (int32_t i = 0; i < mImportContext.hImportedMorphAnims.GetSize(); ++i)
    {
        if (mImportContext.hImportedMorphAnims[i].IsValid())
        {
            switch (mImportContext.hImportedMorphAnims[i]->TransferState)
            {
                case eAssetTransferState::Ready:
                    ++mImportContext.ImportedAssetCount;
                    break;
                case eAssetTransferState::Failed:
                    ++mImportContext.ImportedAssetCount;
                    bSucceed = false;
                    break;
                default:
                    break;
            }
        }
        else
        {
            ++mImportContext.ImportedAssetCount;
        }
    }

    if (bSucceed && (mImportContext.TotalImportedAssetCount == mImportContext.ImportedAssetCount))
    {
        if (mImportContext.hImportedStaticMesh.IsValid())
        {
            AssetSystem::GetInstance().ResolveStaticMeshMaterials(mImportContext.hImportedStaticMesh);
        }

        for (int32_t i = 0; i < mImportContext.hImportedMaterials.GetSize(); ++i)
        {
            if (mImportContext.hImportedMaterials[i].IsValid())
            {
                AssetSystem::GetInstance().ResolveMaterialTextures(mImportContext.hImportedMaterials[i]);
            }
        }
    }

    return bSucceed;
}

eTextureFormat getDefaultTextureFormatPerUsage(eMaterialTextureUsage usage)
{
    switch (usage)
    {
        case eMaterialTextureUsage::Diffuse:
        case eMaterialTextureUsage::Specular:
        case eMaterialTextureUsage::Albedo:
        case eMaterialTextureUsage::Emissive:
            return eTextureFormat::BC7_SRGB;
        case eMaterialTextureUsage::Opacity:
            return eTextureFormat::BC4_UNORM;
        case eMaterialTextureUsage::Normal:
            return eTextureFormat::BC5_UNORM;
        case eMaterialTextureUsage::MetallicRoughness:
        case eMaterialTextureUsage::AmbientOcclusion:
            return eTextureFormat::BC7_UNORM;
        default:
            HO_ASSERT(false, "Invalid texture usage.");
            return eTextureFormat::None;
    }
}

parser::MaterialIR::eTextureUsage toIRTextureUsage(eMaterialTextureUsage usage)
{
    switch (usage)
    {
        case eMaterialTextureUsage::Diffuse:
            return parser::MaterialIR::eTextureUsage::Diffuse;
        case eMaterialTextureUsage::Specular:
            return parser::MaterialIR::eTextureUsage::Specular;
        case eMaterialTextureUsage::Opacity:
            return parser::MaterialIR::eTextureUsage::Opacity;
        case eMaterialTextureUsage::Normal:
            return parser::MaterialIR::eTextureUsage::Normal;
        case eMaterialTextureUsage::Albedo:
            return parser::MaterialIR::eTextureUsage::Albedo;
        case eMaterialTextureUsage::MetallicRoughness:
            return parser::MaterialIR::eTextureUsage::MetallicRoughness;
        case eMaterialTextureUsage::Emissive:
            return parser::MaterialIR::eTextureUsage::Emissive;
        case eMaterialTextureUsage::AmbientOcclusion:
            return parser::MaterialIR::eTextureUsage::AmbientOcclusion;
        default:
            HO_ASSERT(false, "Invalid texture usage.");
            return parser::MaterialIR::eTextureUsage::None;
    }
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

const std::vector<eTextureFormat>& getAvailableTextureFormatsPerUsage(eMaterialTextureUsage usage)
{
    switch (usage)
    {
        case eMaterialTextureUsage::Diffuse:
        case eMaterialTextureUsage::Specular:
        case eMaterialTextureUsage::Albedo:
        {
            static const std::vector<eTextureFormat> sFormats = {eTextureFormat::BC7_SRGB,
                                                                 eTextureFormat::BC3_SRGB,
                                                                 eTextureFormat::BC1_SRGB,
                                                                 eTextureFormat::R8G8B8A8_SRGB};
            return sFormats;
        }
        case eMaterialTextureUsage::Opacity:

        {
            static const std::vector<eTextureFormat> sFormats = {eTextureFormat::BC4_UNORM, eTextureFormat::R8_UNORM};
            return sFormats;
        }
        case eMaterialTextureUsage::Normal:
        {
            static const std::vector<eTextureFormat> sFormats = {eTextureFormat::BC5_UNORM, eTextureFormat::R8G8_UNORM};
            return sFormats;
        }
        case eMaterialTextureUsage::MetallicRoughness:
        case eMaterialTextureUsage::AmbientOcclusion:
        {
            static const std::vector<eTextureFormat> sFormats = {eTextureFormat::BC7_UNORM,
                                                                 eTextureFormat::BC3_UNORM,
                                                                 eTextureFormat::BC1_UNORM,
                                                                 eTextureFormat::R8G8B8A8_UNORM};
            return sFormats;
        }
        case eMaterialTextureUsage::Emissive:
        {
            static const std::vector<eTextureFormat> sFormats = {eTextureFormat::BC7_SRGB,
                                                                 eTextureFormat::R8G8B8A8_SRGB,
                                                                 eTextureFormat::BC6H_UF16,
                                                                 eTextureFormat::R16G16B16A16_FLOAT};
            return sFormats;
        }
        default:
        {
            HO_ASSERT(false, "Invalid texture usage.");
            static const std::vector<eTextureFormat> sEmpty = {eTextureFormat::None};
            return sEmpty;
        }
    }
}

} // namespace ho