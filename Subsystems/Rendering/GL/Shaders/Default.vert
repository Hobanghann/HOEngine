#version 460 core

const int INSTANCE_DATA_BINDING = 0;
const int UNIFORM_MATRICES_BINDING = 1;

layout (location = 0) in vec3 a_Pos;
layout (location = 1) in vec3 a_Normal;
layout (location = 2) in vec4 a_Tangent;
layout (location = 3) in vec2 a_UV0;
layout (location = 4) in vec2 a_UV1;
layout (location = 5) in vec4 a_Color;

struct InstanceData
{
    mat4 WorldMat;
};

layout(std140, binding = INSTANCE_DATA_BINDING) readonly buffer InstanceBuffer
{
    InstanceData Instances[];
} s_IntanceBuffer;

layout (std140, row_major, binding = UNIFORM_MATRICES_BINDING) uniform Matrices
{
    mat4 View;
    mat4 Proj;
} u_Matrices;

out gl_PerVertex
{
  vec4 gl_Position;
};

layout(location = 0) out vec3 v_WorldPos;
layout(location = 1) out vec3 v_Normal;
layout(location = 2) out vec4 v_Tangent;
layout(location = 3) out vec2 v_UV0;
layout(location = 4) out vec2 v_UV1;
layout(location = 5) out vec4 v_Color;

void main() {
    mat4 worldMat = transpose(s_IntanceBuffer.Instances[gl_BaseInstance + gl_InstanceID].WorldMat);
    mat3 normalMat = transpose(inverse(mat3(worldMat)));

    v_WorldPos = (worldMat * vec4(a_Pos, 1.0f)).xyz;
    v_Normal = normalMat * a_Normal;
    v_Tangent = vec4((worldMat * vec4(a_Tangent.xyz, 0.0f)).xyz, a_Tangent.w);
    v_UV0 = vec2(a_UV0.x, 1.0 - a_UV0.y);
    v_UV1 = vec2(a_UV1.x, 1.0 - a_UV1.y);
    v_Color = a_Color;

    gl_Position = u_Matrices.Proj * u_Matrices.View * worldMat * vec4(a_Pos, 1.0f);
}