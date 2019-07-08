// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/AtomicList.h"
#include "carla/NonCopyable.h"

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
      for (auto &callback : *list) {
        callback(args...);
      }
    }

    void RegisterCallback(CallbackType &&callback) {
      _list.Push(std::move(callback));
    }

    void Clear() {
      _list.Clear();
    }

  private:

    AtomicList<CallbackType> _list;
  };

} // namespace detail
} // namespace client
} // namespace carla
