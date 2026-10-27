#version 460 core

const int TARGET_ACCUMULATION_BINDING = 1;

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

const float PI = 3.14159265359;
const float TEXEL_SIZE_2048 = 0.00048828125; // 1 texel size (2048x2048)
const float TEXEL_SIZE_1024 = 0.0009765625; // 1 texel size (1024x1024)

layout(location = 0) in vec3 v_WorldPos;
layout(location = 1) in vec3 v_Normal;
layout(location = 2) in vec4 v_Tangent;
layout(location = 3) in vec2 v_UV0;
layout(location = 4) in vec2 v_UV1;
layout(location = 5) in vec4 v_Color;

layout(location = TARGET_ACCUMULATION_BINDING) out vec4 out_FragColor;

vec2 GetUV(int textureIndex);

vec3 GetNormal();

float CalcShadowBias(float minBias, float maxBias, vec3 lightDir, vec3 fragNormal);
// Taken from http://the-witness.net/news/2013/09/shadow-mapping-summary-part-1/
vec2 CalcShadowOffsets(vec3 lightDir, vec3 fragNormal);
float CalcFadedShadowFactor(float shadowFactor, float refDist, float fadeStartRatio, float maxShadowVisibleDist);

float CalcDirLightShadow(int shadowDataIdx, vec3 fragToLight, vec3 fragNormal);

float CalcPointLightShadow(int lightIdx, int shadowDataIdx, float fragLightDist, vec3 fragToLight, vec3 fragNormal);

float CalcSpotLightShadow(int lightIdx, int shadowDataIdx, float fragLightDist, vec3 fragToLight, vec3 fragNormal);

