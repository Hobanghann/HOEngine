#version 460 core

const int TARGET_FINAL_BINDING = 0;

const int MAX_DIR_LIGHT_COUNT = 4;
const int MAX_POINT_LIGHT_COUNT = 32;
const int MAX_SPOT_LIGHT_COUNT = 16;

const int MAX_DIR_LIGHT_SHADOW_DATA_COUNT = 1;
const int MAX_POINT_LIGHT_SHADOW_DATA_COUNT = 4;
const int MAX_SPOT_LIGHT_SHADOW_DATA_COUNT = 8;

const int MAX_SUB_MESH_MORPH_TARGET_COUNT = 8;

const int BUFFER_MATRICES_BINDING = 0;
const int BUFFER_MATERIAL_BINDING = 1;
const int BUFFER_DIRECT_LIGHTS_BINDING = 2;
const int BUFFER_DIRECT_LIGHT_SHADOW_DATA_BINDING = 3;
const int BUFFER_INDIRECT_LIGHTS_BINDING = 4;
const int BUFFER_GENERAL_PURPOSE_BINDING = 5;
const int BUFFER_SHADOW_MAP_RENDER_DATA_BINDING = 6;
const int BUFFER_ENVIRONMENT_BAKE_DATA_BINDING = 7;
const int BUFFER_WORLD_GIZMO_DATA_BINDING = 8;

const int BUFFER_INSTANCE_DATA_BINDING = 9;
const int BUFFER_MORPH_CONTROL_DATA_BINDING = 10;

const int BUFFER_MORPH_TARGETS_BINDING = 11;
const int BUFFER_MORPH_TARGET_POSITION_BINDING = 12;
const int BUFFER_MORPH_TARGET_NORMAL_BINDING = 13;
const int BUFFER_MORPH_TARGET_TANGENT_BINDING = 14;

const int TEX_DIFFUSE_BINDING = 1;
const int TEX_SPECULAR_BINDING = 2;
const int TEX_OPACITY_BINDING = 3;
const int TEX_NORMAL_BINDING = 4;
const int TEX_ALBEDO_BINDING = 5;
const int TEX_METALLIC_ROUGHNESS_BINDING = 6;
const int TEX_EMISSIVE_BINDING = 7;
const int TEX_AMBIENT_OCCLUSION_BINDING = 8;

const int TEX_RAD_MAP_BINDING = 9;
const int TEX_IRRADIANCE_MAP_BINDING = 10;
const int TEX_PREFILTERED_ENVIRONMENT_MAP_BINDING = 11;
const int TEX_BRDF_INTEGRATION_MAP_BINDING = 12;

const int TEX_ACCUMULATION_BINDING = 13;
const int TEX_DEFERED_ALBEDO_AO_BINDING = 14;
const int TEX_DEFERED_NORMAL_ROUGHNESS_BINDING = 15;
const int TEX_DEFERED_MATERIAL_BINDING = 16;
const int TEX_LUMINANCE_BINDING = 17;

const int TEX_DIR_LIGHT_DYNAMIC_SHADOW_MAP_BINDING = 18;
const int TEX_POINT_LIGHT_DYNAMIC_SHADOW_MAP_BINDING = 19;
const int TEX_SPOT_LIGHT_DYNAMIC_SHADOW_MAP_BINDING = 20;

const int TEX_DIR_LIGHT_STATIC_SHADOW_MAPS_BINDING = 21;
const int TEX_POINT_LIGHT_STATIC_SHADOW_MAPS_BINDING = TEX_DIR_LIGHT_STATIC_SHADOW_MAPS_BINDING + MAX_DIR_LIGHT_SHADOW_DATA_COUNT;
const int TEX_SPOT_LIGHT_STATIC_SHADOW_MAPS_BINDING = TEX_POINT_LIGHT_STATIC_SHADOW_MAPS_BINDING + MAX_POINT_LIGHT_SHADOW_DATA_COUNT;

layout (std140, row_major, binding = BUFFER_MATRICES_BINDING) uniform Matrices
{
    mat4 View;
    mat4 Proj;
} u_Matrices;

