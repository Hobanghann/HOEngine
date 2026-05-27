#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "Core/Math/Color128.h"
#include "Core/Math/Color32.h"
#include "Core/Templates/GlobalPoolIndex.h"
#include "Path.h"

namespace ho
{
class Image final
{
  public:
    enum class eFormat
    {
        None,

        R8_UNORM,
        R8_SRGB,
        R8G8_UNORM,
        R8G8_SRGB,
        R8G8B8_UNORM,
        R8G8B8_SRGB,
        R8G8B8A8_UNORM,
        R8G8B8A8_SRGB,

        R32_FLOAT,
        R32G32_FLOAT,
        R32G32B32_FLOAT,
        R32G32B32A32_FLOAT,
    };

    FORCE_INLINE Image()
      : mPath(std::string())
      , mNameStr(std::string())
      , mFormat(eFormat::R8G8B8A8_SRGB)
      , mWidth(0)
      , mHeight(0)
      , mLogicalChannelCount(0)
      , mPhysicalChannelCount(0)
      , mBitmap(std::vector<uint8_t>())
    {
    }

    Image(const Image& img) = delete;

    FORCE_INLINE Image(const Path& path,
                       const std::string& nameStr,
                       eFormat format,
                       int32_t width,
                       int32_t height,
                       int32_t logicalChannelCount,
                       int32_t physicalChannelCount,
                       const uint8_t* bitmap)
      : mPath(path)
      , mNameStr(nameStr)
      , mFormat(format)
      , mWidth(width)
      , mHeight(height)
      , mLogicalChannelCount(logicalChannelCount)
      , mPhysicalChannelCount(physicalChannelCount)
      , mBitmap(bitmap, bitmap + width * height * mPhysicalChannelCount * GetChannelBytes(mFormat))
    {
    }

    FORCE_INLINE Image(Image&& other) noexcept
      : mPath(std::move(other.mPath))
      , mNameStr(std::move(other.mNameStr))
      , mFormat(other.mFormat)
      , mWidth(other.mWidth)
      , mHeight(other.mHeight)
      , mLogicalChannelCount(other.mLogicalChannelCount)
      , mPhysicalChannelCount(other.mPhysicalChannelCount)
      , mBitmap(std::move(other.mBitmap))
    {
    }

    FORCE_INLINE Image& operator=(Image&& other) noexcept
    {
        if (this != &other)
        {
            mPath = std::move(other.mPath);
            mNameStr = std::move(other.mNameStr);
            mFormat = other.mFormat;
            mWidth = other.mWidth;
            mHeight = other.mHeight;
            mLogicalChannelCount = other.mLogicalChannelCount;
            mPhysicalChannelCount = other.mPhysicalChannelCount;
            mBitmap = std::move(other.mBitmap);
        }
        return *this;
    }

    Image& operator=(const Image& img) = delete;
    FORCE_INLINE ~Image() = default;

    [[nodiscard]] FORCE_INLINE const Path& GetPath() const;
    [[nodiscard]] FORCE_INLINE const std::string& GetName() const;
    [[nodiscard]] FORCE_INLINE eFormat GetFormat() const;
    [[nodiscard]] FORCE_INLINE int32_t GetWidth() const;
    [[nodiscard]] FORCE_INLINE int32_t GetHeight() const;
    [[nodiscard]] FORCE_INLINE int32_t GetLogicalChannelCount() const;
    [[nodiscard]] FORCE_INLINE int32_t GetPhysicalChannelCount() const;

    [[nodiscard]] FORCE_INLINE Color32 GetColor32(int32_t x, int32_t y) const;
    [[nodiscard]] FORCE_INLINE Color128 GetColor128(int32_t x, int32_t y) const;

    [[nodiscard]] FORCE_INLINE const uint8_t* GetBitmap() const;

    [[nodiscard]] FORCE_INLINE static int32_t GetPixelBytes(eFormat format);
    [[nodiscard]] FORCE_INLINE static int32_t GetChannelBytes(eFormat format);

    [[nodiscard]] FORCE_INLINE bool IsLinear() const;