void main()
{
    vec4 texDiffuse = texture(t_Diffuse, GetUV(TEX_DIFFUSE_BINDING));
    vec4 texSpecular = texture(t_Specular, GetUV(TEX_SPECULAR_BINDING));
    vec4 texAlbedo = texture(t_Albedo, GetUV(TEX_ALBEDO_BINDING));
    vec4 texEmissive = texture(t_Emissive, GetUV(TEX_EMISSIVE_BINDING));
    vec4 texAO = texture(t_AmbientOcclusion, GetUV(TEX_AMBIENT_OCCLUSION_BINDING));

    vec3 diffColor = u_MatAttr.Diffuse.rgb;
    if (u_MatAttr.HasTextures[TEX_DIFFUSE_BINDING] == 1)
    {
        diffColor = texDiffuse.rgb;
    }
    else if (u_MatAttr.HasTextures[TEX_ALBEDO_BINDING] == 1)
    {
        diffColor = texAlbedo.rgb;
    }

    vec3 specColor = u_MatAttr.Specular.rgb;
    if (u_MatAttr.HasTextures[TEX_SPECULAR_BINDING] == 1)
    {
        specColor *= texSpecular.rgb;
    }

    vec3 N = GetNormal();
    vec3 V = normalize(u_General.WorldCameraPos - v_WorldPos);

    vec3 totalDiffuse = vec3(0.0);
    vec3 totalSpecular = vec3(0.0);

    for(int i = 0; i < u_DirectLights.DirLightCount; ++i)
    {
        vec3 L = normalize(-u_DirectLights.DirLights[i].Direction);
        vec3 H = normalize(L + V);

        float NdotL = max(dot(N, L), 0.0);
        float NdotH = max(dot(N, H), 0.0);

        vec3 lightColor = u_DirectLights.DirLights[i].Color * u_DirectLights.DirLights[i].Intensity;

        float shadow = 1.0;
        if(NdotL > 0.0 && u_DirectLights.DirLights[i].ShadowDataIndex >= 0) {
            shadow = CalcDirLightShadow(u_DirectLights.DirLights[i].ShadowDataIndex, L, N);
        }

        totalDiffuse += diffColor * NdotL * lightColor * shadow;

        float specPower = pow(NdotH, u_MatAttr.Shininess);
        totalSpecular += specColor * specPower * lightColor * step(0.0001, NdotL) * shadow;
    }

    for(int i = 0; i < u_DirectLights.PointLightCount; ++i)
    {
        float dist = length(u_DirectLights.PointLights[i].Position - v_WorldPos);

        vec3 L = (u_DirectLights.PointLights[i].Position - v_WorldPos) / dist;
        vec3 H = normalize(L + V);

        float NdotL = max(dot(N, L), 0.0);
        float NdotH = max(dot(N, H), 0.0);

        float attenuation = 1.0 / (u_DirectLights.PointLights[i].Constant + u_DirectLights.PointLights[i].Linear * dist + u_DirectLights.PointLights[i].Quadratic * dist * dist);
        vec3 lightColor = u_DirectLights.PointLights[i].Color * u_DirectLights.PointLights[i].Intensity * attenuation;

        float shadow = 1.0;
        if(NdotL > 0.0 && u_DirectLights.PointLights[i].ShadowDataIndex >= 0) {
            shadow = CalcPointLightShadow(i, u_DirectLights.PointLights[i].ShadowDataIndex, dist, L, N);
        }

        totalDiffuse += diffColor * NdotL * lightColor * shadow;

        float specPower = pow(NdotH, u_MatAttr.Shininess);
        totalSpecular += specColor * specPower * lightColor * step(0.0001, NdotL) * shadow;
    }

    for(int i = 0; i < u_DirectLights.SpotLightCount; ++i)
    {
        float dist = length(u_DirectLights.SpotLights[i].Position - v_WorldPos);
        vec3 L = (u_DirectLights.SpotLights[i].Position - v_WorldPos) / dist;

        float theta = dot(L, normalize(-u_DirectLights.SpotLights[i].Direction));
        float epsilon = u_DirectLights.SpotLights[i].InnerCutOff - u_DirectLights.SpotLights[i].OuterCutOff;
        float angularIntensity = clamp((theta - u_DirectLights.SpotLights[i].OuterCutOff) / epsilon, 0.0, 1.0);

        float attenuation = 1.0 / (u_DirectLights.SpotLights[i].Constant + u_DirectLights.SpotLights[i].Linear * dist + u_DirectLights.SpotLights[i].Quadratic * dist * dist);
        vec3 lightColor = u_DirectLights.SpotLights[i].Color * u_DirectLights.SpotLights[i].Intensity * attenuation * angularIntensity;

        vec3 H = normalize(L + V);
        float NdotL = max(dot(N, L), 0.0);
        float NdotH = max(dot(N, H), 0.0);

        float shadow = 1.0;
        if(NdotL > 0.0 && angularIntensity > 0.0 && u_DirectLights.SpotLights[i].ShadowDataIndex >= 0) {
            shadow = CalcSpotLightShadow(i, u_DirectLights.SpotLights[i].ShadowDataIndex, dist, L, N);
        }

        totalDiffuse += diffColor * NdotL * lightColor * shadow;

        float specPower = pow(NdotH, u_MatAttr.Shininess);
        totalSpecular += specColor * specPower * lightColor * step(0.0001, NdotL) * shadow;
    }

    float ao = 1.0;
    if (u_MatAttr.HasTextures[TEX_AMBIENT_OCCLUSION_BINDING] == 1)
    {
        ao = texAO.r;
    }
    ao = mix(1.0, ao, u_MatAttr.OcclusionStrength);

    vec3 ambient = u_MatAttr.Ambient.rgb * ao;

    vec3 emissive = u_MatAttr.Emissive.rgb * u_MatAttr.EmissiveIntensity;
    if (u_MatAttr.HasTextures[TEX_EMISSIVE_BINDING] == 1)
    {
        emissive = texEmissive.rgb * u_MatAttr.EmissiveIntensity;
    }

    vec3 finalColor = ambient + totalDiffuse + totalSpecular + emissive;
    out_FragColor = vec4(finalColor, 1.0);
}

