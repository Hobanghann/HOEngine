#pragma once

#include <string>

#include "AssetDefs.h"
#include "Core/Templates/FixedArray.h"
#include "IAsset.h"

namespace ho
{
struct GpuShader;
using GpuShaderHandle = WeakTypedHandle<GpuShader, TypedObjectPool>;

struct RShaderAsset final : public RIAsset
{
    HO_INJECT_REFLECTION_CODE(RShaderAsset)

    RShaderAsset() = default;
    RShaderAsset(const RShaderAsset&) = delete;
    RShaderAsset& operator=(const RShaderAsset&) = delete;
    ~RShaderAsset() override = default;

    RShaderAsset(RShaderAsset&&) noexcept = default;
    RShaderAsset& operator=(RShaderAsset&&) noexcept = default;

    int32_t GetByteSize() const override;

    std::string SourceStr;

    FixedArray<uint32_t> Binary{};

    eShaderStage Stage = eShaderStage::None;
    eMaterialAssetType CompatibleMaterialType = eMaterialAssetType::None;

    GpuShaderHandle hRenderProxy = GpuShaderHandle::sNULL;
};

} // namespace ho