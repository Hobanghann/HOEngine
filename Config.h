#pragma once

#if defined(THREAD_DISABLED)
#undef THREAD_ENABLED
#else
#define THREAD_ENABLED
#endif

enum class eEngineApplicationType
{
    None = 0,
    TestApp = 1,
    ModelViewer = 2,
    Editor = 3,
    Game = 4,
};

enum eGraphicsAPI
{
    GL = 0,
};