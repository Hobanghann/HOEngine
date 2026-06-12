#version 460 core

const int UNIFORM_MATRICES_BINDING = 1;
const int UNIFORM_GENERAL_BINDING = 3;

layout (std140, row_major, binding = UNIFORM_MATRICES_BINDING) uniform Matrices
{
    mat4 World;
    mat4 InvWorld;
    mat4 View;
    mat4 Proj;
} u_Matrices;


layout (std140, binding = UNIFORM_GENERAL_BINDING) uniform GeneralUBO
{
    vec3 WorldCameraPos;
} u_General;

layout(location = 0) in vec3 v_worldFarPlanePos;

layout(location = 0) out vec4 out_FragColor;

layout (depth_less) out float gl_FragDepth;

void main() {
    if (abs(u_General.WorldCameraPos.y) < 0.001f) {
        discard;
    }

    vec3 cameraToFarPlane = v_worldFarPlanePos - u_General.WorldCameraPos;
    float t = (-u_General.WorldCameraPos.y / cameraToFarPlane.y);

    if (t < 0.0f) {
        discard;
    }

    vec3 zxPlaneWorldPos = u_General.WorldCameraPos + cameraToFarPlane * t;

    const float gridSize = 10.0f;
    const float gridLineWidth = 0.5f;

    vec2 zxPlaneEdgeWorldPos = floor((zxPlaneWorldPos.xz / gridSize) + 0.5f) * gridSize;
    vec2 distFromEdge = abs(zxPlaneWorldPos.xz - zxPlaneEdgeWorldPos);

    vec2 zxPlaneSizePerPixel = fwidth(zxPlaneWorldPos.xz);

    vec2 gridLineRatioPerPixel = (gridLineWidth * 0.5f - distFromEdge) / zxPlaneSizePerPixel;

    vec2 finalRatio = clamp(gridLineRatioPerPixel + 0.5f, 0.0f, 1.0f);

    const float fadeEdge0 = 100.0f;
    const float fadeEdge1 = 600.0f;
    float zxPlaneDist = length(zxPlaneWorldPos - u_General.WorldCameraPos);
    float fade = 1.0f - smoothstep(fadeEdge0, fadeEdge1, zxPlaneDist);

    float finalAlpha = max(finalRatio.x, finalRatio.y) * fade;

    vec4 zxPlaneClipCoord = u_Matrices.Proj * u_Matrices.View * vec4(zxPlaneWorldPos, 1.0f);
    vec3 zxPlaneNDC = zxPlaneClipCoord.xyz / zxPlaneClipCoord.w;

    gl_FragDepth = (zxPlaneNDC.z + 1.0f) * 0.5f;

    out_FragColor = vec4(0.5f, 0.5f, 0.5f, finalAlpha);
}