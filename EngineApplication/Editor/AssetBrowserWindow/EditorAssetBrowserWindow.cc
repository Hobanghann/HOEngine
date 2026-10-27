#include "EditorAssetBrowserWindow.h"

#include <IconsMaterialDesignIcons.h>
#include <charconv>
#include <imgui.h>
#include <imgui_internal.h>

#include "../Utils/EditorDisplayUtils.h"
#include "AssetLoadPopupFuncs.h"
#include "Core/IO/ResourceParseFuncs.h"
#include "DirectoryHierarchyView.h"
#include "DirectoryItemView.h"
#include "Platforms/IPlatformApplication.h"
#include "Subsystems/Asset/AssetImportFuncs.h"
#include "Subsystems/Asset/AssetSystem.h"
#include "Subsystems/Rendering/RenderingSystem.h"

namespace ho
{
static eTextureFormat getDefaultTextureFormatPerUsage(eMaterialTextureUsage usage);
static parser::MaterialIR::eTextureUsage toIRTextureUsage(eMaterialTextureUsage usage);

EditorAssetBrowserWindow::EditorAssetBrowserWindow()
  : IEngineWindow(StringHandle("EditorAssetBrowserWindow"))
  , mpDirectoryHierarchyView(std::make_unique<DirectoryHierarchyView>())
  , mpDirectoryItemView(std::make_unique<DirectoryItemView>())
{
}

EditorAssetBrowserWindow::~EditorAssetBrowserWindow() = default;

bool EditorAssetBrowserWindow::OnInit()
{
    mpOpenedDir = mDirectoryHierarchy.GetRoot();
    if (!mpOpenedDir)
    {
        return false;
    }
    return true;
}

bool EditorAssetBrowserWindow::OnPreUpdate(float deltaTime)
{
    return true;
}

bool EditorAssetBrowserWindow::OnUpdate(float deltaTime)
{
    processModelLoadPipeline();
    processTextureLoadPipeline();
    processShaderLoadPipeline();

    processTextureBakePipeline();
    return true;
}

bool EditorAssetBrowserWindow::OnPostUpdate(float deltaTime)
{
    return true;
}

bool EditorAssetBrowserWindow::OnRender()
{
    return true;
}

void EditorAssetBrowserWindow::OnShutdown() {}

void EditorAssetBrowserWindow::DrawUI()
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    if (ImGui::Begin("Asset Browser", &mbOpen, 0))
    {
        SetFocused(ImGui::IsWindowFocused());

        // =============================================================================================================
        // Draw Menu bar
        if (ImGui::BeginChild("##MenuBar", ImVec2(0.0f, 30.0f), 0, 0))
        {
            ImGui::SetCursorPos(ImVec2(ImGui::GetStyle().ItemSpacing.x, ImGui::GetStyle().ItemSpacing.y));
            if (ImGui::Button(ICON_MDI_PLUS " Add"))
            {
                mbOpenMenuBarAddPopup = true;
            }
            drawMenuBarAddPopup();
            drawNewFolderPopup();

            ImGui::SameLine();

            if (ImGui::Button(ICON_MDI_DOWNLOAD_BOX_OUTLINE " Import"))
            {
                mbOpenMenuBarImportPopup = true;
            }
            drawMenuBarImportPopup();

            ImGui::SameLine();

            if (ImGui::Button(ICON_MDI_CONTENT_SAVE_ALL " Save All"))
            {
                // TODO: Serialize current directory hierarchy in project folder
            }
        }
        ImGui::EndChild();

        ImGui::SetCursorPosY(ImGui::GetCursorPosY() - ImGui::GetStyle().ItemSpacing.y);
        // =============================================================================================================

        // =============================================================================================================
        // Draw Directory Hierarchy View (Left Side)
        handleDirectoryHierarchyViewRequest();
        mpDirectoryHierarchyView->Draw(&mDirectoryHierarchy, mpOpenedDir);
        // =============================================================================================================

        ImGui::SameLine(0.0f, 0.0f);

        // =============================================================================================================
        // Draw Directory Items View (Right Side)
        mpOpenedDir = mpOpenedDir ? mpOpenedDir : mDirectoryHierarchy.GetRoot();
        handleDirectoryItemViewRequest();
        mpDirectoryItemView->UpdateOpenedDirectoryView(mpOpenedDir, &mpSelectedDir, &mpSelectedItem);
        mpDirectoryItemView->Draw(mpSelectedDir, mpSelectedItem);
        // =============================================================================================================

        // =============================================================================================================
        // Draw Global Popup
        drawDirectoryPropertiesPopup();
        drawItemPropertiesPopup();
        // =============================================================================================================
    }
    ImGui::End();
    ImGui::PopStyleVar();

    // =============================================================================================================
    // Draw Asset Import Popups
    drawModelImportWizard();
    drawShaderImportWizard();
    drawTextureImportWizard();
    drawTextureBakeWizard();
    // =============================================================================================================
}

void EditorAssetBrowserWindow::processModelLoadPipeline()
{
    switch (mModelLoadState)
    {
        case eModelLoadState::None:
        case eModelLoadState::ParseSetting:
        case eModelLoadState::ImportSetting:
        case eModelLoadState::Fail:
            return;
        case eModelLoadState::Parsing:
            parseModel();
            break;
        case eModelLoadState::ImportBegin:
            beginModelImport();
            if (mModelImportContext.bImportBeginSucceed)
            {
                mModelLoadState = eModelLoadState::Importing;
            }
            else
            {
                mModelLoadState = eModelLoadState::Fail;
            }
            break;
        case eModelLoadState::Importing:
            checkModelImport();
            break;
        default:
            HO_ASSERT(false, "Invalid state.");
    }
}

void EditorAssetBrowserWindow::processTextureLoadPipeline()
{
    switch (mTextureLoadState)
    {
        case eTextureLoadState::None:
        case eTextureLoadState::ParseSetting:
        case eTextureLoadState::ImportSetting:
        case eTextureLoadState::Fail:
            return;
        case eTextureLoadState::Parsing:
            parseTexture();
            break;
        case eTextureLoadState::ImportBegin:
            beginTextureImport();
            if (mTextureImportContext.bImportBeginSucceed)
            {
                mTextureLoadState = eTextureLoadState::Importing;
            }
            else
            {
                mTextureLoadState = eTextureLoadState::Fail;
            }
            break;
        case eTextureLoadState::Importing:
            checkTextureImport();
            break;
        default:
            HO_ASSERT(false, "Invalid state.");
    }
}

void EditorAssetBrowserWindow::processShaderLoadPipeline()
{
    switch (mShaderLoadState)
    {
        case eShaderLoadState::None:
        case eShaderLoadState::ParseSetting:
        case eShaderLoadState::ImportSetting:
        case eShaderLoadState::Fail:
            return;
        case eShaderLoadState::Parsing:
            parseShader();
            break;
        case eShaderLoadState::Importing:
            importShader();
            break;
        default:
            HO_ASSERT(false, "Invalid state.");
    }
}

void EditorAssetBrowserWindow::processTextureBakePipeline()
{
    switch (mTextureBakeState)
    {
        case eTextureBakeState::None:
        case eTextureBakeState::BakeSetting:
        case eTextureBakeState::Fail:
            return;
        case eTextureBakeState::Baking:
            bakeTexture();
            break;
        default:
            HO_ASSERT(false, "Invalid state.");
    }
}

