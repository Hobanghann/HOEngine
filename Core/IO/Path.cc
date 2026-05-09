#include "Path.h"

#include <cassert>

namespace ho
{

#ifdef HO_PROJECT_ROOT
std::filesystem::path Path::sProjectRootPath = std::filesystem::path(std::string(HO_PROJECT_ROOT));
#else
#error "HO_PROJECT_ROOT must be defined"
#endif

#ifdef HO_ASSET_ROOT
std::filesystem::path Path::sAssetRootPath = std::filesystem::path(std::string(HO_ASSET_ROOT));
#else
#error "HO_ASSET_ROOT must be defined"
#endif

} // namespace ho
