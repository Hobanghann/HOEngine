#pragma once

#if defined(THREAD_DISABLED)
#undef THREAD_ENABLED
#else
#define THREAD_ENABLED
#endif

#define GRAPHICS_API_GL