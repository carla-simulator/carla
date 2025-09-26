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

#if __has_include("carla/AtomicSharedPtr.h")
#include "carla/AtomicSharedPtr.h"
#endif

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



namespace carla
{
  namespace futex
  {
    template <typename T, typename U, typename = std::enable_if_t<std::is_convertible<T, U>::value>>
    inline static void wait(
      std::atomic<T>& target,
      U desired,
      boost::optional<std::chrono::milliseconds> timeout = { })
    {
#ifdef _WIN32
      DWORD timeout_ms = INFINITE;
      if (timeout.has_value())
        timeout_ms = (DWORD)timeout.value().count();
      while (true)
      {
        T current = target.load(std::memory_order_cquire);
        if (current == desired)
          break;
        (void)WaitOnAddress(
          (volatile PVOID)&target,
          (PVOID)&current,
          sizeof(T),
          timeout_ms);
      }
#elif defined(HAS_FUTEX)
      uint32_t timeout_ms;
      auto timeout_ms_ptr = &timeout_ms;
      if (timeout.has_value())
        timeout_ms = static_cast<uint32_t>(timeout.value().count());
      else
        timeout_ms_ptr = nullptr;
      while (true)
      {
        T expected = target.load(std::memory_order_acquire);
        if (expected == desired)
          break;
        (void)syscall(
          SYS_futex,
          reinterpret_cast<uint32_t*>(&target),
          FUTEX_WAIT,
          &expected,
          timeout_ms_ptr, nullptr, 0);
      }
#else
      while (target.load(std::memory_order_acquire) != desired)
        std::this_thread::yield();
#endif
    }

    template <typename T, typename U>
    inline static void wait(
      AtomicSharedPtr<T>& target,
      U desired,
      boost::optional<std::chrono::milliseconds> timeout = { })
    {
#ifdef _WIN32
      auto timeout_ms = (DWORD)timeout.value().count();
      timeout_ms = timeout_ms != 0 ? timeout_ms : INFINITE;
      while (true)
      {
        T current = target.load();
        if (current == desired)
          break;
        (void)WaitOnAddress(
          reinterpret_cast<volatile PVOID*>(&target),
          (PVOID)&current,
          sizeof(T),
          timeout_ms);
      }
#elif defined(HAS_FUTEX)
      auto timeout_ms = static_cast<uint32_t>(timeout.value().count());
      auto timeout_ms_ptr = timeout_ms != 0 ? &timeout_ms : nullptr;
      while (true)
      {
        T expected = target.load();
        if (expected == desired)
          break;
        (void)syscall(
          SYS_futex,
          reinterpret_cast<uint32_t*>(target),
          FUTEX_WAIT,
          &expected,
          timeout_ms_ptr, nullptr, 0);
      }
#else
      while (target.load() != desired)
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
        WakeByAddressAll((PVOID)&target);
      else
        WakeByAddressSingle((PVOID)&target);
#elif defined(HAS_FUTEX)
      syscall(
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
