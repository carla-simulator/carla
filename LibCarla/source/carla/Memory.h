// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <boost/enable_shared_from_this.hpp>
#include <boost/make_shared.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

namespace carla {

  /// Use this SharedPtr (boost::shared_ptr) to keep compatibility with
  /// boost::python, but it would be nice if in the future we can make a Python
  /// adaptor for std::shared_ptr.
  template <typename T>
  using SharedPtr = boost::shared_ptr<T>;

  template <typename T>
  using WeakPtr = boost::weak_ptr<T>;

  template <typename T>
  using EnableSharedFromThis = boost::enable_shared_from_this<T>;

  template <typename T, typename... Args>
  static inline auto MakeShared(Args &&... args) {
    return boost::make_shared<T>(std::forward<Args>(args)...);
  }

} // namespace carla
