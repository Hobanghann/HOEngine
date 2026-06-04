#pragma once

#include <cstdint>
#include <glad/include/glad/glad.h>

namespace ho
{

struct StaticMeshNativeHandleGL
{
    GLuint GlVbo = 0;
    GLuint GlEbo = 0;
};

struct MaterialNativeHandleGL
{
    GLuint GlUbo = 0;
};

struct TextureNativeHandleGL
{
    GLuint GlTexture = 0;
};

struct ShaderNativeHandleGL
{
    GLuint GlProgram = 0;
};
} // namespace ho