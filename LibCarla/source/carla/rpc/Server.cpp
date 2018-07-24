// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/rpc/Server.h"

#include <rpc/this_handler.h>

namespace carla {
namespace rpc {

  void Server::RespondError(std::string error_message) {
    ::rpc::this_handler().respond_error(std::move(error_message));
  }

} // namespace carla
} // namespace rpc
