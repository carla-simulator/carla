// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <type_traits>

namespace carla {

  template <typename... Ts>
  struct are_same;

  template <typename T0, typename T1, typename... Ts>
  struct are_same<T0, T1, Ts...> {
    static constexpr bool value = std::is_same<T0, T1>::value && are_same<T0, Ts...>::value;
  };

  template <typename T0, typename T1>
  struct are_same<T0, T1> {
    static constexpr bool value = std::is_same<T0, T1>::value;
  };

} // namespace carla
