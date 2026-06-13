#pragma once

#include <filesystem>
#include <string>

#include "Macros.h"

namespace ho
{
class Path final
{
  public:
    Path() = default;
    Path(const Path& path) = default;

    explicit Path(const std::string& pathStr)
      : mPath(pathStr)
    {
    }

    explicit Path(const std::wstring& pathStr)
      : mPath(pathStr)
    {
    }

    Path(Path&& path) noexcept = default;

    FORCE_INLINE Path& operator=(const Path& rhs);
    FORCE_INLINE Path& operator=(Path&& rhs) noexcept;

    [[nodiscard]] FORCE_INLINE bool operator==(const Path& rhs) const;
    [[nodiscard]] FORCE_INLINE bool operator!=(Path&& rhs) const;

    [[nodiscard]] FORCE_INLINE Path operator/(const Path& rhs) const;

    [[nodiscard]] FORCE_INLINE Path GetParentPath() const;
    [[nodiscard]] FORCE_INLINE Path GetFileName() const;
    [[nodiscard]] FORCE_INLINE Path GetExtension() const;

    FORCE_INLINE void ResolveProjectPath();
    FORCE_INLINE void ResolveAssetPath();
    FORCE_INLINE void RemoveExtension();

    [[nodiscard]] FORCE_INLINE Path ResolvedProjectPath() const;
    [[nodiscard]] FORCE_INLINE Path ResolvedAssetPath() const;
    [[nodiscard]] FORCE_INLINE Path RemovedExtension() const;

    [[nodiscard]] FORCE_INLINE std::string ToString() const;
    [[nodiscard]] FORCE_INLINE std::wstring ToWString() const;

  private:
    Path(const std::filesystem::path& path)
      : mPath(path)
    {
    }

    static std::filesystem::path sProjectRootPath; // System Root ~ HOEngine
    static std::filesystem::path sAssetRootPath;   // System Root ~ HOEngine/Assets/

    std::filesystem::path mPath;
};

Path& Path::operator=(const Path& rhs)
{
    if (this != &rhs)
    {
        mPath = rhs.mPath;
    }
    return *this;
}

Path& Path::operator=(Path&& rhs) noexcept
{
    if (this != &rhs)
    {
        mPath = std::move(rhs.mPath);
    }
    return *this;
}

bool Path::operator==(const Path& rhs) const
{
    return mPath == rhs.mPath;
}

bool Path::operator!=(Path&& rhs) const
{
    return !(*this == rhs);
}

Path Path::operator/(const Path& rhs) const
{
    return Path(mPath / rhs.mPath);
}

Path Path::GetParentPath() const
{
    return mPath.parent_path();
}

Path Path::GetFileName() const
{
    return Path(mPath.filename());
}

Path Path::GetExtension() const
{
    return Path(mPath.extension());
}

void Path::ResolveProjectPath()
{
    if (!mPath.is_absolute())
    {
        mPath = (sProjectRootPath / mPath).lexically_normal();
    }
}

void Path::ResolveAssetPath()
{
    if (!mPath.is_absolute())
    {
        mPath = (sAssetRootPath / mPath).lexically_normal();
    }
}

void Path::RemoveExtension()
{
    mPath.replace_extension("");
}

Path Path::ResolvedProjectPath() const
{
    Path copy = *this;
    copy.ResolveProjectPath();
    return copy;
}

Path Path::ResolvedAssetPath() const
{
    Path copy = *this;
    copy.ResolveAssetPath();
    return copy;
}

Path Path::RemovedExtension() const
{
    Path copy = *this;
    copy.RemoveExtension();
    return copy;
}

std::string Path::ToString() const
{
    return mPath.string();
}

std::wstring Path::ToWString() const
{
    return mPath.wstring();
}

} // namespace ho
