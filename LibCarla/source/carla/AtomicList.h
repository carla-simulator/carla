// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/AtomicSharedPtr.h"
#include "carla/NonCopyable.h"

#include <algorithm>
#include <mutex>
#include <vector>

namespace carla {
namespace client {
namespace detail {

  /// Holds an atomic pointer to a list.
  ///
  /// @warning Only Load method is atomic, modifications to the list are locked
  /// with a mutex.
  template <typename T>
  class AtomicList : private NonCopyable {
    using ListT = std::vector<T>;
  public:

    AtomicList() : _list(std::make_shared<ListT>()) {}

    template <typename ValueT>
    void Push(ValueT &&value) {
      std::scoped_lock<std::mutex> lock(_mutex);
      auto new_list = std::make_shared<ListT>(*Load());
      new_list->emplace_back(std::forward<ValueT>(value));
      _list = new_list;
    }

    void DeleteByIndex(size_t index) {
      std::scoped_lock<std::mutex> lock(_mutex);
      auto new_list = std::make_shared<ListT>(*Load());
      auto begin = new_list->begin();
      std::advance(begin, index);
      new_list->erase(begin);
      _list = new_list;
    }

    template <typename ValueT>
    void DeleteByValue(const ValueT &value) {
      std::scoped_lock<std::mutex> lock(_mutex);
      auto new_list = std::make_shared<ListT>(*Load());
      new_list->erase(std::remove(new_list->begin(), new_list->end(), value), new_list->end());
      _list = new_list;
    }

    void Clear() {
      std::scoped_lock<std::mutex> lock(_mutex);
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
