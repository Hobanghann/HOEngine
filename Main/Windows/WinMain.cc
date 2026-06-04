
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#define UNICODE
#define _UNICODE

#define NOMINMAX

#include <stdlib.h>
#include <windows.h>

#include "../EngineMain.h"
#include "Platforms/Windows/GL/Win32ApplicationGL.h"

// Intel integrated graphics drivers do not fully support OpenGL 4.6.
// Therefore, engine currently force the use of a dedicated GPU.
extern "C"
{
    __declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
    (void)hPrevInstance;
    (void)lpCmdLine;
    (void)nShowCmd;

    ho::Win32ApplicationGL::CreateInstance(hInstance);

    ho::EngineMainParam param{};
    param.Argv = __argv;
    param.Argc = __argc;
    EngineMain(param);

    ho::Win32ApplicationGL::DeleteInstance();
    return 0;
}