// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <cstdint>
#include <random>
#include <type_traits>

namespace carla {
namespace traffic_manager {

/// Seeded generator that draws uniform integers or reals over a closed range.
class UniformPRNG {

  using UnderlyingPRNG = std::mt19937;

  UnderlyingPRNG state;

public:

  UniformPRNG(uint64_t seed) :
      state(UnderlyingPRNG(seed)) {
  }

  template <typename T, typename = std::enable_if_t<std::is_arithmetic<T>::value>>
  T next(T begin, T end) {
    using Distribution = typename std::conditional<
        std::is_floating_point<T>::value,
        std::uniform_real_distribution<T>,
        std::uniform_int_distribution<T>>::type;
    return Distribution(begin, end)(state);
  }

  /// A real in [0, 100), for comparing against percentages.
  double next() {
    return next(0.0, 100.0);
  }

};

} // namespace traffic_manager
} // namespace carla
