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

const float PI = 3.14159265359;

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

struct DirLight {
    vec3 Direction;
    vec3 Color;
    float Intensity;
};

struct PointLight {
    vec3 Position;
    vec3 Color;
    float Intensity;
    float Constant;
    float Linear;
    float Quadratic;
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
};

layout (std140, binding = UNIFORM_LIGHTS_BINDING) uniform Lights
{
    int DirLightCount;
    DirLight DirLights[MAX_DIR_LIGHT_COUNT];
    int PointLightCount;
    PointLight PointLights[MAX_POINT_LIGHT_COUNT];
    int SpotLightCount;
    SpotLight SpotLights[MAX_SPOT_LIGHT_COUNT];
} u_Lights;

layout (std140, binding = UNIFORM_GENERAL_BINDING) uniform General
{
    vec3 WorldCameraPos;
} u_General;

layout(location = 0, binding = TEX_DIFFUSE_BINDING) uniform sampler2D t_Diffuse;
layout(location = 1, binding = TEX_SPECULAR_BINDING) uniform sampler2D t_Specular;
layout(location = 2, binding = TEX_OPACITY_BINDING) uniform sampler2D t_Opacity;
layout(location = 3, binding = TEX_NORMAL_BINDING) uniform sampler2D t_Normal;
layout(location = 4, binding = TEX_ALBEDO_BINDING) uniform sampler2D t_Albedo;
layout(location = 5, binding = TEX_METALLIC_ROUGHNESS_BINDING) uniform sampler2D t_MetallicRoughness;
layout(location = 6, binding = TEX_EMISSIVE_BINDING) uniform sampler2D t_Emissive;
layout(location = 7, binding = TEX_AMBIENT_OCCLUSION_BINDING) uniform sampler2D t_AmbientOcclusion;

layout(location = 8, binding = TEX_SKY_MAP_BINDING) uniform samplerCube t_SkyMap;
layout(location = 9, binding = TEX_IRRADIANCE_MAP_BINDING) uniform samplerCube t_IrradianceMap;

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
    return (u_MatAttr.UVMatrices[textureIndex] * vec4(baseUV, 0.0f, 1.0f)).xy;
}

vec3 GetNormal()
{
    if (u_MatAttr.HasTextures[TEX_NORMAL_BINDING] == 0)
    {
        return normalize(v_Normal);
    }

    vec3 texNormal = texture(t_Normal, GetUV(TEX_NORMAL_BINDING)).xyz * 2.0f - 1.0f;

    texNormal *= u_MatAttr.NormalScale;
    texNormal = normalize(texNormal);

    vec3 N = normalize(v_Normal);
    vec3 T = normalize(v_Tangent.xyz - N * dot(v_Tangent.xyz, N));
    vec3 B = normalize(cross(N, T)) * v_Tangent.w;
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * texNormal);
}

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0f);
    float NdotH2 = NdotH * NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0f) + 1.0f);
    denom = PI * denom * denom;

    return nom / max(denom, 0.0000001f);
}

float CalcDirectKforSchlickGGX(float roughness) {
    float r = (roughness + 1.0f);
    return (r * r) / 8.0f;
}

float CalcIndirectKforSchlickGGX(float roughness) {
    return (roughness * roughness) / 2.0f;
}

float GeometrySchlickGGX(float NdotV, float k)
{
    float nom   = NdotV;
    float denom = NdotV * (1.0f - k) + k;

    return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float k)
{
    float NdotV = max(dot(N, V), 0.0f);
    float NdotL = max(dot(N, L), 0.0f);

    float ggx1 = GeometrySchlickGGX(NdotL, k);
    float ggx2 = GeometrySchlickGGX(NdotV, k);

    return ggx1 * ggx2;
}

vec3 fresnelSchlickDirect(float cosTheta, vec3 F0)
{
    return F0 + (1.0f - F0) * pow(clamp(1.0f - cosTheta, 0.0f, 1.0f), 5.0f);
}

vec3 fresnelSchlickIndirect(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0f - roughness), F0) - F0) * pow(clamp(1.0f - cosTheta, 0.0f, 1.0f), 5.0f);
}

