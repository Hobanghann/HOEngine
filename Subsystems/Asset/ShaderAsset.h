#pragma once

#include <string>

#include "AssetDefs.h"
#include "Core/Templates/FixedArray.h"
#include "Core/Templates/GlobalPoolIndex.h"
#include "Core/Templates/WeakLocalPoolIndex.h"

namespace ho
{
struct GpuShader;
using GpuShaderHandle = WeakLocalPoolIndex<GpuShader, ObjectPool>;

struct ShaderAsset
{
    ShaderAsset() = default;
    ShaderAsset(const ShaderAsset&) = delete;
    ShaderAsset& operator=(const ShaderAsset&) = delete;
    ~ShaderAsset() = default;

    ShaderAsset(ShaderAsset&&) noexcept = default;
    ShaderAsset& operator=(ShaderAsset&&) noexcept = default;

    StringHandle hName = StringHandle::sNULL;

    std::string SourceStr;

    FixedArray<uint32_t> Binary{};

    eShaderStage Stage = eShaderStage::None;
    uint8_t CompatibleMaterialTypes = 0;

    GpuShaderHandle hRenderProxy = GpuShaderHandle::sNULL;
};

using ShaderHandle = WeakLocalPoolIndex<ShaderAsset, ObjectPool>;
} // namespace ho