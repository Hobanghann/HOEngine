#include "EngineMain.h"

#include "Core/Log/Logger.h"
#include "Platforms/IPlatformApplication.h"

namespace ho
{
int EngineMain(const EngineMainParam& param)
{
    Logger::Init();

    IPlatformApplication::GetInstance().Init(L"Editor", 1280, 720);

    while (IPlatformApplication::GetInstance().ProcessPlatformMessages())
        ;

    return 0;
}
} // namespace ho