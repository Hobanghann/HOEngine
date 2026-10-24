#pragma once

#include "Core/Math/Matrix4x4.h"
#include "Subsystems/Asset/TextureAsset.h"
#include "Subsystems/Object/ObjectHandle.h"

namespace ho
{

enum class eShadowMapType
{
    DirLightShadowMap = 0,
    PointLightShadowMap = 1,
    SpotLightShadowMap = 2,
    Last,
};

struct ShadowMapBakePassDesc
{
    eShadowMapType ShadowMapType = eShadowMapType::DirLightShadowMap;
    Vector3 LightWorldPos;
    float FarDist = 0.0f;
    Matrix4x4 LightProjViewMats[6] = {};
    ObjectHandle<RTextureAsset> hDstTexture;
};
} // namespace ho