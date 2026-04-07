// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

/// @file ThreadGroup.h
/// @brief Thread management utilities for CARLA
///
/// Provides a simple thread group abstraction for managing multiple threads
/// with automatic cleanup on destruction.

#ifndef CARLA_THREADGROUP_H
#define CARLA_THREADGROUP_H

#include "carla/Debug.h"
#include "carla/NonCopyable.h"

#include <thread>
#include <vector>
#include <cstddef>

namespace carla {

  /// @brief Manages a group of worker threads with automatic cleanup.
  ///
  /// ThreadGroup provides a convenient way to create and manage multiple
  /// threads. All threads are automatically joined when the ThreadGroup
  /// is destroyed, ensuring proper cleanup.
  ///
  /// This class is non-copyable to prevent accidental duplication of
  /// thread ownership.
  ///
  /// @example
  /// ```cpp
  /// ThreadGroup threads;
  ///
  /// // Create a single thread
  /// threads.CreateThread([]() {
  ///   // Thread work here
  /// });
  ///
  /// // Create multiple threads
  /// threads.CreateThreads(4, [](size_t id) {
  ///   // Worker thread with ID
  /// });
  ///
  /// // Threads are automatically joined on destruction
  /// // Or explicitly join with: threads.JoinAll();
  /// ```
  class ThreadGroup : private NonCopyable {
  public:
    /// @brief Default constructor.
    ThreadGroup() = default;

    /// @brief Destructor - joins all threads.
    ///
    /// Automatically calls JoinAll() to ensure all threads are properly
    /// cleaned up before destruction.
    ~ThreadGroup() {
      JoinAll();
    }

    /// @brief Create a new thread with the given functor.
    ///
    /// @tparam F Functor type (callable object).
    /// @param functor Callable to execute in the new thread.
    ///
    /// @example
    /// ```cpp
    /// threads.CreateThread([]() {
    ///   std::cout << "Hello from thread!" << std::endl;
    /// });
    /// ```
    template <typename F>
    void CreateThread(F &&functor) {
      _threads.emplace_back(std::forward<F>(functor));
    }

    /// @brief Create multiple threads with the same functor.
    ///
    /// @tparam F Functor type (callable object).
    /// @param count Number of threads to create.
    /// @param functor Callable to execute in each thread.
    ///
    /// @example
    /// ```cpp
    /// threads.CreateThreads(4, [](size_t id) {
    ///   std::cout << "Worker " << id << " starting" << std::endl;
    /// });
    /// ```
    template <typename F>
    void CreateThreads(size_t count, F functor) {
      _threads.reserve(_threads.size() + count);
      for (size_t i = 0u; i < count; ++i) {
        CreateThread(functor);
      }
    }

    /// @brief Join all threads and clear the thread list.
    ///
    /// Waits for all threads to complete execution. This is called
    /// automatically in the destructor.
    ///
    /// @warning Should not be called from within one of the managed threads.
    ///
    /// @example
    /// ```cpp
    /// threads.CreateThread(long_running_task);
    /// // ... do other work ...
    /// threads.JoinAll();  // Wait for task to complete
    /// ```
    void JoinAll() {
      for (auto &thread : _threads) {
        DEBUG_ASSERT_NE(thread.get_id(), std::this_thread::get_id());
        if (thread.joinable()) {
          thread.join();
        }
      }
      _threads.clear();
    }

  private:
    std::vector<std::thread> _threads;  ///< Managed threads
  };

} // namespace carla

#endif // CARLA_THREADGROUP_H
