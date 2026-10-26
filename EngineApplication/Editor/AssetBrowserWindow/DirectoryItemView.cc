#include "DirectoryItemView.h"

#include <IconsMaterialDesignIcons.h>
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <imgui_internal.h>
#include <string>

#include "../Utils/EditorDisplayUtils.h"

namespace ho
{
void drawByteSize(int64_t byteSize)
{
    constexpr double kKB = 1024.0;
    constexpr double kMB = kKB * 1024.0;
    constexpr double kGB = kMB * 1024.0;

    if (byteSize < 0)
    {
        ImGui::Text("N/A");
        return;
    }

    const double bytes = static_cast<double>(byteSize);
    if (bytes >= kGB)
    {
        ImGui::Text("%.2f GB", bytes / kGB);
    }
    else if (bytes >= kMB)
    {
        ImGui::Text("%.2f MB", bytes / kMB);
    }
    else
    {
        ImGui::Text("%.2f KB", bytes / kKB);
    }
}

void drawColorStripe(const Color128& color)
{
    const ImVec2 pos = ImGui::GetCursorScreenPos();
    const float textHeight = ImGui::GetTextLineHeight();
    constexpr float kStripeWidth = 3.0f;

    ImGui::GetWindowDrawList()->AddRectFilled(
        pos,
        ImVec2(pos.x + kStripeWidth, pos.y + textHeight),
        ImGui::ColorConvertFloat4ToU32(ImVec4(color.R, color.G, color.B, color.A)));

    ImGui::SetCursorScreenPos(ImVec2(pos.x + kStripeWidth + 2.0f, pos.y));
}

void drawDirectoryColorStripe()
{
    const ImVec2 pos = ImGui::GetCursorScreenPos();
    const float textHeight = ImGui::GetTextLineHeight();
    constexpr float kStripeWidth = 3.0f;

    ImGui::GetWindowDrawList()->AddRectFilled(
        pos, ImVec2(pos.x + kStripeWidth, pos.y + textHeight), IM_COL32(214, 158, 46, 255));

    ImGui::SetCursorScreenPos(ImVec2(pos.x + kStripeWidth + 2.0f, pos.y));
}

void DirectoryItemView::UpdateOpenedDirectoryView(DirectoryHierarchy::Directory* pOutOpenedDir,
                                                  DirectoryHierarchy::Directory** pOutSelectedDir,
                                                  DirectoryHierarchy::DirectoryItem** pOutSelectedItem)
{
    if (mCurrentOpenedDirID == pOutOpenedDir->ID && !pOutOpenedDir->bIsDirty)
    {
        return;
    }

    if (mCurrentOpenedDirID != pOutOpenedDir->ID)
    {
        *pOutSelectedDir = nullptr;
        *pOutSelectedItem = nullptr;
    }

    mCurrentOpenedDirID = pOutOpenedDir->ID;
    mpChildren.clear();
    mpItems.clear();
    mpChildren.reserve(pOutOpenedDir->pChildren.size());
    mpItems.reserve(pOutOpenedDir->Items.size());

    for (auto& pChild : pOutOpenedDir->pChildren)
    {
        mpChildren.push_back(pChild.get());
    }

    for (auto& item : pOutOpenedDir->Items)
    {
        mpItems.push_back(&item);
    }

    pOutOpenedDir->bIsDirty = false;
}

void DirectoryItemView::Draw(const DirectoryHierarchy::Directory* pSelectedDir,
                             const DirectoryHierarchy::DirectoryItem* pSelectedItem)
{
    clearRequest();

    static constexpr ImGuiTableFlags sTableFlags =
        ImGuiTableFlags_Resizable | ImGuiTableFlags_Sortable | ImGuiTableFlags_NoSavedSettings |
        ImGuiTableFlags_BordersOuter | ImGuiTableFlags_NoBordersInBody | ImGuiTableFlags_PadOuterX |
        ImGuiTableFlags_ScrollY | ImGuiTableFlags_SortMulti | ImGuiTableFlags_SortTristate;

    if (ImGui::BeginTable("##ItemTable", 4, sTableFlags, ImVec2(0.0f, 0.0f), 0.0f))
    {
        setupTable();
        processSortSpecs();
        drawRows(pSelectedDir, pSelectedItem);
        ImGui::EndTable();
    }

    drawRightClickPopup();
}

void DirectoryItemView::setupTable()
{
    ImGui::TableSetupColumn("Name",
                            ImGuiTableColumnFlags_DefaultSort | ImGuiTableColumnFlags_WidthStretch |
                                ImGuiTableColumnFlags_NoReorder,
                            0.0f,
                            0);
    ImGui::TableSetupColumn("ID", ImGuiTableColumnFlags_WidthStretch | ImGuiTableColumnFlags_NoReorder, 0.0f, 1);
    ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthStretch | ImGuiTableColumnFlags_NoReorder, 0.0f, 2);
    ImGui::TableSetupColumn("Size", ImGuiTableColumnFlags_WidthStretch | ImGuiTableColumnFlags_NoReorder, 0.0f, 3);
    ImGui::TableSetupScrollFreeze(0, 1);
    ImGui::TableHeadersRow();
}