layout (std140, row_major, binding = BUFFER_MATERIAL_BINDING) uniform Material
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

struct DirLight {
    vec3 Direction;
    vec3 Color;
    float Intensity;
    int ShadowDataIndex;
};

struct PointLight {
    vec3 Position;
    vec3 Color;
    float Intensity;
    float Constant;
    float Linear;
    float Quadratic;
    float FarDist;
    int ShadowDataIndex;
};

struct SpotLight {
    vec3 Position;
    vec3 Direction;
    vec3 Color;
    float Intensity;
    float InnerCutOff;
    float OuterCutOff;
    float Constant;
    float Linear;
    float Quadratic;
    float FarDist;
    int ShadowDataIndex;
};

layout (std140, row_major, binding = BUFFER_DIRECT_LIGHTS_BINDING) uniform DirectLights
{
    int DirLightCount;
    DirLight DirLights[MAX_DIR_LIGHT_COUNT];
    int PointLightCount;
    PointLight PointLights[MAX_POINT_LIGHT_COUNT];
    int SpotLightCount;
    SpotLight SpotLights[MAX_SPOT_LIGHT_COUNT];
} u_DirectLights;

struct DirLightShadowData {
    int CascadeCount;
    float CascadeNormalOffsetScales[6];
    float CascadeDepthOffsetScales[6];
    float CascadeFarDist[6];
    mat4 ProjView[6];
};

struct PointLightShadowData {
    vec3 LightWorldPos;
    float NormalOffsetScale;
    float DepthOffsetScale;
    float FarDist;
    mat4 ProjView[6];
};

struct SpotLightShadowData {
    vec3 LightWorldPos;
    float NormalOffsetScale;
    float DepthOffsetScale;
    float FarDist;
    mat4 ProjView;
};

layout (std140, row_major, binding = BUFFER_DIRECT_LIGHT_SHADOW_DATA_BINDING) uniform DirectLightShadowData
{
    int DirLightShadowDataCount;
    DirLightShadowData DirLightsShadowData[MAX_DIR_LIGHT_SHADOW_DATA_COUNT];
    int PointLightShadowDataCount;
    PointLightShadowData PointLightsShadowData[MAX_POINT_LIGHT_SHADOW_DATA_COUNT];
    int SpotLightShadowDataCount;
    SpotLightShadowData SpotLightsShadowData[MAX_SPOT_LIGHT_SHADOW_DATA_COUNT];
} u_DirectLightShadowData;

layout (std140, row_major, binding = BUFFER_INDIRECT_LIGHTS_BINDING) uniform IndirectLights
{
    float Intensity;
} u_IndirectLights;

layout (std140, row_major, binding = BUFFER_GENERAL_PURPOSE_BINDING) uniform General
{
    vec3 WorldCameraPos;
    vec3 CameraForward;
} u_General;

layout (std140, row_major, binding = BUFFER_SHADOW_MAP_RENDER_DATA_BINDING) uniform ShadowRenderData
{
    int CascadeCount;
    vec3 LightWorldPos;
    float FarDist;
    mat4 ProjView[6];
} u_ShadowRenderData;

struct InstanceData
{
    mat4 WorldMat;
};

layout(std430, row_major, binding = BUFFER_INSTANCE_DATA_BINDING) readonly buffer InstanceBuffer
{
    InstanceData Instances[];
} s_InstanceBuffer;

layout (std430, row_major, binding = BUFFER_MORPH_CONTROL_DATA_BINDING) readonly buffer MorphControlData
{
    int TargetOffset;
    int TargetCount;

    int TargetIndices[MAX_SUB_MESH_MORPH_TARGET_COUNT];
    float Weights[MAX_SUB_MESH_MORPH_TARGET_COUNT];
} s_MorphControlData;

struct MorphTarget {
    int PositionOffset;
    int NormalOffset;
    int TangentOffset;
};

layout (std430, row_major, binding = BUFFER_MORPH_TARGETS_BINDING) readonly buffer MorphTargets
{
    MorphTarget Targets[];
} s_MorphTargets;

