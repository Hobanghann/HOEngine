#include "DirectoryHierarchyView.h"

#include <IconsMaterialDesignIcons.h>
#include <imgui_internal.h>

namespace ho
{
void DirectoryHierarchyView::Draw(DirectoryHierarchy* pOutDirHierarchy, DirectoryHierarchy::Directory* pOutOpenedDir)
{
    HO_ASSERT(pOutDirHierarchy, "Invalid directory.");
    HO_ASSERT(pOutOpenedDir, "Invalid opened directory.");

    clearRequest();

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, ImGui::GetStyle().ItemSpacing.y));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 0.0f);

    if (ImGui::BeginChild(
            "##DirectoryHierarchyView", ImVec2(300.0f, 0.0f), ImGuiChildFlags_ResizeX | ImGuiChildFlags_NavFlattened))
    {
        drawTopBorder();
        drawHeader();
        drawDirectoryFilter(pOutDirHierarchy);
        drawDirectoryTree(*pOutDirHierarchy, *pOutOpenedDir);
        drawDirectoryRightClickPopup();
    }

    ImGui::EndChild();
    ImGui::PopStyleVar(4);
}

void DirectoryHierarchyView::drawTopBorder()
{
    const ImVec2 windowPos = ImGui::GetWindowPos();
    const float windowWidth = ImGui::GetWindowWidth();

    const ImVec2 startPos(windowPos.x, windowPos.y);
    const ImVec2 endPos(windowPos.x + windowWidth, windowPos.y);
    const ImU32 borderColor = ImGui::GetColorU32(ImGuiCol_Border);

    ImGui::GetWindowDrawList()->AddLine(startPos, endPos, borderColor, 1.0f);
}

void DirectoryHierarchyView::drawHeader()
{
    static constexpr float sItemGap = 8.0f;

    ImGui::PushItemFlag(ImGuiItemFlags_NoNavDefaultFocus, true);

    ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + sItemGap, ImGui::GetCursorPosY() + sItemGap));

    ImGui::Text("PROJECT NAME");
    ImGui::Spacing();

    ImGui::PopItemFlag();
}

void DirectoryHierarchyView::drawDirectoryFilter(DirectoryHierarchy* pOutDirHierarchy)
{
    HO_ASSERT(pOutDirHierarchy, "Invalid directory.");

    static constexpr float sItemGap = 8.0f;

    ImGui::PushItemFlag(ImGuiItemFlags_NoNavDefaultFocus, true);

    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + sItemGap);
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - sItemGap * 2.0f);

    if (ImGui::InputTextWithHint("##DirectoryFilter",
                                 ICON_MDI_MAGNIFY "    Search Directories",
                                 mDirectoryFilter.InputBuf,
                                 IM_COUNTOF(mDirectoryFilter.InputBuf),
                                 ImGuiInputTextFlags_EscapeClearsAll))
    {
        mDirectoryFilter.Build();

        updateDirectoryVisibilityRecursive(pOutDirHierarchy, pOutDirHierarchy->GetRoot());
    }

    ImGui::PopItemFlag();
}

void DirectoryHierarchyView::drawDirectoryTree(DirectoryHierarchy& directoryHierarchy,
                                               const DirectoryHierarchy::Directory& openedDir)
{
    static constexpr float sItemGap = 8.0f;

    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + sItemGap);

    const float treeWidth = ImGui::GetContentRegionAvail().x - sItemGap;

    if (ImGui::BeginChild("##DirectoryTreeRegion",
                          ImVec2(treeWidth, 0.0f),
                          ImGuiChildFlags_None,
                          ImGuiWindowFlags_HorizontalScrollbar))
    {
        if (ImGui::BeginTable("##DirectoryTree", 1))
        {
            drawDirectoryTreeRecursive(directoryHierarchy.GetRoot(), openedDir);
            ImGui::EndTable();
        }
    }

    ImGui::EndChild();
}

void DirectoryHierarchyView::drawDirectoryTreeRecursive(DirectoryHierarchy::Directory* pOutDrawDir,
                                                        const DirectoryHierarchy::Directory& openedDir)
{
    if (!pOutDrawDir || !pOutDrawDir->bIsVisible)
    {
        return;
    }

    ImGui::TableNextRow();
    ImGui::TableNextColumn();

    drawDirectoryTreeNode(pOutDrawDir, openedDir);
    handleDirectoryDragDrop(pOutDrawDir);
    handleDirectoryNodeInteraction(pOutDrawDir);

    if (pOutDrawDir->bIsExpanded)
    {
        for (const auto& pChild : pOutDrawDir->pChildren)
        {
            drawDirectoryTreeRecursive(pChild.get(), openedDir);
        }

        ImGui::TreePop();
    }
}

void DirectoryHierarchyView::drawDirectoryTreeNode(DirectoryHierarchy::Directory* pOutDrawDir,
                                                   const DirectoryHierarchy::Directory& openedDir)
{
    ImGuiTreeNodeFlags flags =
        ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanFullWidth;

    if (pOutDrawDir->pChildren.empty())
    {
        flags |= ImGuiTreeNodeFlags_Leaf;
    }

    const bool bSelected = openedDir.ID == pOutDrawDir->ID;

    if (bSelected)
    {
        flags |= ImGuiTreeNodeFlags_Selected;
    }

    if (mDirectoryFilter.IsActive())
    {
        flags |= ImGuiTreeNodeFlags_DefaultOpen;
    }

    const char* dirIcon = bSelected ? ICON_MDI_FOLDER_OPEN : ICON_MDI_FOLDER;

    if (pOutDrawDir->bIsExpanded)
    {
        ImGui::SetNextItemOpen(true);
    }

    pOutDrawDir->bIsExpanded = ImGui::TreeNodeEx(pOutDrawDir, flags, "%s %s", dirIcon, pOutDrawDir->NameStr.c_str());
}

