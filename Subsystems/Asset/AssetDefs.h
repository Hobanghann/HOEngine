#pragma once

#include <cstdint>

#include "Macros.h"

namespace ho
{
enum class eTextureFormat : uint8_t
{
    None = 0,

    R8_UNORM = 1,
    R8G8_UNORM = 2,
    R8G8B8A8_UNORM = 3,
    B8G8R8A8_UNORM = 4,
    R8_SRGB = 5,
    R8G8_SRGB = 6,
    R8G8B8A8_SRGB = 7,

    R16_FLOAT = 8,
    R16G16_FLOAT = 9,
    R16G16B16A16_FLOAT = 10,

    R32_FLOAT = 11,
    R32G32_FLOAT = 12,
    R32G32B32A32_FLOAT = 13,

    D24_UNORM_S8_UINT = 14,
    D32_FLOAT = 15,

    BC1_UNORM = 16,
    BC1_SRGB = 17,
    BC3_UNORM = 18,
    BC3_SRGB = 19,
    BC4_UNORM = 20,
    BC4_SNORM = 21,
    BC5_UNORM = 22,
    BC5_SNORM = 23,
    BC6H_UF16 = 24,
    BC6H_SF16 = 25,
    BC7_UNORM = 26,
    BC7_SRGB = 27,
};

enum class eTextureType : uint8_t
{
    None = 0,
    Texture1D = 1,
    Texture2D = 2,
    Texture3D = 3,
    TextureCubeMap = 4,
    Texture1DArray = 5,
    Texture2DArray = 6,
    TextureCubeMapArray = 7,
};

enum class eMeshPrimitiveType : uint8_t
{
    None = 0,
    Point = 1,
    Line = 2,
    LineStrip = 3,
    Triangle = 4,
    TriangleStrip = 5,
};

enum class eAnimExtrapolationMode : uint8_t
{
    None = 0,
    Default = 1,
    Constant = 2,
    Linear = 3,
    Repeat = 4,
};

enum class eAnimInterpolationMode : uint8_t
{
    None = 0,
    Step = 1,
    Linear = 2,
    SphericalLinear = 3,
    CubicSpline = 4,
};

enum class eMaterialAssetType : uint8_t
{
    None = 0,
    Unlit = 1,
    Legacy = 2,
    StandardLit = 3,
};

enum class eShaderStage : uint8_t
{
    None = 0,
    VertexShader = 1,
    GeometryShader = 2,
    FragmentShader = 3,
    ComputeShader = 4,
    Last = 5,
};

enum class eMaterialAlphaMode : uint8_t
{
    None = 0,
    Opaque = 1,
    Blend = 2,
    Mask = 3,
};

enum class eMaterialAlphaBlendMode : uint8_t
{
    None = 0,
    Default = 1,
    Additive = 2,
};

enum class eMaterialTextureUsage : uint16_t
{
    None = 0,

    Diffuse = 1,
    Specular = 2,

    Opacity = 3,
    Normal = 4,

    Albedo = 5,
    MetallicRoughness = 6,
    Emissive = 7,
    AmbientOcclusion = 8,

    Last = 9,
};

} // namespace ho
