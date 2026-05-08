#include "thread.h"

#if defined(THREAD_ENABLED)

#if defined(_WIN32)
#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif
// clang-format off
#include <windows.h>
#include <processthreadsapi.h>
// clang-format on

#elif defined(__linux__)
#include <pthread.h>
#endif

namespace ho
{

Thread::~Thread()
{
    if (mThread.joinable())
    {
        mThread.join();
    }
}

void Thread::Join()
{
    if (mThread.joinable())
    {
        mThread.join();
    }
}

void Thread::SetName(const std::string& nameStr)
{
#if defined(_WIN32)
    std::wstring wNameStr(nameStr.begin(), nameStr.end());
    SetThreadDescription(static_cast<HANDLE>(mThread.native_handle()), wNameStr.c_str());
#elif defined(__linux__)
    pthread_setname_np(mThread.native_handle(), nameStr.c_str());
#endif
}

void Thread::SetCurrentName(const std::string& nameStr)
{
#if defined(_WIN32)
    std::wstring wNameStr(nameStr.begin(), nameStr.end());
    SetThreadDescription(GetCurrentThread(), wNameStr.c_str());
#elif defined(__linux__)
    pthread_setname_np(pthread_self(), nameStr.c_str());
#endif
}

} // namespace ho

#endif // THREAD_ENABLED