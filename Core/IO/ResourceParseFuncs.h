#pragma once

#include <memory>
#include <string>

#include "ModelIR.h"
#include "ShaderIR.h"

namespace ho
{
class Path;
class Image;

namespace parser
{

[[nodiscard]] std::unique_ptr<const ModelIR> parseModelFile(const Path& path,
                                                            const std::string& nameStr,
                                                            bool bMakeStatic,
                                                            bool bConvertToLeftHanded);

[[nodiscard]] std::unique_ptr<const TextureIR> parseTextureFile(const Path& path,
                                                                const std::string& nameStr,
                                                                bool bLinear);

[[nodiscard]] std::unique_ptr<const ShaderIR> parseShaderFile(const Path& path, const std::string& nameStr);
} // namespace parser

} // namespace ho