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
        // Uncompressed formats
        R8,
        RG8,
        RGB8,
        RGBA8,

        // High-Precision / HDR formats
        R32F,
        RG32F,
        RGB32F,
        RGBA32F
    };

    FORCE_INLINE Image()
      : mPath(std::string())
      , mNameStr(std::string())
      , mFormat(eFormat::RGB8)
      , mWidth(0)
      , mHeight(0)
      , mBitmap(std::vector<uint8_t>())
    {
    }

    Image(const Image& img) = delete;
    FORCE_INLINE Image(const Path& path,
                       const std::string& nameStr,
                       eFormat format,
                       int32_t width,
                       int32_t height,
                       const uint8_t* bitmap);

    FORCE_INLINE Image(Image&& other) noexcept
      : mPath(std::move(other.mPath))
      , mNameStr(std::move(other.mNameStr))
      , mFormat(other.mFormat)
      , mWidth(other.mWidth)
      , mHeight(other.mHeight)
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

    [[nodiscard]] FORCE_INLINE Color32 GetColor32(int32_t x, int32_t y) const;
    [[nodiscard]] FORCE_INLINE Color128 GetColor128(int32_t x, int32_t y) const;

    [[nodiscard]] FORCE_INLINE const uint8_t* GetBitmap() const;

    FORCE_INLINE static int32_t GetPixelBytes(eFormat format);

  private:
    Path mPath;
    std::string mNameStr;
    eFormat mFormat;
    int32_t mWidth;
    int32_t mHeight;
    std::vector<uint8_t> mBitmap;
};

FORCE_INLINE int32_t Image::GetPixelBytes(eFormat format)
{
    switch (format)
    {
        case eFormat::R8:
            return 1;
        case eFormat::RG8:
            return 2;
        case eFormat::RGB8:
            return 3;
        case eFormat::RGBA8:
        case eFormat::R32F:
            return 4;
        case eFormat::RG32F:
            return 8;
        case eFormat::RGB32F:
            return 12;
        case eFormat::RGBA32F:
            return 16;

        default:
            HO_ASSERT(false, "Unknown image format.");
            return 0;
    }
}

Image::Image(
    const Path& path, const std::string& nameStr, eFormat format, int32_t width, int32_t height, const uint8_t* bitmap)
  : mPath(path)
  , mNameStr(nameStr)
  , mFormat(format)
  , mWidth(width)
  , mHeight(height)
  , mBitmap(bitmap, bitmap + width * height * GetPixelBytes(format))
{
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

Color32 Image::GetColor32(int32_t x, int32_t y) const
{
    switch (mFormat)
    {
        case eFormat::R32F:
        case eFormat::RG32F:
        case eFormat::RGB32F:
        case eFormat::RGBA32F:
            return Color32(GetColor128(x, y));
        default:
            break;
    }
    HO_ASSERT(x < mWidth && y < mHeight, "out of bound");
    const int32_t idx = y * mWidth + x;

    const std::uint8_t* px = mBitmap.data() + idx * GetPixelBytes(mFormat);
    switch (mFormat)
    {
        case eFormat::R8:
            return Color32(px[0], px[0], px[0], 255);
        case eFormat::RG8:
            return Color32(px[0], px[1], 0, 255);
        case eFormat::RGB8:
            return Color32(px[0], px[1], px[2], 255);
        case eFormat::RGBA8:
            return Color32(px[0], px[1], px[2], px[3]);
        default:
            return Color32(255, 0, 255, 255); // magenta = unsupported
    }
}

Color128 Image::GetColor128(int32_t x, int32_t y) const
{
    switch (mFormat)
    {
        case eFormat::R8:
        case eFormat::RG8:
        case eFormat::RGB8:
        case eFormat::RGBA8:
            return Color128(GetColor32(x, y));
        default:
            break;
    }

    HO_ASSERT(x < mWidth && y < mHeight, "out of bound");
    const int32_t idx = y * mWidth + x;

    const float* px = reinterpret_cast<const float*>(mBitmap.data() + idx * GetPixelBytes(mFormat));
    switch (mFormat)
    {
        case eFormat::R32F:
            return Color128(px[0], px[0], px[0], 255.f);
        case eFormat::RG32F:
            return Color128(px[0], px[1], 0.f, 255.f);
        case eFormat::RGB32F:
            return Color128(px[0], px[1], px[2], 255.f);
        case eFormat::RGBA32F:
            return Color128(px[0], px[1], px[2], px[3]);
        default:
            return Color128(255.f, 0, 255.f, 255.f); // magenta = unsupported
    }
}

const uint8_t* Image::GetBitmap() const
{
    return mBitmap.data();
}

} // namespace ho
