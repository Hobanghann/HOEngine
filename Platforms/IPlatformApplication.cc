#include "IPlatformApplication.h"

namespace ho
{
std::unique_ptr<IPlatformWindow> IPlatformApplication::spMainWindow = nullptr;
IPlatformApplication* IPlatformApplication::spInstance = nullptr;
} // namespace ho