#include <imgui.h>

#include "AssetLoadContext.h"
#include "AssetLoadPopupFuncs.h"
#include "Platforms/IPlatformApplication.h"
#include "Subsystems/Rendering/RenderPass/EnvironmentBakePassDesc.h"

namespace ho
{
static std::string showTextureFileOpenDialog();
static parser::MaterialIR::eTextureUsage toIRTextureUsage(eMaterialTextureUsage usage);
static const char* getTextureFormatName(eTextureFormat format);
static const char* getImageFormatName(Image::eFormat format);
static const std::vector<eTextureFormat>& getAvailableTextureFormatsPerUsage(eMaterialTextureUsage usage);
static const std::vector<eTextureFormat>& getAvailableTextureFormats(bool bIsLinear, bool bIsInteger, int numChannel);
static const char* getMatTypeName(eMaterialAssetType type);
static const char* getShaderStageName(eShaderStage stage);
static const char* getBakeTypeName(eEnvironmentBakeType type);
[[maybe_unused]] static void drawMeshHierarchyRecursive(const parser::MeshIR& meshIR,
                                                        const parser::SkeletonIR& skeletonIR,
                                                        const parser::SkinIR& skinIR,
                                                        int32_t rootIdx);

ePopupResult drawModelParseSettingPopup(ModelParseContext* pOutContext)
{
    ePopupResult result = ePopupResult::None;
    if (!ImGui::IsPopupOpen("Model Parse Setting##ModelParseSettingPopup"))
    {
        ImGui::OpenPopup("Model Parse Setting##ModelParseSettingPopup");
        ImGui::SetNextWindowSize(ImVec2(0.0f, 0.0f));
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
            ImGui::TextUnformatted(pOutContext->Path.ToString().c_str());

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
            ImGui::InputText("##ModelNameInputText", pOutContext->ModelName, HO_ARRAY_SIZE(pOutContext->ModelName));

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
            ImGui::Checkbox("Make Static", &pOutContext->bMakeStatic);

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
            result = ePopupResult::Confirm;
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();

        if (ImGui::Button("Cancel", ImVec2(buttonWidth, 0)))
        {
            result = ePopupResult::Cancel;
            ImGui::CloseCurrentPopup();
        }
        // =============================================================================================================
        ImGui::EndPopup();
    }

    return result;
}

void drawModelParsingPopup(bool bClosePopup)
{
    if (!ImGui::IsPopupOpen("##ModelParsingPopup"))
    {
        ImGui::OpenPopup("##ModelParsingPopup");
        ImGui::SetNextWindowSize(ImVec2(300.0f, 80.0f));
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

        if (bClosePopup)
        {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

ePopupResult drawModelImportSettingPopup(ModelImportContext* pOutContext)
{
    ePopupResult result = ePopupResult::None;
    if (!ImGui::IsPopupOpen("Model Import Setting##ModelImportSettingPopup"))
    {
        ImGui::OpenPopup("Model Import Setting##ModelImportSettingPopup");
        ImGui::SetNextWindowSize(ImVec2(800.0f, 600.0f), ImGuiCond_Appearing);
    }

    if (ImGui::BeginPopupModal("Model Import Setting##ModelImportSettingPopup",
                               nullptr,
                               ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse))
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
            ImGui::Text("%s", pOutContext->pParsedModel->NameStr.c_str());

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
            ImGui::Text("%s", pOutContext->pParsedModel->pMeshIR->NameStr.c_str());

            // SubMeshes
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::AlignTextToFramePadding();
            ImGui::Text("SubMeshes");
            ImGui::TableNextColumn();
            if (ImGui::BeginChild("##SubMeshInfoArea", ImVec2(0, 120.0f), ImGuiChildFlags_Borders, 0))
            {
                for (const auto& subMeshIR : pOutContext->pParsedModel->pMeshIR->SubMeshes)
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
                const char* currentPreview = pOutContext->bImportAsStaticMesh ? sMeshTypeNames[0] : sMeshTypeNames[1];

                ImGui::SetNextItemWidth(-1.0f);
                if (ImGui::BeginCombo("##MeshTypeCombo", currentPreview, ImGuiComboFlags_PopupAlignLeft))
                {
                    for (int32_t i = 0; i < 2; ++i)
                    {
                        const bool bIsStatic = (i == 0);
                        const bool bSelected = (pOutContext->bImportAsStaticMesh == bIsStatic);

                        if (ImGui::Selectable(sMeshTypeNames[i], bSelected))
                        {
                            pOutContext->bImportAsStaticMesh = bIsStatic;
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
        for (int32_t i = 0; i < static_cast<int32_t>(pOutContext->pParsedModel->pMaterialIRs.size()); ++i)
        {
            const parser::MaterialIR& materialIR = *(pOutContext->pParsedModel->pMaterialIRs[i]);
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
                                          getMatTypeName(pOutContext->MaterialTypes[i]),
                                          ImGuiComboFlags_PopupAlignLeft))
                    {
                        for (int32_t j = static_cast<int32_t>(eMaterialAssetType::Unlit);
                             j <= static_cast<int32_t>(eMaterialAssetType::StandardLit);
                             ++j)
                        {
                            const bool bSelected = (j == static_cast<int32_t>(pOutContext->MaterialTypes[i]));
                            if (ImGui::Selectable(getMatTypeName(static_cast<eMaterialAssetType>(j)), bSelected))
                            {
                                pOutContext->MaterialTypes[i] = static_cast<eMaterialAssetType>(j);
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
                                                pOutContext->pParsedModel->pTextureIRs[texIdx]
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
        if (pOutContext->pParsedModel->pTextureIRs.empty())
        {
            ImGui::Text("There is no textures.");
        }
        else
        {
            for (int32_t i = 0; i < static_cast<int32_t>(pOutContext->pParsedModel->pTextureIRs.size()); ++i)
            {
                const parser::TextureIR& textureIR = *(pOutContext->pParsedModel->pTextureIRs[i]);
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

                        const eMaterialTextureUsage currentUsage = pOutContext->TextureUsage[i];
                        eTextureFormat& currentFormat = pOutContext->TextureFormats[i];
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
                        ImGui::Checkbox("Generate Mipmap", &pOutContext->bGenerateMipmap[i]);

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
        if (!pOutContext->bHasMorphTargets)
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
                    for (int32_t i = 0; i < static_cast<int32_t>(pOutContext->pParsedModel->pMeshIR->SubMeshes.size());
                         ++i)
                    {
                        const auto& subMeshIR = pOutContext->pParsedModel->pMeshIR->SubMeshes[i];

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
                ImGui::Checkbox("Import morph targets", &pOutContext->bImportMorphTargets);

                ImGui::EndTable();
            }
        }

        // =============================================================================================================
        // Draw Morph Animation Settings
        ImGui::Spacing();
        ImGui::SeparatorText("MORPH ANIMATIONS");
        if (!pOutContext->bHasMorphAnimation)
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

                for (int32_t i = 0; i < static_cast<int32_t>(pOutContext->pParsedModel->pAnimationIRs.size()); ++i)
                {
                    const auto& animIR = pOutContext->pParsedModel->pAnimationIRs[i];
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
                    ImGui::Checkbox("##ImportMorphAnimCheckbox", &pOutContext->bImportMorphAnimation[i]);

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
            result = ePopupResult::Confirm;
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();

        if (ImGui::Button("Cancel", ImVec2(buttonWidth, 0.0f)))
        {
            result = ePopupResult::Cancel;
            ImGui::CloseCurrentPopup();
        }
        // =============================================================================================================

        ImGui::EndPopup();
    }

    return result;
}

void drawModelImportingPopup(const ModelImportContext& context)
{
    if (!ImGui::IsPopupOpen("##ModelImportingPopup"))
    {
        ImGui::OpenPopup("##ModelImportingPopup");
        ImGui::SetNextWindowSize(ImVec2(360.0f, 100.0f), ImGuiCond_Appearing);
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

        const int32_t totalCount = context.TotalImportedAssetCount;
        const int32_t importedCount = context.ImportedAssetCount;

        const float progress =
            totalCount > 0 ? static_cast<float>(importedCount) / static_cast<float>(totalCount) : 0.0f;

        char buf[32];
        snprintf(buf, sizeof(buf), "%d / %d", importedCount, totalCount);

        ImGui::ProgressBar(progress, ImVec2(-1.0f, 0.0f), buf);

        if (context.bImportComplete)
        {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

ePopupResult drawModelImportFailedPopup(const ModelImportContext& context)
{
    ePopupResult result = ePopupResult::None;
    if (!ImGui::IsPopupOpen("##ModelImportFailedPopup"))
    {
        ImGui::OpenPopup("##ModelImportFailedPopup");
        ImGui::SetNextWindowSize(ImVec2(600.0f, 0.0f), ImGuiCond_Appearing);
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
        // MESH
        ImGui::SeparatorText("MESH");
        if (ImGui::BeginTable("##MeshResultTable", 2, ImGuiTableFlags_SizingStretchProp))
        {
            ImGui::TableSetupColumn("Target", ImGuiTableColumnFlags_WidthFixed, labelColumnWidth);
            ImGui::TableSetupColumn("Status", ImGuiTableColumnFlags_WidthStretch);

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("%s", context.pParsedModel->pMeshIR->NameStr.c_str());

            ImGui::TableNextColumn();
            if (context.hImportedStaticMesh.IsValid())
            {
                ImGui::AlignTextToFramePadding();
                ImGui::TextColored(colorSuccess, "Import Succeed");
            }
            else
            {
                ImGui::AlignTextToFramePadding();
                ImGui::TextColored(colorFailed, "Import Failed");
            }

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

            bool bMaterialImportSucceed = true;
            for (int32_t i = 0; i < context.hImportedMaterials.GetSize(); ++i)
            {
                if (context.hImportedMaterials[i].IsNULL())
                {
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::Text("%s", context.pParsedModel->pMaterialIRs[i]->NameStr.c_str());

                    ImGui::TableNextColumn();
                    ImGui::TextColored(colorFailed, "Import Failed");

                    bMaterialImportSucceed = false;
                }
            }

            if (bMaterialImportSucceed)
            {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("All Materials");

                ImGui::TableNextColumn();
                ImGui::TextColored(colorSuccess, "Import Succeed");
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

            bool bTextureImportSucceed = true;
            for (int32_t i = 0; i < context.hImportedTextures.GetSize(); ++i)
            {
                if (context.hImportedTextures[i].IsNULL())
                {
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::Text("%s", context.pParsedModel->pTextureIRs[i]->NameStr.c_str());

                    ImGui::TableNextColumn();
                    ImGui::TextColored(colorFailed, "Import Failed");

                    bTextureImportSucceed = false;
                }
            }

            if (bTextureImportSucceed)
            {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("All Textures");

                ImGui::TableNextColumn();
                ImGui::TextColored(colorSuccess, "Import Succeed");
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
            ImGui::Text("%s", context.pParsedModel->pMeshIR->NameStr.c_str());

            ImGui::TableNextColumn();
            if (context.hImportedMorphTargets.IsValid())
            {
                ImGui::TextColored(colorSuccess, "Import Succeed");
            }
            else
            {
                ImGui::TextColored(colorFailed, "Import Failed");
            }

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

            bool bMorphAnimImportSucceed = true;
            for (int32_t i = 0; i < context.hImportedMorphAnims.GetSize(); ++i)
            {
                if (context.hImportedMorphAnims[i].IsNULL())
                {
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::Text("%s", context.pParsedModel->pAnimationIRs[i]->NameStr.c_str());

                    ImGui::TableNextColumn();
                    ImGui::TextColored(colorFailed, "Import Failed");

                    bMorphAnimImportSucceed = false;
                }
            }

            if (bMorphAnimImportSucceed)
            {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("All Morph Animations");

                ImGui::TableNextColumn();
                ImGui::TextColored(colorSuccess, "Import Succeed");
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
            result = ePopupResult::Confirm;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    return result;
}

ePopupResult drawShaderParseSettingPopup(ShaderParseContext* pOutContext)
{
    ePopupResult result = ePopupResult::None;
    if (!ImGui::IsPopupOpen("Shader Parse Setting##ShaderParseSettingPopup"))
    {
        ImGui::OpenPopup("Shader Parse Setting##ShaderParseSettingPopup");
        ImGui::SetNextWindowSize(ImVec2(600.0f, 0.0f), ImGuiCond_Appearing);
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
            ImGui::Text("%s", pOutContext->Path.ToString().c_str());

            // Shader Name
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Shader Name");

            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(-1.0f);
            ImGui::InputText("##ShaderNameInputText", pOutContext->ShaderName, HO_ARRAY_SIZE(pOutContext->ShaderName));

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
            result = ePopupResult::Confirm;
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();

        if (ImGui::Button("Cancel", ImVec2(buttonWidth, 0.0f)))
        {
            result = ePopupResult::Cancel;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    return result;
}

void drawShaderParsingPopup(bool bClosePopup)
{
    if (!ImGui::IsPopupOpen("##ShaderParsingPopup"))
    {
        ImGui::OpenPopup("##ShaderParsingPopup");
        ImGui::SetNextWindowSize(ImVec2(300.0f, 80.0f), ImGuiCond_Appearing);
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

        if (bClosePopup)
        {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

ePopupResult drawShaderImportSettingPopup(ShaderImportContext* pOutContext)
{
    ePopupResult result = ePopupResult::None;
    if (!ImGui::IsPopupOpen("Shader Import Setting##ShaderImportSettingPopup"))
    {
        ImGui::OpenPopup("Shader Import Setting##ShaderImportSettingPopup");
        ImGui::SetNextWindowSize(ImVec2(800.0f, 0.0f), ImGuiCond_Appearing);
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
            ImGui::Text("%s", pOutContext->pParsedShader->NameStr.c_str());

            ImGui::EndTable();
        }

        // =============================================================================================================
        // Draw Source Code
        ImGui::Spacing();
        ImGui::SeparatorText("SOURCE");
        ImGui::Spacing();

        ImGui::InputTextMultiline("##ShaderSource",
                                  const_cast<char*>( // NOLINT(cppcoreguidelines-pro-type-const-cast)
                                      pOutContext->pParsedShader->SourceStr.c_str()),
                                  pOutContext->pParsedShader->SourceStr.size() + 1,
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
            if (ImGui::BeginCombo(
                    "##ShaderStageCombo", getShaderStageName(pOutContext->ShaderStage), ImGuiComboFlags_PopupAlignLeft))
            {
                for (int32_t i = static_cast<int32_t>(eShaderStage::VertexShader);
                     i < static_cast<int32_t>(eShaderStage::Last);
                     ++i)
                {
                    const bool bSelected = (i == static_cast<int32_t>(pOutContext->ShaderStage));
                    if (ImGui::Selectable(getShaderStageName(static_cast<eShaderStage>(i)), bSelected))
                    {
                        pOutContext->ShaderStage = static_cast<eShaderStage>(i);
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
                                  getMatTypeName(pOutContext->CompatibleMaterialType),
                                  ImGuiComboFlags_PopupAlignLeft))
            {
                for (int32_t i = static_cast<int32_t>(eMaterialAssetType::Unlit);
                     i <= static_cast<int32_t>(eMaterialAssetType::StandardLit);
                     ++i)
                {
                    const bool bSelected = (i == static_cast<int32_t>(pOutContext->CompatibleMaterialType));
                    if (ImGui::Selectable(getMatTypeName(static_cast<eMaterialAssetType>(i)), bSelected))
                    {
                        pOutContext->CompatibleMaterialType = static_cast<eMaterialAssetType>(i);
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
            result = ePopupResult::Confirm;
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();

        if (ImGui::Button("Cancel", ImVec2(buttonWidth, 0.0f)))
        {
            result = ePopupResult::Cancel;
            ImGui::CloseCurrentPopup();
        }

        // =============================================================================================================
        ImGui::EndPopup();
    }

    return result;
}

void drawShaderImportingPopup(bool bClosePopup)
{
    if (!ImGui::IsPopupOpen("##ShaderImportingPopup"))
    {
        ImGui::OpenPopup("##ShaderImportingPopup");
        ImGui::SetNextWindowSize(ImVec2(300.0f, 80.0f), ImGuiCond_Appearing);
    }

    if (ImGui::BeginPopupModal("##ShaderImportingPopup", nullptr, ImGuiWindowFlags_NoDecoration))
    {
        ImGui::PushFont(nullptr, 30.0f);

        const char* text = "Importing Shader...";
        const ImVec2 textSize = ImGui::CalcTextSize(text);
        const ImVec2 windowSize = ImGui::GetContentRegionAvail();

        const float posX = (windowSize.x - textSize.x) * 0.5f;
        const float posY = ImGui::GetCursorPos().y + (windowSize.y - textSize.y) * 0.5f;

        ImGui::SetCursorPos(ImVec2(posX, posY));
        ImGui::TextUnformatted(text);

        ImGui::PopFont();

        if (bClosePopup)
        {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

ePopupResult drawShaderImportFailedPopup(const ShaderImportContext& context)
{
    ePopupResult result = ePopupResult::None;
    if (!ImGui::IsPopupOpen("##ShaderImportFailedPopup"))
    {
        ImGui::OpenPopup("##ShaderImportFailedPopup");
        ImGui::SetNextWindowSize(ImVec2(600.0f, 0.0f), ImGuiCond_Appearing);
    }

    if (ImGui::BeginPopupModal("##ShaderImportFailedPopup", nullptr, ImGuiWindowFlags_NoDecoration))
    {
        constexpr float labelColumnWidth = 140.0f;

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

        const ImVec4 colorFailed(0.95f, 0.3f, 0.3f, 1.0f);

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
            if (context.pParsedShader != nullptr)
            {
                ImGui::Text("%s", context.pParsedShader->NameStr.c_str());
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
            ImGui::TextColored(colorFailed, "Import Failed");

            ImGui::EndTable();
        }

        // =============================================================================================================
        // Draw Error Message
        ImGui::Spacing();
        ImGui::SeparatorText("ERROR LOG");
        ImGui::Spacing();

        ImGui::InputTextMultiline(
            "##ErrorMessage",
            const_cast<char*>(context.CompileErrorMsg.c_str()), // NOLINT(cppcoreguidelines-pro-type-const-cast)
            context.CompileErrorMsg.size() + 1,
            ImVec2(-FLT_MIN, 350.0f),
            ImGuiInputTextFlags_ReadOnly);

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
            result = ePopupResult::Confirm;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    return result;
}

std::string showTextureFileOpenDialog()
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

ePopupResult drawTextureParseSettingPopup(TextureParseContext* pOutContext)
{
    ePopupResult result = ePopupResult::None;
    if (!ImGui::IsPopupOpen("Texture Parse Setting##TextureParseSettingPopup"))
    {
        ImGui::OpenPopup("Texture Parse Setting##TextureParseSettingPopup");
        ImGui::SetNextWindowSize(ImVec2(600.0f, 0.0f), ImGuiCond_Appearing);
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

            const int32_t texCount = (pOutContext->TextureType == eTextureType::Texture2D) ? 1 : 6;
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
                snprintf(pathBuffer, sizeof(pathBuffer), "%s", pOutContext->Path[i].ToString().c_str());

                ImGui::InputText("##PathInput", pathBuffer, sizeof(pathBuffer), ImGuiInputTextFlags_ReadOnly);

                ImGui::SameLine();

                if (ImGui::Button("...", ImVec2(buttonWidth, 0.0f)))
                {
                    const std::string selectedPath = showTextureFileOpenDialog();
                    if (!selectedPath.empty())
                    {
                        pOutContext->Path[i] = Path(selectedPath);
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
                "##TextureNameInputText", pOutContext->TextureName, HO_ARRAY_SIZE(pOutContext->TextureName));

            // Color Space Combo
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Color Space");

            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(-1.0f);
            if (ImGui::BeginCombo("##TextureCompatibleMaterialCombo",
                                  pOutContext->bIsLinear ? "LINEAR" : "SRGB",
                                  ImGuiComboFlags_PopupAlignLeft))
            {
                for (int32_t i = 0; i < 2; ++i)
                {
                    const bool bSelected = (static_cast<bool>(i) == pOutContext->bIsLinear);
                    if (ImGui::Selectable(static_cast<bool>(i) ? "LINEAR" : "SRGB", bSelected))
                    {
                        pOutContext->bIsLinear = static_cast<bool>(i);
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
            result = ePopupResult::Confirm;
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();

        if (ImGui::Button("Cancel", ImVec2(buttonWidth, 0.0f)))
        {
            result = ePopupResult::Cancel;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    return result;
}

void drawTextureParsingPopup(bool bClosePopup)
{
    if (!ImGui::IsPopupOpen("##TextureParsingPopup"))
    {
        ImGui::OpenPopup("##TextureParsingPopup");
        ImGui::SetNextWindowSize(ImVec2(300.0f, 80.0f), ImGuiCond_Appearing);
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

        if (bClosePopup)
        {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

ePopupResult drawTextureImportSettingPopup(TextureImportContext* pOutContext)
{
    ePopupResult result = ePopupResult::None;
    if (!ImGui::IsPopupOpen("Texture Import Setting##TextureImportSettingPopup"))
    {
        ImGui::OpenPopup("Texture Import Setting##TextureImportSettingPopup");
        ImGui::SetNextWindowSize(ImVec2(600.0f, 0.0f), ImGuiCond_Appearing);
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
            ImGui::Text("%s", pOutContext->pParsedTextures[0]->NameStr.c_str());

            ImGui::EndTable();
        }

        // =============================================================================================================
        // Draw Source Details
        ImGui::Spacing();
        ImGui::SeparatorText("SOURCE");
        ImGui::Spacing();

        const int32_t texCount = (pOutContext->TextureType == eTextureType::Texture2D) ? 1 : 6;
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

                    const parser::TextureIR& textureIR = *(pOutContext->pParsedTextures[i]);

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
            if (ImGui::BeginCombo("##TextureFormatCombo", getTextureFormatName(pOutContext->TextureFormat)))
            {
                const auto& formats =
                    getAvailableTextureFormats(pOutContext->pParsedTextures[0]->Img.IsLinear(),
                                               pOutContext->pParsedTextures[0]->Img.IsInteger(),
                                               pOutContext->pParsedTextures[0]->Img.GetPhysicalChannelCount());

                for (auto format : formats)
                {
                    const bool bSelected = (pOutContext->TextureFormat == format);
                    if (ImGui::Selectable(getTextureFormatName(format), bSelected))
                    {
                        pOutContext->TextureFormat = format;
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
            ImGui::Checkbox("##GenerateMipmap", &pOutContext->bGenerateMipmap);

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
            result = ePopupResult::Confirm;
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();

        if (ImGui::Button("Cancel", ImVec2(buttonWidth, 0.0f)))
        {
            result = ePopupResult::Cancel;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    return result;
}

void drawTextureImportingPopup(const TextureImportContext& context)
{
    if (!ImGui::IsPopupOpen("##TextureImportingPopup"))
    {
        ImGui::OpenPopup("##TextureImportingPopup");
        ImGui::SetNextWindowSize(ImVec2(360.0f, 100.0f), ImGuiCond_Appearing);
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

        const int32_t totalCount = context.TotalImportedTextureCount;
        const int32_t importedCount = context.ImportedTextureCount;

        const float progress =
            totalCount > 0 ? static_cast<float>(importedCount) / static_cast<float>(totalCount) : 0.0f;

        char buf[32];
        snprintf(buf, sizeof(buf), "%d / %d", importedCount, totalCount);

        ImGui::ProgressBar(progress, ImVec2(-1.0f, 0.0f), buf);

        if (context.bImportComplete)
        {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

ePopupResult drawTextureImportFailedPopup(const TextureImportContext& context)
{
    ePopupResult result = ePopupResult::None;
    if (!ImGui::IsPopupOpen("##TextureImportFailedPopup"))
    {
        ImGui::OpenPopup("##TextureImportFailedPopup");
        ImGui::SetNextWindowSize(ImVec2(600.0f, 0.0f), ImGuiCond_Appearing);
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
            if (context.pParsedTextures[0] != nullptr)
            {
                ImGui::Text("%s", context.pParsedTextures[0]->NameStr.c_str());
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
            ImGui::TextColored(colorFailed, "Import Failed");

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
            result = ePopupResult::Confirm;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    return result;
}

ePopupResult drawTextureBakeSettingPopup(TextureBakeContext* pOutContext)
{
    ePopupResult result = ePopupResult::None;
    if (!ImGui::IsPopupOpen("Texture Bake Setting##TextureBakeSettingPopup"))
    {
        ImGui::OpenPopup("Texture Bake Setting##TextureBakeSettingPopup");
        ImGui::SetNextWindowSize(ImVec2(600.0f, 0.0f), ImGuiCond_Appearing);
    }

    if (ImGui::BeginPopupModal("Texture Bake Setting##TextureBakeSettingPopup",
                               nullptr,
                               ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse))
    {
        constexpr float labelColumnWidth = 120.0f;
        // =============================================================================================================
        // Section 1: Source Texture
        if (ImGui::BeginTable("##SourceTextureTable", 2))
        {
            ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed, labelColumnWidth);
            ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Source Texture");

            ImGui::TableNextColumn();
            ImGui::AlignTextToFramePadding();
            ImGui::TextUnformatted(*pOutContext->hSrcTexture->hName);

            ImGui::EndTable();
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // Section 2: Destination Texture Name
        if (ImGui::BeginTable("##ModelNameTable", 2))
        {
            ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed, labelColumnWidth);
            ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Texture Name");

            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(-1.0f);
            ImGui::InputText(
                "##ModelNameInputText", pOutContext->DstTextureName, HO_ARRAY_SIZE(pOutContext->DstTextureName));

            ImGui::EndTable();
        }

        // =============================================================================================================
        // Draw Settings
        ImGui::Spacing();
        ImGui::SeparatorText("SETTINGS");

        bool bCanBake = true;
        std::string warningMessage = "";

        if (ImGui::BeginTable("##TextureSettingsTable", 2, ImGuiTableFlags_SizingStretchProp))
        {
            ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed, labelColumnWidth);
            ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

            // Bake Type Combo
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Bake Type");

            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(-1.0f);
            if (ImGui::BeginCombo(
                    "##BakeTypeCombo", getBakeTypeName(pOutContext->BakeType), ImGuiComboFlags_PopupAlignLeft))
            {
                for (int32_t i = 0; i < static_cast<int32_t>(eEnvironmentBakeType::Last); ++i)
                {
                    const eEnvironmentBakeType currType = static_cast<eEnvironmentBakeType>(i);
                    const bool bSelected = (currType == pOutContext->BakeType);

                    if (ImGui::Selectable(getBakeTypeName(currType), bSelected))
                    {
                        if (pOutContext->BakeType != currType)
                        {
                            pOutContext->BakeType = currType;

                            if (currType == eEnvironmentBakeType::Irradiance && pOutContext->BakedMapSize > 256)
                            {
                                pOutContext->BakedMapSize = 64;
                            }
                            else if (currType == eEnvironmentBakeType::PrefilteredEnvMap &&
                                     pOutContext->BakedMapSize < 128)
                            {
                                pOutContext->BakedMapSize = 128;
                            }
                        }
                    }

                    if (bSelected)
                    {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }

            // Map Size
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Map Size");

            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(-1.0f);

            char sizeBuf[126] = {};
            snprintf(sizeBuf, HO_ARRAY_SIZE(sizeBuf), "%d x %d", pOutContext->BakedMapSize, pOutContext->BakedMapSize);
            if (ImGui::BeginCombo("##BakedMapSizeCombo", sizeBuf))
            {
                const int32_t irradianceSizes[] = {16, 32, 64, 128, 256};
                const int32_t prefilteredSizes[] = {128, 256, 512, 1024};

                const int32_t* pPresets =
                    (pOutContext->BakeType == eEnvironmentBakeType::Irradiance) ? irradianceSizes : prefilteredSizes;
                const int32_t presetCount = (pOutContext->BakeType == eEnvironmentBakeType::Irradiance)
                                                ? HO_ARRAY_SIZE(irradianceSizes)
                                                : HO_ARRAY_SIZE(prefilteredSizes);

                for (int32_t i = 0; i < presetCount; ++i)
                {
                    const int32_t sizePreset = pPresets[i];
                    const bool bSelected = (pOutContext->BakedMapSize == sizePreset);
                    snprintf(sizeBuf, HO_ARRAY_SIZE(sizeBuf), "%d x %d", sizePreset, sizePreset);

                    if (ImGui::Selectable(sizeBuf, bSelected))
                    {
                        pOutContext->BakedMapSize = sizePreset;
                    }

                    if (bSelected)
                    {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }

            // Map Type-Specific Settings
            if (pOutContext->BakeType == eEnvironmentBakeType::Irradiance)
            {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::AlignTextToFramePadding();
                ImGui::Text("Sample Delta");

                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(-1.0f);

                char deltaBuf[32];
                snprintf(deltaBuf, sizeof(deltaBuf), "%.3f rad", pOutContext->SampleDelta);
                if (ImGui::BeginCombo("##SampleDeltaCombo", deltaBuf))
                {
                    const float deltaPresets[] = {0.010f, 0.025f, 0.050f, 0.100f};
                    for (const float deltaPreset : deltaPresets)
                    {
                        const bool bSelected = (pOutContext->SampleDelta == deltaPreset);
                        char itemBuf[32];
                        snprintf(itemBuf, sizeof(itemBuf), "%.3f rad", deltaPreset);

                        if (ImGui::Selectable(itemBuf, bSelected))
                        {
                            pOutContext->SampleDelta = deltaPreset;
                        }

                        if (bSelected)
                        {
                            ImGui::SetItemDefaultFocus();
                        }
                    }
                    ImGui::EndCombo();
                }
            }
            else if (pOutContext->BakeType == eEnvironmentBakeType::PrefilteredEnvMap)
            {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::AlignTextToFramePadding();
                ImGui::Text("Sample Count");

                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(-1.0f);

                char sampleBuf[126] = {};
                snprintf(sampleBuf, HO_ARRAY_SIZE(sampleBuf), "%d samples", pOutContext->SampleCount);
                if (ImGui::BeginCombo("##SampleCountCombo", sampleBuf))
                {
                    const int32_t countPresets[] = {128, 256, 512, 1024, 2048, 4096};
                    for (const int32_t countPreset : countPresets)
                    {
                        const bool bSelected = (pOutContext->SampleCount == countPreset);
                        snprintf(sampleBuf, HO_ARRAY_SIZE(sampleBuf), "%d samples", countPreset);

                        if (ImGui::Selectable(sampleBuf, bSelected))
                        {
                            pOutContext->SampleCount = countPreset;
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

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::Spacing();

        // =============================================================================================================
        // Validation Check
        if (pOutContext->BakeType == eEnvironmentBakeType::Irradiance)
        {
            if (pOutContext->BakedMapSize < 16 || pOutContext->BakedMapSize > 256)
            {
                bCanBake = false;
                warningMessage = "Irradiance Map size must be between 16 and 256.";
            }
        }
        else if (pOutContext->BakeType == eEnvironmentBakeType::PrefilteredEnvMap)
        {
            if (pOutContext->BakedMapSize < 128)
            {
                bCanBake = false;
                warningMessage = "Prefiltered Map size must be at least 128 for mipmaps.";
            }
        }

        // Draw warning message
        if (!bCanBake && !warningMessage.empty())
        {
            ImGui::Spacing();
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
            ImGui::TextWrapped("%s", warningMessage.c_str());
            ImGui::PopStyleColor();
        }

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

        if (!bCanBake)
        {
            ImGui::BeginDisabled();
        }

        if (ImGui::Button("Bake", ImVec2(buttonWidth, 0)))
        {
            result = ePopupResult::Confirm;
            ImGui::CloseCurrentPopup();
        }

        if (!bCanBake)
        {
            ImGui::EndDisabled();
        }

        ImGui::SameLine();

        if (ImGui::Button("Cancel", ImVec2(buttonWidth, 0)))
        {
            result = ePopupResult::Cancel;
            ImGui::CloseCurrentPopup();
        }
        // =============================================================================================================
        ImGui::EndPopup();
    }

    return result;
}

void drawTextureBakingPopup(bool bClosePopup)
{
    if (!ImGui::IsPopupOpen("##TextureParsingPopup"))
    {
        ImGui::OpenPopup("##TextureParsingPopup");
        ImGui::SetNextWindowSize(ImVec2(300.0f, 80.0f), ImGuiCond_Appearing);
    }

    if (ImGui::BeginPopupModal("##TextureParsingPopup", nullptr, ImGuiWindowFlags_NoDecoration))
    {
        ImGui::PushFont(nullptr, 30.0f);

        const char* text = "Baking Texture...";
        const ImVec2 textSize = ImGui::CalcTextSize(text);
        const ImVec2 windowSize = ImGui::GetContentRegionAvail();

        const float posX = (windowSize.x - textSize.x) * 0.5f;
        const float posY = ImGui::GetCursorPos().y + (windowSize.y - textSize.y) * 0.5f;

        ImGui::SetCursorPos(ImVec2(posX, posY));
        ImGui::TextUnformatted(text);

        ImGui::PopFont();

        if (bClosePopup)
        {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

ePopupResult drawTextureBakeFailedPopup(const TextureBakeContext& context)
{
    ePopupResult result = ePopupResult::None;
    if (!ImGui::IsPopupOpen("##TextureBakeFailedPopup"))
    {
        ImGui::OpenPopup("##TextureBakeFailedPopup");
        ImGui::SetNextWindowSize(ImVec2(600.0f, 0.0f), ImGuiCond_Appearing);
    }

    if (ImGui::BeginPopupModal("##TextureBakeFailedPopup", nullptr, ImGuiWindowFlags_NoDecoration))
    {
        constexpr float labelColumnWidth = 140.0f;

        // =============================================================================================================
        // Title
        ImGui::PushFont(nullptr, 20.0f);

        const char* titleText = "Texture Bake Failed";
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
            ImGui::Text("%s", *context.hSrcTexture->hName);

            // Status
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Status");
            ImGui::TableNextColumn();
            ImGui::AlignTextToFramePadding();
            ImGui::TextColored(colorFailed, "Bake Failed");

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
            result = ePopupResult::Confirm;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    return result;
}

// ==========================================================
// UI draw Helpers
// ==========================================================
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

const char* getBakeTypeName(eEnvironmentBakeType type)
{
    switch (type)
    {
        case eEnvironmentBakeType::Irradiance:
            return "Irradiance";
        case eEnvironmentBakeType::PrefilteredEnvMap:
            return "PrefilteredEnvironment";
        default:
            HO_ASSERT(false, "Invalid bake type.");
            return "None";
    }
}

void drawMeshHierarchyRecursive(const parser::MeshIR& meshIR,
                                const parser::SkeletonIR& skeletonIR,
                                const parser::SkinIR& skinIR,
                                int32_t rootIdx)
{
    ImGui::PushID(rootIdx);
    if (ImGui::TreeNode(skeletonIR.BoneNameStrs[rootIdx].c_str()))
    {
        for (const int32_t subMeshIdx : skinIR.BindSubMeshIndices[rootIdx])
        {
            ImGui::BulletText("%s", meshIR.SubMeshes[subMeshIdx].NameStr.c_str());
        }

        for (const int32_t childIdx : skeletonIR.Children[rootIdx])
        {
            drawMeshHierarchyRecursive(meshIR, skeletonIR, skinIR, childIdx);
        }
        ImGui::TreePop();
    }
    ImGui::PopID();
}

} // namespace ho