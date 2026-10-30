#include "EditorAssetBrowserWindow.h"

#include <IconsMaterialDesignIcons.h>
#include <imgui.h>
#include <imgui_internal.h>

#include "../Utils/EditorDisplayUtils.h"
#include "DirectoryHierarchyView.h"
#include "DirectoryItemView.h"
#include "Subsystems/Asset/AssetSystem.h"

namespace ho
{
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
    mModelImportWorkflow.Process();
    if (mModelImportWorkflow.IsSucceed())
    {
        addModelToDirectoryHierarchy();
    }
    mTextureImportWorkflow.Process();
    if (mTextureImportWorkflow.IsSucceed())
    {
        addTextureToDirectoryHierarchy();
    }
    mShaderImportWorkflow.Process();
    if (mShaderImportWorkflow.IsSucceed())
    {
        addShaderToDirectoryHierarchy();
    }
    mTextureBakeWorkflow.Process();
    if (mTextureBakeWorkflow.IsSucceed())
    {
        addBakedTextureToDirectoryHierarchy();
    }
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

        handleDirectoryHierarchyViewRequest();
        mpDirectoryHierarchyView->Draw(&mDirectoryHierarchy, mpOpenedDir);

        ImGui::SameLine(0.0f, 0.0f);

        mpOpenedDir = mpOpenedDir ? mpOpenedDir : mDirectoryHierarchy.GetRoot();
        handleDirectoryItemViewRequest();
        mpDirectoryItemView->UpdateOpenedDirectoryView(mpOpenedDir, &mpSelectedDir, &mpSelectedItem);
        mpDirectoryItemView->Draw(mpSelectedDir, mpSelectedItem);
    }
    ImGui::End();
    ImGui::PopStyleVar();

    drawDirectoryPropertiesPopup();
    drawItemPropertiesPopup();

    mModelImportWorkflow.Draw();
    mTextureImportWorkflow.Draw();
    mShaderImportWorkflow.Draw();
    mTextureBakeWorkflow.Draw();
}

void EditorAssetBrowserWindow::drawMenuBarAddPopup()
{
    if (mbOpenMenuBarAddPopup)
    {
        const ImVec2 buttonMin = ImGui::GetItemRectMin();
        const ImVec2 buttonSize = ImGui::GetItemRectSize();

        ImGui::SetNextWindowPos({buttonMin.x, buttonMin.y + buttonSize.y});

        ImGui::SetNextWindowSize(ImVec2(140.0f, 0.0f), ImGuiCond_Appearing);
        ImGui::OpenPopup("##AssetBrowserMenuAddPopup");
        mbOpenMenuBarAddPopup = false;
    }

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
        ImGui::SetNextWindowSize(ImVec2(400.0f, 0.0f), ImGuiCond_Appearing);
        ImGui::OpenPopup("New Folder##AssetBrowserMenuBarNewFolderPopup");
        mbOpenNewFolderPopup = false;
    }

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
        ImGui::SetNextWindowSize(ImVec2(140.0f, 0.0f), ImGuiCond_Appearing);
        ImGui::OpenPopup("##AssetBrowserMenuImportPopup");
        mbOpenMenuBarImportPopup = false;
    }

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4.0f, 8.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(ImGui::GetStyle().ItemSpacing.x, 6.0f));
    if (ImGui::BeginPopup("##AssetBrowserMenuImportPopup", ImGuiWindowFlags_NoDecoration))
    {
        if (ImGui::MenuItem(ICON_MDI_FILE_TABLE "  New Model"))
        {
            // showModelFileOpenDialog();
            mModelImportWorkflow.Begin();
            ImGui::CloseCurrentPopup();
        }

        if (ImGui::BeginMenu(ICON_MDI_FILE_IMAGE "  New Texture"))
        {
            if (ImGui::MenuItem(ICON_MDI_IMAGE "  2D"))
            {
                mTextureImportWorkflow.Begin2D();
                ImGui::CloseCurrentPopup();
            }

            if (ImGui::MenuItem(ICON_MDI_CUBE_OUTLINE "  Cubemap"))
            {
                mTextureImportWorkflow.BeginCubemap();
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndMenu();
        }

        if (ImGui::MenuItem(ICON_MDI_FILE_CODE "  New Shader"))
        {
            mShaderImportWorkflow.Begin();
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
                mbOpenDirectoryPropertiesPopup = true;
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
                mbOpenItemPropertiesPopup = true;
                break;
            case DirectoryItemView::eRequestType::BakeTexture:
                mTextureBakeWorkflow.Begin(request.pTargetItem->hAsset.Cast<RTextureAsset>());
                break;
            default:
                HO_ASSERT(false, "Invalid request.");
        }

        mpDirectoryItemView->SetRequestFinished(request);
    }
}