void main()
{
    vec4 texOpacity = texture(t_Opacity, GetUV(TEX_OPACITY_BINDING));
    vec4 texAlbedo = texture(t_Albedo, GetUV(TEX_ALBEDO_BINDING));
    vec4 texMetallicRoughness = texture(t_MetallicRoughness, GetUV(TEX_METALLIC_ROUGHNESS_BINDING));
    vec4 texEmissive = texture(t_Emissive, GetUV(TEX_EMISSIVE_BINDING));
    vec4 texAO = texture(t_AmbientOcclusion, GetUV(TEX_AMBIENT_OCCLUSION_BINDING));

    float finalOpacity = u_MatAttr.Opacity;
    if (u_MatAttr.HasTextures[3] == 1)
    {
        finalOpacity *= texOpacity.r;
    }

    vec3 albedo = u_MatAttr.Albedo.rgb;
    if (u_MatAttr.HasTextures[5] == 1)
    {
        albedo = texAlbedo.rgb;
    }

    float metallic = u_MatAttr.Metallic;
    float roughness = u_MatAttr.Roughness;
    if (u_MatAttr.HasTextures[6] == 1)
    {
        roughness *= texMetallicRoughness.g;
        metallic *= texMetallicRoughness.b;
    }

    vec3 F0 = vec3(0.04f);
    F0 = mix(F0, albedo, metallic);

    vec3 N = GetNormal();
    vec3 V = normalize(u_General.WorldCameraPos - v_WorldPos);
    vec3 R = reflect(-V, N);
    vec3 Lo = vec3(0.0f);

    for(int i = 0; i < u_Lights.DirLightCount; ++i)
    {
        vec3 L = normalize(-u_Lights.DirLights[i].Direction);
        vec3 H = normalize(V + L);
        vec3 lightRadiance = u_Lights.DirLights[i].Color * u_Lights.DirLights[i].Intensity;

        float NDF = DistributionGGX(N, H, roughness);
        float k = CalcDirectKforSchlickGGX(roughness);
        float G = GeometrySmith(N, V, L, k);
        vec3 F = fresnelSchlickDirect(max(dot(H, V), 0.0f), F0);

        vec3 num = NDF * G * F;
        float denom = 4.0f * max(dot(N, V), 0.0f) * max(dot(N, L), 0.0f) + 0.0001f;
        vec3 specular = num / denom;

        vec3 kS = F;
        vec3 kD = vec3(1.0f) - kS;
        kD *= 1.0f - metallic;

        float NdotL = max(dot(N, L), 0.0f);
        Lo += (kD * albedo / PI + specular) * lightRadiance * NdotL;
    }

    for(int i = 0; i < u_Lights.PointLightCount; ++i)
    {
        float dist = length(u_Lights.PointLights[i].Position - v_WorldPos);

        vec3 L = (u_Lights.PointLights[i].Position - v_WorldPos) / dist;
        vec3 H = normalize(V + L);

        float attenuation = 1.0f / (u_Lights.PointLights[i].Constant + u_Lights.PointLights[i].Linear * dist + u_Lights.PointLights[i].Quadratic * dist * dist);
        vec3 lightRadiance = u_Lights.PointLights[i].Color * u_Lights.PointLights[i].Intensity * attenuation;

        float NDF = DistributionGGX(N, H, roughness);
        float k = CalcDirectKforSchlickGGX(roughness);
        float G = GeometrySmith(N, V, L, k);
        vec3 F = fresnelSchlickDirect(max(dot(H, V), 0.0f), F0);

        vec3 num = NDF * G * F;
        float denom = 4.0f * max(dot(N, V), 0.0f) * max(dot(N, L), 0.0f) + 0.0001f;
        vec3 specular = num / denom;

        vec3 kS = F;
        vec3 kD = vec3(1.0f) - kS;
        kD *= 1.0f - metallic;

        float NdotL = max(dot(N, L), 0.0f);
        Lo += (kD * albedo / PI + specular) * lightRadiance * NdotL;
    }

    for(int i = 0; i < u_Lights.SpotLightCount; ++i)
    {
        float dist = length(u_Lights.SpotLights[i].Position - v_WorldPos);
        vec3 L = (u_Lights.SpotLights[i].Position - v_WorldPos) / dist;
        vec3 H = normalize(L + V);

        float theta = dot(L, normalize(-u_Lights.SpotLights[i].Direction));
        float epsilon = u_Lights.SpotLights[i].InnerCutOff - u_Lights.SpotLights[i].OuterCutOff;
        float angularIntensity = clamp((theta - u_Lights.SpotLights[i].OuterCutOff) / epsilon, 0.0, 1.0);

        float distanceAttenuation = 1.0f / (u_Lights.SpotLights[i].Constant + u_Lights.SpotLights[i].Linear * dist + u_Lights.SpotLights[i].Quadratic * dist * dist);

        vec3 lightRadiance = u_Lights.SpotLights[i].Color * u_Lights.SpotLights[i].Intensity * distanceAttenuation * angularIntensity;

        float NDF = DistributionGGX(N, H, roughness);
        float k = CalcDirectKforSchlickGGX(roughness);
        float G = GeometrySmith(N, V, L, k);
        vec3 F = fresnelSchlickDirect(max(dot(H, V), 0.0f), F0);

        vec3 num = NDF * G * F;
        float denom = 4.0f * max(dot(N, V), 0.0f) * max(dot(N, L), 0.0f) + 0.0001f;
        vec3 specular = num / denom;

        vec3 kS = F;
        vec3 kD = vec3(1.0f) - kS;
        kD *= 1.0f - metallic;

        float NdotL = max(dot(N, L), 0.0f);
        Lo += (kD * albedo / PI + specular) * lightRadiance * NdotL;
    }

    float ao = 1.0f;
    if (u_MatAttr.HasTextures[8] == 1)
    {
        ao *= texture(t_AmbientOcclusion, GetUV(8)).r;
    }
    ao = mix(1.0f, ao, u_MatAttr.OcclusionStrength);

    vec3 ambient = u_MatAttr.Ambient.rgb * albedo * ao;

    vec3 emissive = u_MatAttr.Emissive.rgb * u_MatAttr.EmissiveIntensity;
    if (u_MatAttr.HasTextures[7] == 1)
    {
        emissive *= texEmissive.rgb;
    }

    vec3 finalColor = ambient + Lo + emissive;

    finalColor = finalColor / (finalColor + vec3(1.0f));
    finalColor = pow(finalColor, vec3(1.0f / 2.2f));

    out_FragColor = vec4(finalColor.rgb, finalOpacity);
}