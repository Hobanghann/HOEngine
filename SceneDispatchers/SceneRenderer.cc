#include "SceneRenderer.h"

#include "Core/Log/Logger.h"
#include "Core/Math/Frustum.h"
#include "Scene/Scene.h"
#include "Subsystems/Asset/AssetSystem.h"
#include "Subsystems/Rendering/RenderBackend/GpuBufferLayouts.h"
#include "Subsystems/Rendering/RenderData/GpuResources.h"
#include "Subsystems/Rendering/RenderingSystem.h"

namespace ho
{
struct SceneRenderContext
{
    eRenderingMode RenderingMode = eRenderingMode::Lit;
    eRenderingFlags RenderingFlags = RenderingFlags_None;
    ObjectHandle<const RScene> hScene = ObjectHandle<const RScene>::sNULL;
    StringHandle hFrameBufferName;
    Vector3 CameraPos;
    Matrix4x4 CameraViewMat;
    Projection CameraProj = Projection::CreatePerspective(1.0f, 1.0f, 0.0f, 1.0f);
    Frustum CameraFrustum;
    DirectLightData DirectLightsData;
    ObjectHandle<const RDirectionalLightComponent> hDirLights[DirectLightData::sMaxDirLightCount] = {};
    ObjectHandle<const RPointLightComponent> hPointLights[DirectLightData::sMaxPointLightCount] = {};
    ObjectHandle<const RSpotLightComponent> hSpotLights[DirectLightData::sMaxSpotLightCount] = {};
    DirectLightShadowData DirectLightsShadowData;
    ObjectHandle<const RDirectionalLightComponent>
        hShadowCastingDirLights[DirectLightShadowData::sMaxDirLightShadowDataCount] = {};
    ObjectHandle<const RPointLightComponent>
        hShadowCastingPointLights[DirectLightShadowData::sMaxPointLightShadowDataCount] = {};
    ObjectHandle<const RSpotLightComponent>
        hShadowCastingSpotLights[DirectLightShadowData::sMaxSpotLightShadowDataCount] = {};
    IndirectLightData IndirectLightsData;
    Vector3 DirLightsWorldPos[DirectLightShadowData::sMaxDirLightShadowDataCount] = {};
    Sphere DirLightsCastersBV[DirectLightShadowData::sMaxDirLightShadowDataCount] = {};
};

SceneRenderer::SceneRenderer()
  : mInstanceDataBuffer(FixedArray<InstanceData>(4096))
{
}

SceneRenderer::~SceneRenderer() {}

void SceneRenderer::RenderForwardWithExternalCamera(eRenderingMode renderingMode,
                                                    eRenderingFlags flags,
                                                    ObjectHandle<const RScene> hScene,
                                                    StringHandle hFrameBufferName,
                                                    const Vector3& cameraPos,
                                                    const Matrix4x4& cameraViewMat,
                                                    const Projection& cameraProj) const
{
    HO_ASSERT(hFrameBufferName.IsValid(), "Invalid frame buffer name.");
    HO_ASSERT(hScene.IsValid(), "Scene is invalid. Scene might be deleted or not created.");

    if (!RenderingSystem::GetInstance().HasFrameBuffer(hFrameBufferName))
    {
        return;
    }

    SceneRenderContext renderCtx;
    renderCtx.RenderingMode = renderingMode;
    renderCtx.RenderingFlags = flags;
    renderCtx.hScene = hScene;
    renderCtx.hFrameBufferName = hFrameBufferName;
    renderCtx.CameraPos = cameraPos;
    renderCtx.CameraViewMat = cameraViewMat;
    renderCtx.CameraProj = cameraProj;

    const Matrix4x4 cameraProjViewMat = cameraProj.GetMatrix() * cameraViewMat;
    renderCtx.CameraFrustum = Frustum::FromMatrix4x4(cameraProjViewMat);

    if (!hasToIgnoreLights(renderingMode))
    {
        extractLights(&renderCtx);
        renderLightsShadowMap(renderCtx);
    }

    if (RenderingSystem::GetInstance().BeginForwardRenderPass(constructForwardPassDesc(renderCtx)))
    {
        // Attach stages.
        RenderingSystem::GetInstance().AttachOpaqueRenderStage(constructOpaqueStageDesc(renderCtx));

        RenderingSystem::GetInstance().AttachMaskedRenderStage(constructMaskedStageDesc(renderCtx));

        if (cameraProj.GetType() == eProjectionType::Perspective)
        {
            RenderingSystem::GetInstance().AttachEnvironmentRenderStage(constructEnvironmentStageDesc(renderCtx));
        }

        RenderingSystem::GetInstance().AttachInWorldDebugRenderStage(constructInWorldDebugStageDesc(renderCtx));

        RenderingSystem::GetInstance().AttachBlendedRenderStage(constructBlendedStageDesc(renderCtx));

        RenderingSystem::GetInstance().AttachOutlineRenderStage(constructOutlineStageDesc(renderCtx));

        RenderingSystem::GetInstance().AttachPostProcessingStage(constructPostProcessingStageDesc(renderCtx));

        // Submit draw commands.
        RenderingSystem::GetInstance().BeginSubmitDrawCommands();
        for (const ObjectHandle<const RMeshComponent> hMeshComp : hScene->GetComponents<RMeshComponent>())
        {
            if (!hMeshComp->GetOwner()->IsVisibleInEditor())
            {
                continue;
            }

            const Transform3D& meshWorldT =
                hMeshComp->GetOwner()->GetComponent<RTransformComponent>()->GetWorldTransform();

            switch (hMeshComp->GetType())
            {
                case RMeshComponent::eMeshType::Static:
                {
                    const ObjectHandle<const RStaticMeshComponent> hStaticMeshComp =
                        hMeshComp.Cast<RStaticMeshComponent>();
                    if (hStaticMeshComp->GetMesh().IsNULL())
                    {
                        continue;
                    }

                    if (renderCtx.CameraFrustum.GetSphereSide(
                            meshWorldT.Transform(hStaticMeshComp->GetMesh()->SphereVolume)) == math::eSide::Outside)
                    {
                        continue;
                    }

                    if (!RenderingSystem::GetInstance().IsUploaded(hStaticMeshComp->GetMesh()))
                    {
                        RenderingSystem::GetInstance().UploadStaticMesh(hStaticMeshComp->GetMesh());
                        continue;
                    }
                    if (flags & RenderingFlags_RenderMeshAabbBoundingVolume)
                    {
                        submitBoundingVolumeDrawCommands(meshWorldT, hStaticMeshComp->GetMesh()->AabbVolume);
                    }
                    if (flags & RenderingFlags_RenderMeshSphereBoundingVolume)
                    {
                        submitBoundingVolumeDrawCommands(meshWorldT, hStaticMeshComp->GetMesh()->SphereVolume);
                    }

                    submitDrawCommands(renderCtx, hStaticMeshComp);
                }
                break;
                case RMeshComponent::eMeshType::Skeletal:
                    break;
                default:
                    HO_ASSERT(false, "Invalid mesh type");
            }
        }

        if (!hasToIgnoreLights(renderingMode) && (flags & RenderingFlags_RenderLightDebugMesh))
        {
            renderLightsDebugMesh(renderCtx);
        }

        if (flags & RenderingFlags_RenderCameraDebugMesh)
        {
            renderCamerasDebugMesh(renderCtx);
        }

        RenderingSystem::GetInstance().EndSubmitDrawCommands();
        RenderingSystem::GetInstance().EndRenderPass();
    }

    // Draw wireframe for LitWireframe mode.
    if (renderingMode == eRenderingMode::LitWireframe)
    {
        renderCtx.RenderingMode = eRenderingMode::Wireframe;
        ForwardRenderPassDesc forwardDesc = constructForwardPassDesc(renderCtx);
        forwardDesc.bClearColorTarget = false;
        forwardDesc.bClearDepthTarget = false;
        forwardDesc.bClearStencilTarget = false;
        if (RenderingSystem::GetInstance().BeginForwardRenderPass(forwardDesc))
        {
            // Attach stage.
            RenderingSystem::GetInstance().AttachOpaqueRenderStage(constructOpaqueStageDesc(renderCtx));

            RenderingSystem::GetInstance().AttachMaskedRenderStage(constructMaskedStageDesc(renderCtx));

            RenderingSystem::GetInstance().AttachBlendedRenderStage(constructBlendedStageDesc(renderCtx));

            RenderingSystem::GetInstance().AttachOutlineRenderStage(constructOutlineStageDesc(renderCtx));

            RenderingSystem::GetInstance().AttachPostProcessingStage(constructPostProcessingStageDesc(renderCtx));

            // Submit draw commands.
            RenderingSystem::GetInstance().BeginSubmitDrawCommands();
            for (const ObjectHandle<const RMeshComponent> hMeshComp : hScene->GetComponents<RMeshComponent>())
            {
                if (!hMeshComp->GetOwner()->IsVisibleInEditor())
                {
                    continue;
                }

                const Transform3D& meshWorldT =
                    hMeshComp->GetOwner()->GetComponent<RTransformComponent>()->GetWorldTransform();

                switch (hMeshComp->GetType())
                {
                    case RMeshComponent::eMeshType::Static:
                    {
                        const ObjectHandle<const RStaticMeshComponent> hStaticMeshComp =
                            hMeshComp.Cast<RStaticMeshComponent>();
                        if (hStaticMeshComp->GetMesh().IsNULL())
                        {
                            continue;
                        }

                        if (renderCtx.CameraFrustum.GetSphereSide(
                                meshWorldT.Transform(hStaticMeshComp->GetMesh()->SphereVolume)) == math::eSide::Outside)
                        {
                            continue;
                        }

                        if (!RenderingSystem::GetInstance().IsUploaded(hStaticMeshComp->GetMesh()))
                        {
                            continue;
                        }

                        submitDrawCommands(renderCtx, hStaticMeshComp);
                    }
                    break;
                    case RMeshComponent::eMeshType::Skeletal:
                        break;
                    default:
                        HO_ASSERT(false, "Invalid mesh type");
                }
            }
            RenderingSystem::GetInstance().EndSubmitDrawCommands();
            RenderingSystem::GetInstance().EndRenderPass();
        }
    }

    if (mbNeedToResizeInstanceDataBuffer)
    {
        mInstanceDataBuffer = FixedArray<InstanceData>(mInstanceDataBuffer.GetSize() * 2);
        mbNeedToResizeInstanceDataBuffer = false;
    }
}

void SceneRenderer::BakePointLightStaticShadowMap(ObjectHandle<RPointLightComponent> hPointLightComp,
                                                  ObjectHandle<const RScene> hScene) const
{
    HO_ASSERT(hPointLightComp.IsValid(), "Point light component is invalid.");
    HO_ASSERT(hScene.IsValid(), "Scene is invalid. Scene might be deleted or not created.");
    HO_ASSERT(hPointLightComp->GetOwner()->GetOwner() == hScene, "Light component dosen't belong to scene.");

    if (hPointLightComp->GetStaticShadowMap().IsValid())
    {
        AssetSystem::GetInstance().DeleteAsset(hPointLightComp->GetStaticShadowMap());
    }

    SceneRenderContext renderCtx;
    renderCtx.hScene = hScene;

    extractPointLight(&renderCtx, hPointLightComp);

    bakePointLightShadowMap(hPointLightComp, renderCtx);
}

void SceneRenderer::BakeSpotLightStaticShadowMap(ObjectHandle<RSpotLightComponent> hSpotLightComp,
                                                 ObjectHandle<const RScene> hScene) const
{
    HO_ASSERT(hSpotLightComp.IsValid(), "Spot light component is invalid.");
    HO_ASSERT(hScene.IsValid(), "Scene is invalid. Scene might be deleted or not created.");
    HO_ASSERT(hSpotLightComp->GetOwner()->GetOwner() == hScene, "Light component dosen't belong to scene.");

    if (hSpotLightComp->GetStaticShadowMap().IsValid())
    {
        AssetSystem::GetInstance().DeleteAsset(hSpotLightComp->GetStaticShadowMap());
    }

    SceneRenderContext renderCtx;
    renderCtx.hScene = hScene;

    extractSpotLight(&renderCtx, hSpotLightComp);

    bakeSpotLightShadowMap(hSpotLightComp, renderCtx);
}

bool SceneRenderer::hasToIgnoreLights(eRenderingMode renderingMode) const
{
    switch (renderingMode)
    {
        case eRenderingMode::Unlit:
        case eRenderingMode::Wireframe:
            return true;

        case eRenderingMode::Lit:
        case eRenderingMode::LitWireframe:
        case eRenderingMode::DetailLighting:
        case eRenderingMode::LightingOnly:
        case eRenderingMode::Reflection:
            return false;

        default:
            HO_ASSERT(false, "Invalid rendering mode");
            return false;
    }
}

void SceneRenderer::extractLights(SceneRenderContext* pOutRenderCtx) const
{
    HO_ASSERT(pOutRenderCtx, "Context is null.");

    for (const ObjectHandle<const RLightComponent> hLightComp : pOutRenderCtx->hScene->GetComponents<RLightComponent>())
    {
        if (!hLightComp->GetOwner()->IsVisibleInEditor() || hLightComp->IsDestroyed())
        {
            continue;
        }
        switch (hLightComp->GetType())
        {
            case RLightComponent::eLightType::Directional:
                extractDirLight(pOutRenderCtx, hLightComp.Cast<const RDirectionalLightComponent>());
                break;
            case RLightComponent::eLightType::Point:
                extractPointLight(pOutRenderCtx, hLightComp.Cast<const RPointLightComponent>());
                break;
            case RLightComponent::eLightType::Spot:
                extractSpotLight(pOutRenderCtx, hLightComp.Cast<const RSpotLightComponent>());
                break;
            default:
                HO_ASSERT(false, "Invalid light type");
                return;
        }
    }
}

void SceneRenderer::renderLightsShadowMap(const SceneRenderContext& renderCtx) const
{
    renderDirLightsShadowMap(renderCtx);
    renderPointLightsShadowMap(renderCtx);
    renderSpotLightsShadowMap(renderCtx);
}

void SceneRenderer::extractDirLight(SceneRenderContext* pOutRenderCtx,
                                    ObjectHandle<const RDirectionalLightComponent> hDirLightComp) const
{
    HO_ASSERT(pOutRenderCtx, "Context is null.");
    HO_ASSERT(hDirLightComp.IsValid(), "Light component is invalid.");

    const RenderingBuiltInAssets& builtInAssets = RenderingSystem::GetInstance().GetBuiltInAssets();

    if (pOutRenderCtx->DirectLightsData.DirLightCount >= DirectLightData::sMaxDirLightCount)
    {
        return;
    }

    pOutRenderCtx->hDirLights[pOutRenderCtx->DirectLightsData.DirLightCount] = hDirLightComp;
    DirLightData& lightData = pOutRenderCtx->DirectLightsData.DirLights[pOutRenderCtx->DirectLightsData.DirLightCount];
    lightData.Color = hDirLightComp->GetColor();
    lightData.Direction =
        hDirLightComp->GetOwner()->GetComponent<RTransformComponent>()->GetWorldTransform().GetForward();
    lightData.Intensity = hDirLightComp->GetInstensity();
    lightData.ShadowDataIndex = -1;

    const int32_t currShadowIdx = pOutRenderCtx->DirectLightsShadowData.DirLightShadowDataCount;
    if (hDirLightComp->IsCastingShadow() && currShadowIdx < DirectLightShadowData::sMaxDirLightShadowDataCount)
    {
        pOutRenderCtx->hShadowCastingDirLights[currShadowIdx] = hDirLightComp;
        lightData.ShadowDataIndex = currShadowIdx;

        DirLightShadowData& shadowData = pOutRenderCtx->DirectLightsShadowData.DirLightsShadowData[currShadowIdx];
        const Vector3 lightDir =
            hDirLightComp->GetOwner()->GetComponent<RTransformComponent>()->GetWorldTransform().GetForward();

        Vector3 frustumCornersInWorldSpace[8] = {};
        Frustum::GetFrustumCorners(frustumCornersInWorldSpace,
                                   pOutRenderCtx->CameraProj.GetMatrix() * pOutRenderCtx->CameraViewMat);
        const Sphere worldFrustumBV = Sphere::FromPositionsRitter(frustumCornersInWorldSpace, 8);
        Sphere castersBV = Sphere(worldFrustumBV.Center, 1.0f);
        for (const ObjectHandle<const RMeshComponent> hMeshComp :
             pOutRenderCtx->hScene->GetComponents<RMeshComponent>())
        {
            if (hMeshComp->GetShadowCastingMode() == RMeshComponent::eShadowCastingMode::None ||
                !hMeshComp->GetOwner()->IsVisibleInEditor())
            {
                continue;
            }

            const Transform3D& meshWorldT =
                hMeshComp->GetOwner()->GetComponent<RTransformComponent>()->GetWorldTransform();

            switch (hMeshComp->GetType())
            {
                case RMeshComponent::eMeshType::Static:
                {
                    const ObjectHandle<const RStaticMeshComponent> hStaticMeshComp =
                        hMeshComp.Cast<RStaticMeshComponent>();
                    if (hStaticMeshComp->GetMesh().IsNULL())
                    {
                        continue;
                    }

                    const Sphere meshWorldBV = meshWorldT.Transform(hStaticMeshComp->GetMesh()->SphereVolume);

                    if (worldFrustumBV.GetSide(meshWorldBV) == math::eSide::Outside)
                    {
                        continue;
                    }

                    castersBV = Sphere::Merge(castersBV, meshWorldBV);
                }
                break;
                case RMeshComponent::eMeshType::Skeletal:
                    break;
                default:
                    HO_ASSERT(false, "Invalid mesh type");
            }
        }

        pOutRenderCtx->DirLightsWorldPos[currShadowIdx] =
            castersBV.Center - 10.0f * pOutRenderCtx->CameraProj.GetFarDistance() * lightDir;
        pOutRenderCtx->DirLightsCastersBV[currShadowIdx] = castersBV;
        int32_t cascadeCount = hDirLightComp->GetCascadeCount();
        shadowData.CascadeCount = cascadeCount;

        float totalNearDist = pOutRenderCtx->CameraProj.GetNearDistance();
        float totalFarDist = hDirLightComp->GetMaxShadowDistance();
        float lambda = 0.75f;

        Vector3 splitFrustumCornersInWorldSpace[8] = {};

        auto getCascadeSplitDist = [&](int32_t idx) -> float
        {
            if (idx == 0)
            {
                return totalNearDist;
            }
            if (idx == cascadeCount)
            {
                return totalFarDist;
            }

            const float p = static_cast<float>(idx) / static_cast<float>(cascadeCount);

            const float logSplit = totalNearDist * math::Pow(totalFarDist / totalNearDist, p);

            const float linearSplit = totalNearDist + (totalFarDist - totalNearDist) * p;

            return lambda * logSplit + (1.0f - lambda) * linearSplit;
        };

        const AABB casterBVInWorldSpace =
            AABB(castersBV.Center - Vector3(castersBV.Radius, castersBV.Radius, castersBV.Radius),
                 castersBV.Center + Vector3(castersBV.Radius, castersBV.Radius, castersBV.Radius));

        for (int32_t cascadeIdx = 0; cascadeIdx < cascadeCount; ++cascadeIdx)
        {
            const float splitNear = getCascadeSplitDist(cascadeIdx);
            const float splitFar = getCascadeSplitDist(cascadeIdx + 1);

            Projection splitProjection = Projection::CreatePerspective(0.0f, 0.0f, 0.0f, 1.0f);
            if (pOutRenderCtx->CameraProj.GetType() == eProjectionType::Perspective)
            {
                splitProjection = Projection::CreatePerspective(pOutRenderCtx->CameraProj.GetFOV(),
                                                                pOutRenderCtx->CameraProj.GetAspectRatio(),
                                                                splitNear,
                                                                splitFar);
            }
            else
            {
                splitProjection = Projection::CreateOrthographic(pOutRenderCtx->CameraProj.GetHeight(),
                                                                 pOutRenderCtx->CameraProj.GetAspectRatio(),
                                                                 splitNear,
                                                                 splitFar);
            }

            const Matrix4x4 splitProjViewMat = splitProjection.GetMatrix() * pOutRenderCtx->CameraViewMat;

            Frustum::GetFrustumCorners(splitFrustumCornersInWorldSpace, splitProjViewMat);

            Vector3 splitFrustumCenterInWorldSpace(0.0f, 0.0f, 0.0f);
            for (const auto& corner : splitFrustumCornersInWorldSpace)
            {
                splitFrustumCenterInWorldSpace += corner;
            }
            splitFrustumCenterInWorldSpace /= 8.0f;

            const Transform3D lightViewT =
                Transform3D::FromPosition(splitFrustumCenterInWorldSpace)
                    .LookedAt(splitFrustumCenterInWorldSpace + lightDir, Vector3::sUnitY, true)
                    .InverseFast();

            Vector3 splitFrustumCornersInLightSpace[8] = {};
            for (int32_t i = 0; i < 8; ++i)
            {
                splitFrustumCornersInLightSpace[i] = Vector3(lightViewT.Transform(splitFrustumCornersInWorldSpace[i]));
            }

            AABB splitFrustumBVInLightSpace = AABB::FromPositions(splitFrustumCornersInLightSpace, 8);
            const AABB casterBVInLightSpace = lightViewT.Transform(casterBVInWorldSpace);

            const bool overlapsXY = casterBVInLightSpace.MaxEdges.X >= splitFrustumBVInLightSpace.MinEdges.X &&
                                    casterBVInLightSpace.MinEdges.X <= splitFrustumBVInLightSpace.MaxEdges.X &&
                                    casterBVInLightSpace.MaxEdges.Y >= splitFrustumBVInLightSpace.MinEdges.Y &&
                                    casterBVInLightSpace.MinEdges.Y <= splitFrustumBVInLightSpace.MaxEdges.Y;

            if (overlapsXY)
            {
                splitFrustumBVInLightSpace.MinEdges.X =
                    math::Max(casterBVInLightSpace.MinEdges.X, splitFrustumBVInLightSpace.MinEdges.X);
                splitFrustumBVInLightSpace.MaxEdges.X =
                    math::Min(casterBVInLightSpace.MaxEdges.X, splitFrustumBVInLightSpace.MaxEdges.X);
                splitFrustumBVInLightSpace.MinEdges.Y =
                    math::Max(casterBVInLightSpace.MinEdges.Y, splitFrustumBVInLightSpace.MinEdges.Y);
                splitFrustumBVInLightSpace.MaxEdges.Y =
                    math::Min(casterBVInLightSpace.MaxEdges.Y, splitFrustumBVInLightSpace.MaxEdges.Y);
            }

            splitFrustumBVInLightSpace.MinEdges.Z =
                math::Min(splitFrustumBVInLightSpace.MinEdges.Z, casterBVInLightSpace.MinEdges.Z);
            splitFrustumBVInLightSpace.MaxEdges.Z =
                math::Max(splitFrustumBVInLightSpace.MaxEdges.Z, casterBVInLightSpace.MaxEdges.Z);

            const Vector3 toAabbCenterTransVec(
                (splitFrustumBVInLightSpace.MinEdges.X + splitFrustumBVInLightSpace.MaxEdges.X) * 0.5f,
                (splitFrustumBVInLightSpace.MinEdges.Y + splitFrustumBVInLightSpace.MaxEdges.Y) * 0.5f,
                0.0f);

            const Matrix4x4 lightViewMat = lightViewT.Translated(-toAabbCenterTransVec).Matrix;

            const float viewWidth = splitFrustumBVInLightSpace.MaxEdges.X - splitFrustumBVInLightSpace.MinEdges.X;
            const float viewHeight = splitFrustumBVInLightSpace.MaxEdges.Y - splitFrustumBVInLightSpace.MinEdges.Y;
            const float nearDist = splitFrustumBVInLightSpace.MinEdges.Z;
            const float farDist = splitFrustumBVInLightSpace.MaxEdges.Z;

            const Matrix4x4 lightProjMat =
                Projection::CreateOrthographic(viewHeight, viewWidth / viewHeight, nearDist, farDist).GetMatrix();

            shadowData.CascadeNormalOffsetScales[cascadeIdx] =
                math::Max(viewWidth / RenderingSystem::sDirLightShadowMapSize,
                          viewHeight / RenderingSystem::sDirLightShadowMapSize);
            shadowData.CascadeDepthOffsetScales[cascadeIdx] =
                shadowData.CascadeNormalOffsetScales[cascadeIdx] / (farDist - nearDist);
            shadowData.CascadeFarDist[cascadeIdx] = splitFar;
            shadowData.ProjViewMats[cascadeIdx] = lightProjMat * lightViewMat;
        }

        shadowData.hStaticShadowMap = builtInAssets.hFallbackDirLightStaticShadowMap;
        const ObjectHandle<RTextureAsset> hShadowMap = hDirLightComp->GetStaticShadowMap();
        if (hShadowMap.IsValid() && hShadowMap->TransferState == eAssetTransferState::Ready)
        {
            if (RenderingSystem::GetInstance().IsUploaded(hShadowMap))
            {
                shadowData.hStaticShadowMap = hDirLightComp->GetStaticShadowMap()->hRenderProxy;
            }
            else
            {
                RenderingSystem::GetInstance().UploadTexture(hShadowMap);
            }
        }

        ++pOutRenderCtx->DirectLightsShadowData.DirLightShadowDataCount;
    }
    ++pOutRenderCtx->DirectLightsData.DirLightCount;
}

void SceneRenderer::renderDirLightsShadowMap(const SceneRenderContext& renderCtx) const
{
    HO_ASSERT(renderCtx.hScene.IsValid(), "Scene is invalid. Scene might be deleted or not created.");

    const RenderingBuiltInAssets& builtInAssets = RenderingSystem::GetInstance().GetBuiltInAssets();

    for (int32_t lightIdx = 0; lightIdx < renderCtx.DirectLightsShadowData.DirLightShadowDataCount; ++lightIdx)
    {
        const DirLightShadowData& shadowData = renderCtx.DirectLightsShadowData.DirLightsShadowData[lightIdx];
        DirLightShadowMapRenderPassDesc desc;
        desc.ShadowDataIndex = lightIdx;
        desc.LightWorldPos = renderCtx.DirLightsWorldPos[lightIdx];
        desc.CascadeCount = shadowData.CascadeCount;

        for (int32_t cascadeIdx = 0; cascadeIdx < shadowData.CascadeCount; ++cascadeIdx)
        {
            desc.LightProjViewMats[cascadeIdx] = shadowData.ProjViewMats[cascadeIdx];
        }

        if (RenderingSystem::GetInstance().BeginDirLightShadowMapRenderPass(desc))
        {
            // Attach stages.
            OpaqueRenderStageDesc opaqueStageDesc;
            for (bool& bBindTexture : opaqueStageDesc.bBindTexture)
            {
                bBindTexture = false;
            }
            opaqueStageDesc.hOverrideShaders[static_cast<int32_t>(eShaderStage::VertexShader)] =
                builtInAssets.hDirLightShadowMapVS;
            opaqueStageDesc.hOverrideShaders[static_cast<int32_t>(eShaderStage::FragmentShader)] =
                builtInAssets.hDirLightShadowMapFS;
            RenderingSystem::GetInstance().AttachOpaqueRenderStage(opaqueStageDesc);

            MaskedRenderStageDesc maskedStageDesc;
            for (bool& bBindTexture : maskedStageDesc.bBindTexture)
            {
                bBindTexture = false;
            }
            maskedStageDesc.bBindTexture[static_cast<int32_t>(eMaterialTextureUsage::Diffuse)] = true;
            maskedStageDesc.bBindTexture[static_cast<int32_t>(eMaterialTextureUsage::Albedo)] = true;
            maskedStageDesc.bBindTexture[static_cast<int32_t>(eMaterialTextureUsage::Opacity)] = true;
            maskedStageDesc.hOverrideShaders[static_cast<int32_t>(eShaderStage::VertexShader)] =
                builtInAssets.hDirLightShadowMapVS;
            maskedStageDesc.hOverrideShaders[static_cast<int32_t>(eShaderStage::FragmentShader)] =
                builtInAssets.hDirLightShadowMapMaskedFS;
            RenderingSystem::GetInstance().AttachMaskedRenderStage(maskedStageDesc);

            // Submit draw commands.
            RenderingSystem::GetInstance().BeginSubmitDrawCommands();
            for (const ObjectHandle<const RMeshComponent> hMeshComp : renderCtx.hScene->GetComponents<RMeshComponent>())
            {
                if (!hMeshComp->GetOwner()->IsVisibleInEditor() ||
                    hMeshComp->GetShadowCastingMode() == RMeshComponent::eShadowCastingMode::None)
                {
                    continue;
                }

                const Transform3D& meshWorldT =
                    hMeshComp->GetOwner()->GetComponent<RTransformComponent>()->GetWorldTransform();

                switch (hMeshComp->GetType())
                {
                    case RMeshComponent::eMeshType::Static:
                    {
                        const ObjectHandle<const RStaticMeshComponent> hStaticMeshComp =
                            hMeshComp.Cast<RStaticMeshComponent>();
                        if (hStaticMeshComp->GetMesh().IsNULL())
                        {
                            continue;
                        }

                        if (renderCtx.DirLightsCastersBV[lightIdx].GetSide(
                                meshWorldT.Transform(hStaticMeshComp->GetMesh()->SphereVolume)) == math::eSide::Outside)
                        {
                            continue;
                        }

                        if (!RenderingSystem::GetInstance().IsUploaded(hStaticMeshComp->GetMesh()))
                        {
                            RenderingSystem::GetInstance().UploadStaticMesh(hStaticMeshComp->GetMesh());
                            continue;
                        }

                        submitShadowMapDrawCommands(renderCtx.DirLightsCastersBV[lightIdx], hStaticMeshComp);
                    }
                    break;
                    case RMeshComponent::eMeshType::Skeletal:
                        break;
                    default:
                        HO_ASSERT(false, "Invalid mesh type");
                }
            }
            RenderingSystem::GetInstance().EndSubmitDrawCommands();
        }
        RenderingSystem::GetInstance().EndRenderPass();
    }
}

void SceneRenderer::extractPointLight(SceneRenderContext* pOutRenderCtx,
                                      ObjectHandle<const RPointLightComponent> hPointLightComp) const
{
    HO_ASSERT(pOutRenderCtx, "Context is null.");
    HO_ASSERT(hPointLightComp.IsValid(), "Light component is invalid.");

    const RenderingBuiltInAssets& builtInAssets = RenderingSystem::GetInstance().GetBuiltInAssets();

    if (pOutRenderCtx->DirectLightsData.PointLightCount >= DirectLightData::sMaxPointLightCount)
    {
        return;
    }

    pOutRenderCtx->hPointLights[pOutRenderCtx->DirectLightsData.PointLightCount] = hPointLightComp;
    PointLightData& lightData =
        pOutRenderCtx->DirectLightsData.PointLights[pOutRenderCtx->DirectLightsData.PointLightCount];
    lightData.Position =
        hPointLightComp->GetOwner()->GetComponent<RTransformComponent>()->GetWorldTransform().GetPosition();
    lightData.Color = hPointLightComp->GetColor();
    lightData.Intensity = hPointLightComp->GetInstensity();
    lightData.Constant = hPointLightComp->GetConstantAtten();
    lightData.Linear = hPointLightComp->GetLinearAtten();
    lightData.Quadratic = hPointLightComp->GetQuadraticAtten();
    lightData.FarDist = hPointLightComp->GetRadius();
    lightData.ShadowDataIndex = -1;

    const int32_t currShadowIdx = pOutRenderCtx->DirectLightsShadowData.PointLightShadowDataCount;
    if (hPointLightComp->IsCastingShadow() && currShadowIdx < DirectLightShadowData::sMaxPointLightShadowDataCount)
    {
        pOutRenderCtx->hShadowCastingPointLights[currShadowIdx] = hPointLightComp;
        lightData.ShadowDataIndex = currShadowIdx;

        PointLightShadowData& shadowData = pOutRenderCtx->DirectLightsShadowData.PointLightsShadowData[currShadowIdx];
        shadowData.LightWorldPos =
            hPointLightComp->GetOwner()->GetComponent<RTransformComponent>()->GetWorldTransform().GetPosition();
        shadowData.FarDist = hPointLightComp->GetRadius();
        shadowData.NormalOffsetScale = 2.0f / RenderingSystem::sPointLightShadowMapSize;
        shadowData.DepthOffsetScale = shadowData.NormalOffsetScale / shadowData.FarDist;
        const Matrix4x4 viewMats[6] = {
            // +X
            Matrix4x4(
                Vector4::sUnitZ,
                Vector4::sUnitY,
                -Vector4::sUnitX,
                Vector4(shadowData.LightWorldPos.Z, -shadowData.LightWorldPos.Y, -shadowData.LightWorldPos.X, 1.0f)),

            // -X
            Matrix4x4(
                -Vector4::sUnitZ,
                Vector4::sUnitY,
                Vector4::sUnitX,
                Vector4(-shadowData.LightWorldPos.Z, -shadowData.LightWorldPos.Y, shadowData.LightWorldPos.X, 1.0f)),

            // +Y
            Matrix4x4(
                Vector4::sUnitX,
                Vector4::sUnitZ,
                -Vector4::sUnitY,
                Vector4(-shadowData.LightWorldPos.X, shadowData.LightWorldPos.Z, -shadowData.LightWorldPos.Y, 1.0f)),

            // -Y
            Matrix4x4(
                Vector4::sUnitX,
                -Vector4::sUnitZ,
                Vector4::sUnitY,
                Vector4(-shadowData.LightWorldPos.X, -shadowData.LightWorldPos.Z, shadowData.LightWorldPos.Y, 1.0f)),

            // +Z
            Matrix4x4(
                Vector4::sUnitX,
                Vector4::sUnitY,
                Vector4::sUnitZ,
                Vector4(-shadowData.LightWorldPos.X, -shadowData.LightWorldPos.Y, -shadowData.LightWorldPos.Z, 1.0f)),

            // -Z
            Matrix4x4(
                -Vector4::sUnitX,
                Vector4::sUnitY,
                -Vector4::sUnitZ,
                Vector4(shadowData.LightWorldPos.X, -shadowData.LightWorldPos.Y, shadowData.LightWorldPos.Z, 1.0f)),
        };
        const Matrix4x4 projMat = Projection::CreatePerspective(math::PI2, 1.0f, 0.1f, shadowData.FarDist).GetMatrix();

        for (int32_t i = 0; i < 6; ++i)
        {
            shadowData.ProjViewMats[i] = projMat * viewMats[i];
        }

        shadowData.hStaticShadowMap = builtInAssets.hFallbackPointLightStaticShadowMap;
        const ObjectHandle<RTextureAsset> hShadowMap = hPointLightComp->GetStaticShadowMap();
        if (hShadowMap.IsValid() && hShadowMap->TransferState == eAssetTransferState::Ready)
        {
            if (RenderingSystem::GetInstance().IsUploaded(hShadowMap))
            {
                shadowData.hStaticShadowMap = hPointLightComp->GetStaticShadowMap()->hRenderProxy;
            }
            else
            {
                RenderingSystem::GetInstance().UploadTexture(hShadowMap);
            }
        }

        ++pOutRenderCtx->DirectLightsShadowData.PointLightShadowDataCount;
    }
    ++pOutRenderCtx->DirectLightsData.PointLightCount;
}

void SceneRenderer::renderPointLightsShadowMap(const SceneRenderContext& renderCtx) const
{
    HO_ASSERT(renderCtx.hScene.IsValid(), "Scene is invalid. Scene might be deleted or not created.");

    const RenderingBuiltInAssets& builtInAssets = RenderingSystem::GetInstance().GetBuiltInAssets();

    for (int32_t lightIdx = 0; lightIdx < renderCtx.DirectLightsShadowData.PointLightShadowDataCount; ++lightIdx)
    {
        const PointLightShadowData& shadowData = renderCtx.DirectLightsShadowData.PointLightsShadowData[lightIdx];
        PointLightShadowMapRenderPassDesc desc;
        desc.ShadowDataIndex = lightIdx;
        desc.LightWorldPos = shadowData.LightWorldPos;
        desc.FarDist = shadowData.FarDist;

        for (int32_t faceIdx = 0; faceIdx < 6; ++faceIdx)
        {
            desc.LightProjViewMats[faceIdx] = shadowData.ProjViewMats[faceIdx];
        }

        if (RenderingSystem::GetInstance().BeginPointLightShadowMapRenderPass(desc))
        {
            const Sphere lightBoundingSphere = Sphere(shadowData.LightWorldPos, shadowData.FarDist);

            // Attach stages.
            OpaqueRenderStageDesc opaqueStageDesc;
            for (bool& bBindTexture : opaqueStageDesc.bBindTexture)
            {
                bBindTexture = false;
            }
            opaqueStageDesc.hOverrideShaders[static_cast<int32_t>(eShaderStage::VertexShader)] =
                builtInAssets.hPointLightShadowMapVS;
            opaqueStageDesc.hOverrideShaders[static_cast<int32_t>(eShaderStage::FragmentShader)] =
                builtInAssets.hPointLightShadowMapFS;
            RenderingSystem::GetInstance().AttachOpaqueRenderStage(opaqueStageDesc);

            MaskedRenderStageDesc maskedStageDesc;
            for (bool& bBindTexture : maskedStageDesc.bBindTexture)
            {
                bBindTexture = false;
            }
            maskedStageDesc.bBindTexture[static_cast<int32_t>(eMaterialTextureUsage::Diffuse)] = true;
            maskedStageDesc.bBindTexture[static_cast<int32_t>(eMaterialTextureUsage::Albedo)] = true;
            maskedStageDesc.bBindTexture[static_cast<int32_t>(eMaterialTextureUsage::Opacity)] = true;
            maskedStageDesc.hOverrideShaders[static_cast<int32_t>(eShaderStage::VertexShader)] =
                builtInAssets.hPointLightShadowMapVS;
            maskedStageDesc.hOverrideShaders[static_cast<int32_t>(eShaderStage::FragmentShader)] =
                builtInAssets.hPointLightShadowMapMaskedFS;
            RenderingSystem::GetInstance().AttachMaskedRenderStage(maskedStageDesc);

            // Submit draw commands.
            RenderingSystem::GetInstance().BeginSubmitDrawCommands();
            for (const ObjectHandle<const RMeshComponent> hMeshComp : renderCtx.hScene->GetComponents<RMeshComponent>())
            {
                if (!hMeshComp->GetOwner()->IsVisibleInEditor() ||
                    hMeshComp->GetShadowCastingMode() != RMeshComponent::eShadowCastingMode::Dynamic)
                {
                    continue;
                }

                const Transform3D& meshWorldT =
                    hMeshComp->GetOwner()->GetComponent<RTransformComponent>()->GetWorldTransform();

                switch (hMeshComp->GetType())
                {
                    case RMeshComponent::eMeshType::Static:
                    {
                        const ObjectHandle<const RStaticMeshComponent> hStaticMeshComp =
                            hMeshComp.Cast<RStaticMeshComponent>();
                        if (hStaticMeshComp->GetMesh().IsNULL())
                        {
                            continue;
                        }

                        if (lightBoundingSphere.GetSide(
                                meshWorldT.Transform(hStaticMeshComp->GetMesh()->SphereVolume)) == math::eSide::Outside)
                        {
                            continue;
                        }

                        if (!RenderingSystem::GetInstance().IsUploaded(hStaticMeshComp->GetMesh()))
                        {
                            RenderingSystem::GetInstance().UploadStaticMesh(hStaticMeshComp->GetMesh());
                            continue;
                        }

                        submitShadowMapDrawCommands(lightBoundingSphere, hStaticMeshComp);
                    }
                    break;
                    case RMeshComponent::eMeshType::Skeletal:
                        break;
                    default:
                        HO_ASSERT(false, "Invalid mesh type");
                }
            }
            RenderingSystem::GetInstance().EndSubmitDrawCommands();
        }
        RenderingSystem::GetInstance().EndRenderPass();
    }
}

void SceneRenderer::bakePointLightShadowMap(ObjectHandle<RPointLightComponent> hPointLightComp,
                                            const SceneRenderContext& renderCtx) const
{
    HO_ASSERT(hPointLightComp.IsValid(), "Light component is invalid.");
    HO_ASSERT(renderCtx.hScene.IsValid(), "Scene is invalid. Scene might be deleted or not created.");

    const RenderingBuiltInAssets& builtInAssets = RenderingSystem::GetInstance().GetBuiltInAssets();

    const PointLightShadowData& shadowData = renderCtx.DirectLightsShadowData.PointLightsShadowData[0];

    char buf[256] = {};
    snprintf(buf,
             HO_ARRAY_SIZE(buf),
             "%s%s%s%s",
             *renderCtx.hScene->GetName(),
             "::TextureCubeMap::",
             *hPointLightComp->GetOwner()->GetName(),
             "StaticShadowMap");
    const ObjectHandle<RTextureAsset> hStaticShadowMap =
        AssetSystem::GetInstance().CreateAsset<RTextureAsset>(StringHandle(buf));
    hStaticShadowMap->TransferState = eAssetTransferState::PendingDownload;
    hPointLightComp->SetStaticShadowMap(hStaticShadowMap);

    ShadowMapBakePassDesc bakeDesc;
    bakeDesc.ShadowMapType = eShadowMapType::PointLightShadowMap;
    bakeDesc.LightWorldPos = shadowData.LightWorldPos;
    bakeDesc.FarDist = shadowData.FarDist;
    for (int32_t faceIdx = 0; faceIdx < 6; ++faceIdx)
    {
        bakeDesc.LightProjViewMats[faceIdx] = shadowData.ProjViewMats[faceIdx];
    }
    bakeDesc.hDstTexture = hStaticShadowMap;

    if (RenderingSystem::GetInstance().BeginShadowMapBakePass(bakeDesc))
    {
        const Sphere lightBoundingSphere = Sphere(shadowData.LightWorldPos, shadowData.FarDist);

        // Attach stages.
        OpaqueRenderStageDesc opaqueStageDesc;
        for (bool& bBindTexture : opaqueStageDesc.bBindTexture)
        {
            bBindTexture = false;
        }
        opaqueStageDesc.hOverrideShaders[static_cast<int32_t>(eShaderStage::VertexShader)] =
            builtInAssets.hPointLightShadowMapVS;
        opaqueStageDesc.hOverrideShaders[static_cast<int32_t>(eShaderStage::FragmentShader)] =
            builtInAssets.hPointLightShadowMapFS;
        RenderingSystem::GetInstance().AttachOpaqueRenderStage(opaqueStageDesc);

        MaskedRenderStageDesc maskedStageDesc;
        for (bool& bBindTexture : maskedStageDesc.bBindTexture)
        {
            bBindTexture = false;
        }
        maskedStageDesc.bBindTexture[static_cast<int32_t>(eMaterialTextureUsage::Diffuse)] = true;
        maskedStageDesc.bBindTexture[static_cast<int32_t>(eMaterialTextureUsage::Albedo)] = true;
        maskedStageDesc.bBindTexture[static_cast<int32_t>(eMaterialTextureUsage::Opacity)] = true;
        maskedStageDesc.hOverrideShaders[static_cast<int32_t>(eShaderStage::VertexShader)] =
            builtInAssets.hPointLightShadowMapVS;
        maskedStageDesc.hOverrideShaders[static_cast<int32_t>(eShaderStage::FragmentShader)] =
            builtInAssets.hPointLightShadowMapMaskedFS;
        RenderingSystem::GetInstance().AttachMaskedRenderStage(maskedStageDesc);

        // Submit draw commands.
        RenderingSystem::GetInstance().BeginSubmitDrawCommands();
        for (const ObjectHandle<const RMeshComponent> hMeshComp : renderCtx.hScene->GetComponents<RMeshComponent>())
        {
            if (hMeshComp->GetShadowCastingMode() != RMeshComponent::eShadowCastingMode::Static)
            {
                continue;
            }

            const Transform3D& meshWorldT =
                hMeshComp->GetOwner()->GetComponent<RTransformComponent>()->GetWorldTransform();

            switch (hMeshComp->GetType())
            {
                case RMeshComponent::eMeshType::Static:
                {
                    const ObjectHandle<const RStaticMeshComponent> hStaticMeshComp =
                        hMeshComp.Cast<RStaticMeshComponent>();
                    if (hStaticMeshComp->GetMesh().IsNULL())
                    {
                        continue;
                    }

                    if (lightBoundingSphere.GetSide(meshWorldT.Transform(hStaticMeshComp->GetMesh()->SphereVolume)) ==
                        math::eSide::Outside)
                    {
                        continue;
                    }

                    if (!RenderingSystem::GetInstance().IsUploaded(hStaticMeshComp->GetMesh()))
                    {
                        RenderingSystem::GetInstance().UploadStaticMesh(hStaticMeshComp->GetMesh());
                        continue;
                    }

                    submitShadowMapDrawCommands(lightBoundingSphere, hStaticMeshComp);
                }
                break;
                case RMeshComponent::eMeshType::Skeletal:
                    break;
                default:
                    HO_ASSERT(false, "Invalid mesh type");
            }
        }
        RenderingSystem::GetInstance().EndSubmitDrawCommands();
    }
    RenderingSystem::GetInstance().EndRenderPass();
}

void SceneRenderer::extractSpotLight(SceneRenderContext* pOutRenderCtx,
                                     ObjectHandle<const RSpotLightComponent> hSpotLightComp) const
{
    HO_ASSERT(pOutRenderCtx, "Context is null.");
    HO_ASSERT(hSpotLightComp.IsValid(), "Light component is invalid.");

    const RenderingBuiltInAssets& builtInAssets = RenderingSystem::GetInstance().GetBuiltInAssets();

    if (pOutRenderCtx->DirectLightsData.SpotLightCount >= DirectLightData::sMaxSpotLightCount)
    {
        return;
    }

    pOutRenderCtx->hSpotLights[pOutRenderCtx->DirectLightsData.SpotLightCount] = hSpotLightComp;
    SpotLightData& lightData =
        pOutRenderCtx->DirectLightsData.SpotLights[pOutRenderCtx->DirectLightsData.SpotLightCount];
    lightData.Position =
        hSpotLightComp->GetOwner()->GetComponent<RTransformComponent>()->GetWorldTransform().GetPosition();
    lightData.Direction =
        hSpotLightComp->GetOwner()->GetComponent<RTransformComponent>()->GetWorldTransform().GetForward();
    lightData.Color = hSpotLightComp->GetColor();
    lightData.Intensity = hSpotLightComp->GetInstensity();
    lightData.InnerCutoff = math::Cos(hSpotLightComp->GetInnerCutoff());
    lightData.OuterCutoff = math::Cos(hSpotLightComp->GetOuterCutoff());
    lightData.Constant = hSpotLightComp->GetConstantAtten();
    lightData.Linear = hSpotLightComp->GetLinearAtten();
    lightData.Quadratic = hSpotLightComp->GetQuadraticAtten();
    lightData.FarDist = hSpotLightComp->GetRadius();
    lightData.ShadowDataIndex = -1;

    const int32_t currShadowIdx = pOutRenderCtx->DirectLightsShadowData.SpotLightShadowDataCount;
    if (hSpotLightComp->IsCastingShadow() && currShadowIdx < DirectLightShadowData::sMaxSpotLightShadowDataCount)
    {
        pOutRenderCtx->hShadowCastingSpotLights[currShadowIdx] = hSpotLightComp;
        lightData.ShadowDataIndex = currShadowIdx;

        SpotLightShadowData& shadowData = pOutRenderCtx->DirectLightsShadowData.SpotLightsShadowData[currShadowIdx];
        const float spotLightFOV = math::Min(math::DegToRad(175.0f), hSpotLightComp->GetOuterCutoff() * 2.0f + 0.1f);
        shadowData.LightWorldPos =
            hSpotLightComp->GetOwner()->GetComponent<RTransformComponent>()->GetWorldTransform().GetPosition();
        shadowData.FarDist = hSpotLightComp->GetRadius();
        shadowData.NormalOffsetScale =
            (2.0f * math::Tan(spotLightFOV * 0.5f)) / RenderingSystem::sSpotLightShadowMapSize;
        shadowData.DepthOffsetScale = shadowData.NormalOffsetScale / shadowData.FarDist;
        const Vector3 lightDir =
            hSpotLightComp->GetOwner()->GetComponent<RTransformComponent>()->GetWorldTransform().GetForward();
        const Transform3D lightWorldT = Transform3D::FromPosition(shadowData.LightWorldPos)
                                            .LookedAt(shadowData.LightWorldPos + lightDir, Vector3::sUnitY, true);
        const Matrix4x4 lightViewMat = lightWorldT.InverseFast().Matrix;

        const float nearPlane = 0.1f;
        const float farPlane = shadowData.FarDist;

        const Matrix4x4 lightProjMat =
            Projection::CreatePerspective(spotLightFOV, 1.0f, nearPlane, farPlane).GetMatrix();

        shadowData.ProjViewMat = lightProjMat * lightViewMat;

        shadowData.hStaticShadowMap = builtInAssets.hFallbackSpotLightStaticShadowMap;
        const ObjectHandle<RTextureAsset> hShadowMap = hSpotLightComp->GetStaticShadowMap();
        if (hShadowMap.IsValid() && hShadowMap->TransferState == eAssetTransferState::Ready)
        {
            if (RenderingSystem::GetInstance().IsUploaded(hShadowMap))
            {
                shadowData.hStaticShadowMap = hSpotLightComp->GetStaticShadowMap()->hRenderProxy;
            }
            else
            {
                RenderingSystem::GetInstance().UploadTexture(hShadowMap);
            }
        }

        ++pOutRenderCtx->DirectLightsShadowData.SpotLightShadowDataCount;
    }
    ++pOutRenderCtx->DirectLightsData.SpotLightCount;
}

void SceneRenderer::renderSpotLightsShadowMap(const SceneRenderContext& renderCtx) const
{
    HO_ASSERT(renderCtx.hScene.IsValid(), "Scene is invalid. Scene might be deleted or not created.");

    const RenderingBuiltInAssets& builtInAssets = RenderingSystem::GetInstance().GetBuiltInAssets();

    for (int32_t lightIdx = 0; lightIdx < renderCtx.DirectLightsShadowData.SpotLightShadowDataCount; ++lightIdx)
    {
        const SpotLightShadowData& shadowData = renderCtx.DirectLightsShadowData.SpotLightsShadowData[lightIdx];

        SpotLightShadowMapRenderPassDesc desc;
        desc.ShadowDataIndex = lightIdx;
        desc.LightWorldPos = shadowData.LightWorldPos;
        desc.FarDist = shadowData.FarDist;
        desc.LightProjViewMat = shadowData.ProjViewMat;

        if (RenderingSystem::GetInstance().BeginSpotLightShadowMapRenderPass(desc))
        {
            const Frustum lightFrustum = Frustum::FromMatrix4x4(desc.LightProjViewMat);

            // Attach stages.
            OpaqueRenderStageDesc opaqueStageDesc;
            for (bool& bBindTexture : opaqueStageDesc.bBindTexture)
            {
                bBindTexture = false;
            }
            opaqueStageDesc.hOverrideShaders[static_cast<int32_t>(eShaderStage::VertexShader)] =
                builtInAssets.hSpotLightShadowMapVS;
            opaqueStageDesc.hOverrideShaders[static_cast<int32_t>(eShaderStage::FragmentShader)] =
                builtInAssets.hSpotLightShadowMapFS;
            RenderingSystem::GetInstance().AttachOpaqueRenderStage(opaqueStageDesc);

            MaskedRenderStageDesc maskedStageDesc;
            for (bool& bBindTexture : maskedStageDesc.bBindTexture)
            {
                bBindTexture = false;
            }
            maskedStageDesc.bBindTexture[static_cast<int32_t>(eMaterialTextureUsage::Diffuse)] = true;
            maskedStageDesc.bBindTexture[static_cast<int32_t>(eMaterialTextureUsage::Albedo)] = true;
            maskedStageDesc.bBindTexture[static_cast<int32_t>(eMaterialTextureUsage::Opacity)] = true;
            maskedStageDesc.hOverrideShaders[static_cast<int32_t>(eShaderStage::VertexShader)] =
                builtInAssets.hSpotLightShadowMapVS;
            maskedStageDesc.hOverrideShaders[static_cast<int32_t>(eShaderStage::FragmentShader)] =
                builtInAssets.hSpotLightShadowMapMaskedFS;
            RenderingSystem::GetInstance().AttachMaskedRenderStage(maskedStageDesc);

            // Submit draw commands.
            RenderingSystem::GetInstance().BeginSubmitDrawCommands();
            for (const ObjectHandle<const RMeshComponent> hMeshComp : renderCtx.hScene->GetComponents<RMeshComponent>())
            {
                if (!hMeshComp->GetOwner()->IsVisibleInEditor() ||
                    hMeshComp->GetShadowCastingMode() != RMeshComponent::eShadowCastingMode::Dynamic)
                {
                    continue;
                }

                const Transform3D& meshWorldT =
                    hMeshComp->GetOwner()->GetComponent<RTransformComponent>()->GetWorldTransform();

                switch (hMeshComp->GetType())
                {
                    case RMeshComponent::eMeshType::Static:
                    {
                        const ObjectHandle<const RStaticMeshComponent> hStaticMeshComp =
                            hMeshComp.Cast<RStaticMeshComponent>();
                        if (hStaticMeshComp->GetMesh().IsNULL())
                        {
                            continue;
                        }

                        if (lightFrustum.GetSphereSide(
                                meshWorldT.Transform(hStaticMeshComp->GetMesh()->SphereVolume)) == math::eSide::Outside)
                        {
                            continue;
                        }

                        if (!RenderingSystem::GetInstance().IsUploaded(hStaticMeshComp->GetMesh()))
                        {
                            RenderingSystem::GetInstance().UploadStaticMesh(hStaticMeshComp->GetMesh());
                            continue;
                        }

                        submitShadowMapDrawCommands(lightFrustum, hStaticMeshComp);
                    }
                    break;
                    case RMeshComponent::eMeshType::Skeletal:
                        break;
                    default:
                        HO_ASSERT(false, "Invalid mesh type");
                }
            }
            RenderingSystem::GetInstance().EndSubmitDrawCommands();
        }
        RenderingSystem::GetInstance().EndRenderPass();
    }
}

void SceneRenderer::bakeSpotLightShadowMap(ObjectHandle<RSpotLightComponent> hSpotLightComp,
                                           const SceneRenderContext& renderCtx) const
{
    HO_ASSERT(hSpotLightComp.IsValid(), "Light component is invalid.");
    HO_ASSERT(renderCtx.hScene.IsValid(), "Scene is invalid. Scene might be deleted or not created.");

    const RenderingBuiltInAssets& builtInAssets = RenderingSystem::GetInstance().GetBuiltInAssets();

    const SpotLightShadowData& shadowData = renderCtx.DirectLightsShadowData.SpotLightsShadowData[0];

    char buf[256] = {};
    snprintf(buf,
             HO_ARRAY_SIZE(buf),
             "%s%s%s%s",
             *renderCtx.hScene->GetName(),
             "::Texture2D::",
             *hSpotLightComp->GetOwner()->GetName(),
             "StaticShadowMap");
    const ObjectHandle<RTextureAsset> hStaticShadowMap =
        AssetSystem::GetInstance().CreateAsset<RTextureAsset>(StringHandle(buf));
    hStaticShadowMap->TransferState = eAssetTransferState::PendingDownload;
    hSpotLightComp->SetStaticShadowMap(hStaticShadowMap);

    ShadowMapBakePassDesc bakeDesc;
    bakeDesc.ShadowMapType = eShadowMapType::SpotLightShadowMap;
    bakeDesc.LightWorldPos = shadowData.LightWorldPos;
    bakeDesc.FarDist = shadowData.FarDist;
    bakeDesc.LightProjViewMats[0] = shadowData.ProjViewMat;
    bakeDesc.hDstTexture = hStaticShadowMap;

    if (RenderingSystem::GetInstance().BeginShadowMapBakePass(bakeDesc))
    {
        const Frustum lightFrustum = Frustum::FromMatrix4x4(bakeDesc.LightProjViewMats[0]);

        // Attach stages.
        OpaqueRenderStageDesc opaqueStageDesc;
        for (bool& bBindTexture : opaqueStageDesc.bBindTexture)
        {
            bBindTexture = false;
        }
        opaqueStageDesc.hOverrideShaders[static_cast<int32_t>(eShaderStage::VertexShader)] =
            builtInAssets.hSpotLightShadowMapVS;
        opaqueStageDesc.hOverrideShaders[static_cast<int32_t>(eShaderStage::FragmentShader)] =
            builtInAssets.hSpotLightShadowMapFS;
        RenderingSystem::GetInstance().AttachOpaqueRenderStage(opaqueStageDesc);

        MaskedRenderStageDesc maskedStageDesc;
        for (bool& bBindTexture : maskedStageDesc.bBindTexture)
        {
            bBindTexture = false;
        }
        maskedStageDesc.bBindTexture[static_cast<int32_t>(eMaterialTextureUsage::Diffuse)] = true;
        maskedStageDesc.bBindTexture[static_cast<int32_t>(eMaterialTextureUsage::Albedo)] = true;
        maskedStageDesc.bBindTexture[static_cast<int32_t>(eMaterialTextureUsage::Opacity)] = true;
        maskedStageDesc.hOverrideShaders[static_cast<int32_t>(eShaderStage::VertexShader)] =
            builtInAssets.hSpotLightShadowMapVS;
        maskedStageDesc.hOverrideShaders[static_cast<int32_t>(eShaderStage::FragmentShader)] =
            builtInAssets.hSpotLightShadowMapMaskedFS;
        RenderingSystem::GetInstance().AttachMaskedRenderStage(maskedStageDesc);

        // Submit draw commands.
        RenderingSystem::GetInstance().BeginSubmitDrawCommands();
        for (const ObjectHandle<const RMeshComponent> hMeshComp : renderCtx.hScene->GetComponents<RMeshComponent>())
        {
            if (hMeshComp->GetShadowCastingMode() != RMeshComponent::eShadowCastingMode::Static)
            {
                continue;
            }

            const Transform3D& meshWorldT =
                hMeshComp->GetOwner()->GetComponent<RTransformComponent>()->GetWorldTransform();

            switch (hMeshComp->GetType())
            {
                case RMeshComponent::eMeshType::Static:
                {
                    const ObjectHandle<const RStaticMeshComponent> hStaticMeshComp =
                        hMeshComp.Cast<RStaticMeshComponent>();
                    if (hStaticMeshComp->GetMesh().IsNULL())
                    {
                        continue;
                    }

                    if (lightFrustum.GetSphereSide(meshWorldT.Transform(hStaticMeshComp->GetMesh()->SphereVolume)) ==
                        math::eSide::Outside)
                    {
                        continue;
                    }

                    if (!RenderingSystem::GetInstance().IsUploaded(hStaticMeshComp->GetMesh()))
                    {
                        RenderingSystem::GetInstance().UploadStaticMesh(hStaticMeshComp->GetMesh());
                        continue;
                    }

                    submitShadowMapDrawCommands(lightFrustum, hStaticMeshComp);
                }
                break;
                case RMeshComponent::eMeshType::Skeletal:
                    break;
                default:
                    HO_ASSERT(false, "Invalid mesh type");
            }
        }

        RenderingSystem::GetInstance().EndSubmitDrawCommands();
    }
    RenderingSystem::GetInstance().EndRenderPass();
}

void SceneRenderer::renderLightsDebugMesh(const SceneRenderContext& renderCtx) const
{
    for (int32_t i = 0; i < renderCtx.DirectLightsData.DirLightCount; ++i)
    {
        submitLightDebugDrawCommands(renderCtx, renderCtx.hDirLights[i]);
    }
    for (int32_t i = 0; i < renderCtx.DirectLightsData.PointLightCount; ++i)
    {
        submitLightDebugDrawCommands(renderCtx, renderCtx.hPointLights[i]);
    }
    for (int32_t i = 0; i < renderCtx.DirectLightsData.SpotLightCount; ++i)
    {
        submitLightDebugDrawCommands(renderCtx, renderCtx.hSpotLights[i]);
    }
}

void SceneRenderer::renderCamerasDebugMesh(const SceneRenderContext& renderCtx) const
{
    for (const auto& hCameraComp : renderCtx.hScene->GetComponents<RCameraComponent>())
    {
        submitCameraDebugDrawCommands(renderCtx, hCameraComp);
    }
}

ForwardRenderPassDesc SceneRenderer::constructForwardPassDesc(const SceneRenderContext& renderCtx) const
{
    const RenderingBuiltInAssets& builtInAssets = RenderingSystem::GetInstance().GetBuiltInAssets();

    ForwardRenderPassDesc desc;
    desc.hFrameBufferName = renderCtx.hFrameBufferName;

    desc.bClearColorTarget = true;
    desc.bClearDepthTarget = true;
    desc.bClearStencilTarget = true;
    desc.ClearColor = Color128(0.0f, 0.0f, 0.0f);
    desc.ClearDepth = 1.0f;
    desc.ClearStencil = 0;

    const Vector2 frameBufferSize = RenderingSystem::GetInstance().GetFrameBufferSize(renderCtx.hFrameBufferName);
    if (frameBufferSize == Vector2::sZero)
    {
        HO_ASSERT(false, "This frame buffer was created incompletely.");
        return desc;
    }

    desc.Viewport.X = 0;
    desc.Viewport.Y = 0;
    desc.Viewport.Width = frameBufferSize.X;
    desc.Viewport.Height = frameBufferSize.Y;

    desc.WorldCameraPos = renderCtx.CameraPos;
    desc.ViewMat = renderCtx.CameraViewMat;
    desc.ProjMat = renderCtx.CameraProj.GetMatrix();

    desc.DirectLightsData = renderCtx.DirectLightsData;
    desc.DirectLightsShadowData = renderCtx.DirectLightsShadowData;

    desc.hIrradianceMap = GpuTextureHandle::sNULL;
    desc.hPrefilteredEnvMap = GpuTextureHandle::sNULL;

    switch (renderCtx.RenderingMode)
    {
        case eRenderingMode::Unlit:
        case eRenderingMode::Wireframe:
            desc.hIrradianceMap = builtInAssets.hBlackCubeMap;
            desc.hPrefilteredEnvMap = builtInAssets.hBlackCubeMap;
            break;
        case eRenderingMode::Lit:
        case eRenderingMode::LitWireframe:
        case eRenderingMode::DetailLighting:
        case eRenderingMode::LightingOnly:
        case eRenderingMode::Reflection:
            // Do nothing.
            break;
        default:
            HO_ASSERT(false, "Invalid rendering mode");
    }

    if (desc.hIrradianceMap.IsNULL())
    {
        if (renderCtx.hScene->GetMainSkyBox().IsNULL() ||
            !renderCtx.hScene->GetMainSkyBox()->GetOwner()->IsVisibleInEditor() ||
            renderCtx.hScene->GetMainSkyBox()->GetIrradianceMap().IsNULL())
        {
            desc.hIrradianceMap = builtInAssets.hGrayCubeMap;
        }
        else
        {
            if (RenderingSystem::GetInstance().IsUploaded(renderCtx.hScene->GetMainSkyBox()->GetIrradianceMap()))
            {
                desc.hIrradianceMap = renderCtx.hScene->GetMainSkyBox()->GetIrradianceMap()->hRenderProxy;
            }
            else
            {
                RenderingSystem::GetInstance().UploadTexture(renderCtx.hScene->GetMainSkyBox()->GetIrradianceMap());
                desc.hIrradianceMap = builtInAssets.hGrayCubeMap;
            }
        }
    }

    if (desc.hPrefilteredEnvMap.IsNULL())
    {
        if (renderCtx.hScene->GetMainSkyBox().IsNULL() ||
            !renderCtx.hScene->GetMainSkyBox()->GetOwner()->IsVisibleInEditor() ||
            renderCtx.hScene->GetMainSkyBox()->GetPrefilteredEnvMap().IsNULL())
        {
            desc.hPrefilteredEnvMap = builtInAssets.hBlackCubeMap;
        }
        else
        {
            if (RenderingSystem::GetInstance().IsUploaded(renderCtx.hScene->GetMainSkyBox()->GetPrefilteredEnvMap()))
            {
                desc.hPrefilteredEnvMap = renderCtx.hScene->GetMainSkyBox()->GetPrefilteredEnvMap()->hRenderProxy;
            }
            else
            {
                RenderingSystem::GetInstance().UploadTexture(renderCtx.hScene->GetMainSkyBox()->GetPrefilteredEnvMap());
                desc.hPrefilteredEnvMap = builtInAssets.hBlackCubeMap;
            }
        }
    }

    if (renderCtx.hScene->GetMainSkyBox().IsValid() &&
        renderCtx.hScene->GetMainSkyBox()->GetOwner()->IsVisibleInEditor())
    {
        desc.IndirectLights.Intensity = renderCtx.hScene->GetMainSkyBox()->GetInstensity();
    }

    return desc;
}

OpaqueRenderStageDesc SceneRenderer::constructOpaqueStageDescForShadowMapPass() const
{
    const RenderingBuiltInAssets& builtInAssets = RenderingSystem::GetInstance().GetBuiltInAssets();

    OpaqueRenderStageDesc desc;
    for (bool& bBindTexture : desc.bBindTexture)
    {
        bBindTexture = false;
    }

    desc.hOverrideShaders[static_cast<int32_t>(eShaderStage::VertexShader)] = builtInAssets.hDirLightShadowMapVS;
    desc.hOverrideShaders[static_cast<int32_t>(eShaderStage::FragmentShader)] = builtInAssets.hDirLightShadowMapFS;

    return desc;
}

MaskedRenderStageDesc SceneRenderer::constructMaskedStageDescForShadowMapPass() const
{
    const RenderingBuiltInAssets& builtInAssets = RenderingSystem::GetInstance().GetBuiltInAssets();

    MaskedRenderStageDesc desc;
    for (bool& bBindTexture : desc.bBindTexture)
    {
        bBindTexture = false;
    }

    desc.bBindTexture[static_cast<int32_t>(eMaterialTextureUsage::Diffuse)] = true;
    desc.bBindTexture[static_cast<int32_t>(eMaterialTextureUsage::Albedo)] = true;
    desc.bBindTexture[static_cast<int32_t>(eMaterialTextureUsage::Opacity)] = true;

    desc.hOverrideShaders[static_cast<int32_t>(eShaderStage::VertexShader)] = builtInAssets.hDirLightShadowMapVS;
    desc.hOverrideShaders[static_cast<int32_t>(eShaderStage::FragmentShader)] =
        builtInAssets.hDirLightShadowMapMaskedFS;

    return desc;
}

BlendedRenderStageDesc SceneRenderer::constructBlendedStageDescForShadowMapPass() const
{
    const RenderingBuiltInAssets& builtInAssets = RenderingSystem::GetInstance().GetBuiltInAssets();

    BlendedRenderStageDesc desc;
    for (bool& bBindTexture : desc.bBindTexture)
    {
        bBindTexture = false;
    }

    desc.hOverrideShaders[static_cast<int32_t>(eShaderStage::VertexShader)] = builtInAssets.hDirLightShadowMapVS;
    desc.hOverrideShaders[static_cast<int32_t>(eShaderStage::FragmentShader)] = builtInAssets.hDirLightShadowMapFS;

    return desc;
}

OpaqueRenderStageDesc SceneRenderer::constructOpaqueStageDesc(const SceneRenderContext& renderCtx) const
{
    const RenderingBuiltInAssets& builtInAssets = RenderingSystem::GetInstance().GetBuiltInAssets();

    OpaqueRenderStageDesc desc;
    const eRenderingMode renderingMode =
        renderCtx.RenderingMode == eRenderingMode::LitWireframe ? eRenderingMode::Lit : renderCtx.RenderingMode;
    switch (renderingMode)
    {
        case eRenderingMode::Unlit:
            desc.bBindTexture[static_cast<int32_t>(eMaterialTextureUsage::Specular)] = false;
            desc.bBindTexture[static_cast<int32_t>(eMaterialTextureUsage::Normal)] = false;
            desc.bBindTexture[static_cast<int32_t>(eMaterialTextureUsage::MetallicRoughness)] = false;
            desc.bBindTexture[static_cast<int32_t>(eMaterialTextureUsage::AmbientOcclusion)] = false;

            desc.hOverrideShaders[static_cast<int32_t>(eShaderStage::VertexShader)] = builtInAssets.hDefaultVS;
            desc.hOverrideShaders[static_cast<int32_t>(eShaderStage::FragmentShader)] =
                builtInAssets.hForwardUnlitOpaqueFS;

            break;
        case eRenderingMode::Lit:
            // Do nothing.
            break;
        case eRenderingMode::Wireframe:
            desc.bOverridePolygonMode = true;
            desc.OverridePipelineState.bWireframe = true;

            desc.hAttributeOverrideMaterial = builtInAssets.hDefaultMaterial;

            desc.hOverrideShaders[static_cast<int32_t>(eShaderStage::VertexShader)] = builtInAssets.hDefaultVS;
            desc.hOverrideShaders[static_cast<int32_t>(eShaderStage::FragmentShader)] =
                builtInAssets.hForwardUnlitOpaqueFS;

            for (bool& bBindTexture : desc.bBindTexture)
            {
                bBindTexture = false;
            }

            break;
        case eRenderingMode::DetailLighting:
            desc.hOverrideTextures[static_cast<int32_t>(eMaterialTextureUsage::Diffuse)] = builtInAssets.hGrayTexture;
            desc.hOverrideTextures[static_cast<int32_t>(eMaterialTextureUsage::Albedo)] = builtInAssets.hGrayTexture;

            break;
        case eRenderingMode::LightingOnly:
            desc.hAttributeOverrideMaterial = builtInAssets.hDefaultMaterial;

            for (bool& bBindTexture : desc.bBindTexture)
            {
                bBindTexture = false;
            }

            break;
        case eRenderingMode::Reflection:
            desc.hAttributeOverrideMaterial = builtInAssets.hMetallic1Roughness0Material;

            for (bool& bBindTexture : desc.bBindTexture)
            {
                bBindTexture = false;
            }

            break;
        default:
            HO_ASSERT(false, "Invalid rendering mode");
    }

    return desc;
}

MaskedRenderStageDesc SceneRenderer::constructMaskedStageDesc(const SceneRenderContext& renderCtx) const
{
    const RenderingBuiltInAssets& builtInAssets = RenderingSystem::GetInstance().GetBuiltInAssets();

    MaskedRenderStageDesc desc;
    const eRenderingMode renderingMode =
        renderCtx.RenderingMode == eRenderingMode::LitWireframe ? eRenderingMode::Lit : renderCtx.RenderingMode;
    switch (renderingMode)
    {
        case eRenderingMode::Unlit:
            desc.bBindTexture[static_cast<int32_t>(eMaterialTextureUsage::Specular)] = false;
            desc.bBindTexture[static_cast<int32_t>(eMaterialTextureUsage::Normal)] = false;
            desc.bBindTexture[static_cast<int32_t>(eMaterialTextureUsage::MetallicRoughness)] = false;
            desc.bBindTexture[static_cast<int32_t>(eMaterialTextureUsage::AmbientOcclusion)] = false;

            desc.hOverrideShaders[static_cast<int32_t>(eShaderStage::VertexShader)] = builtInAssets.hDefaultVS;
            desc.hOverrideShaders[static_cast<int32_t>(eShaderStage::FragmentShader)] =
                builtInAssets.hForwardUnlitOpaqueFS;

            break;
        case eRenderingMode::Lit:
            // Do nothing.
            break;
        case eRenderingMode::Wireframe:
            desc.bOverridePolygonMode = true;
            desc.OverridePipelineState.bWireframe = true;

            desc.hAttributeOverrideMaterial = builtInAssets.hDefaultMaterial;

            desc.hOverrideShaders[static_cast<int32_t>(eShaderStage::VertexShader)] = builtInAssets.hDefaultVS;
            desc.hOverrideShaders[static_cast<int32_t>(eShaderStage::FragmentShader)] =
                builtInAssets.hForwardUnlitOpaqueFS;

            for (bool& bBindTexture : desc.bBindTexture)
            {
                bBindTexture = false;
            }

            break;
        case eRenderingMode::DetailLighting:
            desc.hOverrideTextures[static_cast<int32_t>(eMaterialTextureUsage::Diffuse)] = builtInAssets.hGrayTexture;
            desc.hOverrideTextures[static_cast<int32_t>(eMaterialTextureUsage::Albedo)] = builtInAssets.hGrayTexture;

            break;
        case eRenderingMode::LightingOnly:
            desc.hAttributeOverrideMaterial = builtInAssets.hDefaultMaterial;

            for (bool& bBindTexture : desc.bBindTexture)
            {
                bBindTexture = false;
            }

            break;
        case eRenderingMode::Reflection:
            desc.hAttributeOverrideMaterial = builtInAssets.hMetallic1Roughness0Material;

            for (bool& bBindTexture : desc.bBindTexture)
            {
                bBindTexture = false;
            }

            break;
        default:
            HO_ASSERT(false, "Invalid rendering mode");
    }

    return desc;
}

EnvironmentRenderStageDesc SceneRenderer::constructEnvironmentStageDesc(const SceneRenderContext& renderCtx) const
{
    const RenderingBuiltInAssets& builtInAssets = RenderingSystem::GetInstance().GetBuiltInAssets();

    EnvironmentRenderStageDesc desc;
    desc.hSkyMap = GpuTextureHandle::sNULL;

    const eRenderingMode renderingMode =
        renderCtx.RenderingMode == eRenderingMode::LitWireframe ? eRenderingMode::Lit : renderCtx.RenderingMode;
    switch (renderingMode)
    {
        case eRenderingMode::Unlit:
            desc.hSkyMap = builtInAssets.hBlackCubeMap;
            break;
        case eRenderingMode::Lit:
            // Do nothing.
            break;
        case eRenderingMode::Wireframe:
            desc.hSkyMap = builtInAssets.hBlackCubeMap;
            break;
        case eRenderingMode::DetailLighting:
            break;
        case eRenderingMode::LightingOnly:
            desc.hSkyMap = builtInAssets.hBlackCubeMap;
            break;
        case eRenderingMode::Reflection:
            break;
        default:
            HO_ASSERT(false, "Invalid rendering mode");
    }

    if (desc.hSkyMap.IsNULL())
    {
        if (renderCtx.hScene->GetMainSkyBox().IsNULL() ||
            !renderCtx.hScene->GetMainSkyBox()->GetOwner()->IsVisibleInEditor() ||
            renderCtx.hScene->GetMainSkyBox()->GetSkyMap().IsNULL())
        {
            desc.hSkyMap = builtInAssets.hBlackCubeMap;
        }
        else
        {
            if (RenderingSystem::GetInstance().IsUploaded(renderCtx.hScene->GetMainSkyBox()->GetSkyMap()))
            {
                desc.hSkyMap = renderCtx.hScene->GetMainSkyBox()->GetSkyMap()->hRenderProxy;
            }
            else
            {
                RenderingSystem::GetInstance().UploadTexture(renderCtx.hScene->GetMainSkyBox()->GetSkyMap());
                desc.hSkyMap = builtInAssets.hBlackCubeMap;
            }
        }
    }

    return desc;
}

InWorldDebugRenderStageDesc SceneRenderer::constructInWorldDebugStageDesc(const SceneRenderContext& renderCtx) const
{
    const RenderingBuiltInAssets& builtInAssets = RenderingSystem::GetInstance().GetBuiltInAssets();

    InWorldDebugRenderStageDesc desc;
    desc.OverridePipelineState.bBackfaceCulling = false;
    desc.bOverrideCullMode = true;
    desc.OverridePipelineState.bWireframe = true;
    desc.bOverridePolygonMode = true;
    desc.hAttributeOverrideMaterial = builtInAssets.hDefaultMaterial;
    desc.hOverrideShaders[static_cast<int32_t>(eShaderStage::FragmentShader)] = builtInAssets.hForwardUnlitOpaqueFS;

    desc.bIsPerspective = renderCtx.CameraProj.GetType() == eProjectionType::Perspective;
    desc.bDrawWorldGizmo = renderCtx.RenderingFlags & RenderingFlags_RenderWorldGizmo;

    return desc;
}

BlendedRenderStageDesc SceneRenderer::constructBlendedStageDesc(const SceneRenderContext& renderCtx) const
{
    const RenderingBuiltInAssets& builtInAssets = RenderingSystem::GetInstance().GetBuiltInAssets();

    BlendedRenderStageDesc desc;
    const eRenderingMode renderingMode =
        renderCtx.RenderingMode == eRenderingMode::LitWireframe ? eRenderingMode::Lit : renderCtx.RenderingMode;
    switch (renderingMode)
    {
        case eRenderingMode::Unlit:
            desc.bBindTexture[static_cast<int32_t>(eMaterialTextureUsage::Specular)] = false;
            desc.bBindTexture[static_cast<int32_t>(eMaterialTextureUsage::Normal)] = false;
            desc.bBindTexture[static_cast<int32_t>(eMaterialTextureUsage::MetallicRoughness)] = false;
            desc.bBindTexture[static_cast<int32_t>(eMaterialTextureUsage::AmbientOcclusion)] = false;

            desc.hOverrideShaders[static_cast<int32_t>(eShaderStage::VertexShader)] = builtInAssets.hDefaultVS;
            desc.hOverrideShaders[static_cast<int32_t>(eShaderStage::FragmentShader)] =
                builtInAssets.hForwardUnlitOpaqueFS;

            break;
        case eRenderingMode::Lit:
            // Do nothing.
            break;
        case eRenderingMode::Wireframe:
            desc.bOverridePolygonMode = true;
            desc.OverridePipelineState.bWireframe = true;

            desc.hAttributeOverrideMaterial = builtInAssets.hDefaultMaterial;

            desc.hOverrideShaders[static_cast<int32_t>(eShaderStage::VertexShader)] = builtInAssets.hDefaultVS;
            desc.hOverrideShaders[static_cast<int32_t>(eShaderStage::FragmentShader)] =
                builtInAssets.hForwardUnlitOpaqueFS;

            for (bool& bBindTexture : desc.bBindTexture)
            {
                bBindTexture = false;
            }

            break;
        case eRenderingMode::DetailLighting:
            desc.hOverrideTextures[static_cast<int32_t>(eMaterialTextureUsage::Diffuse)] = builtInAssets.hGrayTexture;
            desc.hOverrideTextures[static_cast<int32_t>(eMaterialTextureUsage::Albedo)] = builtInAssets.hGrayTexture;

            break;
        case eRenderingMode::LightingOnly:
            desc.hAttributeOverrideMaterial = builtInAssets.hDefaultMaterial;

            for (bool& bBindTexture : desc.bBindTexture)
            {
                bBindTexture = false;
            }

            break;
        case eRenderingMode::Reflection:
            desc.hAttributeOverrideMaterial = builtInAssets.hMetallic1Roughness0Material;

            for (bool& bBindTexture : desc.bBindTexture)
            {
                bBindTexture = false;
            }

            break;
        default:
            HO_ASSERT(false, "Invalid rendering mode");
    }

    return desc;
}

OutlineRenderStageDesc SceneRenderer::constructOutlineStageDesc(const SceneRenderContext& renderCtx) const
{
    (void)renderCtx;

    OutlineRenderStageDesc desc;

    return desc;
}

PostProcessingStageDesc SceneRenderer::constructPostProcessingStageDesc(const SceneRenderContext& renderCtx) const
{
    (void)renderCtx;

    PostProcessingStageDesc desc;

    return desc;
}

WorldOverlayDebugRenderStageDesc SceneRenderer::constructWorldOverlayDebugStageDesc(
    const SceneRenderContext& renderCtx) const
{
    (void)renderCtx;
    WorldOverlayDebugRenderStageDesc desc;

    return desc;
}

void SceneRenderer::submitDrawCommands(const SceneRenderContext& renderCtx,
                                       ObjectHandle<const RStaticMeshComponent> hStaticMeshComp) const
{
    const RenderingBuiltInAssets& builtInAssets = RenderingSystem::GetInstance().GetBuiltInAssets();

    const Transform3D& meshWorldT =
        hStaticMeshComp->GetOwner()->GetComponent<RTransformComponent>()->GetWorldTransform();
    for (const auto& subMesh : hStaticMeshComp->GetMesh()->SubMeshes)
    {
        int32_t instCount = 0;
        for (int32_t i = 0; i < subMesh.LocalTransformCount; ++i)
        {
            if (instCount >= mInstanceDataBuffer.GetSize())
            {
                mbNeedToResizeInstanceDataBuffer = true;
                continue;
            }
            const Transform3D subMeshWorldT =
                meshWorldT * hStaticMeshComp->GetMesh()->LocalTransforms[subMesh.LocalTransformOffset + i];
            if (renderCtx.CameraFrustum.GetSphereSide(subMeshWorldT.Transform(subMesh.SphereVolume)) ==
                math::eSide::Outside)
            {
                continue;
            }
            mInstanceDataBuffer[instCount].WorldMat = subMeshWorldT.Matrix;
            ++instCount;
        }
        if (instCount == 0)
        {
            continue;
        }

        // Construct command.
        DrawCommandDesc cmdDesc;

        cmdDesc.hStaticMesh = hStaticMeshComp->GetMesh()->hRenderProxy;
        if (subMesh.hRenderMaterial.IsNULL())
        {
            cmdDesc.hMaterial = builtInAssets.hDefaultMaterial;
        }
        else
        {
            if (!RenderingSystem::GetInstance().IsUploaded(subMesh.hRenderMaterial))
            {
                RenderingSystem::GetInstance().UploadMaterial(subMesh.hRenderMaterial);
                continue;
            }
            else
            {
                cmdDesc.hMaterial = subMesh.hRenderMaterial->hRenderProxy;
            }

            RenderingSystem::GetInstance().UploadMaterialTextures(subMesh.hRenderMaterial);
            RenderingSystem::GetInstance().UploadMaterialShaders(subMesh.hRenderMaterial);
        }

        cmdDesc.VertexOffset = subMesh.PositionOffset;
        cmdDesc.IndexOffset = subMesh.IndexOffset;
        cmdDesc.IndexCount = subMesh.IndexCount;

        if ((renderCtx.RenderingFlags & RenderingFlags_RenderObjectOutline) && hStaticMeshComp->IsStencilEnabled())
        {
            switch (static_cast<eStencilMode>(hStaticMeshComp->GetStencilReference()))
            {
                case eStencilMode::StencilMode_Outline:
                    cmdDesc.StencilWriteBitIndex = 0;
                    break;
                default:
                    HO_ASSERT(false, "Invalid stencil reference.");
                    cmdDesc.StencilWriteBitIndex = -1;
            }
        }

        // Submit draw command to stage.
        switch (cmdDesc.hMaterial->PipelineState.AlphaMode)
        {
            case eMaterialAlphaMode::Opaque:
                RenderingSystem::GetInstance().SubmitDrawCommandsToStage(
                    eRenderStageType::Opaque, cmdDesc, instCount, mInstanceDataBuffer.Data());
                break;
            case eMaterialAlphaMode::Mask:
                RenderingSystem::GetInstance().SubmitDrawCommandsToStage(
                    eRenderStageType::Masked, cmdDesc, instCount, mInstanceDataBuffer.Data());
                break;
            case eMaterialAlphaMode::Blend:
                RenderingSystem::GetInstance().SubmitDrawCommandsToStage(
                    eRenderStageType::Blended, cmdDesc, instCount, mInstanceDataBuffer.Data());
                break;
            default:
                HO_ASSERT(false, "Invalid material alpha mode.");
                return;
        }

        if ((renderCtx.RenderingFlags & RenderingFlags_RenderObjectOutline) && hStaticMeshComp->IsStencilEnabled())
        {
            switch (static_cast<eStencilMode>(hStaticMeshComp->GetStencilReference()))
            {
                case eStencilMode::StencilMode_Outline:
                    cmdDesc.StencilWriteBitIndex = -1;
                    RenderingSystem::GetInstance().SubmitDrawCommandsToStage(
                        eRenderStageType::Outline, cmdDesc, instCount, mInstanceDataBuffer.Data());
                    break;
                default:
                    HO_ASSERT(false, "Invalid stencil reference.");
                    return;
            }
        }
    }
}

void SceneRenderer::submitShadowMapDrawCommands(const Frustum& lightFrustum,
                                                ObjectHandle<const RStaticMeshComponent> hStaticMeshComp) const
{
    const RenderingBuiltInAssets& builtInAssets = RenderingSystem::GetInstance().GetBuiltInAssets();

    const Transform3D& meshWorldT =
        hStaticMeshComp->GetOwner()->GetComponent<RTransformComponent>()->GetWorldTransform();
    for (const auto& subMesh : hStaticMeshComp->GetMesh()->SubMeshes)
    {
        if (subMesh.hRenderMaterial.IsValid() &&
            subMesh.hRenderMaterial->PipelineState.AlphaMode == eMaterialAlphaMode::Blend)
        {
            continue;
        }

        int32_t instCount = 0;
        for (int32_t i = 0; i < subMesh.LocalTransformCount; ++i)
        {
            if (instCount >= mInstanceDataBuffer.GetSize())
            {
                mbNeedToResizeInstanceDataBuffer = true;
                continue;
            }
            const Transform3D subMeshWorldT =
                meshWorldT * hStaticMeshComp->GetMesh()->LocalTransforms[subMesh.LocalTransformOffset + i];
            if (lightFrustum.GetSphereSide(subMeshWorldT.Transform(subMesh.SphereVolume)) == math::eSide::Outside)
            {
                continue;
            }
            mInstanceDataBuffer[instCount].WorldMat = subMeshWorldT.Matrix;
            ++instCount;
        }
        if (instCount == 0)
        {
            continue;
        }

        // Construct command.
        DrawCommandDesc cmdDesc;

        cmdDesc.hStaticMesh = hStaticMeshComp->GetMesh()->hRenderProxy;
        if (subMesh.hRenderMaterial.IsNULL() ||
            subMesh.hRenderMaterial->PipelineState.AlphaMode == eMaterialAlphaMode::Opaque)
        {
            cmdDesc.hMaterial = builtInAssets.hDefaultMaterial;
        }
        else
        {
            if (!RenderingSystem::GetInstance().IsUploaded(subMesh.hRenderMaterial))
            {
                RenderingSystem::GetInstance().UploadMaterial(subMesh.hRenderMaterial);
                continue;
            }
            else
            {
                cmdDesc.hMaterial = subMesh.hRenderMaterial->hRenderProxy;
            }

            if (cmdDesc.hMaterial->PipelineState.AlphaMode == eMaterialAlphaMode::Mask)
            {
                RenderingSystem::GetInstance().UploadMaterialTexture(subMesh.hRenderMaterial,
                                                                     eMaterialTextureUsage::Diffuse);
                RenderingSystem::GetInstance().UploadMaterialTexture(subMesh.hRenderMaterial,
                                                                     eMaterialTextureUsage::Albedo);
                RenderingSystem::GetInstance().UploadMaterialTexture(subMesh.hRenderMaterial,
                                                                     eMaterialTextureUsage::Opacity);
            }
        }

        cmdDesc.VertexOffset = subMesh.PositionOffset;
        cmdDesc.IndexOffset = subMesh.IndexOffset;
        cmdDesc.IndexCount = subMesh.IndexCount;

        // Submit draw command to stage.
        switch (cmdDesc.hMaterial->PipelineState.AlphaMode)
        {
            case eMaterialAlphaMode::Opaque:
                RenderingSystem::GetInstance().SubmitDrawCommandsToStage(
                    eRenderStageType::Opaque, cmdDesc, instCount, mInstanceDataBuffer.Data());
                break;
            case eMaterialAlphaMode::Mask:
                RenderingSystem::GetInstance().SubmitDrawCommandsToStage(
                    eRenderStageType::Masked, cmdDesc, instCount, mInstanceDataBuffer.Data());
                break;
            default:
                HO_ASSERT(false, "Invalid alpha mode.");
        }
    }
}

void SceneRenderer::submitShadowMapDrawCommands(const Sphere& castersBV,
                                                ObjectHandle<const RStaticMeshComponent> hStaticMeshComp) const
{
    const RenderingBuiltInAssets& builtInAssets = RenderingSystem::GetInstance().GetBuiltInAssets();

    const Transform3D& meshWorldT =
        hStaticMeshComp->GetOwner()->GetComponent<RTransformComponent>()->GetWorldTransform();
    for (const auto& subMesh : hStaticMeshComp->GetMesh()->SubMeshes)
    {
        if (subMesh.hRenderMaterial.IsValid() &&
            subMesh.hRenderMaterial->PipelineState.AlphaMode == eMaterialAlphaMode::Blend)
        {
            continue;
        }

        int32_t instCount = 0;
        for (int32_t i = 0; i < subMesh.LocalTransformCount; ++i)
        {
            if (instCount >= mInstanceDataBuffer.GetSize())
            {
                mbNeedToResizeInstanceDataBuffer = true;
                continue;
            }
            const Transform3D subMeshWorldT =
                meshWorldT * hStaticMeshComp->GetMesh()->LocalTransforms[subMesh.LocalTransformOffset + i];
            if (castersBV.GetSide(subMeshWorldT.Transform(subMesh.SphereVolume)) == math::eSide::Outside)
            {
                continue;
            }
            mInstanceDataBuffer[instCount].WorldMat = subMeshWorldT.Matrix;
            ++instCount;
        }
        if (instCount == 0)
        {
            continue;
        }

        // Construct command.
        DrawCommandDesc cmdDesc;

        cmdDesc.hStaticMesh = hStaticMeshComp->GetMesh()->hRenderProxy;
        if (subMesh.hRenderMaterial.IsNULL() ||
            subMesh.hRenderMaterial->PipelineState.AlphaMode == eMaterialAlphaMode::Opaque)
        {
            cmdDesc.hMaterial = builtInAssets.hDefaultMaterial;
        }
        else
        {
            if (!RenderingSystem::GetInstance().IsUploaded(subMesh.hRenderMaterial))
            {
                RenderingSystem::GetInstance().UploadMaterial(subMesh.hRenderMaterial);
                continue;
            }
            else
            {
                cmdDesc.hMaterial = subMesh.hRenderMaterial->hRenderProxy;
            }

            if (cmdDesc.hMaterial->PipelineState.AlphaMode == eMaterialAlphaMode::Mask)
            {
                RenderingSystem::GetInstance().UploadMaterialTexture(subMesh.hRenderMaterial,
                                                                     eMaterialTextureUsage::Diffuse);
                RenderingSystem::GetInstance().UploadMaterialTexture(subMesh.hRenderMaterial,
                                                                     eMaterialTextureUsage::Albedo);
                RenderingSystem::GetInstance().UploadMaterialTexture(subMesh.hRenderMaterial,
                                                                     eMaterialTextureUsage::Opacity);
            }
        }

        cmdDesc.VertexOffset = subMesh.PositionOffset;
        cmdDesc.IndexOffset = subMesh.IndexOffset;
        cmdDesc.IndexCount = subMesh.IndexCount;

        // Submit draw command to stage.
        switch (cmdDesc.hMaterial->PipelineState.AlphaMode)
        {
            case eMaterialAlphaMode::Opaque:
                RenderingSystem::GetInstance().SubmitDrawCommandsToStage(
                    eRenderStageType::Opaque, cmdDesc, instCount, mInstanceDataBuffer.Data());
                break;
            case eMaterialAlphaMode::Mask:
                RenderingSystem::GetInstance().SubmitDrawCommandsToStage(
                    eRenderStageType::Masked, cmdDesc, instCount, mInstanceDataBuffer.Data());
                break;
            default:
                HO_ASSERT(false, "Invalid alpha mode.");
        }
    }
}

void SceneRenderer::submitBoundingVolumeDrawCommands(const Transform3D& worldT, const AABB& aabb) const
{
    const AssetBuiltInAssets& assetBuiltInAssets = AssetSystem::GetInstance().GetBuiltInAsset();
    const RenderingBuiltInAssets& builtInAssets = RenderingSystem::GetInstance().GetBuiltInAssets();

    const AABB worldAABB = worldT.Transform(aabb);
    const Transform3D aabbWorldT = Transform3D::FromScalePosition(worldAABB.GetExtand(), worldAABB.GetCenter());

    const ObjectHandle<RStaticMeshAsset> hUnitCube = assetBuiltInAssets.hCubeMesh;
    if (!RenderingSystem::GetInstance().IsUploaded(hUnitCube))
    {
        RenderingSystem::GetInstance().UploadStaticMesh(hUnitCube);
        return;
    }

    DrawCommandDesc cmdDesc;

    const Transform3D subMeshWorldT = aabbWorldT * hUnitCube->LocalTransforms[0];

    cmdDesc.hStaticMesh = hUnitCube->hRenderProxy;

    cmdDesc.hMaterial = builtInAssets.hDefaultMaterial;

    cmdDesc.VertexOffset = hUnitCube->SubMeshes[0].PositionOffset;
    cmdDesc.IndexOffset = hUnitCube->SubMeshes[0].IndexOffset;
    cmdDesc.IndexCount = hUnitCube->SubMeshes[0].IndexCount;

    InstanceData instData;
    instData.WorldMat = subMeshWorldT.Matrix;

    RenderingSystem::GetInstance().SubmitDrawCommandsToStage(eRenderStageType::InWorldDebug, cmdDesc, 1, &instData);
}

void SceneRenderer::submitBoundingVolumeDrawCommands(const Transform3D& worldT, const Sphere& sphere) const
{
    const AssetBuiltInAssets& assetBuiltInAssets = AssetSystem::GetInstance().GetBuiltInAsset();
    const RenderingBuiltInAssets& builtInAssets = RenderingSystem::GetInstance().GetBuiltInAssets();

    const Sphere worldSphere = worldT.Transform(sphere);
    const Transform3D sphereWorldT = Transform3D::FromScalePosition(
        Vector3(worldSphere.Radius, worldSphere.Radius, worldSphere.Radius), worldSphere.Center);

    const ObjectHandle<RStaticMeshAsset> hUnitSphere = assetBuiltInAssets.hSparseSphereMesh;
    if (!RenderingSystem::GetInstance().IsUploaded(hUnitSphere))
    {
        RenderingSystem::GetInstance().UploadStaticMesh(hUnitSphere);
        return;
    }

    DrawCommandDesc cmdDesc;

    const Transform3D subMeshWorldT = sphereWorldT * hUnitSphere->LocalTransforms[0];

    cmdDesc.hStaticMesh = hUnitSphere->hRenderProxy;

    cmdDesc.hMaterial = builtInAssets.hDefaultMaterial;

    cmdDesc.VertexOffset = hUnitSphere->SubMeshes[0].PositionOffset;
    cmdDesc.IndexOffset = hUnitSphere->SubMeshes[0].IndexOffset;
    cmdDesc.IndexCount = hUnitSphere->SubMeshes[0].IndexCount;

    InstanceData instData;
    instData.WorldMat = subMeshWorldT.Matrix;

    RenderingSystem::GetInstance().SubmitDrawCommandsToStage(eRenderStageType::InWorldDebug, cmdDesc, 1, &instData);
}

void SceneRenderer::submitLightDebugDrawCommands(const SceneRenderContext& renderCtx,
                                                 ObjectHandle<const RLightComponent> hLightComp) const
{
    const AssetBuiltInAssets& assetBuiltInAssets = AssetSystem::GetInstance().GetBuiltInAsset();
    const RenderingBuiltInAssets& builtInAssets = RenderingSystem::GetInstance().GetBuiltInAssets();

    ObjectHandle<RStaticMeshAsset> hLightMesh;
    bool bCastSucceed = !hLightComp->IsCastingShadow();
    switch (hLightComp->GetType())
    {
        case RLightComponent::eLightType::Directional:
            hLightMesh = assetBuiltInAssets.hStageLightMesh;
            if (hLightComp->IsCastingShadow())
            {
                for (int32_t i = 0; i < renderCtx.DirectLightsShadowData.DirLightShadowDataCount; ++i)
                {
                    if (renderCtx.hShadowCastingDirLights[i] == hLightComp)
                    {
                        bCastSucceed = true;
                        break;
                    }
                }
            }
            break;
        case RLightComponent::eLightType::Point:
            hLightMesh = assetBuiltInAssets.hLightBulbMesh;
            if (hLightComp->IsCastingShadow())
            {
                for (int32_t i = 0; i < renderCtx.DirectLightsShadowData.PointLightShadowDataCount; ++i)
                {
                    if (renderCtx.hShadowCastingPointLights[i] == hLightComp)
                    {
                        bCastSucceed = true;
                        break;
                    }
                }
            }
            break;
        case RLightComponent::eLightType::Spot:
            hLightMesh = assetBuiltInAssets.hFlashLightMesh;
            if (hLightComp->IsCastingShadow())
            {
                for (int32_t i = 0; i < renderCtx.DirectLightsShadowData.SpotLightShadowDataCount; ++i)
                {
                    if (renderCtx.hShadowCastingSpotLights[i] == hLightComp)
                    {
                        bCastSucceed = true;
                        break;
                    }
                }
            }
            break;
        default:
            HO_ASSERT(false, "Invalid light type");
    }

    const Transform3D& meshWorldT = hLightComp->GetOwner()->GetComponent<RTransformComponent>()->GetWorldTransform();

    if (renderCtx.CameraFrustum.GetSphereSide(meshWorldT.Transform(hLightMesh->SphereVolume)) == math::eSide::Outside)
    {
        return;
    }

    if (!RenderingSystem::GetInstance().IsUploaded(hLightMesh))
    {
        RenderingSystem::GetInstance().UploadStaticMesh(hLightMesh);
        return;
    }

    for (const auto& subMesh : hLightMesh->SubMeshes)
    {
        int32_t instCount = 0;
        for (int32_t i = 0; i < subMesh.LocalTransformCount; ++i)
        {
            if (instCount >= mInstanceDataBuffer.GetSize())
            {
                mbNeedToResizeInstanceDataBuffer = true;
                continue;
            }
            const Transform3D subMeshWorldT =
                meshWorldT * hLightMesh->LocalTransforms[subMesh.LocalTransformOffset + i];
            if (renderCtx.CameraFrustum.GetSphereSide(subMeshWorldT.Transform(subMesh.SphereVolume)) ==
                math::eSide::Outside)
            {
                continue;
            }
            mInstanceDataBuffer[instCount].WorldMat = subMeshWorldT.Matrix;
            ++instCount;
        }
        if (instCount == 0)
        {
            continue;
        }

        // Construct command.
        DrawCommandDesc cmdDesc;

        cmdDesc.hStaticMesh = hLightMesh->hRenderProxy;
        if (!bCastSucceed || subMesh.hRenderMaterial.IsNULL())
        {
            cmdDesc.hMaterial = builtInAssets.hDefaultMaterial;
        }
        else
        {
            if (!RenderingSystem::GetInstance().IsUploaded(subMesh.hRenderMaterial))
            {
                RenderingSystem::GetInstance().UploadMaterial(subMesh.hRenderMaterial);
                continue;
            }
            else
            {
                cmdDesc.hMaterial = subMesh.hRenderMaterial->hRenderProxy;
            }

            RenderingSystem::GetInstance().UploadMaterialTextures(subMesh.hRenderMaterial);
            RenderingSystem::GetInstance().UploadMaterialShaders(subMesh.hRenderMaterial);
        }

        cmdDesc.VertexOffset = subMesh.PositionOffset;
        cmdDesc.IndexOffset = subMesh.IndexOffset;
        cmdDesc.IndexCount = subMesh.IndexCount;

        if ((renderCtx.RenderingFlags & RenderingFlags_RenderObjectOutline) &&
            hLightComp->GetOwner()->IsSelectedInEditor())
        {
            cmdDesc.StencilWriteBitIndex = 0;
        }

        RenderingSystem::GetInstance().SubmitDrawCommandsToStage(
            eRenderStageType::Opaque, cmdDesc, instCount, mInstanceDataBuffer.Data());

        if ((renderCtx.RenderingFlags & RenderingFlags_RenderObjectOutline) &&
            hLightComp->GetOwner()->IsSelectedInEditor())
        {
            cmdDesc.StencilWriteBitIndex = -1;
            RenderingSystem::GetInstance().SubmitDrawCommandsToStage(
                eRenderStageType::Outline, cmdDesc, instCount, mInstanceDataBuffer.Data());
        }
    }
}

void SceneRenderer::submitCameraDebugDrawCommands(const SceneRenderContext& renderCtx,
                                                  ObjectHandle<const RCameraComponent> hCameraComp) const
{
    const AssetBuiltInAssets& assetBuiltInAssets = AssetSystem::GetInstance().GetBuiltInAsset();
    const RenderingBuiltInAssets& builtInAssets = RenderingSystem::GetInstance().GetBuiltInAssets();

    const ObjectHandle<RStaticMeshAsset> hCameraMesh = assetBuiltInAssets.hCameraMesh;

    const Transform3D& meshWorldT = hCameraComp->GetOwner()->GetComponent<RTransformComponent>()->GetWorldTransform();

    if (renderCtx.CameraFrustum.GetSphereSide(meshWorldT.Transform(hCameraMesh->SphereVolume)) == math::eSide::Outside)
    {
        return;
    }

    if (!RenderingSystem::GetInstance().IsUploaded(hCameraMesh))
    {
        RenderingSystem::GetInstance().UploadStaticMesh(hCameraMesh);
        return;
    }

    for (const auto& subMesh : hCameraMesh->SubMeshes)
    {
        int32_t instCount = 0;
        for (int32_t i = 0; i < subMesh.LocalTransformCount; ++i)
        {
            if (instCount >= mInstanceDataBuffer.GetSize())
            {
                mbNeedToResizeInstanceDataBuffer = true;
                continue;
            }
            const Transform3D subMeshWorldT =
                meshWorldT * hCameraMesh->LocalTransforms[subMesh.LocalTransformOffset + i];
            if (renderCtx.CameraFrustum.GetSphereSide(subMeshWorldT.Transform(subMesh.SphereVolume)) ==
                math::eSide::Outside)
            {
                continue;
            }
            mInstanceDataBuffer[instCount].WorldMat = subMeshWorldT.Matrix;
            ++instCount;
        }
        if (instCount == 0)
        {
            continue;
        }

        // Construct command.
        DrawCommandDesc cmdDesc;

        cmdDesc.hStaticMesh = hCameraMesh->hRenderProxy;
        if (subMesh.hRenderMaterial.IsNULL())
        {
            cmdDesc.hMaterial = builtInAssets.hDefaultMaterial;
        }
        else
        {
            if (!RenderingSystem::GetInstance().IsUploaded(subMesh.hRenderMaterial))
            {
                RenderingSystem::GetInstance().UploadMaterial(subMesh.hRenderMaterial);
                continue;
            }
            else
            {
                cmdDesc.hMaterial = subMesh.hRenderMaterial->hRenderProxy;
            }

            RenderingSystem::GetInstance().UploadMaterialTextures(subMesh.hRenderMaterial);
            RenderingSystem::GetInstance().UploadMaterialShaders(subMesh.hRenderMaterial);
        }

        cmdDesc.VertexOffset = subMesh.PositionOffset;
        cmdDesc.IndexOffset = subMesh.IndexOffset;
        cmdDesc.IndexCount = subMesh.IndexCount;

        if ((renderCtx.RenderingFlags & RenderingFlags_RenderObjectOutline) &&
            hCameraComp->GetOwner()->IsSelectedInEditor())
        {
            cmdDesc.StencilWriteBitIndex = 0;
        }

        RenderingSystem::GetInstance().SubmitDrawCommandsToStage(
            eRenderStageType::Opaque, cmdDesc, instCount, mInstanceDataBuffer.Data());

        if ((renderCtx.RenderingFlags & RenderingFlags_RenderObjectOutline) &&
            hCameraComp->GetOwner()->IsSelectedInEditor())
        {
            cmdDesc.StencilWriteBitIndex = -1;
            RenderingSystem::GetInstance().SubmitDrawCommandsToStage(
                eRenderStageType::Outline, cmdDesc, instCount, mInstanceDataBuffer.Data());
        }
    }
}
} // namespace ho