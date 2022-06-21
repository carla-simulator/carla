// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/AtomicList.h"
#include "carla/NonCopyable.h"

#include <atomic>
#include <functional>

namespace carla {
namespace client {
namespace detail {

  template <typename... InputsT>
  class CallbackList : private NonCopyable {
  public:

    using CallbackType = std::function<void(InputsT...)>;

    void Call(InputsT... args) const {
      auto list = _list.Load();
      for (auto &item : *list) {
        item.callback(args...);
      }
    }

    size_t Push(CallbackType &&callback) {
      auto id = ++_counter;
      DEBUG_ASSERT(id != 0u);
      _list.Push(Item{id, std::move(callback)});
      return id;
    }

    void Remove(size_t id) {
      _list.DeleteByValue(id);
    }

    void Clear() {
      _list.Clear();
    }

  private:

    struct Item {
      size_t id;
      CallbackType callback;

      friend bool operator==(const Item &lhs, const Item &rhs) {
        return lhs.id == rhs.id;
      }

      friend bool operator==(const Item &lhs, size_t rhs) {
        return lhs.id == rhs;
      }

      friend bool operator==(size_t lhs, const Item &rhs) {
        return lhs == rhs.id;
      }
    };

    std::atomic_size_t _counter{0u};

    AtomicList<Item> _list;
  };

} // namespace detail
} // namespace client
} // namespace carla
