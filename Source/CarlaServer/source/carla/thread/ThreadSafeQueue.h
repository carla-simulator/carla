// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#pragma once

#include <condition_variable>
#include <memory>
#include <mutex>

namespace carla {
namespace thread {

  /// A thread safe buffer.
  template<typename T>
  class ThreadSafeQueue {
  public:

    ThreadSafeQueue() : _canWait(true) {}

    ThreadSafeQueue(const ThreadSafeQueue &) = delete;
    ThreadSafeQueue &operator=(const ThreadSafeQueue &) = delete;

    void push(std::unique_ptr<T> new_value) {
      std::lock_guard<std::mutex> lock(_mutex);
      _value = std::move(new_value);
      _condition.notify_one();
    }

    void canWait(bool wait){
      std::lock_guard<std::mutex> lock(_mutex);
      _canWait = wait;
      if (!_canWait)
        _condition.notify_all();
    }

    std::unique_ptr<T> wait_and_pop() {
      std::unique_lock<std::mutex> lock(_mutex);
      _condition.wait(lock, [this]() {
        return _value != nullptr || !_canWait;
      });
      return std::move(_value);
    }

    std::unique_ptr<T> try_pop() {
      std::lock_guard<std::mutex> lock(_mutex);
      return std::move(_value);
    }

    void clear() {
      _value = nullptr;
    }

    bool empty() const {
      std::lock_guard<std::mutex> lock(_mutex);
      return _value == nullptr;
    }

  private:

    mutable std::mutex _mutex;

    bool _canWait;

    std::unique_ptr<T> _value;

    std::condition_variable _condition;
  };

} // namespace thread
} // namespace carla