vec2 GetUV(int textureIndex)
{
    int channel = u_MatAttr.UVChannels[textureIndex];
    vec2 baseUV = (channel == 1) ? v_UV1 : v_UV0;
    return (u_MatAttr.UVMatrices[textureIndex] * vec4(baseUV, 0.0, 1.0)).xy;
}

vec3 GetNormal()
{
    if (u_MatAttr.HasTextures[TEX_NORMAL_BINDING] == 0)
    {
        return normalize(v_Normal);
    }

    vec2 normalXY = texture(t_Normal, GetUV(TEX_NORMAL_BINDING)).rg * 2.0 - 1.0;
    float normalZ = sqrt(max(1.0 - dot(normalXY, normalXY), 0.0));
    vec3 texNormal = vec3(normalXY, normalZ);

    texNormal.xy *= u_MatAttr.NormalScale;
    texNormal = normalize(texNormal);

    vec3 N = normalize(v_Normal);
    vec3 T = normalize(v_Tangent.xyz - N * dot(v_Tangent.xyz, N));
    vec3 B = normalize(cross(N, T)) * v_Tangent.w;
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * texNormal);
}

float CalcShadowBias(float minBias, float maxBias, vec3 lightDir, vec3 fragNormal) {
    return max(maxBias * (1.0 - dot(lightDir, fragNormal)), minBias);
}

// Taken from http://the-witness.net/news/2013/09/shadow-mapping-summary-part-1/
vec2 CalcShadowOffsets(vec3 lightDir, vec3 fragNormal) {
    float NdotL = dot(lightDir, fragNormal);
    NdotL = clamp(NdotL, 0.0001, 1.0);
    float normalOffsetScale = sqrt(1.0 - NdotL * NdotL); // sin(acos(L·N))
    float depthOffsetScale = normalOffsetScale / NdotL;    // tan(acos(L·N))
    return vec2(normalOffsetScale, min(8.0, depthOffsetScale));
}

float CalcFadedShadowFactor(float shadowFactor, float refDist, float fadeStartRatio, float maxShadowVisibleDist) {
    float fadeStartDist = maxShadowVisibleDist * fadeStartRatio;
    float fadeRatio = clamp((refDist - fadeStartDist) / (maxShadowVisibleDist - fadeStartDist), 0.0, 1.0);
    return mix(shadowFactor, 1.0, fadeRatio);
}

const vec2 dirLightSampleOffsetDirections[9] = vec2[](
    vec2( 0.0,    0.0),
    vec2(-0.707,  0.707),
    vec2( 0.707,  0.707),
    vec2(-0.707, -0.707),
    vec2( 0.707, -0.707),
    vec2( 0.0,    1.0),
    vec2( 0.0,   -1.0),
    vec2(-1.0,    0.0),
    vec2( 1.0,    0.0)
);

