#pragma once

#include "../RenderData/GpuResources.h"

namespace ho
{
struct BlendedRenderStageDesc
{
    BlendedRenderStageDesc()
    {
        for (bool& bBind : bBindTexture)
        {
            bBind = true;
        }
    }

    GpuStaticMeshHandle hOverrideStaticMesh;

    GpuMaterial::GpuPipelineState OverridePipelineState;
    bool bOverrideAlphaBlendMode = false;
    bool bOverridePolygonMode = false;
    bool bOverrideCullMode = false;

    GpuMaterialHandle hAttributeOverrideMaterial;

    bool bBindTexture[static_cast<int32_t>(eMaterialTextureUsage::Last)] = {};
    GpuTextureHandle hOverrideTextures[static_cast<int32_t>(eMaterialTextureUsage::Last)] = {};
    GpuShaderHandle hOverrideShaders[static_cast<int32_t>(eShaderStage::Last)] = {};
};
} // namespace ho