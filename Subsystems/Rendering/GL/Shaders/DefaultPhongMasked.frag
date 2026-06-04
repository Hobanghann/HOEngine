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
    float Range;
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
    return (u_MatAttr.UVMatrices[textureIndex] * vec4(baseUV, 1.0f, 0.0f)).xy;
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

void main()
{
    vec4 texDiffuse = texture(t_Diffuse, GetUV(TEX_DIFFUSE_BINDING));
    vec4 texSpecular = texture(t_Specular, GetUV(TEX_SPECULAR_BINDING));
    vec4 texOpacity = texture(t_Opacity, GetUV(TEX_OPACITY_BINDING));
    vec4 texAlbedo = texture(t_Albedo, GetUV(TEX_ALBEDO_BINDING));
    vec4 texEmissive = texture(t_Emissive, GetUV(TEX_EMISSIVE_BINDING));
    vec4 texAO = texture(t_AmbientOcclusion, GetUV(TEX_AMBIENT_OCCLUSION_BINDING));

    float finalOpacity = u_MatAttr.Opacity;

    if (u_MatAttr.HasTextures[3] == 1) {
        finalOpacity *= texOpacity.r;
    }
    else if (u_MatAttr.HasTextures[1] == 1) {
        finalOpacity *= texDiffuse.a;
    }
    else if (u_MatAttr.HasTextures[5] == 1) {
        finalOpacity *= texAlbedo.a;
    }

    if (finalOpacity < u_MatAttr.AlphaThreshold)
    {
        discard;
    }

    vec3 diffColor = u_MatAttr.Diffuse.rgb;
    if (u_MatAttr.HasTextures[1] == 1)
    {
        diffColor = texDiffuse.rgb;
    }
    else if (u_MatAttr.HasTextures[5] == 1)
    {
        diffColor = texAlbedo.rgb;
    }

    vec3 specColor = u_MatAttr.Specular.rgb;
    if (u_MatAttr.HasTextures[2] == 1)
    {
        specColor *= texSpecular.rgb;
    }

    vec3 N = GetNormal();
    vec3 V = normalize(u_General.WorldCameraPos - v_WorldPos);

    vec3 totalDiffuse = vec3(0.0f);
    vec3 totalSpecular = vec3(0.0f);

    for(int i = 0; i < u_Lights.DirLightCount; ++i)
    {
        vec3 L = normalize(-u_Lights.DirLights[i].Direction);
        vec3 H = normalize(L + V);

        float NdotL = max(dot(N, L), 0.0f);
        float NdotH = max(dot(N, H), 0.0f);

        vec3 lightColor = u_Lights.DirLights[i].Color * u_Lights.DirLights[i].Intensity;

        totalDiffuse += diffColor * NdotL * lightColor;

        float specPower = pow(NdotH, u_MatAttr.Shininess);
        totalSpecular += specColor * specPower * lightColor * step(0.0001, NdotL);
    }

    for(int i = 0; i < u_Lights.PointLightCount; ++i)
    {
        float dist = length(u_Lights.PointLights[i].Position - v_WorldPos);

        vec3 L = (u_Lights.PointLights[i].Position - v_WorldPos) / dist;
        vec3 H = normalize(L + V);

        float NdotL = max(dot(N, L), 0.0f);
        float NdotH = max(dot(N, H), 0.0f);

        float attenuation = 1.0f / (u_Lights.PointLights[i].Constant + u_Lights.PointLights[i].Linear * dist + u_Lights.PointLights[i].Quadratic * dist * dist);
        vec3 lightColor = u_Lights.PointLights[i].Color * u_Lights.PointLights[i].Intensity * attenuation;

        totalDiffuse += diffColor * NdotL * lightColor;

        float specPower = pow(NdotH, u_MatAttr.Shininess);
        totalSpecular += specColor * specPower * lightColor * step(0.0001, NdotL);
    }

    for(int i = 0; i < u_Lights.SpotLightCount; ++i)
    {
        float dist = length(u_Lights.SpotLights[i].Position - v_WorldPos);
        vec3 L = (u_Lights.SpotLights[i].Position - v_WorldPos) / dist;

        float theta = dot(L, normalize(-u_Lights.SpotLights[i].Direction));
        float epsilon = u_Lights.SpotLights[i].InnerCutOff - u_Lights.SpotLights[i].OuterCutOff;
        float angularIntensity = clamp((theta - u_Lights.SpotLights[i].OuterCutOff) / epsilon, 0.0, 1.0);

        float distanceAttenuation = 1.0f / (u_Lights.SpotLights[i].Constant + u_Lights.SpotLights[i].Linear * dist + u_Lights.SpotLights[i].Quadratic * dist * dist);

        vec3 lightColor = u_Lights.SpotLights[i].Color * u_Lights.SpotLights[i].Intensity * distanceAttenuation * angularIntensity;

        vec3 H = normalize(L + V);
        float NdotL = max(dot(N, L), 0.0f);
        float NdotH = max(dot(N, H), 0.0f);

        totalDiffuse += diffColor * NdotL * lightColor;

        float specPower = pow(NdotH, u_MatAttr.Shininess);
        totalSpecular += specColor * specPower * lightColor * step(0.0001, NdotL);
    }

    float ao = 1.0f;
    if (u_MatAttr.HasTextures[8] == 1)
    {
        ao = texAO.r;
    }
    ao = mix(1.0f, ao, u_MatAttr.OcclusionStrength);
    vec3 ambient = u_MatAttr.Ambient.rgb * ao;

    vec3 emissive = u_MatAttr.Emissive.rgb * u_MatAttr.EmissiveIntensity;
    if (u_MatAttr.HasTextures[7] == 1)
    {
        emissive = texEmissive.rgb * u_MatAttr.EmissiveIntensity;
    }

    vec3 finalColor = ambient + totalDiffuse + totalSpecular + emissive;

    finalColor = clamp(finalColor, 0.0f, 1.0f);
    finalColor = pow(finalColor, vec3(1.0f / 2.2f));

    out_FragColor = vec4(finalColor, finalOpacity);
}