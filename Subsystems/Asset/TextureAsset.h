#pragma once

#include "AssetDefs.h"
#include "Core/Templates/FixedArray.h"
#include "Core/Templates/GlobalPoolIndex.h"
#include "Core/Templates/WeakLocalPoolIndex.h"

namespace ho
{
struct GpuTexture;
using GpuTextureHandle = WeakLocalPoolIndex<GpuTexture, ObjectPool>;

struct TextureAsset
{
    struct SubTextureLayout
    {
        int32_t Offset = 0;
        int32_t Size = 0;     // The physical size in bytes of SubTexture
        int32_t RowPitch = 0; // The physical size in bytes of one row of pixels or blocks(In compressed format).
    };

    TextureAsset() = default;
    TextureAsset(const TextureAsset&) = delete;
    TextureAsset& operator=(const TextureAsset&) = delete;
    ~TextureAsset() = default;

    TextureAsset(TextureAsset&&) noexcept = default;
    TextureAsset& operator=(TextureAsset&&) noexcept = default;

    StringHandle hName = StringHandle::sNULL;

    int32_t Width = 0;
    int32_t Height = 0;
    int32_t Depth = 0;
    int32_t ArraySize = 0;
    int32_t MipLevels = 0;

    eTextureFormat Format = eTextureFormat::None;
    eTextureType Type = eTextureType::None;

    FixedArray<SubTextureLayout> Layouts;

    FixedArray<uint8_t> DataBlob;

    GpuTextureHandle hRenderProxy = GpuTextureHandle::sNULL;
};

using TextureHandle = WeakLocalPoolIndex<TextureAsset, ObjectPool>;
} // namespace ho