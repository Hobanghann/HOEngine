#pragma once

#include <string>

#include "Core/Math/Color128.h"
#include "Core/Math/Matrix3x3.h"
#include "Core/Math/Vector4.h"
#include "Core/Templates/ID.h"
#include "Path.h"

namespace ho
{
namespace parser
{
struct MaterialIR
{
    enum class eTextureUsage
    {
        // Fallback
        None = 0,

        // Phong / legacy
        Diffuse = 1,
        Specular = 2,
        Shininess = 3,
        Opacity = 4,

        // Geometry
        Height = 5,
        Normal = 6,

        //  PBR Core
        Albedo = 7,
        Emissive = 8,
        Metallic = 9,
        Roughness = 10,
        MetallicRoughness = 11,
        AmbientOcclusion = 12,

        //  PBR Advanced
        SubSurface = 13,
        Sheen = 14,
        Clearcoat = 15,
        ClearcoatNormal = 16,
        Transmission = 17,
        Anisotropy = 18,

        Last = 19,
    };

    enum class eAlphaMode
    {
        // Standard solid rendering. Ignores alpha and writes directly to the depth buffer.
        Opaque,

        // Semi-transparent rendering. Interpolates color using the alpha value.
        Blend,

        // Binary transparency. Pixels are either fully drawn or discarded based on a threshold.
        Mask
    };

    enum class eAlphaBlendMode
    {
        // Result = (Src * Alpha) + (Dest * (1 - Alpha)).
        Default,

        // Result = (Src * Alpha) + Dest.
        Additive
    };

    MaterialIR()
    {
        for (int32_t i = 0; i < static_cast<int32_t>(eTextureUsage::Last); ++i)
        {
            TextureIRIndices[i] = -1;
            UVChannels[i] = 0;
            UVTransforms[i] = Matrix3x3::sIdentity;
        }
    }

    Path ResourcePath = Path(std::string(""));
    std::string NameStr;

    // Legacy / Phong Attributes
    Color128 Ambient{0.001f, 0.001f, 0.001f, 1.f};
    Color128 Diffuse{1.f, 1.f, 1.f, 1.f};
    Color128 Specular{1.f, 1.f, 1.f, 1.f};
    float Shininess = 32.0f;

    // PBR Core Attributes
    Color128 Albedo{1.f, 1.f, 1.f, 1.f};
    float Metallic = 0.0f;
    float Roughness = 0.5f;
    float IndexOfRefraction = 1.45f;

    // Global / Transparency Attributes
    float Opacity = 1.0f;
    eAlphaMode AlphaMode = eAlphaMode::Opaque;
    float AlphaThreshold = 0.5f;
    eAlphaBlendMode BlendMode = eAlphaBlendMode::Default;

    Color128 Emissive{0.f, 0.f, 0.f};
    float EmissiveIntensity = 1.0f;

    // Advanced PBR Attributes
    float TransmissionFactor = 0.0f;

    float ClearcoatFactor = 0.0f;
    float ClearcoatRoughness = 0.0f;

    Color128 SheenColor{0.f, 0.f, 0.f};
    float SheenRoughnessFactor = 0.f;

    float VolumeThicknessFactor = 0.f;
    float VolumeAttenuationDist = 0.f;
    Color128 VolumeAttenuationColor{0.f, 0.f, 0.f};

    float AnisotropyFactor = 0.0f;
    float AnisotropyRotation = 0.0f;

    // TextureIR Strength / Scale
    float NormalScale = 1.0f;
    float OcclusionStrength = 1.0f;
    float ParallaxScale = 0.05f;

    // Render States
    bool bWireframe = false;
    bool bBackfaceCulling = true;

    // Index of ModelIR's pTextureIRs.
    int32_t TextureIRIndices[static_cast<int32_t>(eTextureUsage::Last)];

    int32_t UVChannels[static_cast<int32_t>(eTextureUsage::Last)];

    Matrix3x3 UVTransforms[static_cast<int32_t>(eTextureUsage::Last)];
};
} // namespace parser
} // namespace ho