#pragma once

#include "Core/String/StringHandle.h"
#include "Scene/Scene.h"

namespace ho
{
struct Frustum;
struct SceneRenderContext;
struct InstanceData;

struct DirLightShadowMapRenderPassDesc;
struct ForwardRenderPassDesc;

struct OpaqueRenderStageDesc;
struct MaskedRenderStageDesc;
struct EnvironmentRenderStageDesc;
struct InWorldDebugRenderStageDesc;
struct BlendedRenderStageDesc;
struct OutlineRenderStageDesc;
struct PostProcessingStageDesc;
struct WorldOverlayDebugRenderStageDesc;

enum class eRenderingMode
{
    // Disables all lighting calculations (direct lights & IBL).
    // Renders meshes using only base color/albedo and emissive properties.
    Unlit,

    // Default rendering mode with full lighting, materials, and IBL.
    // Mesh's shading model follows material.
    Lit,

    // Unlit rendering combined with wireframe polygon mode.
    Wireframe,

    // Lit rendering combined with wireframe overlay.
    LitWireframe,

    // Not bind Diffuse/Albedo texture
    DetailLighting,

    // Disables mesh surface textures (uses a neutral gray material) and visual skybox,
    // while maintaining all direct and indirect (IBL) lighting calculations.
    LightingOnly,

    // Not bind Diffuse/Albedo texture, Bind Roughness 0, Metallic 1 MetallicRoughness texture.
    Reflection,
};

enum eRenderingFlags
{
    RenderingFlags_None = 0,
    RenderingFlags_RenderWorldGizmo = 1,
    RenderingFlags_RenderObjectOutline = 1 << 1,
    RenderingFlags_RenderMeshAabbBoundingVolume = 1 << 2,
    RenderingFlags_RenderMeshSphereBoundingVolume = 1 << 3,
    RenderingFlags_RenderLightDebugMesh = 1 << 4,
    RenderingFlags_RenderCameraDebugMesh = 1 << 5,
};

enum eStencilMode
{
    StencilMode_None = 0,
    StencilMode_Outline = 1,
};

class SceneRenderer final
{
  public:
    SceneRenderer();
    SceneRenderer(const SceneRenderer&) = delete;
    SceneRenderer& operator=(const SceneRenderer&) = delete;
    ~SceneRenderer();

    void RenderForwardWithExternalCamera(eRenderingMode renderingMode,
                                         eRenderingFlags flags,
                                         ObjectHandle<const RScene> hScene,
                                         StringHandle hFrameBufferName,
                                         const Vector3& cameraPos,
                                         const Matrix4x4& cameraViewMat,
                                         const Projection& cameraProj) const;

    void BakePointLightStaticShadowMap(ObjectHandle<RPointLightComponent> hPointLightComp,
                                       ObjectHandle<const RScene> hScene) const;
    void BakeSpotLightStaticShadowMap(ObjectHandle<RSpotLightComponent> hSpotLightComp,
                                      ObjectHandle<const RScene> hScene) const;

  private:
    bool hasToIgnoreLights(eRenderingMode renderingMode) const;

    void extractLights(SceneRenderContext* pOutRenderCtx) const;

    void renderLightsShadowMap(const SceneRenderContext& renderCtx) const;

    void extractDirLight(SceneRenderContext* pOutRenderCtx,
                         ObjectHandle<const RDirectionalLightComponent> hDirLightComp) const;
    void renderDirLightsShadowMap(const SceneRenderContext& renderCtx) const;

    void extractPointLight(SceneRenderContext* pOutRenderCtx,
                           ObjectHandle<const RPointLightComponent> hPointLightComp) const;
    void renderPointLightsShadowMap(const SceneRenderContext& renderCtx) const;
    void bakePointLightShadowMap(ObjectHandle<RPointLightComponent> hPointLightComp,
                                 const SceneRenderContext& renderCtx) const;

    void extractSpotLight(SceneRenderContext* pOutRenderCtx,
                          ObjectHandle<const RSpotLightComponent> hSpotLightComp) const;
    void renderSpotLightsShadowMap(const SceneRenderContext& renderCtx) const;
    void bakeSpotLightShadowMap(ObjectHandle<RSpotLightComponent> hSpotLightComp,
                                const SceneRenderContext& renderCtx) const;

    void renderLightsDebugMesh(const SceneRenderContext& renderCtx) const;

    void renderCamerasDebugMesh(const SceneRenderContext& renderCtx) const;

    ForwardRenderPassDesc constructForwardPassDesc(const SceneRenderContext& renderCtx) const;

    OpaqueRenderStageDesc constructOpaqueStageDescForShadowMapPass() const;
    MaskedRenderStageDesc constructMaskedStageDescForShadowMapPass() const;
    BlendedRenderStageDesc constructBlendedStageDescForShadowMapPass() const;

    OpaqueRenderStageDesc constructOpaqueStageDesc(const SceneRenderContext& renderCtx) const;
    MaskedRenderStageDesc constructMaskedStageDesc(const SceneRenderContext& renderCtx) const;
    EnvironmentRenderStageDesc constructEnvironmentStageDesc(const SceneRenderContext& renderCtx) const;

    // It supposed every draw command in world debug stage is for volume rendering.
    InWorldDebugRenderStageDesc constructInWorldDebugStageDesc(const SceneRenderContext& renderCtx) const;

    BlendedRenderStageDesc constructBlendedStageDesc(const SceneRenderContext& renderCtx) const;
    OutlineRenderStageDesc constructOutlineStageDesc(const SceneRenderContext& renderCtx) const;
    PostProcessingStageDesc constructPostProcessingStageDesc(const SceneRenderContext& renderCtx) const;
    WorldOverlayDebugRenderStageDesc constructWorldOverlayDebugStageDesc(const SceneRenderContext& renderCtx) const;

    void submitDrawCommands(const SceneRenderContext& renderCtx,
                            ObjectHandle<const RStaticMeshComponent> hStaticMeshComp) const;
    void submitShadowMapDrawCommands(const Frustum& lightFrustum,
                                     ObjectHandle<const RStaticMeshComponent> hStaticMeshComp) const;
    void submitShadowMapDrawCommands(const Sphere& castersBV,
                                     ObjectHandle<const RStaticMeshComponent> hStaticMeshComp) const;
    void submitBoundingVolumeDrawCommands(const Transform3D& worldT, const AABB& aabb) const;
    void submitBoundingVolumeDrawCommands(const Transform3D& worldT, const Sphere& sphere) const;
    void submitLightDebugDrawCommands(const SceneRenderContext& renderCtx,
                                      ObjectHandle<const RLightComponent> hLightComp) const;
    void submitCameraDebugDrawCommands(const SceneRenderContext& renderCtx,
                                       ObjectHandle<const RCameraComponent> hCameraComp) const;

    mutable FixedArray<InstanceData> mInstanceDataBuffer;
    mutable bool mbNeedToResizeInstanceDataBuffer = false;
};
} // namespace ho