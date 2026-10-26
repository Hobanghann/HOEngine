#pragma once

#include <imgui.h>
#include <vector>

#include "../Utils/DirectoryHierarchy.h"
#include "Core/Math/MathFuncs.h"

namespace ho
{
class DirectoryItemView final
{
  public:
    enum class eRequestType
    {
        None,
        SelectDirectory,
        OpenDirectory,
        MoveDirectory,
        DeleteDirectory,
        OpenDirectoryPropertiesPopup,

        SelectItem,
        OpenItem,
        MoveItem,
        DeleteItem,
        OpenItemPropertiesPopup,

        BakeTexture,
    };

    struct Request
    {
        eRequestType Type = eRequestType::None;
        DirectoryHierarchy::Directory* pTargetDir = nullptr;
        DirectoryHierarchy::DirectoryItem* pTargetItem = nullptr;
        DirectoryHierarchy::Directory* pDstDir = nullptr;
    };

    void UpdateOpenedDirectoryView(DirectoryHierarchy::Directory* pOutOpenedDir,
                                   DirectoryHierarchy::Directory** pOutSelectedDir,
                                   DirectoryHierarchy::DirectoryItem** pOutSelectedItem);

    void Draw(const DirectoryHierarchy::Directory* pSelectedDir,
              const DirectoryHierarchy::DirectoryItem* pSelectedItem);

    bool HasPendingRequest() const
    {
        return mPendingRequest.Type != eRequestType::None;
    }

    const Request& GetPendingRequest() const
    {
        return mPendingRequest;
    }

    void SetRequestFinished(const Request& completedRequest) {}

  private:
    enum class eContextTarget
    {
        None,
        Directory,
        Item,
    };

    void setupTable();
    void processSortSpecs();
    void drawRows(const DirectoryHierarchy::Directory* pSelectedDir,
                  const DirectoryHierarchy::DirectoryItem* pSelectedItem);
    void drawDirectoryRow(DirectoryHierarchy::Directory* pDir, const DirectoryHierarchy::Directory* pSelectedDir);
    void drawItemRow(DirectoryHierarchy::DirectoryItem* pItem, const DirectoryHierarchy::DirectoryItem* pSelectedItem);

    void handleDirectoryInteraction(DirectoryHierarchy::Directory* pDir);
    void handleItemInteraction(DirectoryHierarchy::DirectoryItem* pItem);
    void handleDirectoryDragDrop(DirectoryHierarchy::Directory* pDir);
    void handleItemDragSource(DirectoryHierarchy::DirectoryItem* pItem);

    void drawDirectoryName(DirectoryHierarchy::Directory* pDir);
    void drawItemName(DirectoryHierarchy::DirectoryItem* pItem);

    bool mbOpenRightClickPopup = false;
    void drawRightClickPopup();
    void drawDirectoryContextItems();
    void drawItemContextItems();

    void sortItemsWithSortSpecs(ImGuiTableSortSpecs* pSortSpecs);

    void sendRequest(eRequestType type,
                     DirectoryHierarchy::Directory* pTargetDir,
                     DirectoryHierarchy::DirectoryItem* pTargetItem,
                     DirectoryHierarchy::Directory* pDstDir)
    {
        mPendingRequest.Type = type;
        mPendingRequest.pTargetDir = pTargetDir;
        mPendingRequest.pTargetItem = pTargetItem;
        mPendingRequest.pDstDir = pDstDir;
    }

    void clearRequest()
    {
        mPendingRequest = Request();
    }

    uint32_t mCurrentOpenedDirID = 0;
    std::vector<DirectoryHierarchy::Directory*> mpChildren;
    std::vector<DirectoryHierarchy::DirectoryItem*> mpItems;

    DirectoryHierarchy::Directory* mpRenamingDir = nullptr;
    DirectoryHierarchy::DirectoryItem* mpRenamingItem = nullptr;
    char mRenameBuffer[256]{};
    const void* mpLastRenamingTarget = nullptr;

    eContextTarget mContextTarget = eContextTarget::None;
    DirectoryHierarchy::Directory* mpContextDir = nullptr;
    DirectoryHierarchy::DirectoryItem* mpContextItem = nullptr;

    // Pending request that is processed within a single frame and reset every frame.
    Request mPendingRequest;
};
} // namespace ho
