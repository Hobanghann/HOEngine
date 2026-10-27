#pragma once

#include "AssetDefs.h"
#include "Core/Templates/FixedArray.h"
#include "IAsset.h"

namespace ho
{
struct GpuTexture;
using GpuTextureHandle = WeakTypedHandle<GpuTexture, TypedObjectPool>;

struct RTextureAsset final : public RIAsset
{
    HO_INJECT_REFLECTION_CODE(RTextureAsset)

    // Layout for a specific mip level of a specific array element.
    // For 3D textures, all depth slices at the given mip level are stored contiguously in a single layout.
    struct SubTextureLayout
    {
        int32_t Offset = 0;   // Byte offset within DataBlob
        int32_t Size = 0;     // Total physical size in bytes (includes all depth slices for 3D textures)
        int32_t RowPitch = 0; // Physical size in bytes of one row of pixels or compressed blocks
    };

    RTextureAsset() = default;
    RTextureAsset(const RTextureAsset&) = delete;
    RTextureAsset& operator=(const RTextureAsset&) = delete;
    ~RTextureAsset() override = default;

    RTextureAsset(RTextureAsset&&) noexcept = default;
    RTextureAsset& operator=(RTextureAsset&&) noexcept = default;

    int32_t GetByteSize() const override;

    int32_t Width = 0;
    int32_t Height = 0;
    int32_t Depth = 0;
    int32_t ArraySize = 0;
    int32_t MipLevels = 0;

    eTextureFormat Format = eTextureFormat::None;
    eTextureType Type = eTextureType::None;

    FixedArray<SubTextureLayout> Layouts;

    FixedArray<uint8_t> DataBlob;

    GpuTextureHandle hRenderProxy;
};

} // namespace ho