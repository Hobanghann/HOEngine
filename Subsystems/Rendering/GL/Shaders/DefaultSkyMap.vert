#version 460 core

const int UNIFORM_MATRICES_BINDING = 1;

const vec3 skyboxVertices[36] = vec3[36](
    vec3(-1.0f,  1.0f, -1.0f), vec3(-1.0f, -1.0f, -1.0f), vec3( 1.0f, -1.0f, -1.0f),
    vec3( 1.0f, -1.0f, -1.0f), vec3( 1.0f,  1.0f, -1.0f), vec3(-1.0f,  1.0f, -1.0f),

    vec3(-1.0f, -1.0f,  1.0f), vec3(-1.0f, -1.0f, -1.0f), vec3(-1.0f,  1.0f, -1.0f),
    vec3(-1.0f,  1.0f, -1.0f), vec3(-1.0f,  1.0f,  1.0f), vec3(-1.0f, -1.0f,  1.0f),

    vec3( 1.0f, -1.0f, -1.0f), vec3( 1.0f, -1.0f,  1.0f), vec3( 1.0f,  1.0f,  1.0f),
    vec3( 1.0f,  1.0f,  1.0f), vec3( 1.0f,  1.0f, -1.0f), vec3( 1.0f, -1.0f, -1.0f),

    vec3(-1.0f, -1.0f,  1.0f), vec3(-1.0f,  1.0f,  1.0f), vec3( 1.0f,  1.0f,  1.0f),
    vec3( 1.0f,  1.0f,  1.0f), vec3( 1.0f, -1.0f,  1.0f), vec3(-1.0f, -1.0f,  1.0f),

    vec3(-1.0f,  1.0f, -1.0f), vec3( 1.0f,  1.0f, -1.0f), vec3( 1.0f,  1.0f,  1.0f),
    vec3( 1.0f,  1.0f,  1.0f), vec3(-1.0f,  1.0f,  1.0f), vec3(-1.0f,  1.0f, -1.0f),

    vec3(-1.0f, -1.0f, -1.0f), vec3(-1.0f, -1.0f,  1.0f), vec3( 1.0f, -1.0f, -1.0f),
    vec3( 1.0f, -1.0f, -1.0f), vec3(-1.0f, -1.0f,  1.0f), vec3( 1.0f, -1.0f,  1.0f)
);

layout (std140, row_major, binding = UNIFORM_MATRICES_BINDING) uniform Matrices
{
    mat4 World;
    mat4 InvWorld;
    mat4 View;
    mat4 Proj;
} u_Matrices;

out gl_PerVertex
{
  vec4 gl_Position;
};

layout(location = 0) out vec3 v_UVW;

void main() {
    vec3 pos = skyboxVertices[gl_VertexID];

    v_UVW = pos;

    vec4 clipPos = u_Matrices.Proj * mat4(mat3(u_Matrices.View)) * vec4(pos, 1.0);

    gl_Position = clipPos.xyww;
}