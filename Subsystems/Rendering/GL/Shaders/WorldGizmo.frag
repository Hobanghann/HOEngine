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
    vec3 cameraToFarPlane = v_worldFarPlanePos - u_General.WorldCameraPos;

    float t = (-u_General.WorldCameraPos.y / cameraToFarPlane.y);

    if (t < -0.0f) {
        discard;
    }

    vec3 xzPlaneWorldPos = u_General.WorldCameraPos + cameraToFarPlane * t;

    if(!(abs(mod(xzPlaneWorldPos.x, 10.0f) - 0.0f) <= 0.4f || abs(mod(xzPlaneWorldPos.z, 10.0f) - 0.0f) <= 0.4f)) {
        discard;
    }

    vec4 xzPlaneClipCoord = u_Matrices.Proj * u_Matrices.View * vec4(xzPlaneWorldPos, 1.0f);
    vec3 xzPlaneNDC = xzPlaneClipCoord.xyz / xzPlaneClipCoord.w;

    gl_FragDepth = (xzPlaneNDC.z + 1.0f) * 0.5f;

    out_FragColor = vec4(0.5f, 0.5f, 0.5f, 1.0f);
}