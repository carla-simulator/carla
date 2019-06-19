// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/AtomicSharedPtr.h"
#include "carla/NonCopyable.h"

#include <mutex>
#include <vector>

namespace carla {
namespace client {
namespace detail {

  /// Holds an atomic pointer to a list.
  ///
  /// @warning Only Load method is atomic, modifications to the list are locked
  /// with a mutex.
  template <typename T, typename ListT = std::vector<T>>
  class AtomicList : private NonCopyable {
  public:

    AtomicList() : _list(std::make_shared<ListT>()) {}

    template <typename ValueT>
    void Push(ValueT &&value) {
      std::lock_guard<std::mutex> lock(_mutex);
      auto new_list = std::make_shared<ListT>(*Load());
      new_list->push_back(std::forward<ValueT>(value));
      _list = new_list;
    }

    void Delete(unsigned int index) {
      std::lock_guard<std::mutex> lock(_mutex);
      auto new_list = std::make_shared<ListT>(*Load());
      new_list->erase(new_list->begin() + index);
      _list = new_list;
    }

    void Clear() {
      std::lock_guard<std::mutex> lock(_mutex);
      _list = std::make_shared<ListT>();
    }

    /// Returns a pointer to the list.
    std::shared_ptr<const ListT> Load() const {
      return _list.load();
    }

  private:

    std::mutex _mutex;

    AtomicSharedPtr<const ListT> _list;
  };

} // namespace detail
} // namespace client
} // namespace carla
