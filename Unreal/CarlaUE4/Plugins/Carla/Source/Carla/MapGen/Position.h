// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <type_traits>

namespace MapGen {

  template<typename T>
  class Position {
  public:
    using number_type = T;

    static_assert(
        std::is_arithmetic<number_type>::value &&
        !std::is_same<number_type, bool>::value, "not a valid number type");

    number_type x;

    number_type y;

    Position() = default;

    constexpr Position(T X, T Y) : x(X), y(Y) {}

    constexpr bool operator==(const Position &rhs) const {
      return (x == rhs.x) && (y == rhs.y);
    }

    constexpr bool operator!=(const Position &rhs) const {
      return !(*this == rhs);
    }

    Position &operator+=(const Position &rhs) {
      x += rhs.x;
      y += rhs.y;
      return *this;
    }

    friend Position operator+(Position lhs, const Position &rhs) {
      lhs += rhs;
      return lhs;
    }

    Position &operator-=(const Position &rhs) {
      x -= rhs.x;
      y -= rhs.y;
      return *this;
    }

    friend Position operator-(Position lhs, const Position &rhs) {
      lhs -= rhs;
      return lhs;
    }
  };

} // namespace MapGen
