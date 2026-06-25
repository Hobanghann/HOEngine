#version 460 core

const int UNIFORM_MATRICES_BINDING = 1;
const int UNIFORM_GENERAL_BINDING = 3;

const vec3 FarPlaneNDC[6] = vec3[6](
    vec3(1.0f, 1.0f, 0.99999f),
    vec3(1.0f, -1.0f, 0.99999f),
    vec3(-1.0f, -1.0f, 0.99999f),

    vec3(-1.0f, -1.0f, 0.99999f),
    vec3(-1.0f, 1.0f, 0.99999f),
    vec3(1.0f, 1.0f, 0.99999f)
);

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

out gl_PerVertex
{
  vec4 gl_Position;
};

layout(location = 0) out vec3 v_worldFarPlanePos;

void main() {
    vec3 ndc = FarPlaneNDC[gl_VertexID];

    vec4 farPlaneWorldHomogeneous = inverse(u_Matrices.Proj * u_Matrices.View) * vec4(ndc, 1.0);

    v_worldFarPlanePos = farPlaneWorldHomogeneous.xyz / farPlaneWorldHomogeneous.w;

    gl_Position = vec4(ndc, 1.0);
}