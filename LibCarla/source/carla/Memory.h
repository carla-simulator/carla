// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <memory>
#include <utility>

namespace carla {

  template <typename T>
  using SharedPtr = std::shared_ptr<T>;

  template <typename T>
  using WeakPtr = std::weak_ptr<T>;

  template <typename T>
  using EnableSharedFromThis = std::enable_shared_from_this<T>;

  template <typename T, typename... Args>
  static inline auto MakeShared(Args &&... args) {
    return std::make_shared<T>(std::forward<Args>(args)...);
  }

} // namespace carla