void EditorAssetBrowserWindow::drawMenuBarAddPopup()
{
    if (mbOpenMenuBarAddPopup)
    {
        const ImVec2 buttonMin = ImGui::GetItemRectMin();
        const ImVec2 buttonSize = ImGui::GetItemRectSize();

        ImGui::SetNextWindowPos({buttonMin.x, buttonMin.y + buttonSize.y});

        ImGui::OpenPopup("##AssetBrowserMenuAddPopup");
        mbOpenMenuBarAddPopup = false;
    }

    ImGui::SetNextWindowSize(ImVec2(140.0f, 0.0f), ImGuiCond_Appearing);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4.0f, 8.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(ImGui::GetStyle().ItemSpacing.x, 6.0f));
    if (ImGui::BeginPopup("##AssetBrowserMenuAddPopup", ImGuiWindowFlags_NoDecoration))
    {
        if (ImGui::MenuItem(ICON_MDI_FOLDER_PLUS "  New Folder"))
        {
            mbOpenNewFolderPopup = true;
            ImGui::CloseCurrentPopup();
        }

        if (ImGui::MenuItem(ICON_MDI_FILE_PLUS "  New Asset"))
        {
            // TODO: New Asset Button(load serialized asset)
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    ImGui::PopStyleVar(2);
}

void EditorAssetBrowserWindow::drawNewFolderPopup()
{
    if (mbOpenNewFolderPopup)
    {
        ImGui::OpenPopup("New Folder##AssetBrowserMenuBarNewFolderPopup");
        mbOpenNewFolderPopup = false;
    }

    ImGui::SetNextWindowSize(ImVec2(400.0f, 0.0f), ImGuiCond_Appearing);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4.0f, 8.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(ImGui::GetStyle().ItemSpacing.x, 10.0f));
    if (ImGui::BeginPopupModal("New Folder##AssetBrowserMenuBarNewFolderPopup",
                               nullptr,
                               ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse))
    {
        static char sDirNameBuf[256] = "";

        ImGui::AlignTextToFramePadding();
        ImGui::Text("Folder Name:");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(-1.0f);
        ImGui::InputText("##DirNameInputText", sDirNameBuf, HO_ARRAY_SIZE(sDirNameBuf));

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        const float windowWidth = ImGui::GetContentRegionAvail().x;
        const float buttonWidth = 80.0f;
        const float allButtonWidth = buttonWidth + buttonWidth + ImGui::GetStyle().ItemSpacing.x;

        const float beginX = (windowWidth - allButtonWidth) * 0.5f;

        if (beginX > 0.0f)
        {
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + beginX);
        }

        const bool bIsInputEmpty = (strlen(sDirNameBuf) == 0);

        if (bIsInputEmpty)
        {
            ImGui::BeginDisabled(true);
        }

        if (ImGui::Button("Add", ImVec2(buttonWidth, 0)) || (!bIsInputEmpty && ImGui::IsKeyPressed(ImGuiKey_Enter)))
        {
            mDirectoryHierarchy.AddDirectory(sDirNameBuf, mDirectoryHierarchy.GetRoot());
            sDirNameBuf[0] = '\0';
            mbOpenNewFolderPopup = false;
            ImGui::CloseCurrentPopup();
        }

        if (bIsInputEmpty)
        {
            ImGui::EndDisabled();
        }

        ImGui::SameLine();

        if (ImGui::Button("Cancel", ImVec2(buttonWidth, 0)) || ImGui::IsKeyPressed(ImGuiKey_Escape))
        {
            sDirNameBuf[0] = '\0';
            mbOpenNewFolderPopup = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    ImGui::PopStyleVar(2);
}

void EditorAssetBrowserWindow::drawMenuBarImportPopup()
{
    if (mbOpenMenuBarImportPopup)
    {
        const ImVec2 buttonMin = ImGui::GetItemRectMin();
        const ImVec2 buttonSize = ImGui::GetItemRectSize();

        ImGui::SetNextWindowPos({buttonMin.x, buttonMin.y + buttonSize.y});

        ImGui::OpenPopup("##AssetBrowserMenuImportPopup");
        mbOpenMenuBarImportPopup = false;
    }

    ImGui::SetNextWindowSize(ImVec2(140.0f, 0.0f), ImGuiCond_Appearing);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4.0f, 8.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(ImGui::GetStyle().ItemSpacing.x, 6.0f));
    if (ImGui::BeginPopup("##AssetBrowserMenuImportPopup", ImGuiWindowFlags_NoDecoration))
    {
        if (ImGui::MenuItem(ICON_MDI_FILE_TABLE "  New Model"))
        {
            showModelFileOpenDialog();
            ImGui::CloseCurrentPopup();
        }

        if (ImGui::BeginMenu(ICON_MDI_FILE_IMAGE "  New Texture"))
        {
            if (ImGui::MenuItem(ICON_MDI_IMAGE "  2D"))
            {
                mTextureParseContext.TextureType = eTextureType::Texture2D;
                mTextureLoadState = eTextureLoadState::ParseSetting;
                ImGui::CloseCurrentPopup();
            }

            if (ImGui::MenuItem(ICON_MDI_CUBE_OUTLINE "  Cubemap"))
            {
                mTextureParseContext.TextureType = eTextureType::TextureCubeMap;
                mTextureLoadState = eTextureLoadState::ParseSetting;
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndMenu();
        }

        if (ImGui::MenuItem(ICON_MDI_FILE_CODE "  New Shader"))
        {
            showShaderFileOpenDialog();
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    ImGui::PopStyleVar(2);
}

void EditorAssetBrowserWindow::handleDirectoryHierarchyViewRequest()
{
    if (mpDirectoryHierarchyView->HasPendingRequest())
    {
        const DirectoryHierarchyView::Request& request = mpDirectoryHierarchyView->GetPendingRequest();

        switch (request.Type)
        {
            case DirectoryHierarchyView::eRequestType::SelectDirectory:
                mpSelectedDir = request.pTargetDir;
                mpSelectedItem = nullptr;
                break;
            case DirectoryHierarchyView::eRequestType::OpenDirectory:
                mpOpenedDir = request.pTargetDir;
                mDirectoryHierarchy.SetAncestorExpanded(*mpOpenedDir, true);
                mpOpenedDir->bIsVisible = true;
                mDirectoryHierarchy.SetAncestorVisible(*mpOpenedDir, true);
                mpSelectedDir = nullptr;
                mpSelectedItem = nullptr;
                break;
            case DirectoryHierarchyView::eRequestType::MoveDirectory:
                mDirectoryHierarchy.MoveDirectory(request.pTargetDir, request.pDstDir);
                break;
            case DirectoryHierarchyView::eRequestType::DeleteDirectory:
                if (mpOpenedDir == request.pTargetDir)
                {
                    mpSelectedItem = nullptr;
                }
                if (mpSelectedDir == request.pTargetDir)
                {
                    mpSelectedDir = nullptr;
                }
                mpOpenedDir = mDirectoryHierarchy.DeleteDirectory(*request.pTargetDir);
                break;
            case DirectoryHierarchyView::eRequestType::OpenDirectoryPropertiesPopup:
                ImGui::OpenPopup(
                    (mpSelectedDir->NameStr + std::string(" Properties###AssetBrowserDirectoryPropertiesPopup"))
                        .c_str());
                break;
            case DirectoryHierarchyView::eRequestType::MoveItem:
                mDirectoryHierarchy.MoveItem(request.pTargetItem, request.pDstDir);
                break;
            default:
                HO_ASSERT(false, "Invalid request.");
        }

        mpDirectoryHierarchyView->SetRequestFinished(request);
    }
}

void EditorAssetBrowserWindow::handleDirectoryItemViewRequest()
{
    if (mpDirectoryItemView->HasPendingRequest())
    {
        const DirectoryItemView::Request& request = mpDirectoryItemView->GetPendingRequest();

        switch (request.Type)
        {
            case DirectoryItemView::eRequestType::SelectDirectory:
                mpSelectedDir = request.pTargetDir;
                mpSelectedItem = nullptr;
                break;
            case DirectoryItemView::eRequestType::OpenDirectory:
                mpOpenedDir = request.pTargetDir;
                mDirectoryHierarchy.SetAncestorExpanded(*mpOpenedDir, true);
                mpOpenedDir->bIsVisible = true;
                mDirectoryHierarchy.SetAncestorVisible(*mpOpenedDir, true);
                mpSelectedDir = nullptr;
                mpSelectedItem = nullptr;
                break;
            case DirectoryItemView::eRequestType::MoveDirectory:
                mDirectoryHierarchy.MoveDirectory(request.pTargetDir, request.pDstDir);
                break;
            case DirectoryItemView::eRequestType::DeleteDirectory:
                if (mpOpenedDir == request.pTargetDir)
                {
                    mpSelectedItem = nullptr;
                }
                if (mpSelectedDir == request.pTargetDir)
                {
                    mpSelectedDir = nullptr;
                }
                mpOpenedDir = mDirectoryHierarchy.DeleteDirectory(*request.pTargetDir);
                break;
            case DirectoryItemView::eRequestType::OpenDirectoryPropertiesPopup:
                ImGui::OpenPopup(
                    (mpSelectedDir->NameStr + std::string(" Properties###AssetBrowserDirectoryPropertiesPopup"))
                        .c_str());
                break;

            case DirectoryItemView::eRequestType::SelectItem:
                mpSelectedDir = nullptr;
                mpSelectedItem = request.pTargetItem;
                break;
            case DirectoryItemView::eRequestType::OpenItem:
                // TODO: show asset preview
                break;
            case DirectoryItemView::eRequestType::MoveItem:
                mDirectoryHierarchy.MoveItem(request.pTargetItem, request.pDstDir);
                break;
            case DirectoryItemView::eRequestType::DeleteItem:
                if (mpSelectedItem == request.pTargetItem)
                {
                    mpSelectedItem = nullptr;
                }
                mpSelectedItem = mDirectoryHierarchy.DeleteItem(*request.pTargetItem);
                break;
            case DirectoryItemView::eRequestType::OpenItemPropertiesPopup:
                ImGui::OpenPopup(
                    (mpSelectedItem->NameStr + std::string(" Properties###AssetBrowserItemPropertiesPopup")).c_str());
                break;
            case DirectoryItemView::eRequestType::BakeTexture:
                initializeTextureBakeContext(request.pTargetItem->hAsset.Cast<RTextureAsset>());
                mTextureBakeState = eTextureBakeState::BakeSetting;
                break;
            default:
                HO_ASSERT(false, "Invalid request.");
        }

        mpDirectoryItemView->SetRequestFinished(request);
    }
}

void EditorAssetBrowserWindow::showModelFileOpenDialog()
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
    if (IPlatformApplication::GetInstance().ShowOpenFileDialog(
            &path, "Import Model", filterNamesStr, filterExtensionsStr, HO_ARRAY_SIZE(filterNamesStr), std::string()))
    {
        initializeModelParseContext(path);

        mModelLoadState = eModelLoadState::ParseSetting;
    }
}

void EditorAssetBrowserWindow::initializeModelParseContext(const std::string& importPathStr)
{
    mModelParseContext.Path = Path(importPathStr);

    const std::string defaultName = mModelParseContext.Path.GetFileName().RemovedExtension().ToString();
    strncpy_s(mModelParseContext.ModelName, defaultName.c_str(), sizeof(mModelParseContext.ModelName) - 1);
    mModelParseContext.bMakeStatic = false;

    mModelParseContext.bParseComplete = false;
}

void EditorAssetBrowserWindow::resetModelParseContext()
{
    mModelParseContext.Path = Path();
    mModelParseContext.bMakeStatic = false;

    mModelParseContext.bParseComplete = false;
}

void EditorAssetBrowserWindow::initializeModelImportContext()
{
    mModelImportContext.bImportAsStaticMesh = true;

    mModelImportContext.MaterialTypes = FixedArray<eMaterialAssetType>(
        static_cast<int32_t>(mModelImportContext.pParsedModel->pMaterialIRs.size()), eMaterialAssetType::StandardLit);

    mModelImportContext.TextureUsage =
        FixedArray<eMaterialTextureUsage>(static_cast<int32_t>(mModelImportContext.pParsedModel->pTextureIRs.size()));
    mModelImportContext.TextureFormats =
        FixedArray<eTextureFormat>(static_cast<int32_t>(mModelImportContext.pParsedModel->pTextureIRs.size()));

    for (int32_t texIdx = 0;
         texIdx < static_cast<int32_t>(static_cast<int32_t>(mModelImportContext.pParsedModel->pTextureIRs.size()));
         ++texIdx)
    {
        for (const auto& pMaterialIR : mModelImportContext.pParsedModel->pMaterialIRs)
        {
            for (int32_t usageIdx = 1; usageIdx < static_cast<int32_t>(eMaterialTextureUsage::Last); ++usageIdx)
            {
                if (pMaterialIR->TextureIRIndices[static_cast<int32_t>(
                        toIRTextureUsage(static_cast<eMaterialTextureUsage>(usageIdx)))] == texIdx)
                {
                    mModelImportContext.TextureUsage[texIdx] = static_cast<eMaterialTextureUsage>(usageIdx);
                    mModelImportContext.TextureFormats[texIdx] =
                        getDefaultTextureFormatPerUsage(static_cast<eMaterialTextureUsage>(usageIdx));
                }
            }
        }
    }
    mModelImportContext.bGenerateMipmap =
        FixedArray<bool>(static_cast<int32_t>(mModelImportContext.pParsedModel->pTextureIRs.size()), true);

    mModelImportContext.bHasMorphTargets = false;
    for (const auto& subMesh : mModelImportContext.pParsedModel->pMeshIR->SubMeshes)
    {
        if (!subMesh.MorphTargets.empty())
        {
            mModelImportContext.bHasMorphTargets = true;
            break;
        }
    }
    mModelImportContext.bImportMorphTargets = true;

    mModelImportContext.bHasMorphAnimation = false;
    for (const auto& pAnimIR : mModelImportContext.pParsedModel->pAnimationIRs)
    {
        if (!pAnimIR->MorphTargetTracks.empty())
        {
            mModelImportContext.bHasMorphAnimation = true;
            break;
        }
    }

    mModelImportContext.bImportMorphAnimation =
        FixedArray<bool>(static_cast<int32_t>(mModelImportContext.pParsedModel->pAnimationIRs.size()), true);

    mModelImportContext.bImportBeginComplete = false;
    mModelImportContext.bImportBeginSucceed = true;

    mModelImportContext.bImportComplete = false;
    mModelImportContext.bImportSucceed = true;

    mModelImportContext.TotalImportedAssetCount = 0;
    mModelImportContext.ImportedAssetCount = 0;
}

void EditorAssetBrowserWindow::resetModelImportContext()
{
    mModelImportContext.pParsedModel = nullptr;

    mModelImportContext.bImportAsStaticMesh = true;

    mModelImportContext.MaterialTypes.Release();

    mModelImportContext.TextureUsage.Release();
    mModelImportContext.TextureFormats.Release();
    mModelImportContext.bGenerateMipmap.Release();

    mModelImportContext.bHasMorphTargets = false;
    mModelImportContext.bImportMorphTargets = false;

    mModelImportContext.bHasMorphAnimation = false;
    mModelImportContext.bImportMorphAnimation.Release();

    mModelImportContext.bImportBeginComplete = false;
    mModelImportContext.bImportBeginSucceed = true;

    mModelImportContext.bImportComplete = false;
    mModelImportContext.bImportSucceed = true;

    mModelImportContext.TotalImportedAssetCount = 0;
    mModelImportContext.ImportedAssetCount = 0;

    mModelImportContext.hImportedStaticMesh = ObjectHandle<RStaticMeshAsset>::sNULL;
    mModelImportContext.hImportedMaterials.Release();
    mModelImportContext.hImportedTextures.Release();
    mModelImportContext.hImportedMorphTargets = ObjectHandle<RMorphTargetsAsset>::sNULL;
    mModelImportContext.hImportedMorphAnims.Release();
}

void EditorAssetBrowserWindow::cleanModelImportContext()
{
    AssetSystem::GetInstance().DeleteAsset(mModelImportContext.hImportedStaticMesh);

    for (int32_t i = 0; i < mModelImportContext.hImportedMaterials.GetSize(); ++i)
    {
        AssetSystem::GetInstance().DeleteAsset(mModelImportContext.hImportedMaterials[i]);
    }

    for (int32_t i = 0; i < mModelImportContext.hImportedTextures.GetSize(); ++i)
    {
        AssetSystem::GetInstance().DeleteAsset(mModelImportContext.hImportedTextures[i]);
    }

    AssetSystem::GetInstance().DeleteAsset(mModelImportContext.hImportedMorphTargets);

    for (int32_t i = 0; i < mModelImportContext.hImportedMorphAnims.GetSize(); ++i)
    {
        AssetSystem::GetInstance().DeleteAsset(mModelImportContext.hImportedMorphAnims[i]);
    }
}

void EditorAssetBrowserWindow::drawModelImportWizard()
{
    ePopupResult result = ePopupResult::None;
    switch (mModelLoadState)
    {
        case eModelLoadState::None:
            return;
        case eModelLoadState::ParseSetting:
            result = drawModelParseSettingPopup(&mModelParseContext);
            if (result == ePopupResult::Confirm)
            {
                mModelLoadState = eModelLoadState::Parsing;
            }
            else if (result == ePopupResult::Cancel)
            {
                resetModelParseContext();
                mModelLoadState = eModelLoadState::None;
            }
            break;
        case eModelLoadState::Parsing:
            drawModelParsingPopup(mModelParseContext.bParseComplete);
            if (mModelParseContext.bParseComplete)
            {
                initializeModelImportContext();
                mModelLoadState = eModelLoadState::ImportSetting;
            }
            break;
        case eModelLoadState::ImportSetting:
            result = drawModelImportSettingPopup(&mModelImportContext);
            if (result == ePopupResult::Confirm)
            {
                mModelLoadState = eModelLoadState::ImportBegin;
            }
            else if (result == ePopupResult::Cancel)
            {
                resetModelParseContext();
                resetModelImportContext();
                mModelLoadState = eModelLoadState::None;
            }
            break;
        case eModelLoadState::Importing:
            drawModelImportingPopup(mModelImportContext);
            if (mModelImportContext.bImportComplete)
            {
                if (mModelImportContext.bImportSucceed)
                {
                    addModelToDirectoryHierarchy();
                    resetModelParseContext();
                    resetModelImportContext();
                    mModelLoadState = eModelLoadState::None;
                }
                else
                {
                    mModelLoadState = eModelLoadState::Fail;
                }
            }
            break;
        case eModelLoadState::Fail:
            result = drawModelImportFailedPopup(mModelImportContext);
            if (result == ePopupResult::Confirm)
            {
                resetModelParseContext();
                cleanModelImportContext();
                resetModelImportContext();
                mModelLoadState = eModelLoadState::None;
            }
            break;
        default:
            HO_ASSERT(false, "Invalid state.");
    }
}

void EditorAssetBrowserWindow::parseModel()
{
    mModelImportContext.pParsedModel = parser::parseModelFile(
        mModelParseContext.Path, std::string(mModelParseContext.ModelName), mModelParseContext.bMakeStatic);
    mModelParseContext.bParseComplete = true;
}

void EditorAssetBrowserWindow::beginModelImport()
{
    HO_ASSERT(mModelParseContext.bParseComplete, "Model is not parsed yet.");
    HO_ASSERT(mModelImportContext.pParsedModel, "Model is parsed incompletely.");

    mModelImportContext.bImportBeginSucceed = true;

    // =============================================================================================================
    // Try Import

    // Import mesh
    if (mModelImportContext.bImportAsStaticMesh)
    {
        mModelImportContext.hImportedStaticMesh =
            AssetSystem::GetInstance().ImportStaticMesh(*mModelImportContext.pParsedModel->pMeshIR,
                                                        mModelImportContext.pParsedModel->pMaterialIRs,
                                                        *mModelImportContext.pParsedModel->pSkeletonIR,
                                                        *mModelImportContext.pParsedModel->pSkinIR);
        ++mModelImportContext.TotalImportedAssetCount;
        if (mModelImportContext.hImportedStaticMesh.IsNULL())
        {
            mModelImportContext.bImportBeginSucceed = false;
        }
    }
    else
    {
        HO_ASSERT(false, "Currently not supported skeletal mesh.");
    }

    // Import textures
    mModelImportContext.hImportedTextures =
        FixedArray<ObjectHandle<RTextureAsset>>(mModelImportContext.TextureFormats.GetSize());
    for (int32_t i = 0; i < mModelImportContext.TextureFormats.GetSize(); ++i)
    {
        mModelImportContext.hImportedTextures[i] =
            AssetSystem::GetInstance().ImportTexture2D(*mModelImportContext.pParsedModel->pTextureIRs[i],
                                                       mModelImportContext.TextureFormats[i],
                                                       mModelImportContext.bGenerateMipmap[i]);
        ++mModelImportContext.TotalImportedAssetCount;
        if (mModelImportContext.hImportedTextures[i].IsNULL())
        {
            mModelImportContext.bImportBeginSucceed = false;
        }
    }

    // Import materials
    mModelImportContext.hImportedMaterials =
        FixedArray<ObjectHandle<RMaterialAsset>>(mModelImportContext.MaterialTypes.GetSize());
    for (int32_t i = 0; i < mModelImportContext.MaterialTypes.GetSize(); ++i)
    {
        mModelImportContext.hImportedMaterials[i] =
            AssetSystem::GetInstance().ImportTexturedMaterial(*mModelImportContext.pParsedModel->pMaterialIRs[i],
                                                              mModelImportContext.MaterialTypes[i],
                                                              mModelImportContext.pParsedModel->pTextureIRs);
        ++mModelImportContext.TotalImportedAssetCount;
        if (mModelImportContext.hImportedMaterials[i].IsNULL())
        {
            mModelImportContext.bImportBeginSucceed = false;
        }
    }

    // Import morph targets
    if (mModelImportContext.bHasMorphTargets && mModelImportContext.bImportMorphTargets)
    {
        mModelImportContext.hImportedMorphTargets =
            AssetSystem::GetInstance().ImportMorphTargets(*mModelImportContext.pParsedModel->pMeshIR);
        ++mModelImportContext.TotalImportedAssetCount;
        if (mModelImportContext.hImportedMorphTargets.IsNULL())
        {
            mModelImportContext.bImportBeginSucceed = false;
        }
    }

    // Import morph animations
    mModelImportContext.hImportedMorphAnims =
        FixedArray<ObjectHandle<RMorphAnimationAsset>>(mModelImportContext.bImportMorphAnimation.GetSize());
    if (mModelImportContext.bHasMorphAnimation)
    {
        for (int32_t i = 0; i < mModelImportContext.bImportMorphAnimation.GetSize(); ++i)
        {
            if (mModelImportContext.bImportMorphAnimation[i])
            {
                mModelImportContext.hImportedMorphAnims[i] = AssetSystem::GetInstance().ImportMorphAnimation(
                    *mModelImportContext.pParsedModel->pAnimationIRs[i]);
                ++mModelImportContext.TotalImportedAssetCount;
                if (mModelImportContext.hImportedMorphAnims[i].IsNULL())
                {
                    mModelImportContext.bImportBeginSucceed = false;
                }
            }
        }
    }

    mModelImportContext.bImportBeginComplete = true;
}

void EditorAssetBrowserWindow::checkModelImport()
{
    HO_ASSERT(mModelImportContext.bImportBeginComplete, "Import is not begun yet.");
    HO_ASSERT(mModelImportContext.bImportBeginSucceed, "Beginning import failed.");

    mModelImportContext.ImportedAssetCount = 0;
    mModelImportContext.bImportSucceed = true;

    if (mModelImportContext.hImportedStaticMesh.IsValid())
    {
        switch (mModelImportContext.hImportedStaticMesh->TransferState)
        {
            case eAssetTransferState::Ready:
                ++mModelImportContext.ImportedAssetCount;
                break;
            case eAssetTransferState::Failed:
                ++mModelImportContext.ImportedAssetCount;
                mModelImportContext.bImportSucceed = false;
                break;
            default:
                break;
        }
    }

    for (int32_t i = 0; i < mModelImportContext.hImportedMaterials.GetSize(); ++i)
    {
        if (mModelImportContext.hImportedMaterials[i].IsValid())
        {
            switch (mModelImportContext.hImportedMaterials[i]->TransferState)
            {
                case eAssetTransferState::Ready:
                    ++mModelImportContext.ImportedAssetCount;
                    break;
                case eAssetTransferState::Failed:
                    ++mModelImportContext.ImportedAssetCount;
                    mModelImportContext.bImportSucceed = false;
                    break;
                default:
                    break;
            }
        }
    }

    for (int32_t i = 0; i < mModelImportContext.hImportedTextures.GetSize(); ++i)
    {
        if (mModelImportContext.hImportedTextures[i].IsValid())
        {
            switch (mModelImportContext.hImportedTextures[i]->TransferState)
            {
                case eAssetTransferState::Ready:
                    ++mModelImportContext.ImportedAssetCount;
                    break;
                case eAssetTransferState::Failed:
                    ++mModelImportContext.ImportedAssetCount;
                    mModelImportContext.bImportSucceed = false;
                    break;
                default:
                    break;
            }
        }
    }

    if (mModelImportContext.hImportedMorphTargets.IsValid())
    {
        switch (mModelImportContext.hImportedMorphTargets->TransferState)
        {
            case eAssetTransferState::Ready:
                ++mModelImportContext.ImportedAssetCount;
                break;
            case eAssetTransferState::Failed:
                ++mModelImportContext.ImportedAssetCount;
                mModelImportContext.bImportSucceed = false;
                break;
            default:
                break;
        }
    }

    for (int32_t i = 0; i < mModelImportContext.hImportedMorphAnims.GetSize(); ++i)
    {
        if (mModelImportContext.hImportedMorphAnims[i].IsValid())
        {
            switch (mModelImportContext.hImportedMorphAnims[i]->TransferState)
            {
                case eAssetTransferState::Ready:
                    ++mModelImportContext.ImportedAssetCount;
                    break;
                case eAssetTransferState::Failed:
                    ++mModelImportContext.ImportedAssetCount;
                    mModelImportContext.bImportSucceed = false;
                    break;
                default:
                    break;
            }
        }
    }

    if (mModelImportContext.TotalImportedAssetCount == mModelImportContext.ImportedAssetCount)
    {
        mModelImportContext.bImportComplete = true;

        if (mModelImportContext.bImportSucceed)
        {
            if (mModelImportContext.hImportedStaticMesh.IsValid())
            {
                AssetSystem::GetInstance().ResolveStaticMeshMaterials(mModelImportContext.hImportedStaticMesh);
            }

            for (int32_t i = 0; i < mModelImportContext.hImportedMaterials.GetSize(); ++i)
            {
                if (mModelImportContext.hImportedMaterials[i].IsValid())
                {
                    AssetSystem::GetInstance().ResolveMaterialTextures(mModelImportContext.hImportedMaterials[i]);
                }
            }
        }
    }
}

void EditorAssetBrowserWindow::addModelToDirectoryHierarchy()
{
    HO_ASSERT(mModelImportContext.bImportComplete, "Model is not imported yet.");
    HO_ASSERT(mModelImportContext.bImportSucceed, "Failed to import model.");

    DirectoryHierarchy::Directory* pModelDir =
        mDirectoryHierarchy.AddDirectory(mModelImportContext.pParsedModel->NameStr, mDirectoryHierarchy.GetRoot());
    if (mModelImportContext.hImportedStaticMesh.IsValid())
    {
        mDirectoryHierarchy.AddAsset(mModelImportContext.hImportedStaticMesh, pModelDir);
    }

    for (int32_t i = 0; i < mModelImportContext.hImportedMaterials.GetSize(); ++i)
    {
        if (mModelImportContext.hImportedMaterials[i].IsValid())
        {
            mDirectoryHierarchy.AddAsset(mModelImportContext.hImportedMaterials[i], pModelDir);
        }
    }

    for (int32_t i = 0; i < mModelImportContext.TextureFormats.GetSize(); ++i)
    {
        if (mModelImportContext.hImportedTextures[i].IsValid())
        {
            mDirectoryHierarchy.AddAsset(mModelImportContext.hImportedTextures[i], pModelDir);
        }
    }

    if (mModelImportContext.hImportedMorphTargets.IsValid())
    {
        mDirectoryHierarchy.AddAsset(mModelImportContext.hImportedMorphTargets, pModelDir);
    }

    for (int32_t i = 0; i < mModelImportContext.bImportMorphAnimation.GetSize(); ++i)
    {
        if (mModelImportContext.hImportedMorphAnims[i].IsValid())
        {
            mDirectoryHierarchy.AddAsset(mModelImportContext.hImportedMorphAnims[i], pModelDir);
        }
    }
}

void EditorAssetBrowserWindow::showShaderFileOpenDialog()
{
    std::string filterNamesStr[] = {
        "Shader File(*.vert; *.frag; *.comp)",
    };
    std::string filterExtensionsStr[] = {
        "*.vert;*.frag;*.comp",
    };
    static_assert(HO_ARRAY_SIZE(filterNamesStr) == HO_ARRAY_SIZE(filterExtensionsStr));

    std::string path;
    if (IPlatformApplication::GetInstance().ShowOpenFileDialog(
            &path, "Import Shader", filterNamesStr, filterExtensionsStr, HO_ARRAY_SIZE(filterNamesStr), std::string()))
    {
        initializeShaderParseContext(path);

        mShaderLoadState = eShaderLoadState::ParseSetting;
    }
}

void EditorAssetBrowserWindow::initializeShaderParseContext(const std::string& importPathStr)
{
    mShaderParseContext.bParseComplete = false;
    mShaderParseContext.Path = Path(importPathStr);

    const std::string defaultName = mShaderParseContext.Path.GetFileName().RemovedExtension().ToString();
    strncpy_s(mShaderParseContext.ShaderName, defaultName.c_str(), sizeof(mShaderParseContext.ShaderName) - 1);
}

void EditorAssetBrowserWindow::resetShaderParseContext()
{
    mShaderParseContext.bParseComplete = false;
    mShaderParseContext.Path = Path();
}

void EditorAssetBrowserWindow::initializeShaderImportContext()
{
    mShaderImportContext.bImportComplete = false;

    static const Path sVertExt = Path(std::string(".vert"));
    static const Path sGeomExt = Path(std::string(".geom"));
    static const Path sFragExt = Path(std::string(".frag"));
    static const Path sCompExt = Path(std::string(".comp"));
    const Path ext = mShaderImportContext.pParsedShader->ResourcePath.GetExtension();
    if (ext == sVertExt)
    {
        mShaderImportContext.ShaderStage = eShaderStage::VertexShader;
    }
    else if (ext == sGeomExt)
    {
        mShaderImportContext.ShaderStage = eShaderStage::GeometryShader;
    }
    else if (ext == sFragExt)
    {
        mShaderImportContext.ShaderStage = eShaderStage::FragmentShader;
    }
    else if (ext == sCompExt)
    {
        mShaderImportContext.ShaderStage = eShaderStage::ComputeShader;
    }
    else
    {
        HO_ASSERT(false, "Invalid extension. extension must be '.vert' or '.geom' or '.frag' or '.comp'.");
    }

    mShaderImportContext.CompatibleMaterialType = eMaterialAssetType::StandardLit;

    mShaderImportContext.bImportSucceed = false;
    mShaderImportContext.CompileErrorMsg.clear();
    mShaderImportContext.hImportedShader = ObjectHandle<RShaderAsset>::sNULL;
}

void EditorAssetBrowserWindow::resetShaderImportContext()
{
    mShaderImportContext.bImportComplete = false;
    mShaderImportContext.pParsedShader = nullptr;
    mShaderImportContext.ShaderStage = eShaderStage::None;
    mShaderImportContext.CompatibleMaterialType = eMaterialAssetType::None;
    mShaderImportContext.bImportSucceed = false;
    mShaderImportContext.CompileErrorMsg.clear();
    mShaderImportContext.hImportedShader = ObjectHandle<RShaderAsset>::sNULL;
}

void EditorAssetBrowserWindow::cleanShaderImportContext()
{
    AssetSystem::GetInstance().DeleteAsset(mShaderImportContext.hImportedShader);
    mShaderImportContext.hImportedShader = ObjectHandle<RShaderAsset>::sNULL;
}

void EditorAssetBrowserWindow::drawShaderImportWizard()
{
    ePopupResult result = ePopupResult::None;
    switch (mShaderLoadState)
    {
        case eShaderLoadState::None:
            return;
        case eShaderLoadState::ParseSetting:
            result = drawShaderParseSettingPopup(&mShaderParseContext);
            if (result == ePopupResult::Confirm)
            {
                mShaderLoadState = eShaderLoadState::Parsing;
            }
            else if (result == ePopupResult::Cancel)
            {
                resetShaderParseContext();
                mShaderLoadState = eShaderLoadState::None;
            }
            break;
        case eShaderLoadState::Parsing:
            drawShaderParsingPopup(mShaderParseContext.bParseComplete);
            if (mShaderParseContext.bParseComplete)
            {
                initializeShaderImportContext();
                mShaderLoadState = eShaderLoadState::ImportSetting;
            }
            break;
        case eShaderLoadState::ImportSetting:
            result = drawShaderImportSettingPopup(&mShaderImportContext);
            if (result == ePopupResult::Confirm)
            {
                mShaderLoadState = eShaderLoadState::Importing;
            }
            else if (result == ePopupResult::Cancel)
            {
                resetShaderParseContext();
                resetShaderImportContext();
                mShaderLoadState = eShaderLoadState::None;
            }
            break;
        case eShaderLoadState::Importing:
            drawShaderImportingPopup(mShaderImportContext.bImportComplete);
            if (mShaderImportContext.bImportComplete)
            {
                if (mShaderImportContext.bImportSucceed)
                {
                    addShaderToDirectoryHierarchy();
                    resetShaderParseContext();
                    resetShaderImportContext();
                    mShaderLoadState = eShaderLoadState::None;
                }
                else
                {
                    mShaderLoadState = eShaderLoadState::Fail;
                }
            }
            break;
        case eShaderLoadState::Fail:
            result = drawShaderImportFailedPopup(mShaderImportContext);
            if (result == ePopupResult::Confirm)
            {
                resetShaderParseContext();
                cleanShaderImportContext();
                resetShaderImportContext();
                mShaderLoadState = eShaderLoadState::None;
            }
            break;
        default:
            HO_ASSERT(false, "Invalid state.");
    }
}

void EditorAssetBrowserWindow::parseShader()
{
    mShaderImportContext.pParsedShader =
        parser::parseShaderFile(mShaderParseContext.Path, std::string(mShaderParseContext.ShaderName));
    mShaderParseContext.bParseComplete = true;
}

void EditorAssetBrowserWindow::importShader()
{
    HO_ASSERT(mShaderParseContext.bParseComplete, "Shader is not parsed yet.");
    HO_ASSERT(mShaderImportContext.pParsedShader, "Shader is parsed incompletely.");

    mShaderImportContext.bImportSucceed = true;

    mShaderImportContext.hImportedShader =
        AssetSystem::GetInstance().ImportShaderImmediate(&mShaderImportContext.CompileErrorMsg,
                                                         *mShaderImportContext.pParsedShader,
                                                         mShaderImportContext.ShaderStage,
                                                         mShaderImportContext.CompatibleMaterialType);

    if (mShaderImportContext.hImportedShader.IsNULL())
    {
        mShaderImportContext.bImportSucceed = false;
    }

    mShaderImportContext.bImportComplete = true;
}

void EditorAssetBrowserWindow::addShaderToDirectoryHierarchy()
{
    HO_ASSERT(mShaderImportContext.bImportComplete, "Shader is not imported yet.");
    HO_ASSERT(mShaderImportContext.bImportSucceed, "Shader is imported incompletely.");

    DirectoryHierarchy::Directory* pTargetDir = mpOpenedDir ? mpOpenedDir : mDirectoryHierarchy.GetRoot();

    if (mShaderImportContext.hImportedShader.IsValid())
    {
        mDirectoryHierarchy.AddAsset(mShaderImportContext.hImportedShader, pTargetDir);
    }
}

void EditorAssetBrowserWindow::resetTextureParseContext()
{
    for (auto& path : mTextureParseContext.Path)
    {
        path = Path();
    }

    const std::string defaultName = std::string("Unnamed");
    strncpy_s(mShaderParseContext.ShaderName, defaultName.c_str(), sizeof(mShaderParseContext.ShaderName) - 1);

    mTextureParseContext.bIsLinear = false;
    mTextureParseContext.bParseComplete = false;
}

void EditorAssetBrowserWindow::initializeTextureImportContext()
{
    mTextureImportContext.TextureType = mTextureParseContext.TextureType;

    const bool bIsLinear = mTextureImportContext.pParsedTextures[0]->Img.IsLinear();
    const bool bIsInteger = mTextureImportContext.pParsedTextures[0]->Img.IsInteger();
    const int32_t channelCount = mTextureImportContext.pParsedTextures[0]->Img.GetPhysicalChannelCount();
    if (bIsInteger)
    {
        if (bIsLinear)
        {
            switch (channelCount)
            {
                case 1:
                    mTextureImportContext.TextureFormat = eTextureFormat::BC4_UNORM;
                    break;
                case 2:
                    mTextureImportContext.TextureFormat = eTextureFormat::BC5_UNORM;
                    break;
                case 3:
                case 4:
                    mTextureImportContext.TextureFormat = eTextureFormat::BC7_UNORM;
                    break;
                default:
                    HO_ASSERT(false, "Invalid channel count.");
            }
        }
        else
        {
            switch (channelCount)
            {
                case 1:
                    mTextureImportContext.TextureFormat = eTextureFormat::R8_SRGB;
                    break;
                case 2:
                    mTextureImportContext.TextureFormat = eTextureFormat::R8G8_SRGB;
                    break;
                case 3:
                case 4:
                    mTextureImportContext.TextureFormat = eTextureFormat::BC7_SRGB;
                    break;
                default:
                    HO_ASSERT(false, "Invalid channel count.");
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
                    mTextureImportContext.TextureFormat = eTextureFormat::R16_FLOAT;
                    break;
                case 2:
                    mTextureImportContext.TextureFormat = eTextureFormat::R16G16_FLOAT;
                    break;
                case 3:
                case 4:
                    mTextureImportContext.TextureFormat = eTextureFormat::BC6H_UF16;
                    break;
                default:
                    HO_ASSERT(false, "Invalid channel count.");
            }
        }
        else
        {
            HO_ASSERT(false, "Invalid channel count.");
        }
    }

    mTextureImportContext.bGenerateMipmap = true;

    mTextureImportContext.bImportBeginComplete = false;
    mTextureImportContext.bImportBeginSucceed = true;

    mTextureImportContext.bImportComplete = false;
    mTextureImportContext.bImportSucceed = true;

    mTextureImportContext.TotalImportedTextureCount = 0;
    mTextureImportContext.ImportedTextureCount = 0;

    mTextureImportContext.hImportedTexture = ObjectHandle<RTextureAsset>::sNULL;
}

void EditorAssetBrowserWindow::resetTextureImportContext()
{
    mTextureImportContext.bImportComplete = false;
    mTextureImportContext.TextureType = eTextureType::None;
    for (auto& pParsedTexture : mTextureImportContext.pParsedTextures)
    {
        pParsedTexture = nullptr;
    }
    mTextureImportContext.TextureFormat = eTextureFormat::R8G8B8A8_SRGB;
    mTextureImportContext.bGenerateMipmap = true;

    mTextureImportContext.bImportBeginComplete = false;
    mTextureImportContext.bImportBeginSucceed = true;

    mTextureImportContext.bImportComplete = false;
    mTextureImportContext.bImportSucceed = true;

    mTextureImportContext.TotalImportedTextureCount = 0;
    mTextureImportContext.ImportedTextureCount = 0;

    mTextureImportContext.hImportedTexture = ObjectHandle<RTextureAsset>::sNULL;
}

void EditorAssetBrowserWindow::cleanTextureImportContext()
{
    AssetSystem::GetInstance().DeleteAsset(mTextureImportContext.hImportedTexture);
    mTextureImportContext.hImportedTexture = ObjectHandle<RTextureAsset>::sNULL;
}

void EditorAssetBrowserWindow::drawTextureImportWizard()
{
    ePopupResult result = ePopupResult::None;
    switch (mTextureLoadState)
    {
        case eTextureLoadState::None:
            return;
        case eTextureLoadState::ParseSetting:
            result = drawTextureParseSettingPopup(&mTextureParseContext);
            if (result == ePopupResult::Confirm)
            {
                mTextureLoadState = eTextureLoadState::Parsing;
            }
            else if (result == ePopupResult::Cancel)
            {
                resetTextureParseContext();
                mTextureLoadState = eTextureLoadState::None;
            }
            break;
        case eTextureLoadState::Parsing:
            drawTextureParsingPopup(mTextureParseContext.bParseComplete);
            if (mTextureParseContext.bParseComplete)
            {
                initializeTextureImportContext();
                mTextureLoadState = eTextureLoadState::ImportSetting;
            }
            break;
        case eTextureLoadState::ImportSetting:
            result = drawTextureImportSettingPopup(&mTextureImportContext);
            if (result == ePopupResult::Confirm)
            {
                mTextureLoadState = eTextureLoadState::ImportBegin;
            }
            else if (result == ePopupResult::Cancel)
            {
                resetTextureParseContext();
                resetTextureImportContext();
                mTextureLoadState = eTextureLoadState::None;
            }
            break;
        case eTextureLoadState::Importing:
            drawTextureImportingPopup(mTextureImportContext);
            if (mTextureImportContext.bImportComplete)
            {
                if (mTextureImportContext.bImportSucceed)
                {
                    addTextureToDirectoryHierarchy();
                    resetTextureParseContext();
                    resetTextureImportContext();
                    mTextureLoadState = eTextureLoadState::None;
                }
                else
                {
                    mTextureLoadState = eTextureLoadState::Fail;
                }
            }
            break;
        case eTextureLoadState::Fail:
            result = drawTextureImportFailedPopup(mTextureImportContext);
            if (result == ePopupResult::Confirm)
            {
                resetTextureParseContext();
                cleanTextureImportContext();
                resetTextureImportContext();
                mTextureLoadState = eTextureLoadState::None;
            }
            break;
        default:
            HO_ASSERT(false, "Invalid state.");
    }
}

void EditorAssetBrowserWindow::parseTexture()
{
    const int32_t texCount = mTextureParseContext.TextureType == eTextureType::Texture2D ? 1 : 6;
    for (int32_t i = 0; i < texCount; ++i)
    {
        mTextureImportContext.pParsedTextures[i] =
            parser::parseTextureFile(mTextureParseContext.Path[i],
                                     std::string(mTextureParseContext.TextureName),
                                     mTextureParseContext.bIsLinear);
        mTextureParseContext.bParseComplete = true;
    }
}

void EditorAssetBrowserWindow::beginTextureImport()
{
    HO_ASSERT(mTextureParseContext.bParseComplete, "Texture is not parsed yet.");

    mTextureImportContext.bImportBeginSucceed = true;

    if (mTextureImportContext.TextureType == eTextureType::Texture2D)
    {
        HO_ASSERT(mTextureImportContext.pParsedTextures[0], "Texture is parsed incompletely.");
        mTextureImportContext.hImportedTexture =
            AssetSystem::GetInstance().ImportTexture2D(*mTextureImportContext.pParsedTextures[0],
                                                       mTextureImportContext.TextureFormat,
                                                       mTextureImportContext.bGenerateMipmap);
        ++mTextureImportContext.TotalImportedTextureCount;
        if (mTextureImportContext.hImportedTexture.IsNULL())
        {
            mTextureImportContext.bImportBeginSucceed = false;
        }
    }
    else if (mTextureImportContext.TextureType == eTextureType::TextureCubeMap)
    {
        for (auto& pParsedTexture : mTextureImportContext.pParsedTextures)
        {
            (void)pParsedTexture;
            HO_ASSERT(pParsedTexture, "Texture is parsed incompletely.");
        }

        const int32_t width = mTextureImportContext.pParsedTextures[0]->Img.GetWidth();
        const int32_t height = mTextureImportContext.pParsedTextures[0]->Img.GetHeight();
        ;

        if (width != height)
        {
            mTextureImportContext.bImportBeginSucceed = false;
        }

        for (int32_t i = 1; i < 6 && mTextureImportContext.bImportBeginSucceed; ++i)
        {
            if (mTextureImportContext.pParsedTextures[i]->Img.GetWidth() !=
                    mTextureImportContext.pParsedTextures[i]->Img.GetHeight() ||
                mTextureImportContext.pParsedTextures[i]->Img.GetWidth() != width ||
                mTextureImportContext.pParsedTextures[i]->Img.GetHeight() != height)
            {
                mTextureImportContext.bImportBeginSucceed = false;
            }
        }

        if (mTextureImportContext.bImportBeginSucceed)
        {
            mTextureImportContext.hImportedTexture =
                AssetSystem::GetInstance().ImportTextureCubemap(mTextureImportContext.pParsedTextures[0]->NameStr,
                                                                *mTextureImportContext.pParsedTextures[0],
                                                                *mTextureImportContext.pParsedTextures[1],
                                                                *mTextureImportContext.pParsedTextures[2],
                                                                *mTextureImportContext.pParsedTextures[3],
                                                                *mTextureImportContext.pParsedTextures[4],
                                                                *mTextureImportContext.pParsedTextures[5],
                                                                mTextureImportContext.TextureFormat,
                                                                mTextureImportContext.bGenerateMipmap);

            ++mTextureImportContext.TotalImportedTextureCount;

            if (mTextureImportContext.hImportedTexture.IsNULL())
            {
                mTextureImportContext.bImportBeginSucceed = false;
            }
        }
    }
    else
    {
        HO_ASSERT(false, "Invlaid texture type.");
    }

    mTextureImportContext.bImportBeginComplete = true;
}

void EditorAssetBrowserWindow::checkTextureImport()
{
    HO_ASSERT(mTextureImportContext.bImportBeginComplete, "Import is not begun yet.");
    HO_ASSERT(mTextureImportContext.bImportBeginSucceed, "Beginning import failed.");

    mTextureImportContext.ImportedTextureCount = 0;
    mTextureImportContext.bImportSucceed = true;

    if (mTextureImportContext.hImportedTexture.IsValid())
    {
        switch (mTextureImportContext.hImportedTexture->TransferState)
        {
            case eAssetTransferState::Ready:
                ++mTextureImportContext.ImportedTextureCount;
                break;
            case eAssetTransferState::Failed:
                ++mTextureImportContext.ImportedTextureCount;
                mTextureImportContext.bImportSucceed = false;
                break;
            default:
                break;
        }
    }

    if (mTextureImportContext.TotalImportedTextureCount == mTextureImportContext.ImportedTextureCount)
    {
        mTextureImportContext.bImportComplete = true;
    }
}

void EditorAssetBrowserWindow::addTextureToDirectoryHierarchy()
{
    HO_ASSERT(mTextureImportContext.bImportComplete, "Texture is not imported yet.");
    HO_ASSERT(mTextureImportContext.bImportSucceed, "Texture is imported incompletely.");

    DirectoryHierarchy::Directory* pTargetDir = mpOpenedDir ? mpOpenedDir : mDirectoryHierarchy.GetRoot();

    if (mTextureImportContext.hImportedTexture.IsValid())
    {
        mDirectoryHierarchy.AddAsset(mTextureImportContext.hImportedTexture, pTargetDir);
    }
}

void EditorAssetBrowserWindow::initializeTextureBakeContext(ObjectHandle<RTextureAsset> hSrcTexture)
{
    HO_ASSERT(hSrcTexture.IsValid(), "Invalid source texture.");
    mTextureBakeContext.bBakeComplete = false;

    mTextureBakeContext.hSrcTexture = hSrcTexture;

    const std::string defaultName = std::string(getAssetDisplayName(hSrcTexture)).append("_Baked");
    strncpy_s(mTextureBakeContext.DstTextureName, defaultName.c_str(), sizeof(mTextureBakeContext.DstTextureName) - 1);

    mTextureBakeContext.BakeType = eEnvironmentBakeType::Irradiance;
    mTextureBakeContext.BakedMapSize = 64;
    mTextureBakeContext.SampleDelta = 0.05f;
    mTextureBakeContext.SampleCount = 1024;
}

void EditorAssetBrowserWindow::resetTextureBakeContext()
{
    mTextureBakeContext.bBakeComplete = false;
    mTextureBakeContext.hSrcTexture = ObjectHandle<RTextureAsset>::sNULL;
    mTextureBakeContext.DstTextureName[0] = '\0';
    mTextureBakeContext.BakeType = eEnvironmentBakeType::Irradiance;
    mTextureBakeContext.BakedMapSize = 0;
    mTextureBakeContext.SampleDelta = 0.0f;
    mTextureBakeContext.SampleCount = 0;
    mTextureBakeContext.bBakeSucceed = false;
    mTextureBakeContext.hDstTexture = ObjectHandle<RTextureAsset>::sNULL;
}

void EditorAssetBrowserWindow::cleanTextureBakeContext()
{
    AssetSystem::GetInstance().DeleteAsset(mTextureBakeContext.hDstTexture);
    mTextureBakeContext.hDstTexture = ObjectHandle<RTextureAsset>::sNULL;
}

void EditorAssetBrowserWindow::drawTextureBakeWizard()
{
    ePopupResult result = ePopupResult::None;
    switch (mTextureBakeState)
    {
        case eTextureBakeState::None:
            return;
        case eTextureBakeState::BakeSetting:
            result = drawTextureBakeSettingPopup(&mTextureBakeContext);
            if (result == ePopupResult::Confirm)
            {
                mTextureBakeState = eTextureBakeState::Baking;
            }
            else if (result == ePopupResult::Cancel)
            {
                resetTextureBakeContext();
                mTextureBakeState = eTextureBakeState::None;
            }
            break;
        case eTextureBakeState::Baking:
            drawTextureBakingPopup(mTextureBakeContext.bBakeComplete);
            if (mTextureBakeContext.bBakeComplete)
            {
                if (mTextureBakeContext.bBakeSucceed)
                {
                    addBakedTextureToDirectoryHierarchy();
                    resetTextureBakeContext();
                    mTextureBakeState = eTextureBakeState::None;
                }
                else
                {
                    mTextureBakeState = eTextureBakeState::Fail;
                }
            }
            break;
        case eTextureBakeState::Fail:
            result = drawTextureBakeFailedPopup(mTextureBakeContext);
            if (result == ePopupResult::Confirm)
            {
                resetTextureBakeContext();
                cleanTextureBakeContext();
                mTextureLoadState = eTextureLoadState::None;
            }
            break;
        default:
            HO_ASSERT(false, "Invalid state.");
    }
}

void EditorAssetBrowserWindow::bakeTexture()
{
    HO_ASSERT(mTextureBakeContext.hSrcTexture.IsValid(), "Bake setting not completed yet.");

    if (!RenderingSystem::GetInstance().IsUploaded(mTextureBakeContext.hSrcTexture))
    {
        RenderingSystem::GetInstance().UploadTexture(mTextureBakeContext.hSrcTexture);
        mTextureBakeContext.bBakeSucceed = false;
        mTextureBakeContext.bBakeComplete = false;
        return;
    }

    if (mTextureBakeContext.hDstTexture.IsNULL())
    {
        char buf[256] = {};
        snprintf(buf,
                 HO_ARRAY_SIZE(buf),
                 "%s%s%s",
                 mTextureBakeContext.hSrcTexture->AssetPath.GetParentPath().ToString().c_str(),
                 "::TextureCubeMap::",
                 mTextureBakeContext.DstTextureName);
        const std::string newTexNameStr = getUniqueAssetName(buf);
        mTextureBakeContext.hDstTexture =
            AssetSystem::GetInstance().CreateAsset<RTextureAsset>(StringHandle(newTexNameStr));
        mTextureBakeContext.hDstTexture->TransferState = eAssetTransferState::PendingDownload;
        EnvironmentBakePassDesc bakeDesc;
        bakeDesc.BakeType = mTextureBakeContext.BakeType;
        bakeDesc.BakedMapSize = mTextureBakeContext.BakedMapSize;
        bakeDesc.SampleDelta = mTextureBakeContext.SampleDelta;
        bakeDesc.SampleCount = mTextureBakeContext.SampleCount;
        bakeDesc.hSrcTexture = mTextureBakeContext.hSrcTexture->hRenderProxy;
        bakeDesc.hDstTexture = mTextureBakeContext.hDstTexture;
        if (RenderingSystem::GetInstance().BeginEnvironmentBakePass(bakeDesc))
        {
            RenderingSystem::GetInstance().EndRenderPass();
        }
        mTextureBakeContext.bBakeSucceed = false;
        mTextureBakeContext.bBakeComplete = false;
        return;
    }

    if (mTextureBakeContext.hDstTexture->TransferState == eAssetTransferState::PendingDownload)
    {
        mTextureBakeContext.bBakeSucceed = false;
        mTextureBakeContext.bBakeComplete = false;
        return;
    }

    if (mTextureBakeContext.hDstTexture->TransferState == eAssetTransferState::Ready)
    {
        mTextureBakeContext.bBakeSucceed = true;
    }
    else if (mTextureBakeContext.hDstTexture->TransferState == eAssetTransferState::Failed)
    {
        mTextureBakeContext.bBakeSucceed = false;
    }

    mTextureBakeContext.bBakeComplete = true;
}

void EditorAssetBrowserWindow::addBakedTextureToDirectoryHierarchy()
{
    HO_ASSERT(mTextureBakeContext.bBakeComplete, "Texture is not baked yet.");
    HO_ASSERT(mTextureBakeContext.bBakeSucceed, "Texture is baked incompletely.");

    DirectoryHierarchy::Directory* pTargetDir = mpOpenedDir ? mpOpenedDir : mDirectoryHierarchy.GetRoot();

    if (mTextureBakeContext.hDstTexture.IsValid())
    {
        mDirectoryHierarchy.AddAsset(mTextureBakeContext.hDstTexture, pTargetDir);
    }
}

void EditorAssetBrowserWindow::drawDirectoryPropertiesPopup()
{
    if (!mpSelectedDir)
    {
        return;
    }

    static uint32_t sLastPopupDirID = 0;
    static std::string sPopupNameStr;
    static char sNameBuffer[256];
    static std::string sDirLocationStr;

    // Update popup data cache
    if (sLastPopupDirID != mpSelectedDir->ID || mpSelectedDir->bIsDirty || sPopupNameStr.empty())
    {
        sLastPopupDirID = mpSelectedDir->ID;
        sPopupNameStr = mpSelectedDir->NameStr + std::string(" Properties###AssetBrowserDirectoryPropertiesPopup");
        strncpy_s(sNameBuffer, mpSelectedDir->NameStr.c_str(), sizeof(sNameBuffer) - 1);
        sDirLocationStr = mDirectoryHierarchy.GetDirectoryPath(*mpSelectedDir).ToString();
        mpSelectedDir->bIsDirty = false;
    }

    ImGui::SetNextWindowSize(ImVec2(600.0f, 0.0f), ImGuiCond_Appearing);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(12.0f, 8.0f));

    if (ImGui::BeginPopupModal(sPopupNameStr.c_str(),
                               nullptr,
                               ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar))
    {
        const float contentWidth = ImGui::GetContentRegionAvail().x;
        const float windowPaddingX = ImGui::GetStyle().WindowPadding.x;

        // =============================================================================================================
        // Draw Directory Name
        ImGui::Spacing();
        ImGui::AlignTextToFramePadding();
        ImGui::Text(ICON_MDI_FOLDER);
        ImGui::SameLine();

        const float inputWidth = contentWidth - ImGui::GetCursorPosX() + windowPaddingX;
        ImGui::SetNextItemWidth(inputWidth);

        if (mpSelectedDir->NameStr == "All")
        {
            ImGui::BeginDisabled();
        }

        ImGui::InputText("##DirRenameInput", sNameBuffer, sizeof(sNameBuffer));

        if (mpSelectedDir->NameStr == "All")
        {
            ImGui::EndDisabled();
        }

        // =============================================================================================================

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // =============================================================================================================
        // Draw Directory Metadata
        ImGui::Text("Type:");
        ImGui::SameLine();
        {
            const char* typeText = "File Folder";
            const float textWidth = ImGui::CalcTextSize(typeText).x;
            ImGui::SetCursorPosX(contentWidth + windowPaddingX - textWidth);
            ImGui::Text("%s", typeText);
        }

        ImGui::Text("Location:");
        ImGui::SameLine();
        {
            const float textWidth = ImGui::CalcTextSize(sDirLocationStr.c_str()).x;
            ImGui::SetCursorPosX(contentWidth + windowPaddingX - textWidth);
            ImGui::Text("%s", sDirLocationStr.c_str());
        }

        auto drawByteSizeRightAligned = [contentWidth, windowPaddingX](int64_t byteSize)
        {
            static constexpr double sKB = 1024.0;
            static constexpr double sMB = sKB * 1024.0;
            static constexpr double sGB = sMB * 1024.0;

            char sizeBuffer[64];
            if (byteSize < 0)
            {
                strcpy_s(sizeBuffer, "N/A");
            }
            else
            {
                const double bytes = static_cast<double>(byteSize);
                if (bytes >= sGB)
                {
                    sprintf_s(sizeBuffer, "%.2f GB", bytes / sGB);
                }
                else if (bytes >= sMB)
                {
                    sprintf_s(sizeBuffer, "%.2f MB", bytes / sMB);
                }
                else
                {
                    sprintf_s(sizeBuffer, "%.2f KB", bytes / sKB);
                }
            }

            const float textWidth = ImGui::CalcTextSize(sizeBuffer).x;
            ImGui::SetCursorPosX(contentWidth + windowPaddingX - textWidth);
            ImGui::Text("%s", sizeBuffer);
        };

        ImGui::Text("Size:");
        ImGui::SameLine();
        drawByteSizeRightAligned(mpSelectedDir->ByteSize);

        ImGui::Text("Contains:");
        ImGui::SameLine();
        {
            char containsBuffer[128];
            sprintf_s(
                containsBuffer, "%zu Files, %zu Folders", mpSelectedDir->Items.size(), mpSelectedDir->pChildren.size());

            const float textWidth = ImGui::CalcTextSize(containsBuffer).x;
            ImGui::SetCursorPosX(contentWidth + windowPaddingX - textWidth);
            ImGui::Text("%s", containsBuffer);
        }
        // =============================================================================================================

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // =============================================================================================================
        // Draw Buttons
        const float buttonWidth = 80.0f;
        const float totalButtonsWidth = (buttonWidth * 3.0f) + (ImGui::GetStyle().ItemSpacing.x * 2.0f);
        ImGui::SetCursorPosX(ImGui::GetWindowWidth() - totalButtonsWidth - ImGui::GetStyle().WindowPadding.x);

        if (ImGui::Button("OK", ImVec2(buttonWidth, 0.0f)) || ImGui::IsKeyPressed(ImGuiKey_Enter))
        {
            if (strlen(sNameBuffer) > 0)
            {
                mpSelectedDir->NameStr = sNameBuffer;
                mpSelectedDir->bIsDirty = true;
            }
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();

        if (ImGui::Button("Cancel", ImVec2(buttonWidth, 0.0f)) || ImGui::IsKeyPressed(ImGuiKey_Escape))
        {
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();

        if (ImGui::Button("Apply", ImVec2(buttonWidth, 0.0f)))
        {
            if (strlen(sNameBuffer) > 0)
            {
                mpSelectedDir->NameStr = sNameBuffer;
                mpSelectedDir->bIsDirty = true;
            }
        }
        // =============================================================================================================
        ImGui::EndPopup();
    }

    ImGui::PopStyleVar();
}

void EditorAssetBrowserWindow::drawItemPropertiesPopup()
{
    if (!mpSelectedItem)
    {
        return;
    }

    static uint32_t sLastPopupItemID = 0;
    static std::string sPopupNameStr;
    static char sNameBuffer[256];
    static std::string sItemLocationStr;

    // Update popup data cache
    if (sLastPopupItemID != mpSelectedItem->ID || mpSelectedItem->bIsDirty || sPopupNameStr.empty())
    {
        sLastPopupItemID = mpSelectedItem->ID;
        sPopupNameStr = mpSelectedItem->NameStr + std::string(" Properties###AssetBrowserItemPropertiesPopup");
        strncpy_s(sNameBuffer, mpSelectedItem->NameStr.c_str(), sizeof(sNameBuffer) - 1);
        sItemLocationStr = mDirectoryHierarchy.GetItemPath(*mpSelectedItem).ToString();
        mpSelectedItem->bIsDirty = false;
    }

    ImGui::SetNextWindowSize(ImVec2(600.0f, 0.0f), ImGuiCond_Appearing);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(12.0f, 8.0f));

    if (ImGui::BeginPopupModal(sPopupNameStr.c_str(),
                               nullptr,
                               ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar))
    {
        const float contentWidth = ImGui::GetContentRegionAvail().x;
        const float windowPaddingX = ImGui::GetStyle().WindowPadding.x;

        // =============================================================================================================
        // Draw Item Name
        ImGui::Spacing();
        ImGui::AlignTextToFramePadding();
        ImGui::Text("%s", getAssetDisplayIcon(mpSelectedItem->hAsset));
        ImGui::SameLine();

        const float inputWidth = contentWidth - ImGui::GetCursorPosX() + windowPaddingX;
        ImGui::SetNextItemWidth(inputWidth);
        ImGui::InputText("##ItemRenameInput", sNameBuffer, sizeof(sNameBuffer));
        // =============================================================================================================

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // =============================================================================================================
        // Draw Item Metadata
        ImGui::Text("Type:");
        ImGui::SameLine();
        {
            const char* typeText = *mpSelectedItem->hAsset->GetClass().GetName();
            const float textWidth = ImGui::CalcTextSize(typeText).x;
            ImGui::SetCursorPosX(contentWidth + windowPaddingX - textWidth);
            ImGui::Text("%s", typeText);
        }

        ImGui::Text("Location:");
        ImGui::SameLine();
        {
            const float textWidth = ImGui::CalcTextSize(sItemLocationStr.c_str()).x;
            ImGui::SetCursorPosX(contentWidth + windowPaddingX - textWidth);
            ImGui::Text("%s", sItemLocationStr.c_str());
        }

        auto drawByteSizeRightAligned = [contentWidth, windowPaddingX](int64_t byteSize)
        {
            static constexpr double sKB = 1024.0;
            static constexpr double sMB = sKB * 1024.0;
            static constexpr double sGB = sMB * 1024.0;

            char sizeBuffer[64];
            if (byteSize < 0)
            {
                strcpy_s(sizeBuffer, "N/A");
            }
            else
            {
                const double bytes = static_cast<double>(byteSize);
                if (bytes >= sGB)
                {
                    sprintf_s(sizeBuffer, "%.2f GB", bytes / sGB);
                }
                else if (bytes >= sMB)
                {
                    sprintf_s(sizeBuffer, "%.2f MB", bytes / sMB);
                }
                else
                {
                    sprintf_s(sizeBuffer, "%.2f KB", bytes / sKB);
                }
            }

            const float textWidth = ImGui::CalcTextSize(sizeBuffer).x;
            ImGui::SetCursorPosX(contentWidth + windowPaddingX - textWidth);
            ImGui::Text("%s", sizeBuffer);
        };

        ImGui::Text("Size:");
        ImGui::SameLine();
        drawByteSizeRightAligned(mpSelectedItem->ByteSize);
        // =============================================================================================================

        // =============================================================================================================
        // Draw Buttons

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        const float buttonWidth = 80.0f;
        const float totalButtonsWidth = (buttonWidth * 3.0f) + (ImGui::GetStyle().ItemSpacing.x * 2.0f);
        ImGui::SetCursorPosX(ImGui::GetWindowWidth() - totalButtonsWidth - ImGui::GetStyle().WindowPadding.x);

        if (ImGui::Button("OK", ImVec2(buttonWidth, 0.0f)) || ImGui::IsKeyPressed(ImGuiKey_Enter))
        {
            if (strlen(sNameBuffer) > 0)
            {
                mpSelectedItem->NameStr = sNameBuffer;
                mpSelectedItem->bIsDirty = true;
            }
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();

        if (ImGui::Button("Cancel", ImVec2(buttonWidth, 0.0f)) || ImGui::IsKeyPressed(ImGuiKey_Escape))
        {
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();

        if (ImGui::Button("Apply", ImVec2(buttonWidth, 0.0f)))
        {
            if (strlen(sNameBuffer) > 0)
            {
                mpSelectedItem->NameStr = sNameBuffer;
                mpSelectedItem->bIsDirty = true;
            }
        }
        // =============================================================================================================
        ImGui::EndPopup();
    }

    ImGui::PopStyleVar();
}

// NOTE: Supposed duplicated name handled to 'Name (n)'
std::string EditorAssetBrowserWindow::getUniqueAssetName(const std::string& baseNameStr) const
{
    static const int32_t sMaxDupCount = 2048;
    static std::vector<bool> sbIsNumUsed(sMaxDupCount);

    std::fill(sbIsNumUsed.begin(), sbIsNumUsed.end(), false);
    int32_t maxNum = -1;
    for (const auto& hAsset : AssetSystem::GetInstance().GetAssets<RIAsset>())
    {
        std::string objNameStr(*hAsset->hName);
        if (baseNameStr.size() > objNameStr.size())
        {
            continue;
        }

        const std::string::size_type idx = objNameStr.find(baseNameStr);
        if (idx != 0)
        {
            continue;
        }

        int32_t num = -1;
        if (baseNameStr.size() == objNameStr.size())
        {
            num = 0;
        }
        else if (objNameStr.find(" (", idx) == baseNameStr.size() && objNameStr[objNameStr.size() - 1] == ')')
        {
            const char* first = objNameStr.c_str() + baseNameStr.size() + 2;
            const char* last = objNameStr.c_str() + objNameStr.size() - 1;

            int value = 0;
            auto result = std::from_chars(first, last, value);

            if (result.ec == std::errc{} && result.ptr == last)
            {
                num = value;
            }
        }

        HO_ASSERT(num < sMaxDupCount, "Duplicated object count exceeds maximum limit.");
        if (num >= 0)
        {
            sbIsNumUsed[num] = true;
            if (maxNum < num)
            {
                maxNum = num;
            }
        }
    }

    int32_t targetNum = -1;
    for (int32_t i = 0; i <= maxNum; ++i)
    {
        if (!sbIsNumUsed[i])
        {
            targetNum = i;
            break;
        }
    }

    if (targetNum == -1)
    {
        targetNum = maxNum + 1;
    }

    if (targetNum == 0)
    {
        return baseNameStr;
    }

    return std::string(baseNameStr).append(" (").append(std::to_string(targetNum)).append(")");
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
} // namespace ho