
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#define UNICODE
#define _UNICODE

#define NOMINMAX

#include <stdlib.h>
#include <windows.h>

#include "../EngineMain.h"
#include "Config.h"
#include "Platforms/Windows/GL/Win32ApplicationGL.h"

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
    (void)hPrevInstance;
    (void)lpCmdLine;
    (void)nShowCmd;

#if defined(GRAPHICS_API_GL)
    ho::Win32ApplicationGL::CreateInstance(hInstance);
#elif defined(GRAPHICS_API_D3D)
#error "Direct3D is not supported currently."
#endif

    ho::EngineMainParam param{};
    param.Argv = __argv;
    param.Argc = __argc;
    EngineMain(param);
    return 0;
}