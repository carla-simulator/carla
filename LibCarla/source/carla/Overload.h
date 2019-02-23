// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

namespace carla {
namespace detail {

  template <typename... Ts>
  struct Overload;

  template <typename T, typename... Ts>
  struct Overload<T, Ts...> : T, Overload<Ts...> {
    Overload(T &&func, Ts &&... rest)
      : T(std::forward<T>(func)),
        Overload<Ts...>(std::forward<Ts>(rest)...) {}

    using T::operator();

    using Overload<Ts...>::operator();
  };

  template <typename T>
  struct Overload<T> : T {
    Overload(T &&func) : T(std::forward<T>(func)) {}

    using T::operator();
  };

} // namespace detail

  template <typename... FuncTs>
  inline static auto MakeOverload(FuncTs &&... fs) {
    return detail::Overload<FuncTs...>(std::forward<FuncTs>(fs)...);
  }

} // namespace carla