void DirectoryItemView::processSortSpecs()
{
    ImGuiTableSortSpecs* pSortSpecs = ImGui::TableGetSortSpecs();
    if (pSortSpecs != nullptr && pSortSpecs->SpecsDirty)
    {
        sortItemsWithSortSpecs(pSortSpecs);
        pSortSpecs->SpecsDirty = false;
    }
}

void DirectoryItemView::drawRows(const DirectoryHierarchy::Directory* pSelectedDir,
                                 const DirectoryHierarchy::DirectoryItem* pSelectedItem)
{
    const int32_t numChildren = static_cast<int32_t>(mpChildren.size());
    const int32_t numItems = static_cast<int32_t>(mpItems.size());

    ImGuiListClipper clipper;
    clipper.Begin(numChildren + numItems);

    while (clipper.Step())
    {
        for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; ++row)
        {
            ImGui::TableNextRow();

            if (row < numChildren)
            {
                drawDirectoryRow(mpChildren[row], pSelectedDir);
            }
            else
            {
                drawItemRow(mpItems[row - numChildren], pSelectedItem);
            }
        }
    }
}

void DirectoryItemView::drawDirectoryRow(DirectoryHierarchy::Directory* pDir,
                                         const DirectoryHierarchy::Directory* pSelectedDir)
{
    ImGui::PushID(static_cast<int>(pDir->ID));

    ImGui::TableNextColumn();
    const bool bSelected = pDir == pSelectedDir;

    if (ImGui::Selectable("##SelectableDir", bSelected, ImGuiSelectableFlags_SpanAllColumns))
    {
        sendRequest(eRequestType::SelectDirectory, pDir, nullptr, nullptr);
    }
    handleDirectoryInteraction(pDir);
    handleDirectoryDragDrop(pDir);

    ImGui::SameLine();
    drawDirectoryColorStripe();
    drawDirectoryName(pDir);

    ImGui::TableNextColumn();
    ImGui::Text("%d", pDir->ID);

    ImGui::TableNextColumn();
    ImGui::TextUnformatted("Directory");

    ImGui::TableNextColumn();
    drawByteSize(pDir->ByteSize);

    ImGui::PopID();
}

void DirectoryItemView::drawItemRow(DirectoryHierarchy::DirectoryItem* pItem,
                                    const DirectoryHierarchy::DirectoryItem* pSelectedItem)
{
    ImGui::PushID(static_cast<int>(pItem->ID));

    ImGui::TableNextColumn();
    const bool bSelected = pItem == pSelectedItem;

    if (ImGui::Selectable("##SelectableItem", bSelected, ImGuiSelectableFlags_SpanAllColumns))
    {
        sendRequest(eRequestType::SelectItem, nullptr, pItem, nullptr);
    }
    handleItemInteraction(pItem);
    handleItemDragSource(pItem);

    ImGui::SameLine();
    drawColorStripe(getAssetDisplayColor(pItem->hAsset));
    drawItemName(pItem);

    ImGui::TableNextColumn();
    ImGui::Text("%d", pItem->ID);

    ImGui::TableNextColumn();
    ImGui::Text("%s", getAssetTypeDisplayName(pItem->hAsset));

    ImGui::TableNextColumn();
    drawByteSize(pItem->ByteSize);

    ImGui::PopID();
}

void DirectoryItemView::handleDirectoryInteraction(DirectoryHierarchy::Directory* pDir)
{
    if (!ImGui::IsItemHovered())
    {
        return;
    }

    if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
    {
        sendRequest(eRequestType::OpenDirectory, pDir, nullptr, nullptr);
    }

    if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
    {
        sendRequest(eRequestType::SelectDirectory, pDir, nullptr, nullptr);
        mContextTarget = eContextTarget::Directory;
        mpContextDir = pDir;
        mpContextItem = nullptr;
        mbOpenRightClickPopup = true;
    }
}

