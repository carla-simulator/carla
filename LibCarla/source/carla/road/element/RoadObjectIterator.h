// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Debug.h"
#include "carla/road/element/RoadObjectVisitor.h"

#include <iterator>
#include <memory>

namespace carla {
namespace road {
namespace element {

  template <typename T, typename IT>
  class RoadObjectIterator : private RoadObjectVisitor {
  public:

    static_assert(std::is_same<std::unique_ptr<RoadObject>, typename IT::value_type>::value, "Not compatible.");

    using value_type = T;
    using difference_type = typename IT::difference_type;
    using pointer = T *;
    using reference = T &;

    RoadObjectIterator(IT begin, IT end)
      : _it(begin),
        _end(end) {
      _success = false;
      for (; !IsAtEnd(); ++_it) {
        DEBUG_ASSERT((*_it) != nullptr);
        (*_it)->AcceptVisitor(*this);
        if (_success) {
          break;
        }
      }
    }

    RoadObjectIterator &operator++() {
      _success = false;
      while (!_success) {
        ++_it;
        if (IsAtEnd()) {
          break;
        }
        DEBUG_ASSERT((*_it) != nullptr);
        (*_it)->AcceptVisitor(*this);
      }
      return *this;
    }

    reference operator*() const {
      DEBUG_ASSERT((*_it) != nullptr);
      return static_cast<T &>(**_it);
    }

    pointer operator->() const {
      DEBUG_ASSERT((*_it) != nullptr);
      return static_cast<T *>(_it->get());
    }

    bool operator!=(const RoadObjectIterator &rhs) const {
      return _it != rhs._it;
    }

    bool operator==(const RoadObjectIterator &rhs) const {
      return !((*this) != rhs);
    }

    bool IsAtEnd() const {
      return _it == _end;
    }

  private:

    void Visit(T &) {
      _success = true;
    }

    IT _it;

    IT _end;

    bool _success;
  };

  template <typename T, typename Container>
  static auto MakeRoadObjectIterator(const Container &c) {
    auto begin = std::begin(c);
    auto end = std::end(c);
    return RoadObjectIterator<T, decltype(begin)>(begin, end);
  }

  template <typename T, typename IT>
  static auto MakeRoadObjectIterator(IT begin, IT end) {
    return RoadObjectIterator<T, decltype(begin)>(begin, end);
  }

} // namespace element
} // namespace road
} // namespace carla
