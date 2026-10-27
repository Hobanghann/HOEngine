#pragma once

#include <cstdint>
#include <glad/include/glad/glad.h>

#include "../../RenderData/FrameBufferDesc.h"

namespace ho
{

struct FrameBufferNativeHandleGL
{
    GLuint GlFBO = 0;

    /// View texture IDs dynamically created via glTextureView in AttachColorTargetRange()
    /// for subresource/multi-slice color target bindings (e.g., Cubemap faces).
    GLuint GlColorTargetViews[FrameBufferDesc::sMaxColorTargetCount] = {};

    /// View texture ID dynamically created via glTextureView in AttachDepthStencilTargetRange()
    /// for subresource/multi-slice depth-stencil target bindings.
    GLuint GlDepthTargetView = 0;
};

struct BufferNativeHandleGL
{
    GLuint GlBuffer = 0;
};

struct StaticMeshNativeHandleGL

{
    GLuint GlVBO = 0;
    GLuint GlEBO = 0;
};

struct TextureNativeHandleGL
{
    GLuint GlTexture = 0;
};

struct ShaderNativeHandleGL
{
    GLuint GlProgram = 0;
};

struct MorphTargetsNativeHandleGL
{
    GLuint GlMorphTargetsSSBO;
    GLuint GlTargetAttributesSSBO;
};
} // namespace ho