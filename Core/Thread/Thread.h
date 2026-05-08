#pragma once

#include <chrono>
#include <string>
#include <thread>

#include "Core/Config.h"
#include "Core/Macros.h"

namespace ho
{

#if defined(THREAD_ENABLED)
class Thread
{
  public:
    using Entry = void (*)(void*);

#if defined(__cpp_lib_hardware_interference_size)
    static constexpr size_t sCacheLineBytes = std::hardware_destructive_interference_size;
#else
    static constexpr size_t sCacheLineBytes = 128;
#endif

    FORCE_INLINE explicit Thread(Entry entry, void* pUserData = nullptr)
      : mThread([=] { entry(pUserData); })
    {
    }

    Thread(const Thread&) = delete;
    Thread& operator=(const Thread&) = delete;

    virtual ~Thread();

    void SetName(const std::string& nameStr);
    static void SetCurrentName(const std::string& nameStr);

    void Join();

    FORCE_INLINE static void Yield()
    {
        std::this_thread::yield();
    }

    FORCE_INLINE static void Sleep(int32_t ms)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
    }

  private:
    std::thread mThread;
};

#else

class Thread
{
  public:
    using Entry = void (*)(void*);

    static constexpr size_t sCacheLineBytes = sizeof(void*);

    FORCE_INLINE explicit Thread(Entry entry, void* pUserData = nullptr)
    {
        entry(pUserData);
    }

    Thread(const Thread&) = delete;
    Thread& operator=(const Thread&) = delete;

    ~Thread() = default;

    FORCE_INLINE void SetName(const std::string&) {}

    FORCE_INLINE static void SetCurrentName(const std::string&) {}

    FORCE_INLINE void Join() {}

    FORCE_INLINE static void Yield() {}

    FORCE_INLINE static void Sleep(int32_t) {}
};

#endif // THREAD_ENABLED

} // namespace ho