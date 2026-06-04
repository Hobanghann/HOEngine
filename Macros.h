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

#define HO_DISABLE_COMPILER_WARNING_FAILING_TO_HOLD_LOCK() HO_DISABLE_SPECIFIC_COMPILER_WARNING(26110)

#elif defined(__clang__)
#define HO_DISABLE_COMPILER_WARNING_PUSH() _Pragma("clang diagnostic push")
#define HO_DISABLE_COMPILER_WARNING_POP() _Pragma("clang diagnostic pop")
#define HO_PRAGMA_TO_STR(x) #x
#define HO_DISABLE_SPECIFIC_COMPILER_WARNING(w) _Pragma(HO_PRAGMA_TO_STR(clang diagnostic ignored(w)))

#define HO_DISABLE_COMPILER_WARNING_FAILING_TO_HOLD_LOCK()
#endif

/*
 * This function is adapted from Godot Engine's spin_lock.h.
 *
 * The original work is licensed under the MIT License:
 *
 * Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.
 * Copyright (c) 2014-present Godot Engine contributors.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Local modifications: function name normalized to CPU_PAUSE and minor
 * formatting changes. No functional changes to the original logic.
 */

#ifdef _MSC_VER
#include <intrin.h>
#endif

#if defined(_MSC_VER)
// ----- MSVC.
#if defined(_M_ARM) || defined(_M_ARM64)
#define CPU_PAUSE() __yield()
#elif defined(_M_IX86) || defined(_M_X64)
#define CPU_PAUSE() _mm_pause()
#else
#define CPU_PAUSE() ((void)0) // Fallback
#endif

#elif defined(__GNUC__) || defined(__clang__)
// ----- GCC/Clang.
#if defined(__i386__) || defined(__x86_64__)
#define CPU_PAUSE() __builtin_ia32_pause()
#elif defined(__arm__) || defined(__aarch64__)
#define CPU_PAUSE() __asm__ __volatile__("yield")
#elif defined(__powerpc__)
#define CPU_PAUSE() __asm__ __volatile__("or 27,27,27")
#elif defined(__riscv)
#define CPU_PAUSE() __asm__ __volatile__(".insn i 0x0F, 0, x0, x0, 0x010")
#else
#define CPU_PAUSE() ((void)0) // Fallback
#endif

#else
// ----- Unknown Compiler.
#define CPU_PAUSE() ((void)0)
#endif