void DirectoryHierarchyView::handleDirectoryDragDrop(DirectoryHierarchy::Directory* pOutDrawDir)
{
    // ID 0 means root directory.
    if (pOutDrawDir->ID > 0 && ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
    {
        const DirectoryHierarchy::Directory* pDraggedDir = pOutDrawDir;

        ImGui::SetDragDropPayload("DND_DIRECTORY", &pDraggedDir, sizeof(DirectoryHierarchy::Directory*));

        ImGui::PushFont(nullptr, 30.0f);
        ImGui::Text(ICON_MDI_FOLDER_OPEN);
        ImGui::PopFont();

        ImGui::EndDragDropSource();
    }

    if (!ImGui::BeginDragDropTarget())
    {
        return;
    }

    const ImGuiPayload* payload = nullptr;
    if (ImGui::AcceptDragDropPayload("DND_DIRECTORY"))
    {
        payload = ImGui::AcceptDragDropPayload("DND_DIRECTORY");
        DirectoryHierarchy::Directory* pDraggedDir = *static_cast<DirectoryHierarchy::Directory**>(payload->Data);

        sendRequest(eRequestType::MoveDirectory, pDraggedDir, nullptr, pOutDrawDir);
    }
    else if (ImGui::AcceptDragDropPayload("DND_ITEM"))
    {
        payload = ImGui::AcceptDragDropPayload("DND_ITEM");
        DirectoryHierarchy::DirectoryItem* pDraggedItem =
            *static_cast<DirectoryHierarchy::DirectoryItem**>(payload->Data);

        sendRequest(eRequestType::MoveItem, nullptr, pDraggedItem, pOutDrawDir);
    }

    ImGui::EndDragDropTarget();
}

void DirectoryHierarchyView::handleDirectoryNodeInteraction(DirectoryHierarchy::Directory* pOutDrawDir)
{
    if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
    {
        sendRequest(eRequestType::OpenDirectory, pOutDrawDir, nullptr, nullptr);
    }

    if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
    {
        sendRequest(eRequestType::SelectDirectory, pOutDrawDir, nullptr, nullptr);

        mpContextMenuDirectory = pOutDrawDir;
        mbOpenDirectoryRightClickPopup = true;
    }
}

void DirectoryHierarchyView::drawDirectoryRightClickPopup()
{
    if (!mpContextMenuDirectory)
    {
        return;
    }

    if (mbOpenDirectoryRightClickPopup)
    {
        ImGui::SetNextWindowPos(ImGui::GetMousePos(), ImGuiCond_Appearing);
        ImGui::SetNextWindowSize(ImVec2(140.0f, 0.0f), ImGuiCond_Appearing);
        ImGui::OpenPopup("###DirectoryHierarchyViewRightClickPopup");
        mbOpenDirectoryRightClickPopup = false;
    }

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4.0f, 8.0f));

    if (ImGui::BeginPopup("###DirectoryHierarchyViewRightClickPopup"))
    {
        DirectoryHierarchy::Directory* pTargetDir = mpContextMenuDirectory;

        if (ImGui::MenuItem(ICON_MDI_FOLDER_OPEN "   Open", "O") || ImGui::IsKeyPressed(ImGuiKey_O))
        {
            sendRequest(eRequestType::OpenDirectory, pTargetDir, nullptr, nullptr);
            mpContextMenuDirectory = nullptr;
            ImGui::CloseCurrentPopup();
        }

        const bool bIsRootDirectory = pTargetDir->ID == 0;

        if (bIsRootDirectory)
        {
            ImGui::BeginDisabled();
        }

        if (ImGui::MenuItem(ICON_MDI_DELETE "   Delete", "D") || (!bIsRootDirectory && ImGui::IsKeyPressed(ImGuiKey_D)))
        {
            sendRequest(eRequestType::DeleteDirectory, pTargetDir, nullptr, nullptr);
            mpContextMenuDirectory = nullptr;
            ImGui::CloseCurrentPopup();
        }

        if (bIsRootDirectory)
        {
            ImGui::EndDisabled();
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (ImGui::MenuItem(ICON_MDI_INFORMATION_SLAB_CIRCLE "   Properties", "R") || ImGui::IsKeyPressed(ImGuiKey_R))
        {
            sendRequest(eRequestType::OpenDirectoryPropertiesPopup, pTargetDir, nullptr, nullptr);
            mpContextMenuDirectory = nullptr;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    ImGui::PopStyleVar();
}

void DirectoryHierarchyView::updateDirectoryVisibilityRecursive(DirectoryHierarchy* pOutDirHierarchy,
                                                                DirectoryHierarchy::Directory* pOutDir)
{
    HO_ASSERT(pOutDirHierarchy && pOutDir, "Invalid directory.");

    if (!mDirectoryFilter.IsActive())
    {
        pOutDir->bIsVisible = true;

        for (auto& pChild : pOutDir->pChildren)
        {
            updateDirectoryVisibilityRecursive(pOutDirHierarchy, pChild.get());
        }

        return;
    }

    pOutDir->bIsVisible = false;

    if (mDirectoryFilter.PassFilter(pOutDir->NameStr.c_str()))
    {
        pOutDir->bIsVisible = true;

        pOutDirHierarchy->SetAncestorVisible(*pOutDir, true);
        pOutDirHierarchy->SetAncestorExpanded(*pOutDir, true);
    }

    for (auto& pChild : pOutDir->pChildren)
    {
        updateDirectoryVisibilityRecursive(pOutDirHierarchy, pChild.get());
    }
}

} // namespace ho