void DirectoryItemView::handleItemInteraction(DirectoryHierarchy::DirectoryItem* pItem)
{
    if (!ImGui::IsItemHovered())
    {
        return;
    }

    if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
    {
        sendRequest(eRequestType::OpenItem, nullptr, pItem, nullptr);
    }

    if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
    {
        sendRequest(eRequestType::SelectItem, nullptr, pItem, nullptr);
        mContextTarget = eContextTarget::Item;
        mpContextDir = nullptr;
        mpContextItem = pItem;
        mbOpenRightClickPopup = true;
    }
}

void DirectoryItemView::handleDirectoryDragDrop(DirectoryHierarchy::Directory* pDir)
{
    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
    {
        const DirectoryHierarchy::Directory* pDraggedDir = pDir;
        ImGui::SetDragDropPayload(
            "DND_DIRECTORY", static_cast<const void*>(&pDraggedDir), sizeof(DirectoryHierarchy::Directory*));
        ImGui::PushFont(nullptr, 30.0f);
        ImGui::Text("%s", ICON_MDI_FOLDER_OPEN);
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
        DirectoryHierarchy::Directory* pDraggedDir =
            *reinterpret_cast<DirectoryHierarchy::Directory* const*>(payload->Data);
        sendRequest(eRequestType::MoveDirectory, pDraggedDir, nullptr, pDir);
    }
    else if (ImGui::AcceptDragDropPayload("DND_ITEM"))
    {
        payload = ImGui::AcceptDragDropPayload("DND_ITEM");
        DirectoryHierarchy::DirectoryItem* pDraggedItem =
            *reinterpret_cast<DirectoryHierarchy::DirectoryItem* const*>(payload->Data);
        sendRequest(eRequestType::MoveItem, nullptr, pDraggedItem, pDir);
    }

    ImGui::EndDragDropTarget();
}

void DirectoryItemView::handleItemDragSource(DirectoryHierarchy::DirectoryItem* pItem)
{
    if (!ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
    {
        return;
    }

    const DirectoryHierarchy::DirectoryItem* pDraggedItem = pItem;
    ImGui::SetDragDropPayload(
        "DND_ITEM", static_cast<const void*>(&pDraggedItem), sizeof(DirectoryHierarchy::DirectoryItem*));
    ImGui::PushFont(nullptr, 30.0f);
    ImGui::Text("%s", getAssetDisplayIcon(pDraggedItem->hAsset));
    ImGui::PopFont();
    ImGui::EndDragDropSource();
}

void DirectoryItemView::drawDirectoryName(DirectoryHierarchy::Directory* pDir)
{
    if (mpRenamingDir != pDir)
    {
        ImGui::Text("%s  %s", ICON_MDI_FOLDER, pDir->NameStr.c_str());
        return;
    }

    if (mpLastRenamingTarget != pDir)
    {
        mpLastRenamingTarget = pDir;
        strncpy_s(mRenameBuffer, pDir->NameStr.c_str(), sizeof(mRenameBuffer) - 1);
        ImGui::SetKeyboardFocusHere();
    }

    ImGui::TextUnformatted(ICON_MDI_FOLDER);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
    ImGui::InputText("##DirRenameInput", mRenameBuffer, sizeof(mRenameBuffer));

    if (ImGui::IsItemDeactivated())
    {
        if (std::strlen(mRenameBuffer) > 0)
        {
            pDir->NameStr = mRenameBuffer;
            pDir->bIsDirty = true;
        }

        mpRenamingDir->bIsDirty = true;
        mpRenamingDir = nullptr;
        mpLastRenamingTarget = nullptr;
    }
}

void DirectoryItemView::drawItemName(DirectoryHierarchy::DirectoryItem* pItem)
{
    if (mpRenamingItem != pItem)
    {
        ImGui::Text("%s  %s", getAssetDisplayIcon(pItem->hAsset), pItem->NameStr.c_str());
        return;
    }

    if (mpLastRenamingTarget != pItem)
    {
        mpLastRenamingTarget = pItem;
        strncpy_s(mRenameBuffer, pItem->NameStr.c_str(), sizeof(mRenameBuffer) - 1);
        ImGui::SetKeyboardFocusHere();
    }

    ImGui::Text("%s", getAssetDisplayIcon(pItem->hAsset));
    ImGui::SameLine();
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
    ImGui::InputText("##ItemRenameInput", mRenameBuffer, sizeof(mRenameBuffer));

    if (ImGui::IsItemDeactivated())
    {
        if (std::strlen(mRenameBuffer) > 0)
        {
            pItem->NameStr = mRenameBuffer;
        }

        mpRenamingItem->bIsDirty = true;
        mpRenamingItem = nullptr;
        mpLastRenamingTarget = nullptr;
    }
}

void DirectoryItemView::drawRightClickPopup()
{
    if (mbOpenRightClickPopup)
    {
        ImGui::SetNextWindowPos(ImGui::GetMousePos(), ImGuiCond_Appearing);
        ImGui::SetNextWindowSize(ImVec2(140.0f, 0.0f), ImGuiCond_Appearing);
        ImGui::OpenPopup("###DirectoryItemViewRightClickPopup");
        mbOpenRightClickPopup = false;
    }

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4.0f, 8.0f));
    if (ImGui::BeginPopup("###DirectoryItemViewRightClickPopup"))
    {
        switch (mContextTarget)
        {
            case eContextTarget::Directory:
                drawDirectoryContextItems();
                break;
            case eContextTarget::Item:
                drawItemContextItems();
                break;
            default:
                break;
        }

        ImGui::EndPopup();
    }
    else
    {
        mContextTarget = eContextTarget::None;
        mpContextDir = nullptr;
        mpContextItem = nullptr;
    }
    ImGui::PopStyleVar();
}

