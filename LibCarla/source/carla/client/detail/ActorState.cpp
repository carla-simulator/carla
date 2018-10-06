// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/client/detail/ActorState.h"

#include <string>

namespace carla {
namespace client {
namespace detail {

  std::string ActorState::GetDisplayId() const {
    using namespace std::string_literals;
    return "Actor "s + std::to_string(GetId()) + " (" + GetTypeId() + ')';
  }

} // namespace detail
} // namespace client
} // namespace carla