void EditorAssetBrowserWindow::addModelToDirectoryHierarchy()
{
    if (!mModelImportWorkflow.IsSucceed())
    {
        return;
    }

    const ModelImportWorkflow::ModelParseContext& parseCtx = mModelImportWorkflow.GetParseContext();
    const ModelImportWorkflow::ModelImportContext& importCtx = mModelImportWorkflow.GetImportContext();

    DirectoryHierarchy::Directory* pModelDir =
        mDirectoryHierarchy.AddDirectory(parseCtx.pParsedModel->NameStr, mDirectoryHierarchy.GetRoot());
    if (importCtx.hImportedStaticMesh.IsValid())
    {
        mDirectoryHierarchy.AddAsset(importCtx.hImportedStaticMesh, pModelDir);
    }

    for (const auto& hMaterial : importCtx.hImportedMaterials)
    {
        if (hMaterial.IsValid())
        {
            mDirectoryHierarchy.AddAsset(hMaterial, pModelDir);
        }
    }

    for (const auto& hTexture : importCtx.hImportedTextures)
    {
        if (hTexture.IsValid())
        {
            mDirectoryHierarchy.AddAsset(hTexture, pModelDir);
        }
    }

    if (importCtx.hImportedMorphTargets.IsValid())
    {
        mDirectoryHierarchy.AddAsset(importCtx.hImportedMorphTargets, pModelDir);
    }

    for (const auto& hMorphAnim : importCtx.hImportedMorphAnims)
    {
        if (hMorphAnim.IsValid())
        {
            mDirectoryHierarchy.AddAsset(hMorphAnim, pModelDir);
        }
    }
}

void EditorAssetBrowserWindow::addTextureToDirectoryHierarchy()
{
    if (!mTextureImportWorkflow.IsSucceed())
    {
        return;
    }

    DirectoryHierarchy::Directory* pTargetDir = mpOpenedDir ? mpOpenedDir : mDirectoryHierarchy.GetRoot();

    const TextureImportWorkflow::TextureImportContext& importCtx = mTextureImportWorkflow.GetImportContext();
    if (importCtx.hImportedTexture.IsValid())
    {
        mDirectoryHierarchy.AddAsset(importCtx.hImportedTexture, pTargetDir);
    }
}

void EditorAssetBrowserWindow::addShaderToDirectoryHierarchy()
{
    if (!mShaderImportWorkflow.IsSucceed())
    {
        return;
    }

    DirectoryHierarchy::Directory* pTargetDir = mpOpenedDir ? mpOpenedDir : mDirectoryHierarchy.GetRoot();

    const ShaderImportWorkflow::ShaderImportContext& importCtx = mShaderImportWorkflow.GetImportContext();
    if (importCtx.hImportedShader.IsValid())
    {
        mDirectoryHierarchy.AddAsset(importCtx.hImportedShader, pTargetDir);
    }
}

void EditorAssetBrowserWindow::addBakedTextureToDirectoryHierarchy()
{
    if (!mTextureBakeWorkflow.IsSucceed())
    {
        return;
    }

    DirectoryHierarchy::Directory* pTargetDir = mpOpenedDir ? mpOpenedDir : mDirectoryHierarchy.GetRoot();

    if (mTextureBakeWorkflow.GetBakeContext().hDstTexture.IsValid())
    {
        mDirectoryHierarchy.AddAsset(mTextureBakeWorkflow.GetBakeContext().hDstTexture, pTargetDir);
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

    if (mbOpenDirectoryPropertiesPopup)
    {
        ImGui::SetNextWindowSize(ImVec2(600.0f, 0.0f), ImGuiCond_Appearing);

        ImGui::OpenPopup(sPopupNameStr.c_str());
        mbOpenDirectoryPropertiesPopup = false;
    }

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

    if (mbOpenItemPropertiesPopup)
    {
        ImGui::SetNextWindowSize(ImVec2(600.0f, 0.0f), ImGuiCond_Appearing);
        ImGui::OpenPopup(sPopupNameStr.c_str());
        mbOpenItemPropertiesPopup = false;
    }

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
} // namespace ho