// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Debug.h"

#include <type_traits>
#include <iterator>

namespace carla {

  /// A view over a range of elements in a container. Basically a pair of begin
  /// and end iterators.
  template<typename IT>
  class ListView {
  public:

    using iterator = IT;
    using const_iterator = typename std::add_const<IT>::type;
    using size_type = size_t;
    using difference_type = typename std::iterator_traits<iterator>::difference_type;
    using value_type = typename std::iterator_traits<iterator>::value_type;
    using pointer = typename std::iterator_traits<iterator>::pointer;
    using reference = typename std::iterator_traits<iterator>::reference;

    explicit ListView(iterator begin, iterator end)
      : _begin(begin), _end(end) {
      DEBUG_ASSERT(std::distance(_begin, _end) >= 0);
    }

    ListView(const ListView &) = default;
    ListView &operator=(const ListView &) = delete;

    iterator begin() {
      return _begin;
    }

    const_iterator begin() const {
      return _begin;
    }

    const_iterator cbegin() const {
      return _begin;
    }

    iterator end() {
      return _end;
    }

    const_iterator end() const {
      return _end;
    }

    const_iterator cend() const {
      return _end;
    }

    bool empty() const {
      return _begin == _end;
    }

    size_type size() const {
      return static_cast<size_t>(std::distance(begin(), end()));
    }

  private:

    const iterator _begin;

    const iterator _end;
  };

  template <typename Iterator>
  static inline auto MakeListView(Iterator begin, Iterator end) {
    return ListView<Iterator>(begin, end);
  }

  template <typename Container>
  static inline auto MakeListView(Container &c) {
    return MakeListView(std::begin(c), std::end(c));
  }

} // namespace carla
