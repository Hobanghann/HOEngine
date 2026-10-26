#pragma once

#include <imgui.h>

#include "../Utils/DirectoryHierarchy.h"

namespace ho
{
class DirectoryHierarchyView final
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

        MoveItem,
    };

    struct Request
    {
        eRequestType Type = eRequestType::None;
        DirectoryHierarchy::Directory* pTargetDir = nullptr;
        DirectoryHierarchy::DirectoryItem* pTargetItem = nullptr;
        DirectoryHierarchy::Directory* pDstDir = nullptr;
    };

    void Draw(DirectoryHierarchy* pOutDirHierarchy, DirectoryHierarchy::Directory* pOutOpenedDir);

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
    void drawTopBorder();
    void drawHeader();
    void drawDirectoryFilter(DirectoryHierarchy* pOutDirHierarchy);
    void drawDirectoryTree(DirectoryHierarchy& directoryHierarchy, const DirectoryHierarchy::Directory& openedDir);
    void drawDirectoryTreeRecursive(DirectoryHierarchy::Directory* pOutDrawDir,
                                    const DirectoryHierarchy::Directory& openedDir);
    void drawDirectoryTreeNode(DirectoryHierarchy::Directory* pOutDrawDir,
                               const DirectoryHierarchy::Directory& openedDir);
    void handleDirectoryDragDrop(DirectoryHierarchy::Directory* pOutDrawDir);
    void handleDirectoryNodeInteraction(DirectoryHierarchy::Directory* pOutDrawDir);
    bool mbOpenDirectoryRightClickPopup = false;
    void drawDirectoryRightClickPopup();

    void updateDirectoryVisibilityRecursive(DirectoryHierarchy* pOutDirHierarchy,
                                            DirectoryHierarchy::Directory* pOutDir);

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

    DirectoryHierarchy::Directory* mpContextMenuDirectory = nullptr;

    ImGuiTextFilter mDirectoryFilter;

    // Pending request that is processed within a single frame and reset every frame
    Request mPendingRequest;
};
} // namespace ho