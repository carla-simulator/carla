// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <queue>

#include "carla/NonCopyable.h"

namespace carla {
namespace server {

  /// A thread-safe queue.
  ///
  /// From "C++ Concurrency In Action", Anthony Williams.
  template<typename T>
  class ThreadSafeQueue : private NonCopyable {
  public:

    ThreadSafeQueue() : _done(false) {}

    ~ThreadSafeQueue() {
      set_done(true);
    }

    bool empty() const {
      std::lock_guard<std::mutex> lock(_mutex);
      return _queue.empty();
    }

    bool done() const {
      return _done;
    }

    void set_done(bool done = true) {
      _done = done;
      _condition.notify_all();
    }

    void Push(T &&new_value) {
      std::lock_guard<std::mutex> lock(_mutex);
      _queue.push(std::forward<T>(new_value));
      _condition.notify_one();
    }

    bool WaitAndPop(T &value) {
      std::unique_lock<std::mutex> lock(_mutex);
      _condition.wait(lock, [this] { return _done || !_queue.empty(); });
      if (_queue.empty()) {
        return false;
      }
      value = _queue.front();
      _queue.pop();
      return true;
    }

    bool TryPop(T &value) {
      std::lock_guard<std::mutex> lock(_mutex);
      if (_queue.empty()) {
        return false;
      }
      value = _queue.front();
      _queue.pop();
      return true;
    }

  private:

    mutable std::mutex _mutex;

    std::queue<T> _queue;

    std::atomic_bool _done;

    std::condition_variable _condition;
  };

} // namespace server
} // namespace carla
