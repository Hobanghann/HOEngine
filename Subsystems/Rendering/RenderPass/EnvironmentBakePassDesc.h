#pragma once

#include "Subsystems/Asset/TextureAsset.h"
#include "Subsystems/Object/ObjectHandle.h"

namespace ho
{

enum class eEnvironmentBakeType
{
    Irradiance = 0,
    PrefilteredEnvMap = 1,
    Last,
};

struct EnvironmentBakePassDesc
{
    eEnvironmentBakeType BakeType = eEnvironmentBakeType::Irradiance;
    int32_t BakedMapSize = 0;
    float SampleDelta = 0.05f;  // used while baking irradiance map
    int32_t SampleCount = 1024; // used while baking pre-filtered map
    GpuTextureHandle hSrcTexture;
    ObjectHandle<RTextureAsset> hDstTexture;
};
} // namespace ho