#include "TextureUtils.h"

#include "Core/Math/MathFuncs.h"

namespace ho
{
namespace texutils
{
bool isUncompressedTextureFormat(eTextureFormat format)
{
    switch (format)
    {
        case eTextureFormat::R8_UNORM:
        case eTextureFormat::R8G8_UNORM:
        case eTextureFormat::R8G8B8A8_UNORM:
        case eTextureFormat::R8_SRGB:
        case eTextureFormat::R8G8_SRGB:
        case eTextureFormat::R8G8B8A8_SRGB:
        case eTextureFormat::R16_FLOAT:
        case eTextureFormat::R16G16_FLOAT:
        case eTextureFormat::R16G16B16A16_FLOAT:
        case eTextureFormat::R32_FLOAT:
            return true;

        case eTextureFormat::BC1_UNORM:
        case eTextureFormat::BC1_SRGB:
        case eTextureFormat::BC3_UNORM:
        case eTextureFormat::BC3_SRGB:
        case eTextureFormat::BC4_UNORM:
        case eTextureFormat::BC4_SNORM:
        case eTextureFormat::BC5_UNORM:
        case eTextureFormat::BC5_SNORM:
        case eTextureFormat::BC6H_UF16:
        case eTextureFormat::BC6H_SF16:
        case eTextureFormat::BC7_UNORM:
        case eTextureFormat::BC7_SRGB:
            return false;

        default:
            HO_ASSERT(false, "Invalid texture format.");
            return false;
    }
}

bool isLinearTextureFormat(eTextureFormat format)
{
    switch (format)
    {
        case eTextureFormat::R8_UNORM:
        case eTextureFormat::R8G8_UNORM:
        case eTextureFormat::R8G8B8A8_UNORM:
        case eTextureFormat::R16_FLOAT:
        case eTextureFormat::R16G16_FLOAT:
        case eTextureFormat::R16G16B16A16_FLOAT:
        case eTextureFormat::R32_FLOAT:
        case eTextureFormat::BC1_UNORM:
        case eTextureFormat::BC3_UNORM:
        case eTextureFormat::BC4_UNORM:
        case eTextureFormat::BC4_SNORM:
        case eTextureFormat::BC5_UNORM:
        case eTextureFormat::BC5_SNORM:
        case eTextureFormat::BC6H_UF16:
        case eTextureFormat::BC6H_SF16:
        case eTextureFormat::BC7_UNORM:
            return true;

        case eTextureFormat::R8_SRGB:
        case eTextureFormat::R8G8_SRGB:
        case eTextureFormat::R8G8B8A8_SRGB:
        case eTextureFormat::BC1_SRGB:
        case eTextureFormat::BC3_SRGB:
        case eTextureFormat::BC7_SRGB:
            return false;

        default:
            HO_ASSERT(false, "Invalid texture format.");
            return false;
    }
}

bool isIntegerTextureFormat(eTextureFormat format)
{
    switch (format)
    {
        case eTextureFormat::R8_UNORM:
        case eTextureFormat::R8_SRGB:
        case eTextureFormat::R8G8_UNORM:
        case eTextureFormat::R8G8_SRGB:
        case eTextureFormat::R8G8B8A8_UNORM:
        case eTextureFormat::R8G8B8A8_SRGB:
        case eTextureFormat::BC1_UNORM:
        case eTextureFormat::BC1_SRGB:
        case eTextureFormat::BC3_UNORM:
        case eTextureFormat::BC3_SRGB:
        case eTextureFormat::BC4_UNORM:
        case eTextureFormat::BC4_SNORM:
        case eTextureFormat::BC5_UNORM:
        case eTextureFormat::BC5_SNORM:
        case eTextureFormat::BC7_UNORM:
        case eTextureFormat::BC7_SRGB:
            return true;

        case eTextureFormat::R16_FLOAT:
        case eTextureFormat::R16G16_FLOAT:
        case eTextureFormat::R16G16B16A16_FLOAT:
        case eTextureFormat::R32_FLOAT:
        case eTextureFormat::BC6H_UF16:
        case eTextureFormat::BC6H_SF16:
            return false;

        case eTextureFormat::None:
        default:
            HO_ASSERT(false, "Invalid or None texture format.");
            return false;
    }
}

int32_t getTexturePixelBytes(eTextureFormat format)
{
    switch (format)
    {
        case eTextureFormat::R8_UNORM:
        case eTextureFormat::R8_SRGB:
            return 1;
        case eTextureFormat::R8G8_UNORM:
        case eTextureFormat::R8G8_SRGB:
        case eTextureFormat::R16_FLOAT:
            return 2;
        case eTextureFormat::R8G8B8A8_UNORM:
        case eTextureFormat::R8G8B8A8_SRGB:
        case eTextureFormat::R16G16_FLOAT:
        case eTextureFormat::R32_FLOAT:
            return 4;
        case eTextureFormat::R16G16B16A16_FLOAT:
            return 8;
        default:
            HO_ASSERT(false, "It's not uncompressed format.");
            return 0;
    }
}

int32_t getTextureBlockCount(int32_t length)
{
    return math::Max(1, (length + 3) / 4);
}

int32_t getTextureBlockBytes(eTextureFormat format)
{
    switch (format)
    {
        case eTextureFormat::BC1_UNORM:
        case eTextureFormat::BC1_SRGB:
            return 8;
        case eTextureFormat::BC3_UNORM:
        case eTextureFormat::BC3_SRGB:
            return 16;
        case eTextureFormat::BC4_UNORM:
        case eTextureFormat::BC4_SNORM:
            return 8;
        case eTextureFormat::BC5_UNORM:
        case eTextureFormat::BC5_SNORM:
            return 16;
        case eTextureFormat::BC6H_UF16:
        case eTextureFormat::BC6H_SF16:
            HO_ASSERT(false, "Currently not supports BC6 format.");
            return 0;
        case eTextureFormat::BC7_UNORM:
        case eTextureFormat::BC7_SRGB:
            return 16;
        default:
            HO_ASSERT(false, "It's not compressed format.");
            return 0;
    }
}

int32_t calculateMipLevels(int32_t width, int32_t height)
{
    if (width <= 0 || height <= 0)
    {
        return 0;
    }

    int32_t maxDim = math::Max(width, height);
    int32_t mipLevels = 0;
    while (maxDim > 0)
    {
        maxDim >>= 1;
        ++mipLevels;
    }
    return mipLevels;
}

} // namespace texutils
} // namespace ho