float CalcDirLightShadow(int shadowDataIdx, vec3 fragToLight, vec3 fragNormal) {
    int cascadeCount = u_DirectLightShadowData.DirLightsShadowData[shadowDataIdx].CascadeCount;
    vec4 viewPos = u_Matrices.View * vec4(v_WorldPos, 1.0);
    int layer = -1;
    for (int i = 0; i < cascadeCount; ++i)
    {
        if (viewPos.z < u_DirectLightShadowData.DirLightsShadowData[shadowDataIdx].CascadeFarDist[i])
        {
            layer = i;
            break;
        }
    }
    if(layer == -1) {
        return 1.0;
    }

    vec2 offsets = CalcShadowOffsets(fragToLight, fragNormal);
    float bias = CalcShadowBias(0.000006, 0.00006, fragToLight, fragNormal);

    float normalOffsetScale = u_DirectLightShadowData.DirLightsShadowData[shadowDataIdx].CascadeNormalOffsetScales[layer] * offsets.x;
    vec3 samplePos = v_WorldPos + fragNormal * normalOffsetScale;

    vec4 lightSpacePos = u_DirectLightShadowData.DirLightsShadowData[shadowDataIdx].ProjView[layer] * vec4(samplePos, 1.0);
    vec3 lightSpaceNDC = lightSpacePos.xyz / lightSpacePos.w;

    if(lightSpaceNDC.z < 0.0 || lightSpaceNDC.z > 1.0) {
        return 1.0;
    }

    vec2 uvCoord = lightSpaceNDC.xy * 0.5 + 0.5;
    float diskRadius = (1.0 + (viewPos.z / u_DirectLightShadowData.DirLightsShadowData[shadowDataIdx].CascadeFarDist[cascadeCount - 1])) * TEXEL_SIZE_2048;
    float depthOffset = 2.0 * offsets.y * u_DirectLightShadowData.DirLightsShadowData[shadowDataIdx].CascadeDepthOffsetScales[layer];
    float currDepth = lightSpaceNDC.z - bias - depthOffset;

    float shadow = 0.0;
    int samples = 9;

    for (int i = 0; i < samples; ++i)
    {
        vec2 sampleUVCoord = uvCoord + dirLightSampleOffsetDirections[i] * diskRadius;
        float staticShadow = texture(t_DirLightStaticShadowMaps[shadowDataIdx], vec4(sampleUVCoord, layer, currDepth));
        float dynamicShadow = texture(t_DirLightDynamicShadowMap, vec4(sampleUVCoord, float(shadowDataIdx * 6) + layer, currDepth));
        shadow += min(staticShadow, dynamicShadow);
    }

    float shadowFactor = shadow / float(samples);
    return CalcFadedShadowFactor(shadowFactor, viewPos.z, 0.8, u_DirectLightShadowData.DirLightsShadowData[shadowDataIdx].CascadeFarDist[cascadeCount - 1]);
}

const vec3 pointLightSampleOffsetDirections[20] = vec3[](
   vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1),
   vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
   vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
   vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
   vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
);

float CalcPointLightShadow(int lightIdx, int shadowDataIdx, float fragLightDist, vec3 fragToLight, vec3 fragNormal) {
    float farPlaneDist = u_DirectLightShadowData.PointLightsShadowData[shadowDataIdx].FarDist;

    vec2 offsets = CalcShadowOffsets(fragToLight, fragNormal) * fragLightDist;
    float bias = CalcShadowBias(0.0001, 0.001, fragToLight, fragNormal);

    float normalOffsetScale = u_DirectLightShadowData.PointLightsShadowData[shadowDataIdx].NormalOffsetScale * offsets.x;
    vec3 samplePos = v_WorldPos + fragNormal * normalOffsetScale;

    vec3 lightToSample = samplePos - u_DirectLights.PointLights[lightIdx].Position;
    float sampleDist = length(lightToSample);

    vec3 sampleDir = normalize(lightToSample);
    vec3 absDir = abs(sampleDir);
    if (absDir.y > absDir.x && absDir.y > absDir.z) {
        sampleDir.z = -sampleDir.z;
    } else {
        sampleDir.y = -sampleDir.y;
    }
    float diskRadius = (1.0 + (sampleDist / farPlaneDist)) * TEXEL_SIZE_1024;
    float depthOffset = 2.0 * u_DirectLightShadowData.PointLightsShadowData[shadowDataIdx].DepthOffsetScale * offsets.y;
    float currDepth = (sampleDist / farPlaneDist) - bias - depthOffset;

    float shadow = 0.0;
    int samples = 20;

    for (int i = 0; i < samples; ++i) {
        vec3 fetchDir = sampleDir + pointLightSampleOffsetDirections[i] * diskRadius;
        float staticShadow = 0.0;
        float storedStaticDepth = texture(t_PointLightStaticShadowMaps[shadowDataIdx], fetchDir).r;
        if (currDepth > storedStaticDepth) {
            staticShadow = 1.0;
        }
        float dynamicShadow = 0.0;
        float storedDynamicDepth = texture(t_PointLightDynamicShadowMap, vec4(fetchDir, float(shadowDataIdx))).r;
        if (currDepth > storedDynamicDepth) {
            dynamicShadow = 1.0;
        }
        shadow += max(staticShadow, dynamicShadow);
    }

    float shadowFactor = 1.0 - (shadow / float(samples));
    return CalcFadedShadowFactor(shadowFactor, fragLightDist, 0.8, farPlaneDist);
}

