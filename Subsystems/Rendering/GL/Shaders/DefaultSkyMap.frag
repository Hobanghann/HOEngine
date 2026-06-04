#version 460 core

const int TEX_SKY_MAP_BINDING = 9;

layout(location = 8, binding = TEX_SKY_MAP_BINDING) uniform samplerCube t_SkyMap;

layout(location = 0) in vec3 v_UVW;

layout(location = 0) out vec4 out_FragColor;

void main() {
    vec3 finalColor = texture(t_SkyMap, v_UVW).rgb;
    finalColor = finalColor / (finalColor + vec3(1.0f));
    finalColor = pow(finalColor, vec3(1.0f / 2.2f));
    out_FragColor = vec4(finalColor, 1.0f);
}