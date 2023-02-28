// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Debug.h"
#include "carla/NonCopyable.h"

#include <thread>
#include <vector>

namespace carla {

  class ThreadGroup : private NonCopyable {
  public:

    ThreadGroup() = default;

    ~ThreadGroup() {
      JoinAll();
    }

    template <typename F>
    void CreateThread(F &&functor) {
      _threads.emplace_back(std::forward<F>(functor));
    }

    template <typename F>
    void CreateThreads(size_t count, F functor) {
      _threads.reserve(_threads.size() + count);
      for (size_t i = 0u; i < count; ++i) {
        CreateThread(functor);
      }
    }

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

    std::vector<std::thread> _threads;
  };

} // namespace carla
