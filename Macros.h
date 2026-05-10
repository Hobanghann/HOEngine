#pragma once

#ifndef FORCE_INLINE
#if defined(_MSC_VER)
#define FORCE_INLINE __forceinline
#elif defined(__clang__)
#define FORCE_INLINE __attribute__((always_inline)) inline
#else
#define FORCE_INLINE inline
#endif
#endif

#ifndef INLINE
#define INLINE inline
#endif

#ifndef NO_INLINE
#if defined(_MSC_VER)
#define NO_INLINE __declspec(noinline)
#elif defined(__clang__)
#define NO_INLINE __attribute__((noinline))
#else
#define NO_INLINE
#endif
#endif

#ifndef HO_DEBUG_BREAK
#if defined(_MSC_VER)
#define HO_DEBUG_BREAK() __debugbreak()
#elif defined(__clang__)
#define HO_DEBUG_BREAK() __builtin_debugtrap()
#else
#define HO_DEBUG_BREAK() ((void)0)
#endif
#endif

#ifndef HO_ASSERT
#if defined(NDEBUG)
#define HO_ASSERT(cond, msg) ((void)0)
#else
#define HO_ASSERT(cond, msg) ((void)((!!(cond)) || (HO_DEBUG_BREAK(), 0)))
#endif
#endif

#if defined(_MSC_VER)
#define HO_DISABLE_COMPILER_WARNING_PUSH() __pragma(warning(push))
#define HO_DISABLE_COMPILER_WARNING_POP() __pragma(warning(pop))
#define HO_DISABLE_SPECIFIC_COMPILER_WARNING(w) __pragma(warning(disable : w))

#define HO_DISABLE_COMPILER_WARNING_PADDED_DUE_TO_ALIGNMENT_SPECIFIER() HO_DISABLE_SPECIFIC_COMPILER_WARNING(4324)
#define HO_DISABLE_COMPILER_WARNING_FAILING_TO_HOLD_LOCK() HO_DISABLE_SPECIFIC_COMPILER_WARNING(26110)

#elif defined(__clang__)
#define HO_DISABLE_COMPILER_WARNING_PUSH() _Pragma("clang diagnostic push")
#define HO_DISABLE_COMPILER_WARNING_POP() _Pragma("clang diagnostic pop")
#define HO_PRAGMA_TO_STR(x) #x
#define HO_DISABLE_SPECIFIC_COMPILER_WARNING(w) _Pragma(HO_PRAGMA_TO_STR(clang diagnostic ignored(w)))

#define HO_DISABLE_COMPILER_WARNING_PADDED_DUE_TO_ALIGNMENT_SPECIFIER() HO_DISABLE_SPECIFIC_COMPILER_WARNING("-Wpadded")
#define HO_DISABLE_COMPILER_WARNING_FAILING_TO_HOLD_LOCK()
#endif
