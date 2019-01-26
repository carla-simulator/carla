// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/rpc/Server.h"

#include "carla/Exception.h"

#include <exception>

namespace carla {
namespace rpc {

  void Server::RespondError(std::string error_message) {
    /// @todo Remove this function.
    ::carla::throw_exception(std::runtime_error(std::move(error_message)));
  }

} // namespace carla
} // namespace rpc
