#pragma once

#include <list>
#include <memory>
#include <string>

#include "Core/IO/Path.h"
#include "Subsystems/Asset/IAsset.h"
#include "Subsystems/Object/ObjectHandle.h"

namespace ho
{
class DirectoryHierarchy final
{
  public:
    struct Directory;

    struct DirectoryItem
    {
        std::string NameStr;
        uint32_t ID = 0;
        int64_t ByteSize = 0;
        Directory* pParent = nullptr;

        ObjectHandle<RIAsset> hAsset = ObjectHandle<RIAsset>::sNULL;

        bool bIsDirty = false;
    };

    struct Directory
    {
        std::string NameStr;
        uint32_t ID = 0;
        Directory* pParent = nullptr;
        int64_t ByteSize = 0;

        std::list<std::unique_ptr<Directory>> pChildren;

        std::list<DirectoryItem> Items;

        bool bIsVisible = true;
        bool bIsExpanded = false;
        bool bIsDirty = false;
    };

    DirectoryHierarchy();
    DirectoryHierarchy(const DirectoryHierarchy&) = delete;
    DirectoryHierarchy& operator=(const DirectoryHierarchy&) = delete;
    ~DirectoryHierarchy() = default;

    DirectoryItem* AddAsset(ObjectHandle<RIAsset> hAsset, Directory* pOutParent);

    Directory* AddDirectory(const std::string& nameStr, Directory* pOutParent);

    // Return nearest item.
    DirectoryItem* DeleteItem(DirectoryItem& item);

    // Return nearest directory.
    Directory* DeleteDirectory(Directory& dir);

    bool MoveItem(DirectoryItem* pOutItem, Directory* pOutDstParent);

    bool MoveDirectory(Directory* pOutDir, Directory* pOutDstParent);

    void SetAncestorVisible(const Directory& origin, bool flag);
    void SetDescendantVisibleRecursive(const Directory& origin, bool flag);

    void SetAncestorExpanded(const Directory& origin, bool flag);
    void SetDescendantExpandedRecursive(const Directory& origin, bool flag);

    void SetAncestorDirty(const Directory& origin, bool flag);
    void SetDescendantDirtyRecursive(const Directory& origin, bool flag);

    bool IsAncestorDirectory(const Directory& origin, uint32_t dirID) const;
    bool IsAncestorDirectoryContainingItem(const Directory& origin, uint32_t itemID) const;

    bool IsDescendantDirectoryRecursive(const Directory& origin, uint32_t dirID) const;
    bool IsDescendantDirectoryContainingItemRecursive(const Directory& origin, uint32_t dirID) const;

    Path GetDirectoryPath(const Directory& dir) const;
    Path GetItemPath(const DirectoryItem& item) const;

    Directory* GetRoot()
    {
        return mpRoot.get();
    }

    const Directory* GetRoot() const
    {
        return mpRoot.get();
    }

  private:
    void deleteItemAsset(DirectoryItem* pOutItem);
    void deleteDescendantItemsRecursive(Directory* pOutDir);
    void buildDirectoryPathRecursive(const Directory& dir, Path* pOutPath) const;
    void updateDirectoryByteSize(Directory* pDirectory, int64_t bytes);

    std::unique_ptr<Directory> mpRoot;
    uint32_t mDirectoryIdCount = 0;
    uint32_t mItemIdCount = 0;
};
} // namespace ho