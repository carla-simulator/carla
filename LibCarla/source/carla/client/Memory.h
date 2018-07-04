// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

// In this namespace, we use boost::shared_ptr for now to make it compatible
// with boost::python, but it would be nice to make an adaptor for
// std::shared_ptr.
#include <boost/enable_shared_from_this.hpp>
#include <boost/make_shared.hpp>
#include <boost/shared_ptr.hpp>

namespace carla {
namespace client {

  template <typename T>
  using EnableSharedFromThis = boost::enable_shared_from_this<T>;

  template <typename T>
  using SharedPtr = boost::shared_ptr<T>;

  template <typename T, typename ... Args>
  auto MakeShared(Args && ... args) {
    return boost::make_shared<T>(std::forward<Args>(args) ...);
  }

} // namespace client
} // namespace carla
