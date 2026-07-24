#include "IPlatformApplication.h"

namespace ho
{
void IPlatformApplication::DestroyPendingSubWindows()
{
    for (auto it = spSubWindows.begin(); it != spSubWindows.end();)
    {
        if (it->get() && it->get()->IsDestroyed())
        {
            it->reset();

            it = spSubWindows.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

std::unique_ptr<IPlatformWindow> IPlatformApplication::spMainWindow = nullptr;
std::list<std::unique_ptr<IPlatformWindow>> IPlatformApplication::spSubWindows =
    std::list<std::unique_ptr<IPlatformWindow>>();
IPlatformApplication* IPlatformApplication::spInstance = nullptr;
} // namespace ho