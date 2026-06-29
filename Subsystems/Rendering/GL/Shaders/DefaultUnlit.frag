#version 460 core

const int UNIFORM_MATERIAL_BINDING = 0;
const int UNIFORM_MATRICES_BINDING = 1;
const int UNIFORM_LIGHTS_BINDING = 2;
const int UNIFORM_GENERAL_BINDING = 3;

const int TEX_DIFFUSE_BINDING = 1;
const int TEX_SPECULAR_BINDING = 2;
const int TEX_OPACITY_BINDING = 3;
const int TEX_NORMAL_BINDING = 4;
const int TEX_ALBEDO_BINDING = 5;
const int TEX_METALLIC_ROUGHNESS_BINDING = 6;
const int TEX_EMISSIVE_BINDING = 7;
const int TEX_AMBIENT_OCCLUSION_BINDING = 8;

const int TEX_SKY_MAP_BINDING = 9;
const int TEX_IRRADIANCE_MAP_BINDING = 10;

const int MAX_DIR_LIGHT_COUNT = 4;
const int MAX_POINT_LIGHT_COUNT = 32;
const int MAX_SPOT_LIGHT_COUNT = 16;


layout (std140, binding = UNIFORM_MATERIAL_BINDING) uniform Material
{
    vec4 Ambient;
    vec4 Diffuse;
    vec4 Specular;
    vec4 Albedo;
    vec4 Emissive;

    float Shininess;
    float Metallic;
    float Roughness;
    float IndexOfRefraction;

    float Opacity;
    float AlphaThreshold;
    float EmissiveIntensity;
    float NormalScale;
    float OcclusionStrength;

    int UVChannels[9];
    mat4 UVMatrices[9];
    int HasTextures[9];
} u_MatAttr;

layout (std140, binding = UNIFORM_GENERAL_BINDING) uniform General
{
    vec3 WorldCameraPos;
} u_General;

layout(location = 0, binding = TEX_DIFFUSE_BINDING) uniform sampler2D t_Diffuse;
layout(location = 2, binding = TEX_OPACITY_BINDING) uniform sampler2D t_Opacity;
layout(location = 4, binding = TEX_ALBEDO_BINDING) uniform sampler2D t_Albedo;
layout(location = 6, binding = TEX_EMISSIVE_BINDING) uniform sampler2D t_Emissive;

layout(location = 8, binding = TEX_SKY_MAP_BINDING) uniform samplerCube t_SkyMap;

layout(location = 0) in vec3 v_WorldPos;
layout(location = 1) in vec3 v_Normal;
layout(location = 2) in vec4 v_Tangent;
layout(location = 3) in vec2 v_UV0;
layout(location = 4) in vec2 v_UV1;
layout(location = 5) in vec4 v_Color;

layout(location = 0) out vec4 out_FragColor;

vec2 GetUV(int textureIndex)
{
    int channel = u_MatAttr.UVChannels[textureIndex];
    vec2 baseUV = (channel == 1) ? v_UV1 : v_UV0;
    return (u_MatAttr.UVMatrices[textureIndex] * vec4(baseUV, 1.0f, 0.0f)).xy;
}

void main()
{
    vec4 texDiffuse = texture(t_Diffuse, GetUV(TEX_DIFFUSE_BINDING));
    vec4 texOpacity = texture(t_Opacity, GetUV(TEX_OPACITY_BINDING));
    vec4 texAlbedo = texture(t_Albedo, GetUV(TEX_ALBEDO_BINDING));
    vec4 texEmissive = texture(t_Emissive, GetUV(TEX_EMISSIVE_BINDING));

    float finalOpacity = u_MatAttr.Opacity;
    if (u_MatAttr.HasTextures[TEX_OPACITY_BINDING] == 1)
    {
        finalOpacity *= texOpacity.r;
    }

    vec3 baseColor = u_MatAttr.Albedo.rgb;
    if (u_MatAttr.HasTextures[TEX_ALBEDO_BINDING] == 1)
    {
        baseColor = texAlbedo.rgb;
    }
    else if (u_MatAttr.HasTextures[TEX_DIFFUSE_BINDING] == 1)
    {
        baseColor = texDiffuse.rgb;
    }

    vec3 emissive = u_MatAttr.Emissive.rgb * u_MatAttr.EmissiveIntensity;
    if (u_MatAttr.HasTextures[TEX_EMISSIVE_BINDING] == 1)
    {
        emissive = texEmissive.rgb * u_MatAttr.EmissiveIntensity;
    }

    vec3 finalColor = baseColor + emissive;

    finalColor = clamp(finalColor, 0.0f, 1.0f);
    finalColor = pow(finalColor, vec3(1.0f / 2.2f));

    out_FragColor = vec4(finalColor, finalOpacity);
}