void DirectoryItemView::drawDirectoryContextItems()
{
    DirectoryHierarchy::Directory* pDir = mpContextDir;
    if (pDir == nullptr)
    {
        return;
    }

    if (ImGui::MenuItem(ICON_MDI_FOLDER_OPEN "  Open", "O") || ImGui::IsKeyPressed(ImGuiKey_O))
    {
        sendRequest(eRequestType::OpenDirectory, pDir, nullptr, nullptr);
        ImGui::CloseCurrentPopup();
    }

    if (ImGui::MenuItem(ICON_MDI_DELETE "  Delete", "D") || ImGui::IsKeyPressed(ImGuiKey_D))
    {
        sendRequest(eRequestType::DeleteDirectory, pDir, nullptr, nullptr);
        ImGui::CloseCurrentPopup();
    }

    if (ImGui::MenuItem(ICON_MDI_FORM_TEXTBOX "  Rename", "M") || ImGui::IsKeyPressed(ImGuiKey_M))
    {
        mpRenamingDir = pDir;
        mpRenamingItem = nullptr;
        ImGui::CloseCurrentPopup();
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    if (ImGui::MenuItem(ICON_MDI_INFORMATION_SLAB_CIRCLE "  Properties", "R") || ImGui::IsKeyPressed(ImGuiKey_R))
    {
        sendRequest(eRequestType::OpenDirectoryPropertiesPopup, pDir, nullptr, nullptr);
        ImGui::CloseCurrentPopup();
    }
}

void DirectoryItemView::drawItemContextItems()
{
    DirectoryHierarchy::DirectoryItem* pItem = mpContextItem;
    if (pItem == nullptr)
    {
        return;
    }

    const std::string openLabel = std::string(getAssetDisplayIcon(pItem->hAsset)).append("  Open");
    if (ImGui::MenuItem(openLabel.c_str(), "O") || ImGui::IsKeyPressed(ImGuiKey_O))
    {
        sendRequest(eRequestType::OpenItem, nullptr, pItem, nullptr);
        ImGui::CloseCurrentPopup();
    }

    if (ImGui::MenuItem(ICON_MDI_DELETE "  Delete", "D") || ImGui::IsKeyPressed(ImGuiKey_D))
    {
        sendRequest(eRequestType::DeleteItem, nullptr, pItem, nullptr);
        ImGui::CloseCurrentPopup();
    }

    if (ImGui::MenuItem(ICON_MDI_FORM_TEXTBOX "  Rename", "M") || ImGui::IsKeyPressed(ImGuiKey_M))
    {
        mpRenamingDir = nullptr;
        mpRenamingItem = pItem;
        ImGui::CloseCurrentPopup();
    }

    if (pItem->hAsset->GetClass() == ReflectionSystem::GetInstance().GetClassByType<RTextureAsset>() &&
        pItem->hAsset.Cast<RTextureAsset>()->Type == eTextureType::TextureCubeMap)
    {
        if (ImGui::MenuItem(ICON_MDI_POT_MIX "  Bake"))
        {
            sendRequest(eRequestType::BakeTexture, nullptr, pItem, nullptr);
            ImGui::CloseCurrentPopup();
        }
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    if (ImGui::MenuItem(ICON_MDI_INFORMATION_SLAB_CIRCLE "  Properties", "R") || ImGui::IsKeyPressed(ImGuiKey_R))
    {
        sendRequest(eRequestType::OpenItemPropertiesPopup, nullptr, pItem, nullptr);
        ImGui::CloseCurrentPopup();
    }
}

void DirectoryItemView::sortItemsWithSortSpecs(ImGuiTableSortSpecs* pSortSpecs)
{
    if (pSortSpecs == nullptr || pSortSpecs->SpecsCount == 0)
    {
        return;
    }

    std::sort(mpChildren.begin(),
              mpChildren.end(),
              [pSortSpecs](const DirectoryHierarchy::Directory* a, const DirectoryHierarchy::Directory* b)
              {
                  for (int n = 0; n < pSortSpecs->SpecsCount; ++n)
                  {
                      const ImGuiTableColumnSortSpecs& spec = pSortSpecs->Specs[n];
                      if (spec.SortDirection == ImGuiSortDirection_None)
                      {
                          continue;
                      }

                      int delta = 0;
                      bool bCompareSuccess = false;
                      switch (spec.ColumnIndex)
                      {
                          case 0:
                              if (a->NameStr != b->NameStr)
                              {
                                  delta = (a->NameStr < b->NameStr) ? -1 : 1;
                                  bCompareSuccess = true;
                              }
                              break;
                          case 1:
                              if (a->ID != b->ID)
                              {
                                  delta = (a->ID < b->ID) ? -1 : 1;
                                  bCompareSuccess = true;
                              }
                              break;
                          case 3:
                              if (a->ByteSize != b->ByteSize)
                              {
                                  delta = (a->ByteSize < b->ByteSize) ? -1 : 1;
                                  bCompareSuccess = true;
                              }
                              break;
                          default:
                              HO_ASSERT(false, "Invalid index.");
                              break;
                      }

                      if (bCompareSuccess)
                      {
                          return spec.SortDirection == ImGuiSortDirection_Ascending ? delta < 0 : delta > 0;
                      }
                  }

                  return a->ID < b->ID;
              });

    std::sort(mpItems.begin(),
              mpItems.end(),
              [pSortSpecs](const DirectoryHierarchy::DirectoryItem* a, const DirectoryHierarchy::DirectoryItem* b)
              {
                  for (int n = 0; n < pSortSpecs->SpecsCount; ++n)
                  {
                      const ImGuiTableColumnSortSpecs& spec = pSortSpecs->Specs[n];
                      if (spec.SortDirection == ImGuiSortDirection_None)
                      {
                          continue;
                      }

                      int delta = 0;
                      bool bCompareSuccess = false;
                      switch (spec.ColumnIndex)
                      {
                          case 0:
                              if (a->NameStr != b->NameStr)
                              {
                                  delta = (a->NameStr < b->NameStr) ? -1 : 1;
                                  bCompareSuccess = true;
                              }
                              break;
                          case 1:
                              if (a->ID != b->ID)
                              {
                                  delta = (a->ID < b->ID) ? -1 : 1;
                                  bCompareSuccess = true;
                              }
                              break;
                          case 2:
                          {
                              const int32_t aPriority = getAssetDisplayPriority(a->hAsset);
                              const int32_t bPriority = getAssetDisplayPriority(b->hAsset);
                              if (aPriority != bPriority)
                              {
                                  delta = (aPriority < bPriority) ? -1 : 1;
                                  bCompareSuccess = true;
                              }
                              break;
                          }
                          case 3:
                              if (a->ByteSize != b->ByteSize)
                              {
                                  delta = (a->ByteSize < b->ByteSize) ? -1 : 1;
                                  bCompareSuccess = true;
                              }
                              break;
                          default:
                              HO_ASSERT(false, "Invalid index.");
                              break;
                      }

                      if (bCompareSuccess)
                      {
                          return spec.SortDirection == ImGuiSortDirection_Ascending ? delta < 0 : delta > 0;
                      }
                  }

                  return a->ID < b->ID;
              });
}

} // namespace ho
