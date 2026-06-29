#pragma once

#if defined(THREAD_DISABLED)
#undef THREAD_ENABLED
#else
#define THREAD_ENABLED
#endif

#if defined(LOG_DISABLED)
#undef LOG_ENABLED
#else
#define LOG_ENABLED
#endif

enum class eEngineApplicationType
{
    None = 0,
    TestApp = 1,
    Editor = 2,
    Game = 3,
};

enum eGraphicsAPI
{
    GL = 0,
};