  private:
    Path mPath;
    std::string mNameStr;
    eFormat mFormat;
    int32_t mWidth;
    int32_t mHeight;
    int32_t mLogicalChannelCount;
    int32_t mPhysicalChannelCount;
    std::vector<uint8_t> mBitmap;
};

FORCE_INLINE int32_t Image::GetPixelBytes(eFormat format)
{
    switch (format)
    {
        case eFormat::R8_UNORM:
        case eFormat::R8_SRGB:
            return 1;
        case eFormat::R8G8_UNORM:
        case eFormat::R8G8_SRGB:
            return 2;
        case eFormat::R8G8B8_UNORM:
        case eFormat::R8G8B8_SRGB:
            return 3;
        case eFormat::R8G8B8A8_UNORM:
        case eFormat::R8G8B8A8_SRGB:
        case eFormat::R32_FLOAT:
            return 4;
        case eFormat::R32G32_FLOAT:
            return 8;
        case eFormat::R32G32B32_FLOAT:
            return 12;
        case eFormat::R32G32B32A32_FLOAT:
            return 16;

        default:
            HO_ASSERT(false, "Unknown image format.");
            return 0;
    }
}

FORCE_INLINE int32_t Image::GetChannelBytes(eFormat format)
{
    switch (format)
    {
        case eFormat::R8_UNORM:
        case eFormat::R8_SRGB:
        case eFormat::R8G8_UNORM:
        case eFormat::R8G8_SRGB:
        case eFormat::R8G8B8_UNORM:
        case eFormat::R8G8B8_SRGB:
        case eFormat::R8G8B8A8_UNORM:
        case eFormat::R8G8B8A8_SRGB:
            return 1;
        case eFormat::R32_FLOAT:
        case eFormat::R32G32_FLOAT:
        case eFormat::R32G32B32_FLOAT:
        case eFormat::R32G32B32A32_FLOAT:
            return 4;

        default:
            HO_ASSERT(false, "Unknown image format.");
            return 0;
    }
}

const Path& Image::GetPath() const
{
    return mPath;
}

const std::string& Image::GetName() const
{
    return mNameStr;
}

Image::eFormat Image::GetFormat() const
{
    return mFormat;
}

int32_t Image::GetWidth() const
{
    return mWidth;
}

int32_t Image::GetHeight() const
{
    return mHeight;
}

int32_t Image::GetLogicalChannelCount() const
{
    return mLogicalChannelCount;
}

int32_t Image::GetPhysicalChannelCount() const
{
    return mPhysicalChannelCount;
}

Color32 Image::GetColor32(int32_t x, int32_t y) const
{
    switch (mFormat)
    {
        case eFormat::R32_FLOAT:
        case eFormat::R32G32_FLOAT:
        case eFormat::R32G32B32_FLOAT:
        case eFormat::R32G32B32A32_FLOAT:
            return Color32(GetColor128(x, y));
        default:
            break;
    }
    HO_ASSERT(x < mWidth && y < mHeight, "out of bound");
    const int32_t idx = y * mWidth + x;

    const std::uint8_t* px = mBitmap.data() + idx * GetPixelBytes(mFormat);
    switch (mFormat)
    {
        case eFormat::R8_UNORM:
        case eFormat::R8_SRGB:
            return Color32(px[0], px[0], px[0], 255);
        case eFormat::R8G8_UNORM:
        case eFormat::R8G8_SRGB:
            return Color32(px[0], px[1], 0, 255);
        case eFormat::R8G8B8_UNORM:
        case eFormat::R8G8B8_SRGB:
            return Color32(px[0], px[1], px[2], 255);
        case eFormat::R8G8B8A8_UNORM:
        case eFormat::R8G8B8A8_SRGB:
            return Color32(px[0], px[1], px[2], px[3]);
        default:
            HO_ASSERT(false, "Invalid format.");
            return Color32(255, 0, 255, 255); // magenta = unsupported
    }
}

Color128 Image::GetColor128(int32_t x, int32_t y) const
{
    switch (mFormat)
    {
        case eFormat::R8_UNORM:
        case eFormat::R8_SRGB:
        case eFormat::R8G8_UNORM:
        case eFormat::R8G8_SRGB:
        case eFormat::R8G8B8_UNORM:
        case eFormat::R8G8B8_SRGB:
        case eFormat::R8G8B8A8_UNORM:
        case eFormat::R8G8B8A8_SRGB:
            return Color128(GetColor32(x, y));
        default:
            break;
    }

    HO_ASSERT(x < mWidth && y < mHeight, "out of bound");
    const int32_t idx = y * mWidth + x;

    const float* px = reinterpret_cast<const float*>(mBitmap.data() + idx * GetPixelBytes(mFormat));
    switch (mFormat)
    {
        case eFormat::R32_FLOAT:
            return Color128(px[0], px[0], px[0], 1.f);
        case eFormat::R32G32_FLOAT:
            return Color128(px[0], px[1], 0.f, 1.f);
        case eFormat::R32G32B32_FLOAT:
            return Color128(px[0], px[1], px[2], 1.f);
        case eFormat::R32G32B32A32_FLOAT:
            return Color128(px[0], px[1], px[2], px[3]);
        default:
            HO_ASSERT(false, "Invalid format.");
            return Color128(1.f, 0, 1.f, 1.f); // magenta = unsupported
    }
}

const uint8_t* Image::GetBitmap() const
{
    return mBitmap.data();
}

FORCE_INLINE bool Image::IsLinear() const
{
    switch (mFormat)
    {
        case eFormat::R8_UNORM:
        case eFormat::R8G8_UNORM:
        case eFormat::R8G8B8_UNORM:
        case eFormat::R8G8B8A8_UNORM:
        case eFormat::R32_FLOAT:
        case eFormat::R32G32_FLOAT:
        case eFormat::R32G32B32_FLOAT:
        case eFormat::R32G32B32A32_FLOAT:
            return true;
        case eFormat::R8_SRGB:
        case eFormat::R8G8_SRGB:
        case eFormat::R8G8B8_SRGB:
        case eFormat::R8G8B8A8_SRGB:
            return false;
        default:
            HO_ASSERT(false, "Unknown image format.");
            return false;
    }
}
} // namespace ho
