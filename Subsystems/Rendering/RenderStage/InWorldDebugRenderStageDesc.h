#pragma once

#include "../RenderData/GpuResources.h"

namespace ho
{
struct InWorldDebugRenderStageDesc
{
    GpuStaticMeshHandle hOverrideStaticMesh;

    GpuMaterial::GpuPipelineState OverridePipelineState;
    bool bOverrideAlphaBlendMode = false;
    bool bOverridePolygonMode = false;
    bool bOverrideCullMode = false;

    GpuMaterialHandle hAttributeOverrideMaterial;

    bool bBindTexture[static_cast<int32_t>(eMaterialTextureUsage::Last)] = {};
    GpuTextureHandle hOverrideTextures[static_cast<int32_t>(eMaterialTextureUsage::Last)] = {};
    GpuShaderHandle hOverrideShaders[static_cast<int32_t>(eShaderStage::Last)] = {};

    bool bIsPerspective = true;
    bool bDrawWorldGizmo = false;
    float WorldGizmoGapSize = 10.0f;
    float WorldGizmoLineWidth = 0.2f;
    float WorldGizmoFadeStartDist = 100.0f;
    float WorldGizmoFadeEndDist = 600.0f;
};
} // namespace ho