const vec2 spotLightSampleOffsetDirections[9] = vec2[](
    vec2( 0.0,    0.0),
    vec2(-0.707,  0.707),
    vec2( 0.707,  0.707),
    vec2(-0.707, -0.707),
    vec2( 0.707, -0.707),
    vec2( 0.0,    1.0),
    vec2( 0.0,   -1.0),
    vec2(-1.0,    0.0),
    vec2( 1.0,    0.0)
);

float CalcSpotLightShadow(int lightIdx, int shadowDataIdx, float fragLightDist, vec3 fragToLight, vec3 fragNormal) {
    float farPlaneDist = u_DirectLightShadowData.SpotLightsShadowData[shadowDataIdx].FarDist;

    vec2 offsets = CalcShadowOffsets(fragToLight, fragNormal) * fragLightDist;
    float bias = CalcShadowBias(0.0001, 0.001, fragToLight, fragNormal);

    float normalOffsetScale = u_DirectLightShadowData.SpotLightsShadowData[shadowDataIdx].NormalOffsetScale * offsets.x;
    vec3 samplePos = v_WorldPos + fragNormal * normalOffsetScale;

    vec4 lightSpacePos = u_DirectLightShadowData.SpotLightsShadowData[shadowDataIdx].ProjView * vec4(samplePos, 1.0);
    if (lightSpacePos.w <= 0.0) {
        return 1.0;
    }

    vec3 lightSpaceNDC = lightSpacePos.xyz / lightSpacePos.w;

    if (lightSpaceNDC.z < 0.0 || lightSpaceNDC.z > 1.0) {
        return 1.0;
    }

    vec3 lightToSample = samplePos - u_DirectLights.SpotLights[lightIdx].Position;
    float sampleDist = length(lightToSample);
    float normalizedSampleDist = sampleDist / farPlaneDist;

    vec2 uvCoord = lightSpaceNDC.xy * 0.5 + 0.5;
    float diskRadius = (1.0 + normalizedSampleDist) * TEXEL_SIZE_1024;
    float depthOffsetScale = 2.0 * u_DirectLightShadowData.SpotLightsShadowData[shadowDataIdx].DepthOffsetScale * offsets.y;
    float currDepth = normalizedSampleDist - bias - depthOffsetScale;

    float shadow = 0.0;
    int samples = 9;

    for (int i = 0; i < samples; ++i) {
        vec2 sampleUVCoord = uvCoord + spotLightSampleOffsetDirections[i] * diskRadius;
        float staticShadow = 0.0;
        float storedStaticDepth = texture(t_SpotLightStaticShadowMaps[shadowDataIdx], sampleUVCoord).r;
        if (currDepth > storedStaticDepth) {
            staticShadow = 1.0;
        }
        float dynamicShadow = 0.0;
        float storedDynamicDepth = texture(t_SpotLightDynamicShadowMap, vec3(sampleUVCoord, float(shadowDataIdx))).r;
        if (currDepth > storedDynamicDepth) {
            dynamicShadow = 1.0;
        }
        shadow += max(staticShadow, dynamicShadow);
    }

    float shadowFactor = 1.0 - (shadow / float(samples));
    return CalcFadedShadowFactor(shadowFactor, fragLightDist, 0.8, farPlaneDist);
}