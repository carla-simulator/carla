// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/AtomicSharedPtr.h"
#include "carla/NonCopyable.h"

#include <functional>
#include <vector>

namespace carla {
namespace client {
namespace detail {

  template <typename... InputsT>
  class CallbackList : private NonCopyable {
  public:

    using CallbackType = std::function<void(InputsT...)>;

    CallbackList() : _list(std::make_shared<ListType>()) {}

    void Call(InputsT... args) const {
      auto list = _list.load();
      for (auto &callback : *list) {
        callback(args...);
      }
    }

    /// @todo This function cannot be called concurrently.
    void RegisterCallback(CallbackType callback) {
      auto new_list = std::make_shared<ListType>(*_list.load());
      new_list->emplace_back(std::move(callback));
      _list = new_list;
    }

    void Clear() {
      _list = std::make_shared<ListType>();
    }

  private:

    using ListType = std::vector<CallbackType>;

    AtomicSharedPtr<const ListType> _list;
  };

} // namespace detail
} // namespace client
} // namespace carla