layout (std430, row_major, binding = BUFFER_MORPH_TARGET_POSITION_BINDING) readonly buffer MorphPositionBuffer
{
    vec3 Positions[];
} s_MorphPositions;

layout (std430, row_major, binding = BUFFER_MORPH_TARGET_NORMAL_BINDING) readonly buffer MorphNormalBuffer
{
    vec3 Normals[];
} s_MorphNormals;

layout (std430, row_major, binding = BUFFER_MORPH_TARGET_TANGENT_BINDING) readonly buffer MorphTangentBuffer
{
    vec3 Tangents[];
} s_MorphTangents;

layout(binding = TEX_DIFFUSE_BINDING) uniform sampler2D t_Diffuse;
layout(binding = TEX_SPECULAR_BINDING) uniform sampler2D t_Specular;
layout(binding = TEX_OPACITY_BINDING) uniform sampler2D t_Opacity;
layout(binding = TEX_NORMAL_BINDING) uniform sampler2D t_Normal;
layout(binding = TEX_ALBEDO_BINDING) uniform sampler2D t_Albedo;
layout(binding = TEX_METALLIC_ROUGHNESS_BINDING) uniform sampler2D t_MetallicRoughness;
layout(binding = TEX_EMISSIVE_BINDING) uniform sampler2D t_Emissive;
layout(binding = TEX_AMBIENT_OCCLUSION_BINDING) uniform sampler2D t_AmbientOcclusion;

layout(binding = TEX_RAD_MAP_BINDING) uniform samplerCube t_RadianceMap;
layout(binding = TEX_IRRADIANCE_MAP_BINDING) uniform samplerCube t_IrradianceMap;
layout(binding = TEX_PREFILTERED_ENVIRONMENT_MAP_BINDING) uniform samplerCube t_PrefilteredEnvMap;
layout(binding = TEX_BRDF_INTEGRATION_MAP_BINDING) uniform sampler2D t_BRDFIntegrationMap;

layout(binding = TEX_ACCUMULATION_BINDING) uniform sampler2D t_Accumulation;
layout(binding = TEX_DEFERED_ALBEDO_AO_BINDING) uniform sampler2D t_DeferedAlbedoAO;
layout(binding = TEX_DEFERED_NORMAL_ROUGHNESS_BINDING) uniform sampler2D t_DeferedNormalRoughness;
layout(binding = TEX_DEFERED_MATERIAL_BINDING) uniform sampler2D t_DeferedMaterial;
layout(binding = TEX_LUMINANCE_BINDING) uniform sampler2D t_Luminance;

layout(binding = TEX_DIR_LIGHT_DYNAMIC_SHADOW_MAP_BINDING) uniform sampler2DArrayShadow t_DirLightDynamicShadowMap;
layout(binding = TEX_POINT_LIGHT_DYNAMIC_SHADOW_MAP_BINDING) uniform samplerCubeArray t_PointLightDynamicShadowMap;
layout(binding = TEX_SPOT_LIGHT_DYNAMIC_SHADOW_MAP_BINDING) uniform sampler2DArray t_SpotLightDynamicShadowMap;

layout(binding = TEX_DIR_LIGHT_STATIC_SHADOW_MAPS_BINDING) uniform sampler2DArrayShadow t_DirLightStaticShadowMaps[MAX_DIR_LIGHT_SHADOW_DATA_COUNT];
layout(binding = TEX_POINT_LIGHT_STATIC_SHADOW_MAPS_BINDING) uniform samplerCube t_PointLightStaticShadowMaps[MAX_POINT_LIGHT_SHADOW_DATA_COUNT];
layout(binding = TEX_SPOT_LIGHT_STATIC_SHADOW_MAPS_BINDING) uniform sampler2D t_SpotLightStaticShadowMaps[MAX_SPOT_LIGHT_SHADOW_DATA_COUNT];

layout(location = 0) in vec4 v_WorldPos;

layout(location = TARGET_FINAL_BINDING) out float out_FragDepth;

void main()
{
    float lightDist = length(v_WorldPos.xyz - u_ShadowRenderData.LightWorldPos);

    lightDist = lightDist / u_ShadowRenderData.FarDist;

    out_FragDepth = lightDist;
}