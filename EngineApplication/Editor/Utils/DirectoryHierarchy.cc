#include "DirectoryHierarchy.h"

#include "EditorDisplayUtils.h"
#include "Subsystems/Asset/AssetSystem.h"

namespace ho
{
DirectoryHierarchy::DirectoryHierarchy()
  : mpRoot(std::make_unique<Directory>())
{
    mpRoot->NameStr = "All";
    mpRoot->ID = mDirectoryIdCount;
    ++mDirectoryIdCount;
}

DirectoryHierarchy::DirectoryItem* DirectoryHierarchy::AddAsset(ObjectHandle<RIAsset> hAsset, Directory* pOutParent)
{
    HO_ASSERT(hAsset.IsValid(), "Invalid asset.");
    if (!pOutParent)
    {
        HO_ASSERT(false, "Invalid parent directory.");
        pOutParent = mpRoot.get();
    }

    DirectoryItem newItem;

    newItem.NameStr = getAssetDisplayName(hAsset);
    newItem.ID = mItemIdCount;
    ++mItemIdCount;
    newItem.hAsset = hAsset;
    newItem.ByteSize = hAsset->GetByteSize();
    newItem.pParent = pOutParent;
    pOutParent->Items.push_back(newItem);
    updateDirectoryByteSize(pOutParent, newItem.ByteSize);
    pOutParent->bIsDirty = true;
    return &(pOutParent->Items.back());
}

DirectoryHierarchy::Directory* DirectoryHierarchy::AddDirectory(const std::string& nameStr, Directory* pOutParent)
{
    if (!pOutParent)
    {
        HO_ASSERT(false, "Invalid parent directory.");
        pOutParent = mpRoot.get();
    }
    pOutParent->pChildren.push_back(std::make_unique<Directory>());
    pOutParent->pChildren.back()->NameStr = nameStr;
    pOutParent->pChildren.back()->ID = mDirectoryIdCount;
    ++mDirectoryIdCount;
    pOutParent->pChildren.back()->pParent = pOutParent;
    pOutParent->bIsDirty = true;
    return pOutParent->pChildren.back().get();
}

DirectoryHierarchy::DirectoryItem* DirectoryHierarchy::DeleteItem(DirectoryItem& item)
{
    if (!item.pParent)
    {
        HO_ASSERT(false, "Invalid item.");
        return nullptr;
    }

    Directory* pParent = item.pParent;
    for (auto it = pParent->Items.begin(); it != pParent->Items.end(); ++it)
    {
        if (it->ID == item.ID)
        {
            deleteItemAsset(&*it);
            updateDirectoryByteSize(pParent, -item.ByteSize);

            auto nextIt = pParent->Items.erase(it);
            pParent->bIsDirty = true;

            if (nextIt != pParent->Items.end())
            {
                return &(*nextIt);
            }

            if (pParent->Items.empty())
            {
                return nullptr;
            }

            --nextIt;
            return &(*nextIt);
        }
    }

    return nullptr;
}

DirectoryHierarchy::Directory* DirectoryHierarchy::DeleteDirectory(Directory& dir)
{
    if (!dir.pParent)
    {
        HO_ASSERT(false, "Invalid directory.");
        return nullptr;
    }

    Directory* pParent = dir.pParent;
    for (auto it = pParent->pChildren.begin(); it != pParent->pChildren.end(); ++it)
    {
        if ((*it)->ID == dir.ID)
        {
            deleteDescendantItemsRecursive(&dir);
            updateDirectoryByteSize(pParent, -dir.ByteSize);

            auto nextIt = pParent->pChildren.erase(it);
            pParent->bIsDirty = true;

            if (nextIt != pParent->pChildren.end())
            {
                return nextIt->get();
            }

            if (pParent->pChildren.empty())
            {
                return pParent;
            }

            --nextIt;
            return nextIt->get();
        }
    }

    return nullptr;
}

bool DirectoryHierarchy::MoveItem(DirectoryItem* pOutItem, Directory* pOutDstParent)
{
    if (!pOutItem || !pOutItem->pParent)
    {
        HO_ASSERT(false, "Invalid item.");
        return false;
    }

    if (!pOutDstParent)
    {
        HO_ASSERT(false, "Invalid directory.");
        return false;
    }

    Directory* pSrcParent = pOutItem->pParent;
    if (pSrcParent == pOutDstParent)
    {
        return false;
    }

    auto it = std::find_if(pSrcParent->Items.begin(),
                           pSrcParent->Items.end(),
                           [pOutItem](const DirectoryItem& item) { return &item == pOutItem; });

    if (it == pSrcParent->Items.end())
    {
        HO_ASSERT(false, "Item not found in its registered parent.");
        return false;
    }

    pOutDstParent->Items.splice(pOutDstParent->Items.end(), pSrcParent->Items, it);
    pOutItem->pParent = pOutDstParent;
    updateDirectoryByteSize(pSrcParent, -pOutItem->ByteSize);
    updateDirectoryByteSize(pOutDstParent, pOutItem->ByteSize);
    pSrcParent->bIsDirty = true;
    pOutDstParent->bIsDirty = true;

    return true;
}

bool DirectoryHierarchy::MoveDirectory(Directory* pOutDir, Directory* pOutDstParent)
{
    if (!pOutDir || !pOutDir->pParent || !pOutDstParent)
    {
        HO_ASSERT(false, "Invalid directory pointer.");
        return false;
    }

    if (pOutDir == pOutDstParent || pOutDir->pParent == pOutDstParent)
    {
        return false;
    }

    if (IsDescendantDirectoryRecursive(*pOutDir, pOutDstParent->ID))
    {
        return false;
    }

    Directory* pSrcParent = pOutDir->pParent;
    std::unique_ptr<Directory> pTargetDir;
    for (auto it = pSrcParent->pChildren.begin(); it != pSrcParent->pChildren.end(); ++it)
    {
        if ((*it)->ID == pOutDir->ID)
        {
            pTargetDir = std::move(*it);
            pSrcParent->pChildren.erase(it);
            pSrcParent->bIsDirty = true;
            break;
        }
    }
    if (!pTargetDir)
    {
        return false;
    }
    updateDirectoryByteSize(pSrcParent, -pTargetDir->ByteSize);
    pTargetDir->pParent = pOutDstParent;
    pOutDstParent->pChildren.push_back(std::move(pTargetDir));
    updateDirectoryByteSize(pOutDstParent, pOutDstParent->pChildren.back()->ByteSize);
    pOutDstParent->bIsDirty = true;
    return true;
}

void DirectoryHierarchy::SetAncestorVisible(const Directory& origin, bool flag)
{
    Directory* pDir = origin.pParent;
    while (pDir != nullptr)
    {
        pDir->bIsVisible = flag;
        pDir = pDir->pParent;
    }
}

void DirectoryHierarchy::SetDescendantVisibleRecursive(const Directory& origin, bool flag)
{
    for (const auto& pChild : origin.pChildren)
    {
        pChild->bIsVisible = flag;
        SetDescendantVisibleRecursive(*pChild, flag);
    }
}

void DirectoryHierarchy::SetAncestorExpanded(const Directory& origin, bool flag)
{
    Directory* pDir = origin.pParent;
    while (pDir != nullptr)
    {
        pDir->bIsExpanded = flag;
        pDir = pDir->pParent;
    }
}

void DirectoryHierarchy::SetDescendantExpandedRecursive(const Directory& origin, bool flag)
{
    for (const auto& pChild : origin.pChildren)
    {
        pChild->bIsExpanded = flag;
        SetDescendantExpandedRecursive(*pChild, flag);
    }
}

void DirectoryHierarchy::SetAncestorDirty(const Directory& origin, bool flag)
{
    Directory* pDir = origin.pParent;
    while (pDir != nullptr)
    {
        pDir->bIsDirty = flag;
        pDir = pDir->pParent;
    }
}

void DirectoryHierarchy::SetDescendantDirtyRecursive(const Directory& origin, bool flag)
{
    for (const auto& pChild : origin.pChildren)
    {
        pChild->bIsDirty = flag;
        SetDescendantDirtyRecursive(*pChild, flag);
    }
}

bool DirectoryHierarchy::IsAncestorDirectory(const Directory& origin, uint32_t dirID) const
{
    const Directory* pDir = origin.pParent;
    while (pDir != nullptr)
    {
        if (pDir->ID == dirID)
        {
            return true;
        }
        pDir = pDir->pParent;
    }

    return false;
}

bool DirectoryHierarchy::IsAncestorDirectoryContainingItem(const Directory& origin, uint32_t itemID) const
{
    const Directory* pDir = origin.pParent;
    while (pDir != nullptr)
    {
        for (const auto& item : pDir->Items)
        {
            if (item.ID == itemID)
            {
                return true;
            }
        }
        pDir = pDir->pParent;
    }

    return false;
}

bool DirectoryHierarchy::IsDescendantDirectoryRecursive(const Directory& origin, uint32_t dirID) const
{
    for (const auto& pChild : origin.pChildren)
    {
        if (pChild->ID == dirID)
        {
            return true;
        }
        if (IsDescendantDirectoryRecursive(*pChild, dirID))
        {
            return true;
        }
    }

    return false;
}

bool DirectoryHierarchy::IsDescendantDirectoryContainingItemRecursive(const Directory& origin, uint32_t dirID) const
{
    for (const auto& pChild : origin.pChildren)
    {
        for (const auto& item : pChild->Items)
        {
            if (item.ID == dirID)
            {
                return true;
            }
        }
        if (IsDescendantDirectoryContainingItemRecursive(*pChild, dirID))
        {
            return true;
        }
    }

    return false;
}

Path DirectoryHierarchy::GetDirectoryPath(const Directory& dir) const
{
    static Path sBuf;
    sBuf = Path(std::string());

    buildDirectoryPathRecursive(dir, &sBuf);
    return sBuf;
}

Path DirectoryHierarchy::GetItemPath(const DirectoryItem& item) const
{
    return GetDirectoryPath(*item.pParent) / Path(item.NameStr);
}

void DirectoryHierarchy::deleteItemAsset(DirectoryItem* pOutItem)
{
    AssetSystem::GetInstance().DeleteAsset(pOutItem->hAsset);
    pOutItem->hAsset = ObjectHandle<RIAsset>::sNULL;
}

void DirectoryHierarchy::deleteDescendantItemsRecursive(Directory* pOutDir)
{
    if (!pOutDir)
    {
        HO_ASSERT(false, "Invalid directory.");
        return;
    }

    for (auto& item : pOutDir->Items)
    {
        deleteItemAsset(&item);
    }

    for (const auto& pChild : pOutDir->pChildren)
    {
        deleteDescendantItemsRecursive(pChild.get());
    }
}

void DirectoryHierarchy::buildDirectoryPathRecursive(const Directory& dir, Path* pOutPath) const
{
    if (dir.pParent)
    {
        buildDirectoryPathRecursive(*dir.pParent, pOutPath);
    }

    *pOutPath = *pOutPath / Path(dir.NameStr);
}

void DirectoryHierarchy::updateDirectoryByteSize(Directory* pDirectory, int64_t bytes)
{
    bytes = bytes == -1 ? 0 : bytes;

    while (pDirectory != nullptr)
    {
        pDirectory->ByteSize += bytes;
        pDirectory = pDirectory->pParent;
    }
}

} // namespace ho