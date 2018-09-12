// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <type_traits>
#include <iterator>

namespace carla {

  template<typename IT>
  class ListView {
  public:

    using iterator = IT;
    using const_iterator = typename std::add_const<IT>::type;
    using difference_type = typename std::iterator_traits<iterator>::difference_type;
    using value_type = typename std::iterator_traits<iterator>::value_type;
    using pointer = typename std::iterator_traits<iterator>::pointer;
    using reference = typename std::iterator_traits<iterator>::reference;

    explicit ListView(iterator begin, iterator end)
      : _begin(begin), _end(end) {}

    template <typename STL_CONTAINER>
    explicit ListView(STL_CONTAINER &container)
      : _begin(iterator(container.begin())),
        _end(iterator(container.end())) {}

    ListView(const ListView &) = default;
    ListView &operator=(const ListView &) = delete;

    iterator begin() const {
      return _begin;
    }

    iterator end() const {
      return _end;
    }

    bool empty() const {
      return _begin == _end;
    }

    difference_type size() const {
      return std::distance(_begin, _end);
    }

  private:

    const iterator _begin;

    const iterator _end;
  };

  template <typename T>
  static inline auto MakeListView(T begin, T end) {
    return ListView<T>(begin, end);
  }

} // namespace carla
