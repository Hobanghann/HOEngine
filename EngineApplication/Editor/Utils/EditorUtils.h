#pragma once

#include <string>

#include "Scene/Scene.h"

namespace ho
{
// NOTE: Supposed duplicated name handled to 'Name (n)'
std::string getUniqueGameObjectName(ObjectHandle<const RScene> hScene, const std::string& baseNameStr);
std::string getUniqueAssetName(const std::string& baseNameStr);
} // namespace ho