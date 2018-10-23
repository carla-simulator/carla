// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

// #include "carla/client/detail/Client.h"

namespace carla {
namespace client {
namespace detail {

  class Client;

  /// At this point the client is the only persistent state we have, but
  /// conceptually is nice to make the distinction.
  using PersistentState = Client;

} // namespace detail
} // namespace client
} // namespace carla
