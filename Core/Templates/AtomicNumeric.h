#pragma once

#include <atomic>

#include "Macros.h"

namespace ho
{
template <typename T>
class AtomicNumeric
{
  public:
    explicit AtomicNumeric(T value)
    {
        Set(value);
    }

    void Set(T value)
    {
        mValue.store(value, std::memory_order_release);
    }

    [[nodiscard]] T Get() const
    {
        return mValue.load(std::memory_order_acquire);
    }

    T Increment()
    {
        return mValue.fetch_add(1, std::memory_order_acq_rel) + 1;
    }

    T Decrement()
    {
        return mValue.fetch_sub(1, std::memory_order_acq_rel) - 1;
    }

    T Add(T value)
    {
        return mValue.fetch_add(value, std::memory_order_acq_rel) + value;
    }

    T Sub(T value)
    {
        return mValue.fetch_sub(value, std::memory_order_acq_rel) - value;
    }

    T ExchangeIfGreater(T newValue)
    {
        while (true)
        {
            T old = mValue.load(std::memory_order_acquire);
            if (old >= newValue)
            {
                return old;
            }

            if (mValue.compare_exchange_weak(old, newValue, std::memory_order_acq_rel))
            {
                return newValue;
            }
        }
    }

    [[nodiscard]] bool TestEqual(T newValue)
    {
        const T old = mValue.load(std::memory_order_relaxed);
        return old == newValue;
    }

    [[nodiscard]] bool TestNotEqual(T newValue)
    {
        const T old = mValue.load(std::memory_order_relaxed);
        return old != newValue;
    }

    [[nodiscard]] bool TestLess(T newValue)
    {
        const T old = mValue.load(std::memory_order_relaxed);
        return old > newValue;
    }

    [[nodiscard]] bool TestGreater(T newValue)
    {
        const T old = mValue.load(std::memory_order_relaxed);
        return old < newValue;
    }

    [[nodiscard]] bool TestLessEqual(T newValue)
    {
        const T old = mValue.load(std::memory_order_relaxed);
        return old >= newValue;
    }

    [[nodiscard]] bool TestGreaterEqual(T newValue)
    {
        const T old = mValue.load(std::memory_order_relaxed);
        return old <= newValue;
    }

  private:
    std::atomic<T> mValue;

    static_assert(std::atomic<T>::is_always_lock_free);
};
} // namespace ho