#include "TextureBakeWorkflow.h"

#include <imgui.h>

#include "../Utils/EditorDisplayUtils.h"
#include "../Utils/EditorUtils.h"
#include "Subsystems/Asset/AssetSystem.h"
#include "Subsystems/Rendering/RenderingSystem.h"

namespace ho
{
static const char* getBakeTypeName(eEnvironmentBakeType type);

void TextureBakeWorkflow::Begin(ObjectHandle<RTextureAsset> hSrcTexture)
{
    if (mState != eState::None)
    {
        HO_ASSERT(false, "Workflow was already begun.");
        return;
    }
    if (hSrcTexture.IsNULL())
    {
        return;
    }
    mhSrcTexture = hSrcTexture;
    mState = eState::Begin;
    mbNeedToDeleteTexture = false;
}

void TextureBakeWorkflow::Draw()
{
    switch (mState)
    {
        case eState::BakeSetting:
            mbOpenBakeSettingPopup = true;
            break;
        case eState::Baking:
            mbOpenBakingPopup = true;
            break;
        case eState::Failed:
            mbOpenBakeFailedPopup = true;
            break;
        case eState::None:
        case eState::Begin:
        case eState::BakeBegin:
        case eState::Succeed:
        case eState::Finish:
            break;
        default:
            HO_ASSERT(false, "Invalid state.");
    }

    drawBakeSettingPopup();
    drawBakingPopup();
    drawBakeFailedPopup();
}

void TextureBakeWorkflow::Process()
{
    switch (mState)
    {
        case eState::Begin:
        {
            initializeBakeContext(mhSrcTexture);
            mState = eState::BakeSetting;
        }
        break;
        case eState::BakeBegin:
        {
            if (beginBaking())
            {
                mState = eState::Baking;
            }
            else
            {
                if (mBakeContext.hSrcTexture.IsNULL())
                {
                    mState = eState::Failed;
                }
                else
                {
                    mState = eState::BakeBegin;
                }
            }
        }
        break;
        case eState::Baking:
        {
            if (checkBaking())
            {
                switch (mBakeContext.hDstTexture->TransferState)
                {
                    case eAssetTransferState::Ready:
                        mState = eState::Succeed;
                        break;
                    case eAssetTransferState::Failed:
                        mState = eState::Failed;
                        break;
                    default:
                        mState = eState::Failed;
                }
            }
            else
            {
                mState = eState::Baking;
            }
        }
        break;
        case eState::Succeed:
            mState = eState::Finish;
            break;
        case eState::Finish:
            if (mbNeedToDeleteTexture)
            {
                deleteBakedTexture();
            }
            clearBakeContext();
            mState = eState::None;
            break;
        case eState::Failed:
            mbNeedToDeleteTexture = true;
            break;
        case eState::None:
        case eState::BakeSetting:
            return;
        default:
            HO_ASSERT(false, "Invalid state.");
    }
}

void TextureBakeWorkflow::drawBakeSettingPopup()
{
    if (mState != eState::BakeSetting)
    {
        return;
    }

    if (mbOpenBakeSettingPopup)
    {
        ImGui::SetNextWindowSize(ImVec2(600.0f, 0.0f), ImGuiCond_Appearing);
        ImGui::OpenPopup("Texture Bake Setting##TextureBakeSettingPopup");
        mbOpenBakeSettingPopup = false;
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
            ImGui::TextUnformatted(*mBakeContext.hSrcTexture->hName);

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
                "##ModelNameInputText", mBakeContext.DstTextureName, HO_ARRAY_SIZE(mBakeContext.DstTextureName));

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
                    "##BakeTypeCombo", getBakeTypeName(mBakeContext.BakeType), ImGuiComboFlags_PopupAlignLeft))
            {
                for (int32_t i = 0; i < static_cast<int32_t>(eEnvironmentBakeType::Last); ++i)
                {
                    const eEnvironmentBakeType currType = static_cast<eEnvironmentBakeType>(i);
                    const bool bSelected = (currType == mBakeContext.BakeType);

                    if (ImGui::Selectable(getBakeTypeName(currType), bSelected))
                    {
                        if (mBakeContext.BakeType != currType)
                        {
                            mBakeContext.BakeType = currType;

                            if (currType == eEnvironmentBakeType::Irradiance && mBakeContext.BakedMapSize > 256)
                            {
                                mBakeContext.BakedMapSize = 64;
                            }
                            else if (currType == eEnvironmentBakeType::PrefilteredEnvMap &&
                                     mBakeContext.BakedMapSize < 128)
                            {
                                mBakeContext.BakedMapSize = 128;
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
            snprintf(sizeBuf, HO_ARRAY_SIZE(sizeBuf), "%d x %d", mBakeContext.BakedMapSize, mBakeContext.BakedMapSize);
            if (ImGui::BeginCombo("##BakedMapSizeCombo", sizeBuf))
            {
                const int32_t irradianceSizes[] = {16, 32, 64, 128, 256};
                const int32_t prefilteredSizes[] = {128, 256, 512, 1024};

                const int32_t* pPresets =
                    (mBakeContext.BakeType == eEnvironmentBakeType::Irradiance) ? irradianceSizes : prefilteredSizes;
                const int32_t presetCount = (mBakeContext.BakeType == eEnvironmentBakeType::Irradiance)
                                                ? HO_ARRAY_SIZE(irradianceSizes)
                                                : HO_ARRAY_SIZE(prefilteredSizes);

                for (int32_t i = 0; i < presetCount; ++i)
                {
                    const int32_t sizePreset = pPresets[i];
                    const bool bSelected = (mBakeContext.BakedMapSize == sizePreset);
                    snprintf(sizeBuf, HO_ARRAY_SIZE(sizeBuf), "%d x %d", sizePreset, sizePreset);

                    if (ImGui::Selectable(sizeBuf, bSelected))
                    {
                        mBakeContext.BakedMapSize = sizePreset;
                    }

                    if (bSelected)
                    {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }

            // Map Type-Specific Settings
            switch (mBakeContext.BakeType)
            {
                case eEnvironmentBakeType::Irradiance:
                {
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::AlignTextToFramePadding();
                    ImGui::Text("Sample Delta");

                    ImGui::TableNextColumn();
                    ImGui::SetNextItemWidth(-1.0f);

                    char deltaBuf[32];
                    snprintf(deltaBuf, sizeof(deltaBuf), "%.3f rad", mBakeContext.SampleDelta);
                    if (ImGui::BeginCombo("##SampleDeltaCombo", deltaBuf))
                    {
                        const float deltaPresets[] = {0.010f, 0.025f, 0.050f, 0.100f};
                        for (const float deltaPreset : deltaPresets)
                        {
                            const bool bSelected = (mBakeContext.SampleDelta == deltaPreset);
                            char itemBuf[32];
                            snprintf(itemBuf, sizeof(itemBuf), "%.3f rad", deltaPreset);

                            if (ImGui::Selectable(itemBuf, bSelected))
                            {
                                mBakeContext.SampleDelta = deltaPreset;
                            }

                            if (bSelected)
                            {
                                ImGui::SetItemDefaultFocus();
                            }
                        }
                        ImGui::EndCombo();
                    }
                }
                break;
                case eEnvironmentBakeType::PrefilteredEnvMap:
                {
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::AlignTextToFramePadding();
                    ImGui::Text("Sample Count");

                    ImGui::TableNextColumn();
                    ImGui::SetNextItemWidth(-1.0f);

                    char sampleBuf[126] = {};
                    snprintf(sampleBuf, HO_ARRAY_SIZE(sampleBuf), "%d samples", mBakeContext.SampleCount);
                    if (ImGui::BeginCombo("##SampleCountCombo", sampleBuf))
                    {
                        const int32_t countPresets[] = {128, 256, 512, 1024, 2048, 4096};
                        for (const int32_t countPreset : countPresets)
                        {
                            const bool bSelected = (mBakeContext.SampleCount == countPreset);
                            snprintf(sampleBuf, HO_ARRAY_SIZE(sampleBuf), "%d samples", countPreset);

                            if (ImGui::Selectable(sampleBuf, bSelected))
                            {
                                mBakeContext.SampleCount = countPreset;
                            }

                            if (bSelected)
                            {
                                ImGui::SetItemDefaultFocus();
                            }
                        }
                        ImGui::EndCombo();
                    }
                }
                break;
                default:
                    HO_ASSERT(false, "Invalid bake type.");
                    bCanBake = false;
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
        switch (mBakeContext.BakeType)
        {
            case eEnvironmentBakeType::Irradiance:
            {
                if (mBakeContext.BakedMapSize < 16 || mBakeContext.BakedMapSize > 256)
                {
                    bCanBake = false;
                    warningMessage = "Irradiance Map size must be between 16 and 256.";
                }
            }
            break;
            case eEnvironmentBakeType::PrefilteredEnvMap:
            {
                if (mBakeContext.BakedMapSize < 128)
                {
                    bCanBake = false;
                    warningMessage = "Prefiltered Map size must be at least 128 for mipmaps.";
                }
            }
            break;
            default:
                HO_ASSERT(false, "Invalid bake type.");
                bCanBake = false;
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
            mState = eState::BakeBegin;
            ImGui::CloseCurrentPopup();
        }

        if (!bCanBake)
        {
            ImGui::EndDisabled();
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

void TextureBakeWorkflow::drawBakingPopup()
{
    if (mbOpenBakingPopup)
    {
        ImGui::SetNextWindowSize(ImVec2(360.0f, 100.0f), ImGuiCond_Appearing);
        ImGui::OpenPopup("##TextureBakingPopup");
        mbOpenBakingPopup = false;
    }

    if (ImGui::BeginPopupModal("##TextureBakingPopup", nullptr, ImGuiWindowFlags_NoDecoration))
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

        if (mState != eState::Baking)
        {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

void TextureBakeWorkflow::drawBakeFailedPopup()
{
    if (mbOpenBakeFailedPopup)
    {
        ImGui::SetNextWindowSize(ImVec2(600.0f, 0.0f), ImGuiCond_Appearing);
        ImGui::OpenPopup("##TextureBakeFailedPopup");
        mbOpenBakeFailedPopup = false;
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
            ImGui::Text("%s", *mBakeContext.hSrcTexture->hName);

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
            mState = eState::Finish;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void TextureBakeWorkflow::initializeBakeContext(ObjectHandle<RTextureAsset> hSrcTexture)
{
    HO_ASSERT(hSrcTexture.IsValid(), "Invalid source texture.");

    mBakeContext.hSrcTexture = hSrcTexture;

    const std::string defaultName = std::string(getAssetDisplayName(hSrcTexture)).append("_Baked");
    strncpy_s(mBakeContext.DstTextureName, defaultName.c_str(), sizeof(mBakeContext.DstTextureName) - 1);

    mBakeContext.BakeType = eEnvironmentBakeType::Irradiance;
    mBakeContext.BakedMapSize = 64;
    mBakeContext.SampleDelta = 0.05f;
    mBakeContext.SampleCount = 1024;
}

void TextureBakeWorkflow::clearBakeContext()
{
    mBakeContext.hSrcTexture = ObjectHandle<RTextureAsset>::sNULL;
    mBakeContext.DstTextureName[0] = '\0';
    mBakeContext.BakeType = eEnvironmentBakeType::Irradiance;
    mBakeContext.BakedMapSize = 0;
    mBakeContext.SampleDelta = 0.0f;
    mBakeContext.SampleCount = 0;
    mBakeContext.hDstTexture = ObjectHandle<RTextureAsset>::sNULL;
}

void TextureBakeWorkflow::deleteBakedTexture() const
{
    AssetSystem::GetInstance().DeleteAsset(mBakeContext.hDstTexture);
}

bool TextureBakeWorkflow::beginBaking()
{
    if (mState != eState::BakeBegin)
    {
        return false;
    }

    if (mBakeContext.hSrcTexture.IsNULL())
    {
        return false;
    }

    if (!RenderingSystem::GetInstance().IsUploaded(mBakeContext.hSrcTexture))
    {
        RenderingSystem::GetInstance().UploadTexture(mBakeContext.hSrcTexture);
        return false;
    }

    char buf[256] = {};
    snprintf(buf,
             HO_ARRAY_SIZE(buf),
             "%s%s%s",
             mBakeContext.hSrcTexture->AssetPath.GetParentPath().ToString().c_str(),
             "::TextureCubeMap::",
             mBakeContext.DstTextureName);
    const std::string newTexNameStr = getUniqueAssetName(buf);
    mBakeContext.hDstTexture = AssetSystem::GetInstance().CreateAsset<RTextureAsset>(StringHandle(newTexNameStr));
    if (mBakeContext.hDstTexture.IsNULL())
    {
        return false;
    }
    mBakeContext.hDstTexture->TransferState = eAssetTransferState::PendingDownload;
    EnvironmentBakePassDesc bakeDesc;
    bakeDesc.BakeType = mBakeContext.BakeType;
    bakeDesc.BakedMapSize = mBakeContext.BakedMapSize;
    bakeDesc.SampleDelta = mBakeContext.SampleDelta;
    bakeDesc.SampleCount = mBakeContext.SampleCount;
    bakeDesc.hSrcTexture = mBakeContext.hSrcTexture->hRenderProxy;
    bakeDesc.hDstTexture = mBakeContext.hDstTexture;
    if (RenderingSystem::GetInstance().BeginEnvironmentBakePass(bakeDesc))
    {
        RenderingSystem::GetInstance().EndRenderPass();
        return true;
    }

    return false;
}

bool TextureBakeWorkflow::checkBaking()
{
    if (mState != eState::Baking)
    {
        return false;
    }

    return mBakeContext.hDstTexture->TransferState != eAssetTransferState::PendingDownload;
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

} // namespace ho