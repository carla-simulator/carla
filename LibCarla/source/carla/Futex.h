// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <atomic>
#include <utility>
#include <type_traits>
#include <boost/optional.hpp>

#if __cplusplus >= 202002L
    #define HAS_ATOMIC_WAIT_NOTIFY
#elif defined(_WIN32)
    #define WIN32_LEAN_AND_MEAN
    #include <Windows.h>
    #pragma comment(lib, "Synchronization.lib")
#else
    #include <cstdint>
    #include <climits>
    #if __has_include(<sys/syscall.h>)
        #include <sys/syscall.h>
        #if __has_include(<sys/futex.h>)
            #include <sys/futex.h>
            #define HAS_FUTEX
        #elif __has_include(<linux/futex.h>)
            #include <linux/futex.h>
            #define HAS_FUTEX
        #endif
    #endif
#endif

#define LOOP_WAIT_MODE 0

namespace carla
{
  namespace futex
  {
    template <typename T, typename U>
    inline static void wait(
      const std::atomic<T>& target,
      U&& old_value,
      boost::optional<std::chrono::milliseconds> timeout = { })
    {
#ifdef _WIN32

      DWORD timeout_ms = INFINITE;
      if (timeout.has_value())
        timeout_ms = static_cast<DWORD>(timeout.value().count());
      while (target.load(std::memory_order_acquire) == old_value)
      {
        (void)WaitOnAddress(
          reinterpret_cast<volatile PVOID>(&target),
          (PVOID)&old_value,
          sizeof(old_value),
          timeout_ms);
      }

#elif defined(HAS_FUTEX)

      uint32_t timeout_ms;
      auto timeout_ms_ptr = &timeout_ms;
      if (timeout.has_value())
        timeout_ms = static_cast<uint32_t>(timeout.value().count());
      else
        timeout_ms_ptr = nullptr;
      while (target.load(std::memory_order_acquire) == old_value)
      {
        (void)syscall(
          SYS_futex,
          reinterpret_cast<const uint32_t*>(&target),
          FUTEX_WAIT,
          &old_value,
          timeout_ms_ptr, nullptr, 0);
      }

#else

      while (target.load(std::memory_order_acquire) == old_value)
        std::this_thread::yield();

#endif
    }

    template <typename T>
    inline static void wake(
      T& target,
      bool all = true)
    {
#ifdef _WIN32

      if (all)
        (void)WakeByAddressAll((PVOID)&target);
      else
        (void)WakeByAddressSingle((PVOID)&target);

#elif defined(HAS_FUTEX)

      (void)syscall(
        SYS_futex,
        reinterpret_cast<uint32_t*>(&target),
        FUTEX_WAKE,
        all ? INT_MAX : 1,
        nullptr, nullptr, 0);

#else
      // Do nothing
#endif
    }
  }
}
