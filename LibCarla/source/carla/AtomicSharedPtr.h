// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/NonCopyable.h"

#include <memory>

namespace carla {

  /// A very simple atomic shared ptr with relaxed memory order.
  template <typename T>
  class AtomicSharedPtr : private NonCopyable {
  public:

    template <typename... Args>
    explicit AtomicSharedPtr(Args &&... args) : _ptr(std::forward<Args>(args)...) {}

    void store(std::shared_ptr<T> ptr) {
      std::atomic_store_explicit(&_ptr, ptr, std::memory_order_relaxed);
    }

    std::shared_ptr<T> load() const {
      return std::atomic_load_explicit(&_ptr, std::memory_order_relaxed);
    }

    void operator=(std::shared_ptr<T> ptr) {
      store(std::move(ptr));
    }

  private:

    std::shared_ptr<T> _ptr;
  };

} // namespace carla
